/*
 * Time Zone
 *
 *      Reads IANA time zone files (TZif, RFC 8536) and evaluates their POSIX
 * daylight saving rules. See TimeZone.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: November 17, 2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "TimeZone.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>

namespace IASLib
{
    namespace
    {
            // Zone files are small; anything much larger is not one.
        const size_t MAX_ZONE_FILE = 1024 * 1024;

        long long floorDivLL( long long a, long long b )
        {
            long long q = a / b;
            if ( ( a % b != 0 ) && ( ( a < 0 ) != ( b < 0 ) ) )
                q--;
            return q;
        }

            // Proleptic Gregorian date to days since 1 January 1970.
        long long civilDays( long long y, int m, int d )
        {
            y -= ( m <= 2 ) ? 1 : 0;
            long long era = floorDivLL( y, 400 );
            long long yoe = y - era * 400;
            long long doy = ( 153 * ( m + ( m > 2 ? -3 : 9 ) ) + 2 ) / 5 + d - 1;
            long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
            return era * 146097 + doe - 719468;
        }

        long long yearOf( long long llDays )
        {
            long long z = llDays + 719468;
            long long era = floorDivLL( z, 146097 );
            long long doe = z - era * 146097;
            long long yoe = ( doe - doe / 1460 + doe / 36524 - doe / 146096 ) / 365;
            long long doy = doe - ( 365 * yoe + yoe / 4 - yoe / 100 );
            long long mp = ( 5 * doy + 2 ) / 153;
            int m = (int)( mp < 10 ? mp + 3 : mp - 9 );
            return yoe + era * 400 + ( m <= 2 ? 1 : 0 );
        }

        bool leapYear( long long y )
        {
            return ( y % 4 == 0 && y % 100 != 0 ) || y % 400 == 0;
        }

        int monthDays( long long y, int m )
        {
            static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            return ( m == 2 && leapYear( y ) ) ? 29 : days[ m - 1 ];
        }

        bool validName( const std::string &name )
        {
            if ( name.empty() || name.size() > 255 || name[0] == '/' )
                return false;
            for ( size_t n = 0; n < name.size(); n++ )
            {
                char ch = name[ n ];
                if ( ! ( isalnum( (unsigned char)ch ) || ch == '/' || ch == '_' || ch == '-' || ch == '+' || ch == '.' ) )
                    return false;
            }
                // No empty, "." or ".." path components.
            size_t start = 0;
            while ( start <= name.size() )
            {
                size_t end = name.find( '/', start );
                if ( end == std::string::npos )
                    end = name.size();
                std::string part = name.substr( start, end - start );
                if ( part.empty() || part == "." || part == ".." )
                    return false;
                start = end + 1;
            }
            return true;
        }

        bool readFile( const std::string &path, std::vector<unsigned char> &data )
        {
            FILE *file = fopen( path.c_str(), "rb" );
            if ( file == NULL )
                return false;
            data.clear();
            unsigned char buffer[ 8192 ];
            size_t n;
            bool ok = true;
            while ( ( n = fread( buffer, 1, sizeof( buffer ), file ) ) > 0 )
            {
                data.insert( data.end(), buffer, buffer + n );
                if ( data.size() > MAX_ZONE_FILE )
                {
                    ok = false;
                    break;
                }
            }
            fclose( file );
            return ok;
        }

        struct CReader
        {
            const std::vector<unsigned char> &data;
            size_t pos;
            bool ok;

            CReader( const std::vector<unsigned char> &d ) : data( d ), pos( 0 ), ok( true ) {}

            bool have( size_t n ) { if ( ! ok || n > data.size() || pos > data.size() - n ) ok = false; return ok; }
            unsigned long u32( void )
            {
                if ( ! have( 4 ) ) return 0;
                unsigned long v = ( (unsigned long)data[ pos ] << 24 ) | ( (unsigned long)data[ pos + 1 ] << 16 ) |
                                  ( (unsigned long)data[ pos + 2 ] << 8 ) | data[ pos + 3 ];
                pos += 4;
                return v;
            }
            long long s32( void ) { unsigned long v = u32(); return (long long)(int)(unsigned int)v; }
            long long s64( void )
            {
                if ( ! have( 8 ) ) return 0;
                unsigned long long v = 0;
                for ( int n = 0; n < 8; n++ )
                    v = ( v << 8 ) | data[ pos + n ];
                pos += 8;
                return (long long)v;
            }
            unsigned char u8( void ) { if ( ! have( 1 ) ) return 0; return data[ pos++ ]; }
            void skip( size_t n ) { if ( have( n ) ) pos += n; }
        };
    }

        // A POSIX TZ rule, such as "EST5EDT,M3.2.0,M11.1.0". Offsets are seconds
        // east of UTC (the opposite sign to POSIX).
    struct CTimeZone::CRule
    {
        struct CDay
        {
            char    kind;       // 'J' (1-365, no leap day), 'N' (0-365) or 'M'
            int     a, b, c;    // J/N: day; M: month, week, weekday
            long    time;       // seconds after local midnight (may be negative or past 24h)
        };

        int     stdOffset;
        bool    hasDst;
        int     dstOffset;
        CDay    start;
        CDay    end;
    };

    CTimeZone::CTimeZone( void ) : m_nInitialOffset( 0 )
    {
    }

    namespace
    {
            // Parses a POSIX zone name: letters (at least three), or "<...>".
        bool ruleName( const std::string &s, size_t &pos )
        {
            if ( pos < s.size() && s[ pos ] == '<' )
            {
                size_t close = s.find( '>', pos );
                if ( close == std::string::npos )
                    return false;
                pos = close + 1;
                return true;
            }
            size_t start = pos;
            while ( pos < s.size() && isalpha( (unsigned char)s[ pos ] ) )
                pos++;
            return pos - start >= 3;
        }

            // [+-]hh[:mm[:ss]], up to 167 hours (RFC 8536 allows that for times).
        bool ruleTime( const std::string &s, size_t &pos, long &seconds )
        {
            int sign = 1;
            if ( pos < s.size() && ( s[ pos ] == '+' || s[ pos ] == '-' ) )
            {
                sign = ( s[ pos ] == '-' ) ? -1 : 1;
                pos++;
            }
            long parts[ 3 ] = { 0, 0, 0 };
            for ( int n = 0; n < 3; n++ )
            {
                size_t start = pos;
                long v = 0;
                while ( pos < s.size() && isdigit( (unsigned char)s[ pos ] ) && pos - start < 3 )
                    v = v * 10 + ( s[ pos++ ] - '0' );
                if ( pos == start )
                    return false;
                parts[ n ] = v;
                if ( pos < s.size() && s[ pos ] == ':' && n < 2 )
                {
                    pos++;
                    continue;
                }
                break;
            }
            if ( parts[0] > 167 || parts[1] > 59 || parts[2] > 59 )
                return false;
            seconds = sign * ( parts[0] * 3600 + parts[1] * 60 + parts[2] );
            return true;
        }

        bool ruleNumber( const std::string &s, size_t &pos, int &value )
        {
            size_t start = pos;
            value = 0;
            while ( pos < s.size() && isdigit( (unsigned char)s[ pos ] ) && pos - start < 4 )
                value = value * 10 + ( s[ pos++ ] - '0' );
            return pos > start;
        }
    }

    std::shared_ptr<CTimeZone::CRule> CTimeZone::ParseRule( const std::string &s )
    {
        std::shared_ptr<CRule> rule = std::make_shared<CRule>();
        size_t pos = 0;
        long seconds = 0;

        if ( ! ruleName( s, pos ) || ! ruleTime( s, pos, seconds ) )
            return std::shared_ptr<CRule>();
        rule->stdOffset = (int)-seconds;
        rule->hasDst = false;

        if ( pos == s.size() )
            return rule;

        if ( ! ruleName( s, pos ) )
            return std::shared_ptr<CRule>();
        rule->hasDst = true;
        rule->dstOffset = rule->stdOffset + 3600;
        if ( pos < s.size() && s[ pos ] != ',' )
        {
            if ( ! ruleTime( s, pos, seconds ) )
                return std::shared_ptr<CRule>();
            rule->dstOffset = (int)-seconds;
        }

        CRule::CDay *days[ 2 ] = { &rule->start, &rule->end };
        for ( int n = 0; n < 2; n++ )
        {
            if ( pos >= s.size() || s[ pos ] != ',' )
                return std::shared_ptr<CRule>();
            pos++;
            CRule::CDay &day = *days[ n ];
            day.time = 7200;
            if ( pos < s.size() && s[ pos ] == 'M' )
            {
                pos++;
                day.kind = 'M';
                if ( ! ruleNumber( s, pos, day.a ) || pos >= s.size() || s[ pos++ ] != '.' ||
                     ! ruleNumber( s, pos, day.b ) || pos >= s.size() || s[ pos++ ] != '.' ||
                     ! ruleNumber( s, pos, day.c ) )
                    return std::shared_ptr<CRule>();
                if ( day.a < 1 || day.a > 12 || day.b < 1 || day.b > 5 || day.c > 6 )
                    return std::shared_ptr<CRule>();
            }
            else if ( pos < s.size() && s[ pos ] == 'J' )
            {
                pos++;
                day.kind = 'J';
                if ( ! ruleNumber( s, pos, day.a ) || day.a < 1 || day.a > 365 )
                    return std::shared_ptr<CRule>();
            }
            else
            {
                day.kind = 'N';
                if ( ! ruleNumber( s, pos, day.a ) || day.a > 365 )
                    return std::shared_ptr<CRule>();
            }
            if ( pos < s.size() && s[ pos ] == '/' )
            {
                pos++;
                if ( ! ruleTime( s, pos, day.time ) )
                    return std::shared_ptr<CRule>();
            }
        }
        if ( pos != s.size() )
            return std::shared_ptr<CRule>();
        return rule;
    }

    int CTimeZone::RuleOffsetAt( const CRule &rule, long long llUtc )
    {
        if ( ! rule.hasDst )
            return rule.stdOffset;

        auto localStart = []( const CRule::CDay &day, long long year ) -> long long
        {
            long long days;
            if ( day.kind == 'J' )
            {
                    // Day 1-365, never counting 29 February.
                int n = day.a;
                days = civilDays( year, 1, 1 ) + n - 1;
                if ( leapYear( year ) && n >= 60 )
                    days++;
            }
            else if ( day.kind == 'N' )
            {
                days = civilDays( year, 1, 1 ) + day.a;
            }
            else
            {
                    // Weekday c of week b (5 = last) of month a.
                long long first = civilDays( year, day.a, 1 );
                int firstDow = (int)( ( ( first % 7 ) + 7 + 4 ) % 7 );     // 1970-01-01 was a Thursday
                int offset = ( day.c - firstDow + 7 ) % 7;
                int dayOfMonth = 1 + offset + ( day.b - 1 ) * 7;
                while ( dayOfMonth > monthDays( year, day.a ) )
                    dayOfMonth -= 7;
                days = first + dayOfMonth - 1;
            }
            return days * 86400 + day.time;
        };

            // The year on the standard-time clock decides which rule dates apply.
        long long year = yearOf( floorDivLL( llUtc + rule.stdOffset, 86400 ) );

            // DST starts at a standard-time clock reading and ends at a
            // daylight-time one.
        long long start = localStart( rule.start, year ) - rule.stdOffset;
        long long end = localStart( rule.end, year ) - rule.dstOffset;

        bool dst;
        if ( start < end )
            dst = ( llUtc >= start && llUtc < end );
        else
            dst = ! ( llUtc >= end && llUtc < start );      // southern hemisphere
        return dst ? rule.dstOffset : rule.stdOffset;
    }

    bool CTimeZone::Parse( const std::vector<unsigned char> &data )
    {
        CReader r( data );

        auto header = [&]( int &version, unsigned long counts[ 6 ] ) -> bool
        {
            if ( ! r.have( 44 ) || memcmp( &data[ r.pos ], "TZif", 4 ) != 0 )
                return false;
            unsigned char v = data[ r.pos + 4 ];
            version = ( v == 0 ) ? 1 : ( v - '0' );
            if ( version < 1 || version > 9 )
                return false;
            r.skip( 20 );
            for ( int n = 0; n < 6; n++ )
                counts[ n ] = r.u32();
            return r.ok;
        };

        int version = 0;
        unsigned long c[ 6 ];       // isutcnt, isstdcnt, leapcnt, timecnt, typecnt, charcnt
        if ( ! header( version, c ) )
            return false;

        int timeSize = 4;
        if ( version >= 2 )
        {
                // Skip the 32-bit data and use the 64-bit block that follows.
            unsigned long long v1 = (unsigned long long)c[3] * 5 + (unsigned long long)c[4] * 6 + c[5] +
                                    (unsigned long long)c[2] * 8 + c[1] + c[0];
            if ( v1 > data.size() )
                return false;
            r.skip( (size_t)v1 );
            if ( ! header( version, c ) )
                return false;
            timeSize = 8;
        }

        unsigned long isutcnt = c[0], isstdcnt = c[1], leapcnt = c[2], timecnt = c[3], typecnt = c[4], charcnt = c[5];
        if ( typecnt == 0 || typecnt > 256 || timecnt > 100000 || charcnt > 65536 || leapcnt > 10000 )
            return false;
        if ( ( isutcnt != 0 && isutcnt != typecnt ) || ( isstdcnt != 0 && isstdcnt != typecnt ) )
            return false;
            // Leap-second zones keep TAI-like times; they are not UTC.
        if ( leapcnt != 0 )
            return false;

        std::vector<long long> times( timecnt );
        for ( unsigned long n = 0; n < timecnt; n++ )
            times[ n ] = ( timeSize == 8 ) ? r.s64() : r.s32();
        std::vector<unsigned char> indices( timecnt );
        for ( unsigned long n = 0; n < timecnt; n++ )
            indices[ n ] = r.u8();
        std::vector<int> offsets( typecnt );
        for ( unsigned long n = 0; n < typecnt; n++ )
        {
            long long off = r.s32();
            r.u8();     // isdst
            r.u8();     // abbreviation index
            if ( off <= -86400 || off >= 86400 )
                return false;
            offsets[ n ] = (int)off;
        }
        r.skip( charcnt );
        r.skip( (size_t)leapcnt * ( timeSize + 4 ) );
        r.skip( isstdcnt );
        r.skip( isutcnt );
        if ( ! r.ok )
            return false;

        for ( unsigned long n = 0; n < timecnt; n++ )
        {
            if ( indices[ n ] >= typecnt || ( n > 0 && times[ n ] <= times[ n - 1 ] ) )
                return false;
        }

        m_aTransitions = times;
        m_aOffsets.resize( timecnt );
        for ( unsigned long n = 0; n < timecnt; n++ )
            m_aOffsets[ n ] = offsets[ indices[ n ] ];
        m_nInitialOffset = offsets[ 0 ];

            // The footer: "\n" rule "\n", used after the last transition.
        if ( version >= 2 && r.have( 1 ) && data[ r.pos ] == '\n' )
        {
            size_t start = r.pos + 1;
            size_t end = start;
            while ( end < data.size() && data[ end ] != '\n' )
                end++;
            if ( end < data.size() && end > start )
            {
                std::string footer( data.begin() + start, data.begin() + end );
                m_pRule = ParseRule( footer );
                if ( ! m_pRule )
                    return false;
            }
        }
        return true;
    }

    std::shared_ptr<const CTimeZone> CTimeZone::Load( const std::string &name )
    {
        std::vector<std::string> directories;
        const char *tzdir = getenv( "TZDIR" );
        if ( tzdir && *tzdir )
            directories.push_back( tzdir );
        directories.push_back( "/usr/share/zoneinfo" );
        directories.push_back( "/usr/lib/zoneinfo" );
        directories.push_back( "/usr/share/lib/zoneinfo" );
        directories.push_back( "/etc/zoneinfo" );

        for ( size_t n = 0; n < directories.size(); n++ )
        {
            std::vector<unsigned char> data;
            if ( ! readFile( directories[ n ] + "/" + name, data ) )
                continue;
            std::shared_ptr<CTimeZone> zone( new CTimeZone() );
            zone->m_strName = name;
            if ( zone->Parse( data ) )
                return zone;
        }
        return std::shared_ptr<const CTimeZone>();
    }

    std::shared_ptr<const CTimeZone> CTimeZone::Find( const char *pszName )
    {
        if ( pszName == NULL )
            return std::shared_ptr<const CTimeZone>();
        std::string name( pszName );
        if ( ! validName( name ) )
            return std::shared_ptr<const CTimeZone>();

        static std::mutex cacheMutex;
        static std::map< std::string, std::shared_ptr<const CTimeZone> > cache;

        std::lock_guard<std::mutex> lock( cacheMutex );
        std::map< std::string, std::shared_ptr<const CTimeZone> >::iterator it = cache.find( name );
        if ( it != cache.end() )
            return it->second;
        std::shared_ptr<const CTimeZone> zone = Load( name );
        if ( zone )
            cache[ name ] = zone;
        return zone;
    }

    int CTimeZone::OffsetAt( long long llUtc ) const
    {
        if ( m_aTransitions.empty() || llUtc < m_aTransitions[0] )
        {
            if ( m_aTransitions.empty() && m_pRule )
                return RuleOffsetAt( *m_pRule, llUtc );
            return m_nInitialOffset;
        }
        if ( llUtc >= m_aTransitions.back() && m_pRule )
            return RuleOffsetAt( *m_pRule, llUtc );

            // The last transition at or before the instant.
        std::vector<long long>::const_iterator it = std::upper_bound( m_aTransitions.begin(), m_aTransitions.end(), llUtc );
        return m_aOffsets[ ( it - m_aTransitions.begin() ) - 1 ];
    }

    bool CTimeZone::ResolveLocal( long long llLocal, const std::function<bool( long long, int & )> &offsetAt, int &nOffset, bool &bGap )
    {
        bGap = false;

            // Offsets are under a day, so the instant is within a day of the
            // same reading of UTC. Candidates are the offsets a day either side
            // (before and after any change) and the offset at that reading,
            // corrected twice. An offset is consistent if the instant it gives has
            // that offset. Two consistent offsets mean the local time happened
            // twice: take the earlier instant, the larger offset. None means it
            // never happened: take the smaller (pre-change) offset.
        int candidates[ 5 ];
        int count = 0;
        if ( ! offsetAt( llLocal - 86400, candidates[ count++ ] ) ||
             ! offsetAt( llLocal + 86400, candidates[ count++ ] ) ||
             ! offsetAt( llLocal, candidates[ count++ ] ) )
            return false;
        for ( int pass = 0; pass < 2; pass++ )
        {
            int next = 0;
            if ( ! offsetAt( llLocal - candidates[ count - 1 ], next ) )
                return false;
            candidates[ count++ ] = next;
        }

        bool found = false;
        int smallest = candidates[0];
        for ( int n = 0; n < count; n++ )
        {
            int check = 0;
            if ( candidates[ n ] < smallest )
                smallest = candidates[ n ];
            if ( offsetAt( llLocal - candidates[ n ], check ) && check == candidates[ n ] )
            {
                if ( ! found || candidates[ n ] > nOffset )
                    nOffset = candidates[ n ];
                found = true;
            }
        }
        if ( ! found )
        {
            nOffset = smallest;
            bGap = true;
        }
        return true;
    }

    void CTimeZone::OffsetForLocal( long long llLocal, int &nOffset, bool &bGap ) const
    {
        ResolveLocal( llLocal, [this]( long long llUtc, int &nOff ) { nOff = OffsetAt( llUtc ); return true; }, nOffset, bGap );
    }
} // namespace IASLib
