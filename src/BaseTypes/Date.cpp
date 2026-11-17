/***********************************************************************
**  Date
**
**  Description:
**      This object encapsulates the functionality of a date/time combo.
** It also includes the most rediculously robust date parsing routine
** ever written.
**
**  Calendar:
**      Dates are kept as a day number and the milliseconds since midnight.
** Day numbers count real days, so the difference between two dates is
** always the number of days between them. Calendar dates (day, month,
** year) use the historical calendar: the Julian calendar up to 4 October
** 1582 and the Gregorian calendar from 15 October 1582 (the days in between
** never existed). Years are numbered without a year zero: 1 BC is year -1.
**      ISO 8601 text (FormatISO8601, ParseISO8601 and the DF_ISO_8601*
** and DF_YYYYMMDD formats) instead uses the proleptic Gregorian calendar
** with astronomical year numbers (0000 is 1 BC), as ISO 8601 requires. For
** dates from 15 October 1582 on the two agree.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include "Date.h"
#include "StringTokenizer.h"
#include <cctype>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#ifndef IASLIB_WIN32__
    #include <sys/time.h> // NOLINT(modernize-deprecated-headers)
#else
    #include <sys/types.h>
    #include <sys/timeb.h>
#endif

// Seconds between the NTP epoch (1 Jan 1900) and the Unix epoch (1 Jan 1970).
static const long long NTP_TO_UNIX_SECONDS = 2208988800LL;

namespace IASLib
{
    long CDate::m_lOffsetSeconds = 0;

        // Whether years divisible by 4000 are common years (John Herschel's
        // proposed refinement). The Gregorian calendar as decreed in 1582, ISO
        // 8601 and other date libraries make them leap years, so the default
        // is off unless the library is built with IASLIB_HERSCHEL_CORRECTION__.
#ifdef IASLIB_HERSCHEL_CORRECTION__
    static bool bHerschelCorrection = true;
#else
    static bool bHerschelCorrection = false;
#endif

        // Plain arrays rather than CString globals, which would be constructed
        // before the CObject statics they depend on (initialisation order).
    static const char *pszMonthNames = "JanFebMarAprMayJunJulAugSepOctNovDec";
    static const char *pszWeekDays = "SunMonTueWedThuFriSat";
    static const char *pszWeekDaysLong = "Sunday   Monday   Tuesday  WednesdayThursday Friday   Saturday ";

    namespace
    {
            // The CDate day number of 1 January 1970 (Unix day 0). Kept from the
            // original table-based implementation so that day numbers of dates
            // from November 1582 on are unchanged.
        const long UNIX_EPOCH_DAY = 719164;
        const long MAX_YEAR = 1000000;
        const long MS_PER_DAY = 86400000L;
            // Julian Day Number of a CDate day number (JDN 2440588 is 1 Jan 1970).
        const long JULIAN_DAY_OFFSET = 2440588L - UNIX_EPOCH_DAY;

        long floorDiv( long a, long b )
        {
            long q = a / b;
            if ( ( a % b != 0 ) && ( ( a < 0 ) != ( b < 0 ) ) )
                q--;
            return q;
        }

        long floorMod( long a, long b )
        {
            return a - floorDiv( a, b ) * b;
        }

            // Proleptic Gregorian calendar, astronomical years, months 1-12:
            // days since 1 January 1970.
        long daysFromGregorian( long y, int m, int d )
        {
            y -= ( m <= 2 ) ? 1 : 0;
            long era = floorDiv( y, 400 );
            long yoe = y - era * 400;
            long doy = ( 153 * ( m + ( m > 2 ? -3 : 9 ) ) + 2 ) / 5 + d - 1;
            long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
            return era * 146097 + doe - 719468;
        }

        void gregorianFromDays( long z, long &y, int &m, int &d )
        {
            z += 719468;
            long era = floorDiv( z, 146097 );
            long doe = z - era * 146097;
            long yoe = ( doe - doe / 1460 + doe / 36524 - doe / 146096 ) / 365;
            long doy = doe - ( 365 * yoe + yoe / 4 - yoe / 100 );
            long mp = ( 5 * doy + 2 ) / 153;
            d = (int)( doy - ( 153 * mp + 2 ) / 5 + 1 );
            m = (int)( mp < 10 ? mp + 3 : mp - 9 );
            y = yoe + era * 400 + ( m <= 2 ? 1 : 0 );
        }

            // The Julian calendar on the same day numbering. Years run March to
            // February so that the leap day ends each four-year cycle.
        long julianRaw( long y, int m, int d )
        {
            y -= ( m <= 2 ) ? 1 : 0;
            long era = floorDiv( y, 4 );
            long yoe = y - era * 4;
            long doy = ( 153 * ( m + ( m > 2 ? -3 : 9 ) ) + 2 ) / 5 + d - 1;
            return era * 1461 + yoe * 365 + doy;
        }

        long julianOffset( void )
        {
                // Julian 5 October 1582 is Gregorian 15 October 1582.
            static const long offset = daysFromGregorian( 1582, 10, 15 ) - julianRaw( 1582, 10, 5 );
            return offset;
        }

        long daysFromJulian( long y, int m, int d )
        {
            return julianRaw( y, m, d ) + julianOffset();
        }

        void julianFromDays( long z, long &y, int &m, int &d )
        {
            z -= julianOffset();
            long era = floorDiv( z, 1461 );
            long doe = z - era * 1461;
            long yoe = doe / 365;
            if ( yoe > 3 )
                yoe = 3;
            long doy = doe - yoe * 365;
            long mp = ( 5 * doy + 2 ) / 153;
            d = (int)( doy - ( 153 * mp + 2 ) / 5 + 1 );
            m = (int)( mp < 10 ? mp + 3 : mp - 9 );
            y = yoe + era * 4 + ( m <= 2 ? 1 : 0 );
        }

        long gregorianStart( void )
        {
            static const long start = daysFromGregorian( 1582, 10, 15 );
            return start;
        }

        bool gregorianLeap( long y )
        {
            if ( bHerschelCorrection && floorMod( y, 4000 ) == 0 )
                return false;
            return ( floorMod( y, 4 ) == 0 && floorMod( y, 100 ) != 0 ) || floorMod( y, 400 ) == 0;
        }

        bool plainGregorianLeap( long y )
        {
            return ( floorMod( y, 4 ) == 0 && floorMod( y, 100 ) != 0 ) || floorMod( y, 400 ) == 0;
        }

        int monthLength( int m, bool leap )
        {
            static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            return ( m == 2 && leap ) ? 29 : days[ m - 1 ];
        }

            // Days of the Gregorian calendar dropped by the Herschel rule before
            // the given month (astronomical year, month 1-12).
        long herschelDropped( long y, int m )
        {
            if ( ! bHerschelCorrection || y < 4000 )
                return 0;
            long n = ( y - 1 ) / 4000;
            if ( y % 4000 == 0 && m > 2 )
                n++;
            return n;
        }

        long astronomical( long year )
        {
            return ( year < 0 ) ? year + 1 : year;
        }

        long historical( long year )
        {
            return ( year <= 0 ) ? year - 1 : year;
        }

        bool beforeReform( long a, int m, int d )
        {
            return ( a < 1582 ) || ( a == 1582 && ( m < 10 || ( m == 10 && d <= 4 ) ) );
        }
    }

        // Converts a historical calendar date to a day number. Returns false if
        // the date does not exist.
    bool CDate::DateToDay( int nDay, int nMonth, int nYear, long &lDay )
    {
        if ( nYear == 0 || nYear > MAX_YEAR || nYear < -MAX_YEAR || nMonth < 0 || nMonth > 11 || nDay < 1 )
            return false;

        long a = astronomical( nYear );
        int m = nMonth + 1;
        long unixDay;

        if ( beforeReform( a, m, nDay ) )
        {
            if ( nDay > monthLength( m, floorMod( a, 4 ) == 0 ) )
                return false;
            unixDay = daysFromJulian( a, m, nDay );
        }
        else
        {
            if ( a == 1582 && m == 10 && nDay < 15 )
                return false;       // 5-14 October 1582 never existed.
            if ( nDay > monthLength( m, gregorianLeap( a ) ) )
                return false;
            unixDay = daysFromGregorian( a, m, nDay ) - herschelDropped( a, m );
        }

        lDay = unixDay + UNIX_EPOCH_DAY;
        return true;
    }

        // Converts a day number to a historical calendar date.
    void CDate::DayToDate( long lDay, int &nDay, int &nMonth, int &nYear )
    {
        long unixDay = lDay - UNIX_EPOCH_DAY;
        long y;
        int m, d;

        if ( unixDay < gregorianStart() )
        {
            julianFromDays( unixDay, y, m, d );
        }
        else
        {
                // With the Herschel rule, labels after a dropped day are later
                // labels of the plain Gregorian calendar.
            long k = 0;
            for ( int nPass = 0; nPass < 4; nPass++ )
            {
                gregorianFromDays( unixDay + k, y, m, d );
                long kNext = herschelDropped( y, m );
                if ( bHerschelCorrection && y % 4000 == 0 && m == 2 && d == 29 )
                    kNext++;
                if ( kNext == k )
                    break;
                k = kNext;
            }
            gregorianFromDays( unixDay + k, y, m, d );
        }

        nDay = d;
        nMonth = m - 1;
        nYear = (int)historical( y );
    }

    bool CDate::IsValidDate( int nDay, int nMonth, int nYear )
    {
        long lDay;
        return DateToDay( nDay, nMonth, nYear, lDay );
    }

    IMPLEMENT_OBJECT( CDate, CObject );

    /////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CDate::CDate()
    {
        SetToCurrent();
    }

    CDate::CDate( int nDay, int nMonth, int nYear, int nHour, int nMinute, int nSecond, int nMilli )
    {
        m_bHasZone = false;
        m_nUTCOffset = 0;
        SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, nMilli );
    }

    CDate::CDate( const CDate &oSource ) : CObject()
    {
        m_lMilliseconds = oSource.m_lMilliseconds;
        m_lEpochDay = oSource.m_lEpochDay;
        m_bHasZone = oSource.m_bHasZone;
        m_nUTCOffset = oSource.m_nUTCOffset;
    }

    CDate::CDate( const char *strParse )
    {
        *this = GetSafeDate( strParse );
    }

        // tm_year is the full year and tm_mon is 0-11. A non-zero tm_gmtoffset
        // (seconds east of UTC) becomes the date's time zone; the fields are kept
        // as the local time in that zone.
    CDate::CDate( tm_type &tmSource )
    {
        m_bHasZone = false;
        m_nUTCOffset = 0;

        if ( ( tmSource.tm_year == 0 ) && ( tmSource.tm_mon == 0 ) && ( tmSource.tm_mday == 0 ) && ( tmSource.tm_msec == -1 ) )
        {
            SetInvalid();
            return;
        }

        SetDate( tmSource.tm_mday, tmSource.tm_mon, tmSource.tm_year, tmSource.tm_hour, tmSource.tm_min, tmSource.tm_sec, tmSource.tm_msec );
        if ( IsValid() && tmSource.tm_gmtoffset != 0 )
        {
            SetTimeZone( tmSource.tm_gmtoffset );
        }
    }

    CDate::~CDate()
    {

    }

    CDate &CDate::operator =( const CDate &oSource )
    {
        m_lMilliseconds = oSource.m_lMilliseconds;
        m_lEpochDay = oSource.m_lEpochDay;
        m_bHasZone = oSource.m_bHasZone;
        m_nUTCOffset = oSource.m_nUTCOffset;

        return *this;
    }

    bool CDate::IsLeapYear( int nYear ) const
    {
        if ( nYear == 0 || nYear > MAX_YEAR || nYear < -MAX_YEAR )
            return false;

        long a = astronomical( nYear );
        if ( a < 1582 )
            return floorMod( a, 4 ) == 0;
        if ( a == 1582 )
            return false;
        return gregorianLeap( a );
    }

    bool CDate::operator >( const CDate &oCompare ) const
    {
        return UtcMilliseconds() > oCompare.UtcMilliseconds();
    }

    bool CDate::operator <( const CDate &oCompare ) const
    {
        return UtcMilliseconds() < oCompare.UtcMilliseconds();
    }

    bool CDate::operator >=( const CDate &oCompare ) const
    {
        return ! ( *this < oCompare );
    }

    bool CDate::operator <=( const CDate &oCompare ) const
    {
        return ! ( *this > oCompare );
    }

    bool CDate::operator ==( const CDate &oCompare ) const
    {
        return UtcMilliseconds() == oCompare.UtcMilliseconds();
    }

    bool CDate::operator !=( const CDate &oCompare ) const
    {
        return ! ( *this == oCompare );
    }

    void CDate::SetDate( int nDay, int nMonth, int nYear, int nHour, int nMinute, int nSecond, int nMilli )
    {
        m_bHasZone = false;
        m_nUTCOffset = 0;

        long lDay;
        if ( ! DateToDay( nDay, nMonth, nYear, lDay ) ||
             nHour < 0 || nHour > 23 || nMinute < 0 || nMinute > 59 || nSecond < 0 || nSecond > 59 || nMilli < 0 || nMilli > 999 )
        {
            SetInvalid();
            return;
        }

        m_lEpochDay = lDay;
        m_lMilliseconds = ( nHour * 3600000L ) + ( nMinute * 60000L ) + ( nSecond * 1000L ) + nMilli;
    }

        // A C library struct tm style value: tm_year is years since 1900.
    void CDate::SetDate( tm_type &tmTime )
    {
        SetDate( tmTime.tm_mday, tmTime.tm_mon, tmTime.tm_year + 1900, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec, 0 );
    }

    void CDate::SetDate( void )
    {
        SetToCurrent();
    }

    int CDate::GetDay( void ) const
    {
        int nDay, nMonth, nYear;
        DayToDate( m_lEpochDay, nDay, nMonth, nYear );
        return nDay;
    }

    int CDate::GetMonth( void ) const
    {
        int nDay, nMonth, nYear;
        DayToDate( m_lEpochDay, nDay, nMonth, nYear );
        return nMonth;
    }

    int CDate::GetYear( void ) const
    {
        int nDay, nMonth, nYear;
        DayToDate( m_lEpochDay, nDay, nMonth, nYear );
        return nYear;
    }

    int CDate::GetDOY( void ) const
    {
        int nDay, nMonth, nYear;
        DayToDate( m_lEpochDay, nDay, nMonth, nYear );
        long lFirst;
        if ( ! DateToDay( 1, 0, nYear, lFirst ) )
            return 0;
        return (int)( m_lEpochDay - lFirst + 1 );
    }

    int CDate::GetHours( void ) const
    {
        return (int)(m_lMilliseconds / 3600000);
    }

    int CDate::GetMinutes( void ) const
    {
        return (int)( (m_lMilliseconds % 3600000) / 60000 );
    }

    int CDate::GetSeconds( void ) const
    {
        return (int)( ( (m_lMilliseconds % 3600000) % 60000 ) / 1000 );
    }

    int CDate::GetMillisecond( void ) const
    {
        return (int)(m_lMilliseconds % 1000);
    }

    /***********************************************************************************
    **  PrettyDate
    **
    **      Given a date value, this function returns a date in the standard year 2K
    ** compliant "MM/DD/YYYY  HH:MM:SS" format.
    **
    ***********************************************************************************/

    CString CDate::PrettyDate( void ) const
    {
        return FormatDate( CDate::DF_PRETTY );
    }

    /***********************************************************************************
    **  ISO 8601
    **
    **      FormatISO8601 writes the extended format: "YYYY-MM-DD" for a date, or
    ** "YYYY-MM-DDTHH:MM:SS[.sss]" with a "Z" when the date is known to be UTC.
    ** Years use the proleptic Gregorian calendar and astronomical numbering;
    ** years before 0000 or after 9999 are written with a sign and at least four
    ** digits ("-0043", "+10000"), the ISO 8601 expanded representation.
    **
    ***********************************************************************************/

    static CString isoYear( long y )
    {
        char buffer[ 32 ];
        if ( y >= 0 && y <= 9999 )
            snprintf( buffer, sizeof( buffer ), "%04ld", y );
        else if ( y < 0 )
            snprintf( buffer, sizeof( buffer ), "-%04ld", -y );
        else
            snprintf( buffer, sizeof( buffer ), "+%04ld", y );
        return CString( buffer );
    }

        // "Z", "+hh:mm" or "-hh:mm" ("+hh:mm:ss" for an offset with seconds,
        // such as local mean time), or "" when the zone is unknown.
    static CString isoOffset( bool bHasZone, int nOffset )
    {
        if ( ! bHasZone )
            return CString( "" );
        if ( nOffset == 0 )
            return CString( "Z" );
        char buffer[ 32 ];
        int nAbs = ( nOffset < 0 ) ? -nOffset : nOffset;
        if ( nAbs % 60 )
            snprintf( buffer, sizeof( buffer ), "%c%02d:%02d:%02d", nOffset < 0 ? '-' : '+', nAbs / 3600, ( nAbs / 60 ) % 60, nAbs % 60 );
        else
            snprintf( buffer, sizeof( buffer ), "%c%02d:%02d", nOffset < 0 ? '-' : '+', nAbs / 3600, ( nAbs / 60 ) % 60 );
        return CString( buffer );
    }

    void CDate::GetISODate( long &lYear, int &nMonth, int &nDay ) const
    {
        gregorianFromDays( m_lEpochDay - UNIX_EPOCH_DAY, lYear, nMonth, nDay );
    }

    CString CDate::FormatISO8601( bool bTime, bool bMilliseconds ) const
    {
        if ( ! IsValid() )
        {
            return CString( "Invalid Date" );
        }

        long y;
        int m, d;
        GetISODate( y, m, d );

        CString retVal = isoYear( y );
        char buffer[ 64 ];
        snprintf( buffer, sizeof( buffer ), "-%02d-%02d", m, d );
        retVal += buffer;

        if ( bTime )
        {
            if ( bMilliseconds )
                snprintf( buffer, sizeof( buffer ), "T%02d:%02d:%02d.%03d", GetHours(), GetMinutes(), GetSeconds(), GetMillisecond() );
            else
                snprintf( buffer, sizeof( buffer ), "T%02d:%02d:%02d", GetHours(), GetMinutes(), GetSeconds() );
            retVal += buffer;
            retVal += isoOffset( m_bHasZone, m_nUTCOffset );
        }
        return retVal;
    }

    namespace
    {
            // A small cursor over the ISO 8601 text.
        struct CIsoCursor
        {
            const char *p;
            const char *end;

            bool atEnd( void ) const { return p >= end; }
            char peek( size_t n = 0 ) const { return ( p + n < end ) ? p[ n ] : '\0'; }
            bool isDigitAt( size_t n ) const { return ( p + n < end ) && isdigit( (unsigned char)p[ n ] ); }
            size_t digitRun( void ) const
            {
                size_t n = 0;
                while ( isDigitAt( n ) )
                    n++;
                return n;
            }
            long take( size_t n )
            {
                long v = 0;
                for ( size_t i = 0; i < n; i++ )
                    v = v * 10 + ( p[ i ] - '0' );
                p += n;
                return v;
            }
            bool accept( char ch )
            {
                if ( peek() == ch )
                {
                    p++;
                    return true;
                }
                return false;
            }
        };

            // Reads a fraction after '.' or ',' as milliseconds of the given unit.
        bool isoFraction( CIsoCursor &c, long lUnitMs, long &lMs )
        {
            lMs = 0;
            if ( c.peek() != '.' && c.peek() != ',' )
                return true;
            c.p++;
            size_t n = c.digitRun();
            if ( n == 0 )
                return false;
            double fraction = 0.0;
            double scale = 0.1;
            for ( size_t i = 0; i < n; i++ )
            {
                fraction += ( c.p[ i ] - '0' ) * scale;
                scale /= 10.0;
            }
            c.p += n;
            lMs = (long)( fraction * lUnitMs + 1e-6 );
            return true;
        }

        int isoWeeksInYear( long y )
        {
                // A year has 53 weeks if it starts on a Thursday, or is a leap
                // year starting on a Wednesday.
            long jan1 = daysFromGregorian( y, 1, 1 );
            long dow = floorMod( jan1 + 3, 7 );      // 0 = Monday
            if ( dow == 3 || ( dow == 2 && plainGregorianLeap( y ) ) )
                return 53;
            return 52;
        }
    }

    CDate CDate::ParseISO8601( const char *strValue, bool *pbValid )
    {
        CDate dttInvalid( 0L, -1L );
        if ( pbValid )
            *pbValid = false;
        if ( strValue == NULL )
            return dttInvalid;

        std::string text( strValue );
        while ( ! text.empty() && isspace( (unsigned char)text[ text.size() - 1 ] ) )
            text.erase( text.size() - 1 );
        size_t nStart = 0;
        while ( nStart < text.size() && isspace( (unsigned char)text[ nStart ] ) )
            nStart++;

        CIsoCursor c;
        c.p = text.c_str() + nStart;
        c.end = text.c_str() + text.size();

            // Year: four digits, or a sign and four or more (expanded).
        long nSign = 1;
        bool bExpanded = false;
        if ( c.peek() == '+' || c.peek() == '-' )
        {
            nSign = ( c.peek() == '-' ) ? -1 : 1;
            c.p++;
            bExpanded = true;
        }
        size_t nYearDigits = c.digitRun();
        if ( nYearDigits < 4 || ( ! bExpanded && nYearDigits > 4 && nYearDigits != 8 && nYearDigits != 7 ) || ( bExpanded && nYearDigits > 7 ) )
            return dttInvalid;
        if ( ! bExpanded && nYearDigits > 4 )
            nYearDigits = 4;        // Basic format: YYYYMMDD or YYYYDDD follows.
        long y = nSign * c.take( nYearDigits );

        long unixDay;
        bool bExtended = c.accept( '-' );

        if ( c.peek() == 'W' || c.peek() == 'w' )
        {
                // Week date: YYYY-Www[-D] or YYYYWww[D].
            c.p++;
            if ( c.digitRun() < 2 )
                return dttInvalid;
            long w = c.take( 2 );
            long dow = 1;
            if ( bExtended ? c.accept( '-' ) : c.isDigitAt( 0 ) )
            {
                if ( ! c.isDigitAt( 0 ) )
                    return dttInvalid;
                dow = c.take( 1 );
            }
            if ( w < 1 || w > isoWeeksInYear( y ) || dow < 1 || dow > 7 )
                return dttInvalid;
            long jan4 = daysFromGregorian( y, 1, 4 );
            long week1 = jan4 - floorMod( jan4 + 3, 7 );     // Monday of week 1
            unixDay = week1 + ( w - 1 ) * 7 + ( dow - 1 );
        }
        else
        {
            size_t nDigits = c.digitRun();
            long m = 1, d = 1;
            if ( ( bExtended && nDigits == 3 ) || ( ! bExtended && nDigits == 3 ) )
            {
                    // Ordinal date: YYYY-DDD or YYYYDDD.
                long ordinal = c.take( 3 );
                if ( ordinal < 1 || ordinal > ( plainGregorianLeap( y ) ? 366 : 365 ) )
                    return dttInvalid;
                unixDay = daysFromGregorian( y, 1, 1 ) + ordinal - 1;
            }
            else
            {
                if ( bExtended )
                {
                    if ( nDigits != 2 )
                        return dttInvalid;
                    m = c.take( 2 );
                    if ( c.accept( '-' ) )
                    {
                        if ( c.digitRun() != 2 )
                            return dttInvalid;
                        d = c.take( 2 );
                    }
                }
                else if ( nDigits == 4 )
                {
                    m = c.take( 2 );
                    d = c.take( 2 );
                }
                else if ( nDigits != 0 )
                {
                    return dttInvalid;      // Basic YYYYMM is not allowed.
                }
                if ( m < 1 || m > 12 || d < 1 || d > monthLength( (int)m, plainGregorianLeap( y ) ) )
                    return dttInvalid;
                unixDay = daysFromGregorian( y, (int)m, (int)d );
            }
        }

            // Time: "T" (or a space, as RFC 3339 allows) then hh[:mm[:ss]] with a
            // fraction on the last part.
        long lMs = 0;
        bool bZone = false;
        long lOffsetSeconds = 0;

        if ( ( c.peek() == 'T' || c.peek() == 't' || c.peek() == ' ' ) && c.isDigitAt( 1 ) )
        {
            c.p++;
            if ( c.digitRun() < 2 )
                return dttInvalid;
            long h = c.take( 2 );
            long mi = 0, s = 0, frac = 0;
            bool bTimeExtended = false;

            if ( c.peek() == ':' || c.isDigitAt( 0 ) )
            {
                bTimeExtended = c.accept( ':' );
                if ( c.digitRun() < 2 )
                    return dttInvalid;
                mi = c.take( 2 );
                if ( bTimeExtended ? c.accept( ':' ) : c.isDigitAt( 0 ) )
                {
                    if ( c.digitRun() < 2 )
                        return dttInvalid;
                    s = c.take( 2 );
                    if ( ! isoFraction( c, 1000, frac ) )
                        return dttInvalid;
                }
                else if ( ! isoFraction( c, 60000, frac ) )
                {
                    return dttInvalid;
                }
            }
            else if ( ! isoFraction( c, 3600000, frac ) )
            {
                return dttInvalid;
            }

            if ( h == 24 )
            {
                    // 24:00 is the end of the day: midnight of the next day.
                if ( mi != 0 || s != 0 || frac != 0 )
                    return dttInvalid;
            }
            else if ( h > 23 || mi > 59 || s > 60 )
            {
                return dttInvalid;
            }
            if ( s == 60 )
            {
                    // A leap second cannot be represented; use the last
                    // millisecond of the minute.
                s = 59;
                frac = 999;
            }
            lMs = h * 3600000L + mi * 60000L + s * 1000L + frac;

                // Time zone: Z or +hh[[:]mm] / -hh[[:]mm].
            if ( c.accept( 'Z' ) || c.accept( 'z' ) )
            {
                bZone = true;
            }
            else if ( c.peek() == '+' || c.peek() == '-' )
            {
                long nZoneSign = ( c.peek() == '-' ) ? -1 : 1;
                c.p++;
                if ( c.digitRun() < 2 )
                    return dttInvalid;
                long zh = c.take( 2 );
                long zm = 0;
                if ( c.accept( ':' ) )
                {
                    if ( c.digitRun() != 2 )
                        return dttInvalid;
                    zm = c.take( 2 );
                }
                else if ( c.digitRun() == 2 )
                {
                    zm = c.take( 2 );
                }
                long zs = 0;
                if ( c.peek() == ':' && c.isDigitAt( 1 ) )
                {
                    c.p++;
                    if ( c.digitRun() != 2 )
                        return dttInvalid;
                    zs = c.take( 2 );
                }
                if ( zh > 23 || zm > 59 || zs > 59 )
                    return dttInvalid;
                bZone = true;
                lOffsetSeconds = nZoneSign * ( zh * 3600 + zm * 60 + zs );
            }
        }

        if ( ! c.atEnd() )
            return dttInvalid;

            // The local time is kept as written, with its zone. "24:00" is
            // midnight at the start of the next day.
        long lDays = 0;
        if ( lMs >= MS_PER_DAY )
        {
            lMs -= MS_PER_DAY;
            lDays = 1;
        }

        CDate dttResult( unixDay + lDays + UNIX_EPOCH_DAY, lMs );
        if ( bZone )
            dttResult.SetTimeZone( (int)lOffsetSeconds );
        if ( pbValid )
            *pbValid = true;
        return dttResult;
    }

    /***********************************************************************************
    **  FormatDate
    **
    **      Given a date value, this function returns a date in one of the standard
    ** formats that are defined in the header file.
    **
    ***********************************************************************************/

    CString CDate::FormatDate( CDate::FormatFlags nDateFormat ) const
    {
        CString strRetVal = "";
        CString strMonth;
        CString strDOWShort;
        CString strDOWLong;
        if ( ! IsValid() )
        {
            strRetVal = "Invalid Date";
            return strRetVal;
        }

            // The formats that say "GMT" must show UTC.
        if ( m_bHasZone && m_nUTCOffset != 0 &&
             ( nDateFormat == CDate::DF_RFC850 || nDateFormat == CDate::DF_RFC1123 || nDateFormat == CDate::DF_RFC7231 ) )
        {
            return ToUTC().FormatDate( nDateFormat );
        }

        int nDay, nMonth, nYear;
        DayToDate( m_lEpochDay, nDay, nMonth, nYear );
        int nYearShort = (int)floorMod( nYear, 100 );
        int nHour = GetHours();
        int nMinute = GetMinutes();
        int nSecond = GetSeconds();
        int nMillisecond = GetMillisecond();
        int nDOW = GetDOW();
        strDOWShort = CString( pszWeekDays + nDOW * 3, 3 );
        strDOWLong = CString( pszWeekDaysLong + nDOW * 9, 9 );
        strDOWLong.Trim();
        strMonth = CString( pszMonthNames + nMonth * 3, 3 );
        int n12Hour = nHour % 12;
        if ( n12Hour == 0 )
        {
            n12Hour = 12;
        }
        CString strAMPM;
        strAMPM = ( nHour < 12 ) ? "AM" : "PM";
            // Turn the month into 1-12 instead of 0-11
        nMonth++;

            // The ISO 8601 based formats use the proleptic Gregorian calendar.
        long lIsoYear;
        int nIsoMonth, nIsoDay;
        GetISODate( lIsoYear, nIsoMonth, nIsoDay );
        CString strIsoYear = isoYear( lIsoYear );

        switch ( nDateFormat )
        {
            case CDate::DF_PRETTY:
                strRetVal.Format( "%02d/%02d/%d  %02d:%02d:%02d", nMonth, nDay, nYear, nHour, nMinute, nSecond );
                break;

            case CDate::DF_UNIX:
                strRetVal.Format( "%3s %3s %2d %02d:%02d:%02d %d", (const char *)strDOWShort, (const char *)strMonth, nDay, nHour, nMinute, nSecond, nYear );
                break;

            case CDate::DF_RFC850:
                strRetVal.Format( "%s, %02d-%3s-%02d %02d:%02d:%02d GMT", (const char *)strDOWLong, nDay, (const char *)strMonth, nYearShort, nHour, nMinute, nSecond );
                break;

            case CDate::DF_RFC1123:
                strRetVal.Format( "%3s, %02d %3s %d %02d:%02d:%02d GMT", (const char *)strDOWShort, nDay, (const char *)strMonth, nYear, nHour, nMinute, nSecond );
                break;

            case CDate::DF_NETSCAPE:
                strRetVal.Format( "%02d/%3s/%d:%02d:%02d:%02d", nDay, (const char *)strMonth, nYear, nHour, nMinute, nSecond );
                break;

            case CDate::DF_EXACT:
                strRetVal.Format( "%02d/%02d/%d  %02d:%02d:%02d.%03d", nMonth, nDay, nYear, nHour, nMinute, nSecond, nMillisecond );
                break;

            case CDate::DF_SYBASE:
                strRetVal.Format( "%3s %02d %d %02d:%02d %s", (const char *)strMonth, nDay, nYear, n12Hour, nMinute, (const char *)strAMPM );
                break;

            case CDate::DF_SQLITE:
			    strRetVal.Format( "%d/%02d/%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond );
			    break;

            case CDate::DF_ORACLE:
                strMonth.ToUpperCase();
                strRetVal.Format( "%02d-%3s-%04d, %02d:%02d:%02d %s", nDay, (const char *)strMonth, nYear, n12Hour, nMinute, nSecond, (const char *)strAMPM );
                break;

            case CDate::DF_YYYYMMDD:
                strRetVal.Format( "%s%02d%02d", (const char *)strIsoYear, nIsoMonth, nIsoDay );
                break;

            case CDate::DF_ISO_8601:
                strRetVal.Format( "%s-%02d-%02d %02d:%02d:%02d", (const char *)strIsoYear, nIsoMonth, nIsoDay, nHour, nMinute, nSecond );
                break;

            case CDate::DF_ISO_8601_MS:
                strRetVal.Format( "%s-%02d-%02d %02d:%02d:%02d.%03d", (const char *)strIsoYear, nIsoMonth, nIsoDay, nHour, nMinute, nSecond, nMillisecond );
                break;

            case CDate::DF_ISO_8601_MS_PACKED:
                strRetVal.Format( "%s%02d%02d%02d%02d%02d.%03d", (const char *)strIsoYear, nIsoMonth, nIsoDay, nHour, nMinute, nSecond, nMillisecond );
                break;

            case CDate::DF_HHMMSS:
                strRetVal.Format( "%02d%02d%02d", nHour, nMinute, nSecond );
                break;

            case CDate::DF_RFC7231:
                strRetVal.Format( "%3s, %02d %3s %d %02d:%02d:%02d GMT", (const char *)strDOWShort, nDay, (const char *)strMonth, nYear, nHour, nMinute, nSecond );
                break;

            case CDate::DF_ISO_8601_FULL:
                strRetVal = FormatISO8601( true, false );
                break;

            case CDate::DF_ISO_8601_FULL_MS:
                strRetVal = FormatISO8601( true, true );
                break;

            case CDate::DF_ISO_8601_DATE:
                strRetVal = FormatISO8601( false, false );
                break;

            default:
                strRetVal = "Unknown Format";
                break;
        }
            // We return the final string value.
        return strRetVal;
    }

    /***********************************************************************************
    **  DB Date
    **
    **      Converts a date to a string, based on the current database type.
    **
    ***********************************************************************************/

    CString CDate::DBDate( int nDBType ) const
    {
	    switch ( nDBType )
	    {
		    case DB_SYBASE:
                return FormatDate( CDate::DF_SYBASE );

		    case DB_ORACLE:
                return FormatDate( CDate::DF_EXACT );

		    case DB_SQLITE:
                return FormatDate( CDate::DF_SQLITE );

            case DB_MYSQL:
                return FormatDate( CDate::DF_ISO_8601_MS );

		    default:
                return FormatDate( CDate::DF_PRETTY );
	    }
    }

    /***********************************************************************************
    **  SybaseDate
    **
    **      Given a date value, this function returns a date in the Sybase date format
    ** of "Mmm DD YYYY HH:MM AM/PM" format.
    **
    ***********************************************************************************/

    CString CDate::SybaseDate( void ) const
    {
        return FormatDate( CDate::DF_SYBASE );
    }

    /***********************************************************************************
    **  HTTPDate
    **
    **      Given a date value, this function returns a date in the standard HTTP format
    ** accepted and preferred format laid out by RFC 822 and updated in RFC 1123. The
    ** format is "Www, dd Mmm yyyy hh:mm:ss GMT". An invalid date gives a fixed,
    ** well-formed date so that an HTTP header built from it stays parseable.
    **
    ***********************************************************************************/

    CString CDate::HTTPDate( void ) const
    {
        CString strRetVal = "";

        if ( ! IsValid() )
        {
            strRetVal = "Tue, 25 Jan 2000 06:06:06 GMT";
            return strRetVal;
        }

        strRetVal = FormatDate( CDate::DF_RFC1123 );
        return strRetVal;
    }

    /***********************************************************************************
    **  GetSafeDate
    **
    **      This function takes a string date and time and converts it to a "legitimate"
    ** date. Dates that contain letters in addition to numbers are accepted and parsed.
    ** Dates that are acceptable are of the following forms, with their interpreted
    ** value given after the equals sign. For these examples, the current date is
    ** assumed as August 13th, 1998 at 13:05:20 GMT. The function returns an invalid
    ** date (IsValid() is false) for text that is not a date, or a date or time
    ** that does not exist (30 February, hour 25).
    **
    **      ISO 8601 text is read by ParseISO8601 (proleptic Gregorian calendar);
    ** everything else is read in the historical calendar. Years written with three or
    ** more digits are taken literally ("0044" is AD 44); one- and two-digit years
    ** fall in the window from 30 years before to 70 years after the current year.
    ** A time zone in the text is kept with the date: "12:14 EST" is 12:14 at -05:00
    ** (ToUTC() gives 17:14).
    **
    **          ""                                  = "08/13/1998  13:05:20"
    **          "01/01/1998  12:24:44"              = "01/01/1998  12:24:44"
    **          "1/1/2099"                          = "01/01/2099  00:00:00"
    **          "1/1/99"                            = "01/01/1999  00:00:00"
    **          "1/1/65"                            = "01/01/2065  00:00:00"
    **          "1/1/73"                            = "01/01/1973  00:00:00"
    **          "1/1/1998 1:1:1 AM"                 = "01/01/1998  01:01:01"
    **          "1-1-98 10:13 PM"                   = "01/01/1998  22:13:00"
    **          "1-1 1 AM"                          = "01/01/1998  01:00:00"
    **          "1-17  10:05 GMT +5:40"             = "01/17/1998  10:05:00 +05:40" (04:25 UTC)
    **          "Today"                             = "08/13/1998  00:00:00"
    **          "Today +6"                          = "08/19/1998  00:00:00"
    **          "Now"                               = "08/13/1998  13:05:20"
    **          "Now - 4:55"                        = "08/13/1998  08:10:20"
    **          "Now + 120"                         = "08/13/1998  14:25:20"
    **          "Start of day"                      = "08/13/1998  00:00:00"
    **          "August 13th, 1998 at 13:05:20 GMT" = "08/13/1998  13:05:20"
    **          "Aug 14 1998 5:03 PM"               = "08/14/1998  17:03:00"
    **          "1998-08-13T13:05:20Z"              = "08/13/1998  13:05:20"
    **          "13.08.1998"                        = "08/13/1998  00:00:00"
    **
    ***********************************************************************************/
    CDate CDate::GetSafeDate( const char *strValue )
    {
        CString     strWork = strValue ? strValue : "";
        CString     strTemp;

        strWork.Trim();
        if ( strWork.GetLength() == 0 )
        {
            return CDate();     // An empty string means now.
        }

            // ISO 8601 first: it has exact rules, and "2001-06-02" must not be
            // read through the slash formats.
        bool bIso = false;
        CDate dttIso = ParseISO8601( strWork, &bIso );
        if ( bIso )
        {
            return dttIso;
        }

        strTemp = strWork;
        strWork.ToUpperCase();
        strTemp.ToLowerCase();

            // Get rid of nasty hyphen-style dates.
        strWork.Replace( '-', '/' );
        strTemp.Replace( '-', '/' );

        if ( strWork != strTemp )
        {
            return GetAlphaDate( strWork );
        }

        return CDate::DateParse( strWork );
    }

    /***********************************************************************************
    **  GetHMS
    **
    **      Given a string of the form "hh:mm:ss" or "hh:mm" or "hhmmss" or "hmmss" or
    ** "hhmm" or "hmm" or "hh" or "mm" or "h", this function returns the number of seconds
    ** defined by the time period.
    **
    ***********************************************************************************/
    long CDate::GetHMS( const char *strParse, tm_type &tmTime )
    {
        CString     strTime = strParse;
        CString     strHour;
        CString     strMinute;
        CString     strSecond;
        CString     strWork;
        int         nHour = 0;
        int         nMinute = 0;
        int         nSecond = 0;

        strTime.Trim();

        if ( strTime.IndexOf( ":" ) != NOT_FOUND )
        {
            strHour = strTime.Substring( 0, (int)strTime.IndexOf( ":" ) );

            nHour = atoi( strHour );

            strWork = strTime.Substring( strTime.IndexOf( ":" ) + 1 );

            if ( strWork.IndexOf( ":" ) != NOT_FOUND )
            {
                strMinute = strWork.Substring( 0, (int)strWork.IndexOf( ":" ) );
                nMinute = atoi( strMinute );

                strSecond = strWork.Substring( strWork.IndexOf( ":" ) + 1 );
                nSecond = atoi( strSecond );
            }
            else
            {
                nMinute = atoi( strWork );
            }
        }
        else
        {
                // hhmmss
            if ( strTime.GetLength() == 6 )
            {
                nHour = atoi( strTime.Substring( 0, 2 ) );
                nMinute = atoi( strTime.Substring( 2, 2 ) );
                nSecond = atoi( strTime.Substring( 4, 2 ) );
            }

                // hmmss
            if ( strTime.GetLength() == 5 )
            {
                nHour = atoi( strTime.Substring( 0, 1 ) );
                nMinute = atoi( strTime.Substring( 1, 2 ) );
                nSecond = atoi( strTime.Substring( 3, 2 ) );
            }

                // hhmm
            if ( strTime.GetLength() == 4 )
            {
                nHour = atoi( strTime.Substring( 0, 2 ) );
                nMinute = atoi( strTime.Substring( 2, 2 ) );
            }

                // hmm
            if ( strTime.GetLength() == 3 )
            {
                nHour = atoi( strTime.Substring( 0, 1 ) );
                nMinute = atoi( strTime.Substring( 1, 2 ) );
            }

                // mm or hh
            if ( strTime.GetLength() == 2 )
            {
                int nTemp = atoi( strTime );

                if ( nTemp < 24 )
                    nHour = nTemp;
                else
                    nMinute = nTemp;
            }

                // h
            if ( strTime.GetLength() == 1 )
            {
                nHour = atoi( strTime );
            }
        }

        tmTime.tm_hour = nHour;
        tmTime.tm_min = nMinute;
        tmTime.tm_sec = nSecond;

        return ( nHour * 3600000 ) + ( nMinute * 60000 ) + ( nSecond * 1000 );
    }

    /***********************************************************************************
    **  GetAlphaDate
    **
    **      This function takes a date that contains letters in addition to numbers.
    ** Dates that are acceptable are of the following forms:
    **          "1/1/1998 1:1:1 AM"     = "01/01/1998  01:01:01"
    **          "1-1-98 10:13 PM"       = "01/01/1998  22:13:00"
    **          "1-1 1 AM"              = "01/01/1998  01:00:00"
    **          "1-17  10:05 GMT +5:40" = "01/17/1998  04:25:00"
    **          "Today"                 = "08/13/1998  00:00:00"
    **          "Today +6"              = "08/19/1998  00:00:00"
    **          "Now"                   = "08/13/1998  13:05:20"
    **          "Now - 4:55"            = "08/13/1998  08:10:20"
    **          "Now + 120"             = "08/13/1998  14:25:20"
    **          "Start of day"          = "08/13/1998  00:00:00"
    **
    ***********************************************************************************/
    CDate CDate::GetAlphaDate( const char *pszValue )
    {
        CString strValue = pszValue;
        strValue.ToUpperCase();
        strValue.Trim();

            // "Now", "Today" and "Start of day" only at the start, so that dates
            // that merely contain those letters are not mistaken for them.
        if ( strValue.IndexOf( "NOW" ) == 0 )
        {
            return GetNowDate( strValue );
        }

        if ( strValue.IndexOf( "TODAY" ) == 0 )
        {
            return GetTodayDate( strValue );
        }

        if ( strValue.IndexOf( "START OF DAY" ) == 0 )
        {
            return GetTodayDate( "TODAY" );
        }

        return CDate::DateParse( strValue );
    }


    /***********************************************************************************
    **  GetNowDate
    **
    **      Part of the GetAlphaDate hierarchy, this function returns the current time
    ** and date with a possible offset given in hours, minutes and seconds. The offset
    ** is optional, and can be either addition (+) or subtraction (-) from the current
    ** time and date.
    **
    ***********************************************************************************/
    CDate CDate::GetNowDate( const char *strValue )
    {
        CDate dttNow;
        bool bAdd = true;

            // Cut off the "NOW" at the start of the value.
        CString strWork = strValue;
        strWork = strWork.Substring( 3 );
        strWork.Trim();

        if ( strWork.GetLength() > 0 )
        {
            if ( strWork[ 0 ] == '/' )
            {
                bAdd=false;
                strWork = strWork.Substring( 1 );
            }
            else
            {
                if ( strWork[ 0 ] == '+' )
                {
                    strWork = strWork.Substring( 1 );
                }
            }
            tm_type tmTemp;
            tmTemp.tm_hour = 0;
            int nOffset = GetHMS( strWork, tmTemp );

            dttNow.Add( 0, 0, ( bAdd ? nOffset : -nOffset ) / 1000 );
        }

        return dttNow;
    }

    /***********************************************************************************
    **  GetTodayDate
    **
    **      Part of the GetAlphaDate hierarchy, this function returns the current date
    ** at midnight and allows offsets in days either forward (+) or backward (-) from
    ** today's date.
    **
    ***********************************************************************************/
    CDate CDate::GetTodayDate( const char *strValue )
    {
            // Get Today's Date
        CDate dttNow;

            // Cut off the "Today" at the start of the value.
        CString strWork = strValue;
        strWork = strWork.Substring( 5 );
        strWork.Trim();

        if ( strWork.GetLength() > 0 )
        {
            bool bAdd = true;

                // Note: We aren't looking for "-" because we already converted hyphens to slashes.
            if ( strWork[ 0 ] == '/' )
            {
                bAdd = false;
                strWork = strWork.Substring( 1 );
                strWork.Trim();
            }
            else
            {
                if ( strWork[ 0 ] == '+' )
                {
                    strWork = strWork.Substring( 1 );
                    strWork.Trim();
                }
            }
            int nOffset = atoi( strWork );

            if ( bAdd )
                dttNow.m_lEpochDay += nOffset;
            else
                dttNow.m_lEpochDay -= nOffset;
        }

            // Move it to midnight
        dttNow.m_lMilliseconds = 0;
        return dttNow;
    }

        // Puts a one- or two-digit year in the window from 30 years before to 70
        // years after the current year.
    int CDate::AdjustTwoDigitYear( int nYear )
    {
        nYear = nYear % 100;

        CDate dttNow;
        int nNowYear = dttNow.GetFullYear() % 100;
        int nCentury = ( dttNow.GetFullYear() / 100 ) * 100;
        int nLess = 0;
        int nMore = 0;

        if ( nNowYear < 30 )
        {
            nNowYear += 70;
            nLess = 0;
            nMore = -100;
        }
        else
        {
            nNowYear -= 30;
            nLess = 100;
            nMore = 0;
        }

        if ( nYear < nNowYear )
            nYear += nCentury + nLess;
        else
            nYear += nCentury + nMore;

        return nYear;
    }

        // Milliseconds from oThen to this date (positive when this is later).
        // Dates with different time zones are compared as instants.
    long long CDate::Elapsed( const CDate &oThen ) const
    {
        return UtcMilliseconds() - oThen.UtcMilliseconds();
    }

        // Calendar days from oThen's date to this date (positive when this is
        // later), each date in its own time zone.
    int CDate::ElapsedDays( const CDate &oThen ) const
    {
        return (int)( m_lEpochDay - oThen.m_lEpochDay );
    }

    CDate &CDate::Add( int nDays, int nMinutes, int nSeconds )
    {
        if ( ! IsValid() )
        {
            return *this;
        }

        long long llMs = (long long)m_lMilliseconds + (long long)nMinutes * 60000LL + (long long)nSeconds * 1000LL;
        long long llDays = (long long)m_lEpochDay + nDays;

        llDays += llMs / MS_PER_DAY;
        llMs %= MS_PER_DAY;
        if ( llMs < 0 )
        {
            llMs += MS_PER_DAY;
            llDays--;
        }

        m_lEpochDay = (long)llDays;
        m_lMilliseconds = (long)llMs;

        return *this;
    }

    CDate &CDate::StartOfDay( void )
    {
        if ( IsValid() )
        {
            m_lMilliseconds = 0;
        }
        return *this;
    }

    CDate &CDate::StartOfHour( void )
    {
        if ( IsValid() )
        {
            m_lMilliseconds -= m_lMilliseconds % 3600000L;
        }
        return *this;
    }

    /***********************************************************************************
    **  ParseSybase
    **
    **      This function is made solely to tear apart the Sybase Short Date format.
    **
    **   CS_DATES_LONG
    **     mth dd yyyy hh:mm:ss:zzz[am|pm]
    **     October 19 1961 04:15:31:111 pm
    **
    ***********************************************************************************/
    CDate CDate::ParseSybase( const char *strValue )
    {
        CString     strWork = strValue;
        CStringTokenizer stToken( strWork, " :", false );
        CString     strMonth = stToken.NextToken();
        CString     strDay = stToken.NextToken();
        CString     strYear = stToken.NextToken();
        CString     strHour = stToken.NextToken();
        CString     strMinute = stToken.NextToken();
        CString     strSecond = stToken.NextToken();
        CString     strMillisecond = stToken.NextToken();
        CString     strAMPM = strMillisecond.Substring( 3 );
        if ( ( strAMPM.GetLength() == 0 ) && ( stToken.HasMoreTokens() ) )
        {
            strAMPM = stToken.NextToken();
        }
        strMillisecond = strMillisecond.Substring( 0, 3 );
        CDate       dttDate;
        CString     strMonths="JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";

        strMonth.ToUpperCase();
        strMonth = strMonth.Substring( 0, 3 );
        size_t nFound = ( strMonth.GetLength() == 3 ) ? strMonths.IndexOf( strMonth ) : NOT_FOUND;
        if ( nFound == NOT_FOUND || ( nFound % 3 ) != 0 )
        {
            dttDate.SetInvalid();
            return dttDate;
        }
        int nMonth = (int)( nFound / 3 );
        int nDay = atoi( strDay );
        int nYear = atoi( strYear );

        int nHour = atoi( strHour );
        strAMPM.ToUpperCase();
        if ( ( strAMPM == "PM" ) && ( nHour != 12 ) )
        {
            nHour += 12;
        }
        else
        {
            if ( ( strAMPM == "AM" ) && ( nHour == 12 ) )
            {
                nHour = 0;
            }
        }

        int nMinute = atoi( strMinute );
        int nSecond = atoi( strSecond );
        int nMillisecond = atoi( strMillisecond );

        dttDate.SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, nMillisecond );

        return dttDate;
    }

    /***********************************************************************************
    **  ParseUnix
    **
    **      Converts a Unix time_t (seconds since 1 January 1970 UTC, which may be
    ** negative) into a date.
    **
    ***********************************************************************************/
    CDate CDate::ParseUnix( time_t tValue )
    {
        long long llValue = (long long)tValue;
        long long llDays = llValue / 86400;
        long long llSeconds = llValue % 86400;
        if ( llSeconds < 0 )
        {
            llSeconds += 86400;
            llDays--;
        }

        CDate dttDate( (long)( llDays + UNIX_EPOCH_DAY ), (long)( llSeconds * 1000 ) );
        dttDate.SetTimeZone( 0 );
        return dttDate;
    }

    /***********************************************************************************
    **  Julian Day
    **
    **      The Julian Day Number of a day is the Julian Date at noon UT on that day,
    ** so a Julian Date is the day number plus the time since noon as a fraction of a
    ** day (negative before noon).
    **
    ***********************************************************************************/
        // Julian Days are in UT: a date with a time zone is converted first.
    double CDate::GetJulianDate( void ) const
    {
        long long llUtc = UtcMilliseconds();
        long lDay = (long)floorDiv( (long)( llUtc / 1000 ), 86400 );
        long long llMs = llUtc - (long long)lDay * MS_PER_DAY;
        return (double)( lDay + JULIAN_DAY_OFFSET ) + (double)( llMs - MS_PER_DAY / 2 ) / (double)MS_PER_DAY;
    }

    long CDate::GetJulianDayNumber( void ) const
    {
        long long llUtc = UtcMilliseconds();
        return (long)floorDiv( (long)( llUtc / 1000 ), 86400 ) + JULIAN_DAY_OFFSET;
    }

    CDate CDate::FromJulianDate( double dJulianDate )
    {
            // Civil days start half a day before Julian days.
        double dCivil = dJulianDate + 0.5;
        double dDay = floor( dCivil );
        long long llMs = llround( ( dCivil - dDay ) * (double)MS_PER_DAY );
        long lDay = (long)dDay;
        if ( llMs >= MS_PER_DAY )
        {
            llMs -= MS_PER_DAY;
            lDay++;
        }

        CDate dttDate( lDay - JULIAN_DAY_OFFSET, (long)llMs );
        dttDate.SetTimeZone( 0 );
        return dttDate;
    }

    CDate CDate::FromJulianDayNumber( long lJulianDayNumber )
    {
            // The day number names the day; its Julian Date is at noon.
        CDate dttDate( lJulianDayNumber - JULIAN_DAY_OFFSET, MS_PER_DAY / 2 );
        dttDate.SetTimeZone( 0 );
        return dttDate;
    }

    /***********************************************************************************
    **  ParseHTTP
    **
    **      This function is made solely to tear apart the three accepted HTTP date
    ** formats.
    **
    ***********************************************************************************/
    static int httpMonth( const CString &strMonths, const CString &strName )
    {
        if ( strName.GetLength() != 3 )
            return -1;
        size_t nFound = strMonths.IndexOf( strName + "|" );
        if ( nFound == NOT_FOUND || ( nFound % 4 ) != 0 )
            return -1;
        return (int)( nFound / 4 );
    }

    CDate CDate::ParseHTTP( const char *strValue )
    {
        CDate       dttDate;
        CString     strWork;
        CString     strTemp;
        int         nTemp;
        bool        bAsctime = false;
        CString     strWkday = "SUN|MON|TUE|WED|THU|FRI|SAT|";
        CString     strWeekday = "SUNDAY|MONDAY|TUESDAY|WEDNESDAY|THURSDAY|FRIDAY|SATURDAY|";
        CString     strMonth = "JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC|";
        int         nDay;
        int         nMonth;
        int         nYear;
        int         nHour;
        int         nMinute;
        int         nSecond;

        strWork = strValue;
        strWork.Trim();
        strWork.ToUpperCase();
        nTemp = (int)strWork.IndexOf( ',' );
        if ( nTemp == (int)IASLib::NOT_FOUND )
        {
            nTemp = (int)strWork.IndexOf( ' ' );
            bAsctime = true;
        }
        if ( nTemp != (int)IASLib::NOT_FOUND )
        {
            strTemp = strWork.Substring( 0, nTemp );
            strTemp += "|";
                // Remove the tell-tale.
            strWork = strWork.Substring( nTemp + 1 );
            strWork.Trim();

            if ( ( ! bAsctime ) && ( strWkday.IndexOf( strTemp ) != NOT_FOUND ) )
            {
                // This is an RFC 1123 Date: Wkday "," SP 2DIGIT SP month SP 4DIGIT SP 2DIGIT ":" 2DIGIT ":" 2DIGIT SP "GMT" (Mon, 02 Jun 2001 12:14:11 GMT)
                    // Find the end of the date.
                nTemp = (int)strWork.IndexOf( ' ' );
                strTemp = strWork.Substring( 0, nTemp );
                nDay = atoi( strTemp );

                    // Trim the day off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ' ' );
                strTemp = strWork.Substring( 0, nTemp );
                nMonth = httpMonth( strMonth, strTemp );

                    // Trim the month off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ' ' );
                strTemp = strWork.Substring( 0, nTemp );
                nYear = atoi( strTemp );

                    // Trim the year off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ':' );
                strTemp = strWork.Substring( 0, nTemp );
                nHour = atoi( strTemp );

                    // Trim the Hour off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ':' );
                strTemp = strWork.Substring( 0, nTemp );
                nMinute = atoi( strTemp );

                    // Trim the Minute off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ' ' );
                strTemp = strWork.Substring( 0, nTemp );
                nSecond = atoi( strTemp );
                dttDate.SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, 0 );
                if ( dttDate.IsValid() ) dttDate.SetTimeZone( 0 );
            }
            else
            {
                if ( strWeekday.IndexOf( strTemp ) != IASLib::NOT_FOUND )
                {
                    // This is an RFC 850 Date: Weekday "," SP 2DIGIT "-" month "-" 2DIGIT SP 2DIGIT ":" 2DIGIT ":" 2DIGIT SP "GMT" (Monday, 02-Jun-01 12:14:11 GMT)
                        // Find the end of the date.
                    nTemp = (int)strWork.IndexOf( '-' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nDay = atoi( strTemp );

                        // Trim the day off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( '-' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nMonth = httpMonth( strMonth, strTemp );

                        // Trim the month off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( ' ' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nYear = atoi( strTemp );
                    nYear = AdjustTwoDigitYear( nYear );

                        // Trim the year off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( ':' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nHour = atoi( strTemp );

                        // Trim the Hour off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( ':' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nMinute = atoi( strTemp );

                        // Trim the Minute off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( ' ' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nSecond = atoi( strTemp );
                    dttDate.SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, 0 );
                    if ( dttDate.IsValid() ) dttDate.SetTimeZone( 0 );
                }
                else
                {
                    if ( strWkday.IndexOf( strTemp ) != NOT_FOUND )
                    {
                        // This is an ASC-Time date: Wkday SP month SP 1*DIGIT SP 2DIGIT ":" 2DIGIT ":" 2DIGIT SP 4DIGIT (Mon Jun  2 12:14:11 2001)
                            // Find the end of the Month.
                        nTemp = (int)strWork.IndexOf( ' ' );
                        strTemp = strWork.Substring( 0, nTemp );
                        nMonth = httpMonth( strMonth, strTemp );

                            // Trim the Month off.
                        strWork = strWork.Substring( nTemp + 1 );
                        nTemp = (int)strWork.IndexOf( ' ', 1 );
                        strTemp = strWork.Substring( 0, nTemp );
                        strTemp.Trim();
                        nDay = atoi( strTemp );

                            // Trim the Day off.
                        strWork = strWork.Substring( nTemp + 1 );
                        nTemp = (int)strWork.IndexOf( ':' );
                        strTemp = strWork.Substring( 0, nTemp );
                        nHour = atoi( strTemp );

                            // Trim the Hour off.
                        strWork = strWork.Substring( nTemp + 1 );
                        nTemp = (int)strWork.IndexOf( ':' );
                        strTemp = strWork.Substring( 0, nTemp );
                        nMinute = atoi( strTemp );

                            // Trim the Minute off.
                        strWork = strWork.Substring( nTemp + 1 );
                        nTemp = (int)strWork.IndexOf( ' ' );
                        strTemp = strWork.Substring( 0, nTemp );
                        nSecond = atoi( strTemp );

                            // Trim the seconds off.
                        strWork = strWork.Substring( nTemp + 1 );
                        strTemp = strWork;
                        nYear = atoi( strTemp );

                        dttDate.SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, 0 );
                        if ( dttDate.IsValid() ) dttDate.SetTimeZone( 0 );
                    }
                    else
                    {
                            // This is some other format. We'll try, but there's no guarantee.
                        dttDate = Parse( strValue );
                    }
                }
            }
        }
        else
        {
            dttDate = Parse( strValue );
        }

        return dttDate;
    }

    /***********************************************************************************
    **  SynchronizeToTimeServer
    **
    **      This function allows the system to synchronize our current date and time to
    ** a reference time server. Some outside source must get the value of the reference
    ** time server (in seconds since 1 January 1900, as time servers give it) and pass
    ** it to this function. It records the difference between that and the system
    ** clock in the static "m_lOffsetSeconds", which is applied whenever the current
    ** time and date are retrieved. The system clock itself is never changed.
    **      This whole thing seemed like a good idea after I noticed that the production
    ** servers were running up to 15 minutes out of sync with the real time and up to
    ** ten minutes out of sync with each other. It seems that the situation is totally
    ** unacceptable for a system that deals mainly with times and dates.
    **
    ***********************************************************************************/
    void CDate::SynchronizeToTimeServer( unsigned long lServerTime )
    {
        long long llServer = (long long)lServerTime - NTP_TO_UNIX_SECONDS;
        long long llCurrent;

#ifndef IASLIB_WIN32__
        struct timeval tvCurrent;
        gettimeofday( &tvCurrent, NULL );
        llCurrent = (long long)tvCurrent.tv_sec;
#else
        struct _timeb tvCurrent;
        _ftime64_s( &tvCurrent );
        llCurrent = (long long)tvCurrent.time;
#endif
        m_lOffsetSeconds = (long)( llServer - llCurrent );
    }

    void CDate::SetToCurrent( void )
    {
        long long llSeconds;
        long long llMilliseconds;

#ifndef IASLIB_WIN32__
        struct timeval tvCurrent;

        gettimeofday( &tvCurrent, NULL );
        llSeconds = (long long)tvCurrent.tv_sec;
        llMilliseconds = tvCurrent.tv_usec / 1000;
#else
        struct _timeb tstruct;

        _ftime64_s( &tstruct );
        llSeconds = (long long)tstruct.time;
        llMilliseconds = tstruct.millitm;
#endif
        llSeconds += m_lOffsetSeconds;

        long long llDays = llSeconds / 86400;
        long long llRemainder = llSeconds % 86400;
        if ( llRemainder < 0 )
        {
            llRemainder += 86400;
            llDays--;
        }

        m_lEpochDay = (long)( llDays + UNIX_EPOCH_DAY );
        m_lMilliseconds = (long)( llRemainder * 1000 + llMilliseconds );
        m_bHasZone = true;
        m_nUTCOffset = 0;
    }

    void CDate::SetHerschelCorrection( bool bEnabled )
    {
        bHerschelCorrection = bEnabled;
    }

    bool CDate::GetHerschelCorrection( void )
    {
        return bHerschelCorrection;
    }

    /***********************************************************************************
    **  Time zones
    **
    **      A date holds the local (wall clock) time and, when known, its offset from
    ** UTC in seconds east. UtcMilliseconds() is the instant: the local time less the
    ** offset. A date without a zone is treated as UTC wherever an instant is needed.
    **
    ***********************************************************************************/

    long long CDate::UtcMilliseconds( void ) const
    {
        long long llLocal = (long long)m_lEpochDay * MS_PER_DAY + m_lMilliseconds;
        return m_bHasZone ? llLocal - (long long)m_nUTCOffset * 1000LL : llLocal;
    }

    CDate CDate::FromUtcMilliseconds( long long llUtc, int nOffsetSeconds )
    {
        long long llLocal = llUtc + (long long)nOffsetSeconds * 1000LL;
        long long llDay = llLocal / MS_PER_DAY;
        long long llMs = llLocal % MS_PER_DAY;
        if ( llMs < 0 )
        {
            llMs += MS_PER_DAY;
            llDay--;
        }
        CDate dttResult( (long)llDay, (long)llMs );
        dttResult.m_bHasZone = true;
        dttResult.m_nUTCOffset = nOffsetSeconds;
        return dttResult;
    }

    bool CDate::SetTimeZone( int nOffsetSeconds )
    {
        if ( nOffsetSeconds <= -86400 || nOffsetSeconds >= 86400 )
            return false;
        m_bHasZone = true;
        m_nUTCOffset = nOffsetSeconds;
        return true;
    }

    void CDate::ClearTimeZone( void )
    {
        m_bHasZone = false;
        m_nUTCOffset = 0;
    }

    CDate CDate::ToTimeZone( int nOffsetSeconds ) const
    {
        if ( ! IsValid() || nOffsetSeconds <= -86400 || nOffsetSeconds >= 86400 )
            return *this;
        return FromUtcMilliseconds( UtcMilliseconds(), nOffsetSeconds );
    }

    CDate CDate::ToUTC( void ) const
    {
        return ToTimeZone( 0 );
    }

        // The system's offset from UTC (seconds east) at an instant, or false if
        // the system cannot say (the instant is outside what time_t covers).
    static bool systemOffsetAt( long long llUtcSeconds, int &nOffset )
    {
        time_t t = (time_t)llUtcSeconds;
        if ( (long long)t != llUtcSeconds )
            return false;
        struct tm local;
#ifdef IASLIB_WIN32__
        if ( _localtime64_s( &local, &t ) != 0 )
            return false;
        long long llLocal = (long long)_mkgmtime64( &local );
#else
        if ( localtime_r( &t, &local ) == NULL )
            return false;
        long long llLocal = (long long)timegm( &local );
#endif
        nOffset = (int)( llLocal - llUtcSeconds );
        return true;
    }

    CDate CDate::ToLocalTime( void ) const
    {
        if ( ! IsValid() )
            return *this;
        long long llUtc = UtcMilliseconds();
        int nOffset = 0;
        long long llSeconds = llUtc / 1000;
        if ( llUtc % 1000 < 0 )
            llSeconds--;
        llSeconds -= (long long)UNIX_EPOCH_DAY * 86400LL;      // to Unix seconds
        if ( ! systemOffsetAt( llSeconds, nOffset ) )
            return *this;
        return FromUtcMilliseconds( llUtc, nOffset );
    }

    bool CDate::AssumeLocalTimeZone( void )
    {
        if ( ! IsValid() )
            return false;

            // Find the system's offset in force at this local time. Candidates
            // come from the offset at the same reading of UTC, corrected twice.
        long long llLocal = (long long)m_lEpochDay * MS_PER_DAY + m_lMilliseconds;
        long long llLocalSeconds = llLocal / 1000;
        if ( llLocal % 1000 < 0 )
            llLocalSeconds--;
        llLocalSeconds -= (long long)UNIX_EPOCH_DAY * 86400LL;      // to Unix seconds

        int nCandidates[ 3 ];
        int nCount = 0;
        int nOffset = 0;
        if ( ! systemOffsetAt( llLocalSeconds, nOffset ) )
            return false;
        nCandidates[ nCount++ ] = nOffset;
        for ( int nPass = 0; nPass < 2; nPass++ )
        {
            int nNext = 0;
            if ( ! systemOffsetAt( llLocalSeconds - nCandidates[ nCount - 1 ], nNext ) )
                return false;
            nCandidates[ nCount++ ] = nNext;
        }

            // An offset is consistent if the instant it gives has that offset.
            // Two consistent offsets mean the local time happens twice (clocks
            // went back): take the earlier instant, the larger offset. None means
            // it never happened (clocks went forward): take the offset in force
            // before the change, the smaller one, which moves the time forward by
            // the length of the gap.
        bool bFound = false;
        int nSmallest = nCandidates[0];
        for ( int nX = 0; nX < nCount; nX++ )
        {
            int nCheck = 0;
            if ( nCandidates[ nX ] < nSmallest )
                nSmallest = nCandidates[ nX ];
            if ( systemOffsetAt( llLocalSeconds - nCandidates[ nX ], nCheck ) && nCheck == nCandidates[ nX ] )
            {
                if ( ! bFound || nCandidates[ nX ] > nOffset )
                    nOffset = nCandidates[ nX ];
                bFound = true;
            }
        }
        if ( ! bFound )
        {
            nOffset = nSmallest;
                // Keep the instant, so the local time moves forward by the gap.
            long long llUtc = llLocal - (long long)nOffset * 1000LL;
            int nAfter = nOffset;
            long long llUtcSeconds = llUtc / 1000;
            if ( llUtc % 1000 < 0 )
                llUtcSeconds--;
            if ( systemOffsetAt( llUtcSeconds - (long long)UNIX_EPOCH_DAY * 86400LL, nAfter ) )
            {
                *this = FromUtcMilliseconds( llUtc, nAfter );
                return true;
            }
        }

        m_bHasZone = true;
        m_nUTCOffset = nOffset;
        return true;
    }

    int CDate::FixAMPM( int nHour, int nAMPM )
    {
        switch ( nAMPM )
	    {
            case AMPM_NONE:
	            break;

            case AMPM_AM:
	            if ( nHour == 12 )
	                nHour = 0;
	            else if ( nHour > 12 || nHour < 1 )
                    nHour = -1;         // "13 AM" is not a time.
	            break;

            case AMPM_PM:
                if ( nHour > 12 || nHour < 1 )
                    nHour = -1;
                else if ( nHour != 12 )
	                nHour += 12;
	            break;
	    }

        return nHour;
    }

    namespace
    {
            // AdjustTime's offset argument for patterns without a time zone.
        const long NO_ZONE = -999999L;

            // Facts about the text being parsed that the sscanf patterns lose:
            // which numbers were written with three or more digits (so "0044"
            // is a year, not a two-digit year), whether the first number was
            // such a year (YYYY/MM/DD), the milliseconds, and whether a time
            // zone was given.
        struct CDateParseContext
        {
            std::vector<long>   longNumbers;
            bool                yearFirst;
            int                 milliseconds;
            bool                zone;
        };

        thread_local CDateParseContext g_parseContext;

        bool restBlank( const char *pszText, int nUsed )
        {
            if ( nUsed < 0 )
                return false;
            for ( const char *p = pszText + nUsed; *p; p++ )
            {
                if ( ! isspace( (unsigned char)*p ) )
                    return false;
            }
            return true;
        }

        bool isLongNumber( long lValue )
        {
            for ( size_t nX = 0; nX < g_parseContext.longNumbers.size(); nX++ )
            {
                if ( g_parseContext.longNumbers[ nX ] == lValue )
                    return true;
            }
            return false;
        }

        bool isWeekdayName( const std::string &word )
        {
            static const char *names[] = { "sun", "sunday", "mon", "monday", "tue", "tues", "tuesday", "wed", "wednesday",
                                           "thu", "thur", "thurs", "thursday", "fri", "friday", "sat", "saturday", NULL };
            for ( int nX = 0; names[ nX ]; nX++ )
            {
                if ( word == names[ nX ] )
                    return true;
            }
            return false;
        }

            // Puts the text in the form the patterns expect: lower case, no
            // leading weekday, no ordinal suffixes ("13th"), commas and "at" as
            // spaces, "GMT +5" joined, and milliseconds removed and recorded.
        std::string normalizeDateText( const char *pszText, CDateParseContext &ctx )
        {
            std::string text( pszText ? pszText : "" );
            for ( size_t nX = 0; nX < text.size(); nX++ )
                text[ nX ] = (char)tolower( (unsigned char)text[ nX ] );

            ctx.longNumbers.clear();
            ctx.yearFirst = false;
            ctx.milliseconds = 0;
            ctx.zone = false;

                // Fractional seconds: ":ss.fff"
            for ( size_t nX = 0; nX + 4 < text.size(); nX++ )
            {
                if ( text[ nX ] == ':' && isdigit( (unsigned char)text[ nX + 1 ] ) && isdigit( (unsigned char)text[ nX + 2 ] ) &&
                     text[ nX + 3 ] == '.' && isdigit( (unsigned char)text[ nX + 4 ] ) )
                {
                    size_t nEnd = nX + 4;
                    int nMs = 0;
                    int nDigits = 0;
                    while ( nEnd < text.size() && isdigit( (unsigned char)text[ nEnd ] ) )
                    {
                        if ( nDigits < 3 )
                        {
                            nMs = nMs * 10 + ( text[ nEnd ] - '0' );
                            nDigits++;
                        }
                        nEnd++;
                    }
                    while ( nDigits < 3 )
                    {
                        nMs *= 10;
                        nDigits++;
                    }
                    ctx.milliseconds = nMs;
                    text.erase( nX + 3, nEnd - ( nX + 3 ) );
                    break;
                }
            }

                // Sybase writes milliseconds as a fourth time field: "hh:mm:ss:fff".
            for ( size_t nX = 0; nX + 12 <= text.size() + 4 && nX + 7 < text.size(); nX++ )
            {
                if ( text[ nX ] == ':' && isdigit( (unsigned char)text[ nX + 1 ] ) && isdigit( (unsigned char)text[ nX + 2 ] ) &&
                     text[ nX + 3 ] == ':' && isdigit( (unsigned char)text[ nX + 4 ] ) && isdigit( (unsigned char)text[ nX + 5 ] ) &&
                     text[ nX + 6 ] == ':' && nX + 9 < text.size() && isdigit( (unsigned char)text[ nX + 7 ] ) &&
                     isdigit( (unsigned char)text[ nX + 8 ] ) && isdigit( (unsigned char)text[ nX + 9 ] ) &&
                     ( nX + 10 == text.size() || ! isdigit( (unsigned char)text[ nX + 10 ] ) ) )
                {
                    ctx.milliseconds = ( text[ nX + 7 ] - '0' ) * 100 + ( text[ nX + 8 ] - '0' ) * 10 + ( text[ nX + 9 ] - '0' );
                    text.erase( nX + 6, 4 );
                        // "...:111pm" -> "... pm"
                    if ( nX + 6 < text.size() && isalpha( (unsigned char)text[ nX + 6 ] ) )
                        text.insert( nX + 6, " " );
                    break;
                }
            }

                // Trim.
            size_t nStart = 0;
            while ( nStart < text.size() && isspace( (unsigned char)text[ nStart ] ) )
                nStart++;
            text.erase( 0, nStart );

                // A leading weekday name adds nothing (and is not checked).
            size_t nWord = 0;
            while ( nWord < text.size() && isalpha( (unsigned char)text[ nWord ] ) )
                nWord++;
            if ( nWord > 0 && isWeekdayName( text.substr( 0, nWord ) ) &&
                 ( nWord == text.size() || text[ nWord ] == ',' || text[ nWord ] == ' ' || text[ nWord ] == '.' ) )
            {
                size_t nNext = nWord;
                while ( nNext < text.size() && ( text[ nNext ] == ',' || text[ nNext ] == ' ' || text[ nNext ] == '.' ) )
                    nNext++;
                text.erase( 0, nNext );
            }

            std::string result;
            for ( size_t nX = 0; nX < text.size(); nX++ )
            {
                char ch = text[ nX ];

                    // Ordinal suffixes after a number.
                if ( nX > 0 && isdigit( (unsigned char)text[ nX - 1 ] ) && nX + 1 < text.size() )
                {
                    std::string two = text.substr( nX, 2 );
                    bool bEnd = ( nX + 2 >= text.size() ) || ! isalpha( (unsigned char)text[ nX + 2 ] );
                    if ( bEnd && ( two == "st" || two == "nd" || two == "rd" || two == "th" ) )
                    {
                        nX++;
                        continue;
                    }
                }

                result += ( ch == ',' ) ? ' ' : ch;
            }

                // " at " between the date and the time.
            size_t nAt;
            while ( ( nAt = result.find( " at " ) ) != std::string::npos )
                result.replace( nAt, 4, " " );

                // "gmt +5:40" -> "gmt+5:40" (hyphens are already slashes).
            static const char *zones[] = { "gmt", "utc", NULL };
            for ( int nZ = 0; zones[ nZ ]; nZ++ )
            {
                size_t nPos = 0;
                while ( ( nPos = result.find( zones[ nZ ], nPos ) ) != std::string::npos )
                {
                    size_t nAfter = nPos + 3;
                    size_t nSpace = nAfter;
                    while ( nSpace < result.size() && result[ nSpace ] == ' ' )
                        nSpace++;
                    if ( nSpace > nAfter && nSpace < result.size() && ( result[ nSpace ] == '+' || result[ nSpace ] == '/' ) )
                        result.erase( nAfter, nSpace - nAfter );
                    nPos = nAfter;
                }
            }

                // Note numbers written with three or more digits.
            bool bFirst = true;
            for ( size_t nX = 0; nX < result.size(); )
            {
                if ( isdigit( (unsigned char)result[ nX ] ) )
                {
                    size_t nEnd = nX;
                    long lValue = 0;
                    while ( nEnd < result.size() && isdigit( (unsigned char)result[ nEnd ] ) )
                    {
                        if ( nEnd - nX < 9 )
                            lValue = lValue * 10 + ( result[ nEnd ] - '0' );
                        nEnd++;
                    }
                    if ( nEnd - nX >= 3 )
                    {
                        ctx.longNumbers.push_back( lValue );
                        if ( bFirst )
                            ctx.yearFirst = true;
                    }
                    bFirst = false;
                    nX = nEnd;
                }
                else
                {
                    nX++;
                }
            }

            return result;
        }
    }

    CDate CDate::FinishParse( tm_type &tmVals )
    {
        tmVals.tm_msec = g_parseContext.milliseconds;
        CDate dttResult( tmVals );
        if ( dttResult.IsValid() && g_parseContext.zone )
        {
            dttResult.SetTimeZone( tmVals.tm_gmtoffset );
        }
        return dttResult;
    }

    CDate CDate::DateParse( const char *strRawDate )
    {
        CDateParseContext &ctx = g_parseContext;
        std::string strNormal = normalizeDateText( strRawDate, ctx );
        const char *strDate = strNormal.c_str();
        CDate       dttNow;
        char        str_mon[500] = "", str_wday[500] = "", str_gmtoff[500] = "", str_ampm[500] = "", str_trash[500] = "";
        int         tm_sec = 0, tm_min = 0, tm_hour = 0, tm_mday = 0, tm_year = 0;
        long        tm_mon = 0, tm_wday = 0, ampm, gmtoff = 0;
        int         nUsed = -1;
        tm_type     tmVals;

        tmVals.tm_msec = 0;
        tmVals.tm_sec = 0;
        tmVals.tm_min = 0;
        tmVals.tm_hour = 0;
        tmVals.tm_mday = dttNow.GetDay();
        tmVals.tm_mon = dttNow.GetMonth();
        tmVals.tm_year = dttNow.GetYear();
        tmVals.tm_isdst = 0;
        tmVals.tm_gmtoffset = 0;
        tmVals.tm_wday = 0;
        tmVals.tm_yday = 0;
        ampm = AMPM_NONE;

            /* YYYYMMDDHHMMSS[.fff] (DF_ISO_8601_MS_PACKED): ISO 8601 basic format without the "T". */
        {
            size_t nDigits = 0;
            while ( isdigit( (unsigned char)strDate[ nDigits ] ) )
                nDigits++;
            if ( nDigits == 14 && ( strDate[ 14 ] == '\0' || strDate[ 14 ] == '.' ) )
            {
                std::string strIso = std::string( strDate, 8 ) + "T" + std::string( strDate + 8 );
                bool bIso = false;
                CDate dttIso = ParseISO8601( strIso.c_str(), &bIso );
                if ( bIso )
                    return dttIso;
            }
        }

            /* DD/mth/YY:HH:MM:SS (Netscape and Apache logs, no zone) */
        if ( ( sscanf( strDate, "%d/%400[a-zA-Z]/%d:%d:%d:%d%n", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, &nUsed ) == 6 )
             && restBlank( strDate, nUsed ) && ScanMonths( str_mon, tm_mon ) )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = tm_hour;
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* mth DD YY */
        if ( ( sscanf( strDate, "%400[a-zA-Z] %d %d%n", str_mon, &tm_mday, &tm_year, &nUsed ) == 3 )
             && restBlank( strDate, nUsed ) && ScanMonths( str_mon, tm_mon ) )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* mth DD HH:MM:SS zone YY (asctime with a zone, weekday removed) */
        if ( ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d:%d %400[^\t\n\r ] %d%n", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_year, &nUsed ) == 7 )
             && restBlank( strDate, nUsed ) && ScanMonths( str_mon, tm_mon ) && ScanGMTOffset( str_gmtoff, gmtoff ) )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = tm_hour;
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* mth DD HH:MM:SS YY (asctime, weekday removed) */
        if ( ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d:%d %d%n", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, &tm_year, &nUsed ) == 6 )
             && restBlank( strDate, nUsed ) && ScanMonths( str_mon, tm_mon ) )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = tm_hour;
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD.MM.YY [HH:MM[:SS]] (day first, as written in much of Europe) */
        {
            int nDotDay = 0, nDotMonth = 0, nDotYear = 0;
            int nFields = 0;
            tm_hour = tm_min = tm_sec = 0;
            if ( ( sscanf( strDate, "%d.%d.%d %d:%d:%d%n", &nDotDay, &nDotMonth, &nDotYear, &tm_hour, &tm_min, &tm_sec, &nUsed ) == 6 ) && restBlank( strDate, nUsed ) )
                nFields = 6;
            else if ( ( sscanf( strDate, "%d.%d.%d %d:%d%n", &nDotDay, &nDotMonth, &nDotYear, &tm_hour, &tm_min, &nUsed ) == 5 ) && restBlank( strDate, nUsed ) )
                nFields = 5;
            else if ( ( sscanf( strDate, "%d.%d.%d%n", &nDotDay, &nDotMonth, &nDotYear, &nUsed ) == 3 ) && restBlank( strDate, nUsed ) )
                nFields = 3;
            if ( nFields > 0 )
            {
                tmVals.tm_mday = nDotDay;
                tmVals.tm_mon = nDotMonth - 1;
                tmVals.tm_year = nDotYear;
                tmVals.tm_hour = ( nFields >= 5 ) ? tm_hour : 0;
                tmVals.tm_min = ( nFields >= 5 ) ? tm_min : 0;
                tmVals.tm_sec = ( nFields == 6 ) ? tm_sec : 0;
                AdjustTime( tmVals, NO_ZONE );
                return FinishParse( tmVals );
            }
        }

        /* And do the sscanfs.  WARNING: you can add more formats here,
        ** but be careful!  You can easily screw up the parsing of existing
        ** formats when you add new ones.  The order is important.
        */

            /* DD/mth/YY:HH:MM:SS zone */
        if (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d:%d:%d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d:%d:%d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
        &&
            ( ScanMonths( str_mon, tm_mon )  )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* DD/mth/YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8  )
#else
                ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* DD/mth/YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#else
                ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* DD/mth/YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD/mth/YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD mth YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* mth DD??, YY at HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[^\t\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[^\t\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* mth DD??, YY at HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* mth DD??, YY at HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[apmAPM] %400[^\t\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[apmAPM] %400[^\t\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[^\t\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[^\t\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* mth DD??, YY at HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[apmAPM]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[apmAPM]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* mth DD YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[^\t\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[^\t\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* mth DD YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d:%d", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d:%d", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* mth DD YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d %400[apmAPM] %400[^\t\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d %400[apmAPM] %400[^\t\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d %400[^\t\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d %400[^\t\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* mth DD YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min ) == 5 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD mth YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d %400[^\t\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* DD mth YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }
            /* DD mth YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD mth HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d:%d:%d", &tm_mday, str_mon, &tm_hour, &tm_min, &tm_sec ) == 5 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d:%d:%d", &tm_mday, str_mon, &tm_hour, &tm_min, &tm_sec ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD mth HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_hour, &tm_min, str_ampm ) == 5 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_hour, &tm_min, str_ampm ) == 5 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d:%d", &tm_mday, str_mon, &tm_hour, &tm_min ) == 4 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d:%d", &tm_mday, str_mon, &tm_hour, &tm_min ) == 4 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* HH:MM:SS ampm zone DD/mth/YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM] %400[^\t\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 8 )
#else
                ( sscanf( strDate, "%d:%d:%d %400[apmAPM] %400[^\t\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %400[^\t\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#else
            ( sscanf( strDate, "%d:%d:%d %400[^\t\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* HH:MM ampm zone DD/mth/YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d %400[apmAPM] %400[^\t\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#else
                ( sscanf( strDate, "%d:%d %400[apmAPM] %400[^\t\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %400[^\t\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 6 )
#else
            ( sscanf( strDate, "%d:%d %400[^\t\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* HH:MM:SS ampm DD/mth/YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon, &tm_year ) == 7 )
#else
                ( sscanf( strDate, "%d:%d:%d %400[apmAPM] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon, &tm_year ) == 6 )
#else
            ( sscanf( strDate, "%d:%d:%d %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* HH:MM ampm DD/mth/YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d %400[apmAPM] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_ampm, &tm_mday, str_mon, &tm_year ) == 6 )
#else
                ( sscanf( strDate, "%d:%d %400[apmAPM] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_ampm, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 )
#else
            ( sscanf( strDate, "%d:%d %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* HH:MM:SS ampm zone DD mth YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM] %400[^\t\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 8 )
#else
                ( sscanf( strDate, "%d:%d:%d %400[apmAPM] %400[^\t\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %400[^\t\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#else
            ( sscanf( strDate, "%d:%d:%d %400[^\t\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* HH:MM ampm zone DD mth YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d %400[apmAPM] %400[^\t\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#else
                ( sscanf( strDate, "%d:%d %400[apmAPM] %400[^\t\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %400[^\t\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 6 )
#else
            ( sscanf( strDate, "%d:%d %400[^\t\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* HH:MM:SS ampm DD mth YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon, &tm_year ) == 7 )
#else
                ( sscanf( strDate, "%d:%d:%d %400[apmAPM] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon, &tm_year ) == 6 )
#else
            ( sscanf( strDate, "%d:%d:%d %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* HH:MM ampm DD mth YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d %400[apmAPM] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_ampm, &tm_mday, str_mon, &tm_year ) == 6 )
#else
                ( sscanf( strDate, "%d:%d %400[apmAPM] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_ampm, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 )
#else
            ( sscanf( strDate, "%d:%d %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* wdy, DD/mth/YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* wdy, DD/mth/YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[apmAPM] %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[apmAPM] %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* wdy, DD/mth/YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* wdy, DD/mth/YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 6 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* wdy, DD mth YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* wdy, DD mth YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[^\t\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* wdy, DD mth YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* wdy, DD mth YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 6 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* wdy mth DD HH:MM:SS ampm zone YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_year ) == 9 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_year ) == 9 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %400[^\t\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_year ) == 8 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %400[^\t\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_year ) == 8 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* wdy mth DD HH:MM ampm zone YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^\t\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_year ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^\t\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_year ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d %400[^\t\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff, &tm_year ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d %400[^\t\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff, &tm_year ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

            /* mth DD HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d %d:%d:%d", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 5 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d:%d", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* mth DD HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 5 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 5 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d %d:%d", str_mon, &tm_mday, &tm_hour, &tm_min ) == 4 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d", str_mon, &tm_mday, &tm_hour, &tm_min ) == 4 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD/mth/YY */
        if (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d", &tm_mday, str_mon, &tm_year ) == 3 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d", &tm_mday, str_mon, &tm_year ) == 3 )
#endif
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD mth YY */
        if (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d", &tm_mday, str_mon, &tm_year ) == 3 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d", &tm_mday, str_mon, &tm_year ) == 3 )
#endif
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* MM/DD/YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d/%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8  )
#else
                ( sscanf( strDate, "%ld/%d/%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d:%d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d:%d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* MM/DD/YYYY HH:MM:SS ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d:%d", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d:%d", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#endif
        )
        {
            if ( tm_mon > 12 || ctx.yearFirst )
            {
                    // Format YYYY/MM/DD Cannot be determined by any other means.
                int nTemp = tm_year;
                tm_year = tm_mon;
                tm_mon = tm_mday;
                tm_mday = nTemp;
            }
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* MM/DD HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 7  )
#else
                ( sscanf( strDate, "%ld/%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 7  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d:%d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d:%d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 6 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* MM/DD HH:MM:SS ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 5 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 6  )
#else
                ( sscanf( strDate, "%d %d:%d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 6  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d:%d %400[^\t\n\r ]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 5 )
#else
            ( sscanf( strDate, "%d %d:%d:%d %400[^\t\n\r ]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 5 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* DD HH:MM:SS ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 5 )
#else
            ( sscanf( strDate, "%d %d:%d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 5 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d:%d", &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 4 )
#else
            ( sscanf( strDate, "%d %d:%d:%d", &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 4 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* MM/DD/YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d/%d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7  )
#else
                ( sscanf( strDate, "%ld/%d/%d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* MM/DD/YYYY HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min ) == 5 )
#endif
        )
        {
            if ( tm_mon > 12 || ctx.yearFirst )
            {
                    // Format YYYY/MM/DD Cannot be determined by any other means.
                int nTemp = tm_year;
                tm_year = tm_mon;
                tm_mon = tm_mday;
                tm_mday = nTemp;
            }
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* MM/DD HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 6  )
#else
                ( sscanf( strDate, "%ld/%d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 6  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 5 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* MM/DD HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 5 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min ) == 4 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min ) == 4 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 5  )
#else
                ( sscanf( strDate, "%d %d:%d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 5  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d %400[^\t\n\r ]", &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 4 )
#else
            ( sscanf( strDate, "%d %d:%d %400[^\t\n\r ]", &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 4 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* DD HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, str_ampm ) == 4 )
#else
            ( sscanf( strDate, "%d %d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, str_ampm ) == 4 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d", &tm_mday, &tm_hour, &tm_min ) == 3 )
#else
            ( sscanf( strDate, "%d %d:%d", &tm_mday, &tm_hour, &tm_min ) == 3 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* MM/DD/YY HH ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d/%d %d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_ampm, str_gmtoff ) == 6  )
#else
                ( sscanf( strDate, "%ld/%d/%d %d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_ampm, str_gmtoff ) == 6  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_gmtoff ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_gmtoff ) == 5 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* MM/DD/YYYY HH ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_ampm ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_ampm ) == 5 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d", &tm_mon, &tm_mday, &tm_year, &tm_hour ) == 4 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d", &tm_mon, &tm_mday, &tm_year, &tm_hour ) == 4 )
#endif
        )
        {
            if ( tm_mon > 12 || ctx.yearFirst )
            {
                    // Format YYYY/MM/DD Cannot be determined by any other means.
                int nTemp = tm_year;
                tm_year = tm_mon;
                tm_mon = tm_mday;
                tm_mday = nTemp;
            }
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* MM/DD HH ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d %d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, str_ampm, str_gmtoff ) == 5  )
#else
                ( sscanf( strDate, "%ld/%d %d %400[apmAPM] %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, str_ampm, str_gmtoff ) == 5  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, str_gmtoff ) == 4 )
#else
            ( sscanf( strDate, "%ld/%d %d %400[^\t\n\r ]", &tm_mon, &tm_mday, &tm_hour, str_gmtoff ) == 4 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* MM/DD HH ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, str_ampm ) == 4 )
#else
            ( sscanf( strDate, "%ld/%d %d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, str_ampm ) == 4 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d", &tm_mon, &tm_mday, &tm_hour ) == 3 )
#else
            ( sscanf( strDate, "%ld/%d %d", &tm_mon, &tm_mday, &tm_hour ) == 3 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD HH ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, &tm_hour, str_ampm, str_gmtoff ) == 4  )
#else
                ( sscanf( strDate, "%d %d %400[apmAPM] %400[^\t\n\r ]", &tm_mday, &tm_hour, str_ampm, str_gmtoff ) == 4  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d %400[^\t\n\r ]", &tm_mday, &tm_hour, str_gmtoff ) == 3 )
#else
            ( sscanf( strDate, "%d %d %400[^\t\n\r ]", &tm_mday, &tm_hour, str_gmtoff ) == 3 )
#endif
            )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, gmtoff );
            return FinishParse( tmVals );
        }

        /* DD HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, str_ampm ) == 4 )
#else
            ( sscanf( strDate, "%d %d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, str_ampm ) == 4 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d", &tm_mday, &tm_hour, &tm_min ) == 3 )
#else
            ( sscanf( strDate, "%d %d:%d", &tm_mday, &tm_hour, &tm_min ) == 3 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

        /* MM/DD/YYYY */
#ifdef IASLIB_WIN32__
        if ( sscanf_s( strDate, "%ld/%d/%d", &tm_mon, &tm_mday, &tm_year ) == 3 )
#else
        if ( ( sscanf( strDate, "%ld/%d/%d%n", &tm_mon, &tm_mday, &tm_year, &nUsed ) == 3 ) && restBlank( strDate, nUsed ) )
#endif
        {
            if ( tm_mon > 12 || ctx.yearFirst )
            {
                    // Format YYYY/MM/DD Cannot be determined by any other means.
                int nTemp = tm_year;
                tm_year = tm_mon;
                tm_mon = tm_mday;
                tm_mday = nTemp;
            }
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

        /* MM/DD */
#ifdef IASLIB_WIN32__
        if ( sscanf_s( strDate, "%ld/%d", &tm_mon, &tm_mday ) == 2 )
#else
        if ( ( sscanf( strDate, "%ld/%d%n", &tm_mon, &tm_mday, &nUsed ) == 2 ) && restBlank( strDate, nUsed ) )
#endif
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

        /* HH:MM:SS ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM]", &tm_hour, &tm_min, &tm_sec, str_ampm ) == 4 )
#else
            ( sscanf( strDate, "%d:%d:%d %400[apmAPM]", &tm_hour, &tm_min, &tm_sec, str_ampm ) == 4 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d", &tm_hour, &tm_min, &tm_sec ) == 3 )
#else
            ( sscanf( strDate, "%d:%d:%d", &tm_hour, &tm_min, &tm_sec ) == 3 )
#endif
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %400[apmAPM]", &tm_hour, &tm_min, str_ampm ) == 3 )
#else
            ( sscanf( strDate, "%d:%d %400[apmAPM]", &tm_hour, &tm_min, str_ampm ) == 3 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d", &tm_hour, &tm_min ) == 2 )
#else
            ( sscanf( strDate, "%d:%d", &tm_hour, &tm_min ) == 2 )
#endif
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* DD mth */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z]", &tm_mday, str_mon ) == 2 )
#else
            ( ( sscanf( strDate, "%d %400[a-zA-Z]%n", &tm_mday, str_mon, &nUsed ) == 2 ) && restBlank( strDate, nUsed ) )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

        /* YYYYMMDD */
        if ( ( sscanf( strDate, "%4d%2ld%2d%n", &tm_year, &tm_mon, &tm_mday, &nUsed ) == 3 ) && restBlank( strDate, nUsed ) )
        {
            tmVals.tm_year = tm_year;
            tmVals.tm_mon = tm_mon - 1;
            tmVals.tm_mday = tm_mday;
            ctx.longNumbers.push_back( tm_year );
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

        /* DD */
#ifdef IASLIB_WIN32__
        if ( sscanf_s( strDate, "%d", &tm_mday ) == 1 )
#else
        if ( ( sscanf( strDate, "%d%n", &tm_mday, &nUsed ) == 1 ) && restBlank( strDate, nUsed ) )
#endif
        {
            tmVals.tm_mday = tm_mday;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            /* mth DD */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d", str_mon, &tm_mday ) == 2 )
#else
            ( ( sscanf( strDate, "%400[a-zA-Z] %d%n", str_mon, &tm_mday, &nUsed ) == 2 ) && restBlank( strDate, nUsed ) )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            AdjustTime( tmVals, NO_ZONE );
            return FinishParse( tmVals );
        }

            // Nothing matched: not a date.
        return CDate( 0L, -1L );
    }

    bool CDate::ScanTokens( Token_Type *aTokens, int nLength, CString strCompare, long &lValue )
    {
        int nCount = 0;
        while ( nCount < nLength )
        {
            if ( strCompare == aTokens->strData )
            {
                lValue = aTokens->lToken;
                return true;
            }
            aTokens++;
            nCount++;
        }
        return false;
    }

    bool CDate::ScanAMPM( const char *pszAMPM, long  &lValue )
    {
        static Token_Type AMPMTable[2] = {
                                            { "am", AMPM_AM },
                                            { "pm", AMPM_PM },
                                        };
        CString strAMPM = pszAMPM;
        strAMPM.ToLowerCase();
        return ScanTokens( AMPMTable, 2, strAMPM, lValue );
    }


    bool CDate::ScanWday( const char *pszWday, long &lValue )
    {
        static Token_Type WdayTable[14] = {
                                            { "sun", 0 },
                                            { "sunday", 0 },
                                            { "mon", 1 },
                                            { "monday", 1 },
                                            { "tue", 2 },
                                            { "tuesday", 2 },
                                            { "wed", 3 },
                                            { "wednesday", 3 },
                                            { "thu", 4 },
                                            { "thursday", 4 },
                                            { "fri", 5 },
                                            { "friday", 5 },
                                            { "sat", 6 },
                                            { "saturday", 6 },
                                        };
        CString strWday = pszWday;
        strWday.ToLowerCase();
        return ScanTokens( WdayTable, 14, strWday, lValue );
    }

    bool CDate::ScanMonths( const char *pszMonth, long &lValue )
    {
        static Token_Type MonthTable[23] = {
                                            { "jan", 0 },
                                            { "january", 0 },
                                            { "feb", 1 },
                                            { "february", 1 },
                                            { "mar", 2 },
                                            { "march", 2 },
                                            { "apr", 3 },
                                            { "april", 3 },
                                            { "may", 4 },
                                            { "jun", 5 },
                                            { "june", 5 },
                                            { "jul", 6 },
                                            { "july", 6 },
                                            { "aug", 7 },
                                            { "august", 7 },
                                            { "sep", 8 },
                                            { "september", 8 },
                                            { "oct", 9 },
                                            { "october", 9 },
                                            { "nov", 10 },
                                            { "november", 10 },
                                            { "dec", 11 },
                                            { "december", 11 },
                                        };
        CString strMonth = pszMonth;
        strMonth.ToLowerCase();
        return ScanTokens( MonthTable, 23, strMonth, lValue );
    }

        // Offsets are in seconds east of UTC (UTC+1 is 3600, EST is -18000).
        // Hyphens have been turned into slashes, so "/0500" is UTC-5.
    bool CDate::ScanGMTOffset( const char *pszOffset, long &lValue )
    {
        static Token_Type GMTOffsetTable[] = {
                                                { "gmt", 0L }, { "utc", 0L }, { "ut", 0L },
                                                { "0000", 0L }, { "+0000", 0L }, { "/0000", 0L },
                                                { "0100", 3600L }, { "+0100", 3600L }, { "/0100", -3600L },
                                                { "0200", 7200L }, { "+0200", 7200L }, { "/0200", -7200L },
                                                { "0300", 10800L }, { "+0300", 10800L }, { "/0300", -10800L },
                                                { "0400", 14400L }, { "+0400", 14400L }, { "/0400", -14400L },
                                                { "0500", 18000L }, { "+0500", 18000L }, { "/0500", -18000L },
                                                { "0600", 21600L }, { "+0600", 21600L }, { "/0600", -21600L },
                                                { "0700", 25200L }, { "+0700", 25200L }, { "/0700", -25200L },
                                                { "0800", 28800L }, { "+0800", 28800L }, { "/0800", -28800L },
                                                { "0900", 32400L }, { "+0900", 32400L }, { "/0900", -32400L },
                                                { "1000", 36000L }, { "+1000", 36000L }, { "/1000", -36000L },
                                                { "1100", 39600L }, { "+1100", 39600L }, { "/1100", -39600L },
                                                { "1200", 43200L }, { "+1200", 43200L }, { "/1200", -43200L },
                                                { "wet", 0L }, { "west", 3600L },                   /* Western European time */
                                                { "cet", 3600L }, { "cest", 7200L }, { "ced", 7200L }, /* Central European time */
                                                { "mez", 3600L }, { "mesz", 7200L },                /* Mitteleuropdische Zeit */
                                                { "eet", 7200L }, { "eest", 10800L },               /* Eastern European time */
                                                { "jst", 32400L }, { "jdt", 36000L },               /* Japan time */
                                                { "bst", 3600L },                                   /* British Summer Time */
                                                { "nst", -12600L }, { "ndt", -9000L },              /* Newfoundland */
                                                { "ast", -14400L }, { "adt", -10800L },
                                                { "est", -18000L }, { "edt", -14400L },
                                                { "cst", -21600L }, { "cdt", -18000L },
                                                { "mst", -25200L }, { "mdt", -21600L },
                                                { "pst", -28800L }, { "pdt", -25200L },
                                                { "akst", -32400L }, { "akdt", -28800L },
                                                { "yst", -32400L }, { "ydt", -28800L },
                                                { "hst", -36000L }, { "hdt", -32400L },
                                                    /* Military zones: A-M (no J) are east of UTC, N-Y west, Z is UTC. */
                                                { "a", 3600L }, { "b", 7200L }, { "c", 10800L }, { "d", 14400L },
                                                { "e", 18000L }, { "f", 21600L }, { "g", 25200L }, { "h", 28800L },
                                                { "i", 32400L }, { "k", 36000L }, { "l", 39600L }, { "m", 43200L },
                                                { "n", -3600L }, { "o", -7200L }, { "p", -10800L }, { "q", -14400L },
                                                { "r", -18000L }, { "s", -21600L }, { "t", -25200L }, { "u", -28800L },
                                                { "v", -32400L }, { "w", -36000L }, { "x", -39600L }, { "y", -43200L },
                                                { "z", 0L },
                                            };
        CString strOffset = pszOffset;
        strOffset.ToLowerCase();
        strOffset.Trim();
        lValue = 0;

        if ( ScanTokens( GMTOffsetTable, sizeof( GMTOffsetTable ) / sizeof( Token_Type ), strOffset, lValue ) )
            return true;

            // Numeric offsets: [gmt|utc|ut] then a sign, then h, hh, hhmm, h:mm
            // or hh:mm. Without the prefix the sign is required.
        std::string text( (const char *)strOffset );
        if ( text.compare( 0, 3, "gmt" ) == 0 || text.compare( 0, 3, "utc" ) == 0 )
            text.erase( 0, 3 );
        else if ( text.compare( 0, 2, "ut" ) == 0 )
            text.erase( 0, 2 );

        if ( text.empty() || ( text[0] != '+' && text[0] != '/' ) )
            return false;
        long lSign = ( text[0] == '/' ) ? -1 : 1;
        text.erase( 0, 1 );

        long lHours = 0;
        long lMinutes = 0;
        size_t nColon = text.find( ':' );
        if ( nColon != std::string::npos )
        {
            std::string hours = text.substr( 0, nColon );
            std::string minutes = text.substr( nColon + 1 );
            if ( hours.empty() || hours.size() > 2 || minutes.size() != 2 )
                return false;
            for ( size_t nX = 0; nX < hours.size(); nX++ )
                if ( ! isdigit( (unsigned char)hours[ nX ] ) ) return false;
            for ( size_t nX = 0; nX < minutes.size(); nX++ )
                if ( ! isdigit( (unsigned char)minutes[ nX ] ) ) return false;
            lHours = atol( hours.c_str() );
            lMinutes = atol( minutes.c_str() );
        }
        else
        {
            if ( text.empty() || text.size() > 4 || text.size() == 3 )
                return false;
            for ( size_t nX = 0; nX < text.size(); nX++ )
                if ( ! isdigit( (unsigned char)text[ nX ] ) ) return false;
            if ( text.size() <= 2 )
            {
                lHours = atol( text.c_str() );
            }
            else
            {
                lHours = atol( text.substr( 0, 2 ).c_str() );
                lMinutes = atol( text.substr( 2, 2 ).c_str() );
            }
        }

        if ( lHours > 14 || lMinutes > 59 )
            return false;

        lValue = lSign * ( lHours * 3600 + lMinutes * 60 );
        return true;
    }

        // Records the time zone (or NO_ZONE) and resolves one- and two-digit
        // years. Years written with three or more digits are used as they are.
    void CDate::AdjustTime( tm_type &tmVals, long gmtoff )
    {
        if ( tmVals.tm_year >= 0 && tmVals.tm_year < 100 && ! isLongNumber( tmVals.tm_year ) )
        {
            tmVals.tm_year = AdjustTwoDigitYear( tmVals.tm_year );
        }

        if ( gmtoff == NO_ZONE )
        {
            tmVals.tm_gmtoffset = 0;
            g_parseContext.zone = false;
        }
        else
        {
            tmVals.tm_gmtoffset = gmtoff;
            g_parseContext.zone = true;
        }
    }

        // Toggles between UTC and the zone nOffset minutes east of UTC: a UTC
        // date becomes local time in that zone, any other date becomes UTC (a
        // date without a zone is taken to be in that zone).
    void CDate::TZAdjust( int nOffset )
    {
        if ( IsGMT() )
        {
            *this = ToTimeZone( nOffset * 60 );
        }
        else
        {
            SetToGMT( nOffset );
        }
    }

        // Converts to UTC; a date without a zone is taken to be nOffset minutes
        // east of UTC.
    void CDate::SetToGMT( int nOffset )
    {
        if ( ! m_bHasZone )
        {
            SetTimeZone( nOffset * 60 );
        }
        *this = ToUTC();
    }

        // The current time in seconds since 1 January 1970 UTC.
    CString CDate::EpochTimestamp( void )
    {
        CDate dttNow;
        long long llSeconds = (long long)( dttNow.m_lEpochDay - UNIX_EPOCH_DAY ) * 86400LL + dttNow.m_lMilliseconds / 1000;

        CString retVal( (const char *)(std::to_string( llSeconds )).c_str() );

        return retVal;
    }

        // The current time in seconds since 1 January 1900 UTC (the NTP epoch).
    CString CDate::NTPTimestamp( void )
    {
        CDate dttNow;
        long long llSeconds = (long long)( dttNow.m_lEpochDay - UNIX_EPOCH_DAY ) * 86400LL + dttNow.m_lMilliseconds / 1000 + NTP_TO_UNIX_SECONDS;

        CString retVal( (const char *)(std::to_string( llSeconds )).c_str() );

        return retVal;
    }

} // namespace IASLib
