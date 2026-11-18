/**
 * SJSON stream-mode state. See SjsonStream.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonStream.h"
#include "SjsonCodec.h"
#include "SjsonDictionary.h"
#include "SjsonUtil.h"
#include "JsonNode.h"
#include "NumberNode.h"
#include "TextNode.h"

#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace IASLib
{
    // ---- text channel --------------------------------------------------

    std::string CSjsonTextChannel::read( size_t nLength )
    {
        if ( m_nPos + nLength > m_strData.size() )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "text channel overrun (stream out of sync)" );
        std::string strOut = m_strData.substr( m_nPos, nLength );
        m_nPos += nLength;
        return strOut;
    }

    std::string CSjsonTextChannel::readTerminated( void )
    {
        size_t nEnd = m_strData.find( '\0', m_nPos );
        if ( nEnd == std::string::npos )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "text channel overrun (stream out of sync)" );
        std::string strOut = m_strData.substr( m_nPos, nEnd - m_nPos );
        m_nPos = nEnd + 1;
        return strOut;
    }

    size_t CSjsonTextChannel::size( void ) const
    {
        size_t nSize = 0;
        for ( size_t n = 0; n < m_aOut.size(); n++ )
            nSize += m_aOut[ n ].size();
        return nSize;
    }

    std::string CSjsonTextChannel::pending( void ) const
    {
        std::string strOut;
        for ( size_t n = 0; n < m_aOut.size(); n++ )
            strOut += m_aOut[ n ];
        return strOut;
    }

    // ---- timestamps ----------------------------------------------------

    namespace
    {
        struct Timestamp
        {
            long        lOrdinal;
            long        lSeconds;
            long        lFraction;
            int         nDigits;
            bool        bHasTime;
            std::string strZone;
        };

        bool digitsAt( const std::string &s, size_t nPos, size_t nCount )
        {
            if ( nPos + nCount > s.size() )
                return false;
            for ( size_t n = nPos; n < nPos + nCount; n++ )
                if ( s[ n ] < '0' || s[ n ] > '9' )
                    return false;
            return true;
        }

        int number( const std::string &s, size_t nPos, size_t nCount )
        {
            return atoi( s.substr( nPos, nCount ).c_str() );
        }

        const long g_alPow10[] = { 1L, 10L, 100L, 1000L, 10000L, 100000L, 1000000L, 10000000L, 100000000L, 1000000000L };

        bool formatTimestamp( const Timestamp &t, std::string &strOut )
        {
            int nYear, nMonth, nDay;
            if ( ! SjsonUtil::dateFromOrdinal( t.lOrdinal, nYear, nMonth, nDay ) )
                return false;
            if ( ! t.bHasTime && ( t.lSeconds || t.lFraction ) )
                return false;
            char szBuffer[ 64 ];
            snprintf( szBuffer, sizeof( szBuffer ), "%04d-%02d-%02d", nYear, nMonth, nDay );
            strOut = szBuffer;
            if ( t.bHasTime )
            {
                snprintf( szBuffer, sizeof( szBuffer ), "T%02ld:%02ld:%02ld", t.lSeconds / 3600, t.lSeconds / 60 % 60, t.lSeconds % 60 );
                strOut += szBuffer;
                if ( t.nDigits )
                {
                    snprintf( szBuffer, sizeof( szBuffer ), ".%0*ld", t.nDigits, t.lFraction );
                    strOut += szBuffer;
                }
                strOut += t.strZone;
            }
            return true;
        }

            // Parses YYYY-MM-DD[THH:MM:SS[.f{1,9}][Z|+HH:MM]] exactly, as
            // Python's _Timestamp.parse (it must format back to the same text).
        bool parseTimestamp( const std::string &s, Timestamp &t )
        {
            if ( ! digitsAt( s, 0, 4 ) || s.size() < 10 || s[ 4 ] != '-' || ! digitsAt( s, 5, 2 ) || s[ 7 ] != '-' || ! digitsAt( s, 8, 2 ) )
                return false;
            int nYear = number( s, 0, 4 ), nMonth = number( s, 5, 2 ), nDay = number( s, 8, 2 );
            if ( ! SjsonUtil::validDate( nYear, nMonth, nDay ) )
                return false;
            t.lOrdinal = SjsonUtil::dateOrdinal( nYear, nMonth, nDay );
            t.lSeconds = t.lFraction = 0;
            t.nDigits = 0;
            t.bHasTime = false;
            t.strZone.clear();
            size_t n = 10;
            if ( n < s.size() )
            {
                if ( s[ n ] != 'T' || ! digitsAt( s, n + 1, 2 ) || s.size() < n + 9 || s[ n + 3 ] != ':' || ! digitsAt( s, n + 4, 2 ) || s[ n + 6 ] != ':' || ! digitsAt( s, n + 7, 2 ) )
                    return false;
                int nHour = number( s, n + 1, 2 ), nMinute = number( s, n + 4, 2 ), nSecond = number( s, n + 7, 2 );
                if ( nHour > 23 || nMinute > 59 || nSecond > 59 )
                    return false;
                t.bHasTime = true;
                t.lSeconds = nHour * 3600L + nMinute * 60L + nSecond;
                n += 9;
                if ( n < s.size() && s[ n ] == '.' )
                {
                    size_t nStart = ++n;
                    while ( n < s.size() && s[ n ] >= '0' && s[ n ] <= '9' )
                        n++;
                    t.nDigits = (int)( n - nStart );
                    if ( t.nDigits < 1 || t.nDigits > 9 )
                        return false;
                    t.lFraction = atol( s.substr( nStart, n - nStart ).c_str() );
                }
                if ( n < s.size() )
                {
                    if ( s[ n ] == 'Z' && n + 1 == s.size() )
                        t.strZone = "Z";
                    else if ( ( s[ n ] == '+' || s[ n ] == '-' ) && n + 6 == s.size() && digitsAt( s, n + 1, 2 ) && s[ n + 3 ] == ':' && digitsAt( s, n + 4, 2 ) )
                        t.strZone = s.substr( n );
                    else
                        return false;
                }
            }
            std::string strBack;
            return formatTimestamp( t, strBack ) && strBack == s;
        }

        void writeSigned( long long llValue, CSjsonBitWriter &out )
        {
            out.writeBool( llValue < 0 );
            out.writeNybble( llValue < 0 ? (unsigned long long)( -llValue ) : (unsigned long long)llValue );
        }

        long long readSigned( CSjsonBitReader &in )
        {
            bool bNegative = in.readBool();
            long long llMagnitude = (long long)in.readNybble();
            return bNegative ? -llMagnitude : llMagnitude;
        }

        unsigned long long doubleBits( double d )
        {
            unsigned long long u;
            memcpy( &u, &d, sizeof( u ) );
            return u;
        }

        double bitsDouble( unsigned long long u )
        {
            double d;
            memcpy( &d, &u, sizeof( d ) );
            return d;
        }

            // IEEE-754 bit pattern -> integer that orders like the float, as
            // (negative, magnitude) of: bits < 2^63 ? bits : -(bits - 2^63) - 1.
        const unsigned long long SIGN = 1ULL << 63;

        unsigned bitLength( unsigned long long u )
        {
            unsigned n = 0;
            while ( u ) { n++; u >>= 1; }
            return n;
        }
    }

    // ---- stream state ----------------------------------------------------

    CSjsonStreamState::CSjsonStreamState( unsigned nWidth )
        : m_nWidth( nWidth ), m_nSize( (size_t)1 << nWidth ), m_bJournal( false )
    {
        if ( nWidth > MAX_WIDTH )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "cache width must be 0 to 7 bits" );
    }

    void CSjsonStreamState::begin( void )
    {
        m_mapJournal.clear();
        m_bJournal = true;
        m_aPath.clear();
        m_text.resetOut();
    }

    void CSjsonStreamState::commit( void )
    {
        m_mapJournal.clear();
        m_bJournal = false;
        m_aPath.clear();
    }

    void CSjsonStreamState::rollback( void )
    {
        m_text.resetOut();
        for ( std::map<std::string, std::pair<bool, std::vector<Value> > >::iterator it = m_mapJournal.begin(); it != m_mapJournal.end(); ++it )
        {
            if ( ! it->second.first )
                m_mapCaches.erase( it->first );
            else
                m_mapCaches[ it->first ] = it->second.second;
        }
        m_mapJournal.clear();
        m_bJournal = false;
        m_aPath.clear();
    }

    std::string CSjsonStreamState::key( const std::string &strLeaf ) const
    {
        std::string strKey;
        for ( size_t n = 0; n < m_aPath.size(); n++ )
        {
            strKey += m_aPath[ n ];
            strKey += '/';
        }
        return strKey + strLeaf;
    }

    void CSjsonStreamState::remember( const std::string &strKey, const CJsonNode &value )
    {
        std::map<std::string, std::vector<Value> >::iterator it = m_mapCaches.find( strKey );
        if ( m_bJournal && m_mapJournal.find( strKey ) == m_mapJournal.end() )
        {
            if ( it == m_mapCaches.end() )
                m_mapJournal[ strKey ] = std::make_pair( false, std::vector<Value>() );
            else
                m_mapJournal[ strKey ] = std::make_pair( true, it->second );
        }
        std::vector<Value> &aCache = m_mapCaches[ strKey ];
        for ( size_t n = 0; n < aCache.size(); n++ )
        {
            if ( SjsonCodec::sameValue( aCache[ n ].get(), &value ) )
            {
                aCache.erase( aCache.begin() + (long)n );
                break;
            }
        }
        aCache.insert( aCache.begin(), Value( value.clone() ) );
        if ( aCache.size() > m_nSize )
            aCache.resize( m_nSize );
    }

    void CSjsonStreamState::writeIndex( size_t nIndex, CSjsonBitWriter &out ) const
    {
        if ( m_nWidth )
            out.writeBits( nIndex, m_nWidth );
    }

    const CJsonNode &CSjsonStreamState::cached( const std::vector<Value> &aCache, unsigned long long nIndex ) const
    {
        if ( nIndex >= aCache.size() )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "stream cache index out of range (stream out of sync)" );
        return *aCache[ (size_t)nIndex ];
    }

    void CSjsonStreamState::encode( const std::string &strKey, const CJsonNode &value, const PlainEncoder &plain,
                                    CSjsonDictionary *pDictionary, CSjsonBitWriter &out )
    {
        std::vector<Value> aCache;
        std::map<std::string, std::vector<Value> >::const_iterator it = m_mapCaches.find( strKey );
        if ( it != m_mapCaches.end() )
            aCache = it->second;
        for ( size_t n = 0; n < aCache.size(); n++ )
        {
            if ( SjsonCodec::sameValue( aCache[ n ].get(), &value ) )
            {
                remember( strKey, value );
                out.writeBool( true );
                writeIndex( n, out );
                return;
            }
        }
        bool bHaveBest = false;
        CSjsonBitWriter best;
        for ( size_t n = 0; n < aCache.size(); n++ )
        {
            CSjsonBitWriter delta;
            if ( encodeDelta( *aCache[ n ], value, delta ) )
            {
                CSjsonBitWriter candidate;
                candidate.writeBinary( "01" );
                writeIndex( n, candidate );
                candidate.append( delta );
                if ( ! bHaveBest || candidate.bitLength() < best.bitLength() )
                {
                    best = candidate;
                    bHaveBest = true;
                }
            }
        }
        std::pair<unsigned long long, unsigned long long> checkpoint;
        if ( pDictionary )
            checkpoint = pDictionary->checkpoint();
        size_t nTextMark = m_text.mark(), nTextBefore = m_text.size();
        CSjsonBitWriter direct;
        direct.writeBinary( "00" );
        plain( direct );
        size_t nDirectCost = direct.bitLength() + CSjsonTextChannel::BITS_PER_BYTE * ( m_text.size() - nTextBefore );
        if ( bHaveBest && best.bitLength() < nDirectCost )
        {
            if ( pDictionary )
                pDictionary->rollback( checkpoint );
            m_text.truncate( nTextMark );
        }
        else
            best = direct;
        remember( strKey, value );
        out.append( best );
    }

    CJsonNode *CSjsonStreamState::decode( CSjsonBitReader &in, const std::string &strKey, const PlainDecoder &plain )
    {
        std::vector<Value> aCache;
        std::map<std::string, std::vector<Value> >::const_iterator it = m_mapCaches.find( strKey );
        if ( it != m_mapCaches.end() )
            aCache = it->second;
        CJsonNode *pValue;
        if ( in.readBool() )
            pValue = cached( aCache, in.readBits( m_nWidth ) ).clone();
        else if ( in.readBool() )
            pValue = decodeDelta( cached( aCache, in.readBits( m_nWidth ) ), in );
        else
            pValue = plain( in );
        try
        {
            remember( strKey, *pValue );
        }
        catch ( ... )
        {
            delete pValue;
            throw;
        }
        return pValue;
    }

    bool CSjsonStreamState::encodeDelta( const CJsonNode &reference, const CJsonNode &value, CSjsonBitWriter &out )
    {
        SjsonKind eKind = SjsonCodec::kindOf( &reference );
        if ( eKind != SjsonCodec::kindOf( &value ) )
            return false;
        if ( eKind == SJSON_KIND_FLOAT )
        {
            unsigned long long a = doubleBits( reference.doubleValue() ), b = doubleBits( value.doubleValue() );
            unsigned long long x = a ^ b;
            if ( x == 0 )
                return false;
                // Form 0: XOR of the bit patterns.
            unsigned nLeading = 64 - bitLength( x );
            if ( nLeading > 31 )
                nLeading = 31;
            unsigned nTrailing = 0;
            while ( ! ( ( x >> nTrailing ) & 1 ) )
                nTrailing++;
            unsigned nMeaningful = 64 - nLeading - nTrailing;
            CSjsonBitWriter xorForm;
            xorForm.writeBool( false );
            xorForm.writeBits( nLeading, 5 );
            xorForm.writeBits( nMeaningful - 1, 6 );
            xorForm.writeBits( x >> nTrailing, nMeaningful );
                // Form 1: difference of the bit patterns read as ordered integers.
            bool bNegA = a >= SIGN, bNegB = b >= SIGN;
                // ordinal = bits (non-negative) or -(bits - 2^63) - 1, as two's complement.
            unsigned long long oa = bNegA ? ~( a - SIGN ) : a;
            unsigned long long ob = bNegB ? ~( b - SIGN ) : b;
            long long sa = (long long)oa, sb = (long long)ob;
            bool bNegative = sb < sa;
            unsigned long long ullMagnitude = bNegative ? (unsigned long long)sa - (unsigned long long)sb : (unsigned long long)sb - (unsigned long long)sa;
            unsigned nLength = bitLength( ullMagnitude );
            CSjsonBitWriter diffForm;
            diffForm.writeBool( true );
            diffForm.writeBool( bNegative );
            diffForm.writeBits( nLength - 1, 6 );
            if ( nLength > 1 )
                diffForm.writeBits( ullMagnitude - ( 1ULL << ( nLength - 1 ) ), nLength - 1 );
            out.append( diffForm.bitLength() < xorForm.bitLength() ? diffForm : xorForm );
            return true;
        }
        if ( eKind == SJSON_KIND_INT )
        {
            std::string strDiff = SjsonUtil::decimalSubtract( SjsonCodec::integerText( value ), SjsonCodec::integerText( reference ) );
            unsigned long long ullMagnitude;
            bool bNegative;
            if ( ! SjsonUtil::decimalMagnitude( strDiff, ullMagnitude, bNegative ) || ullMagnitude > CSjsonBitWriter::NYBBLE_MAX )
                return false;
            out.writeBool( bNegative );
            out.writeNybble( ullMagnitude );
            return true;
        }
        if ( eKind == SJSON_KIND_STRING )
        {
            std::string strRef = SjsonCodec::stringText( reference ), strNew = SjsonCodec::stringText( value );
            Timestamp tRef, tNew;
            if ( parseTimestamp( strRef, tRef ) && parseTimestamp( strNew, tNew ) &&
                 tRef.bHasTime == tNew.bHasTime && tRef.nDigits == tNew.nDigits && tRef.strZone == tNew.strZone )
            {
                double dApprox = ( (double)( tNew.lOrdinal - tRef.lOrdinal ) * 86400.0 + (double)( tNew.lSeconds - tRef.lSeconds ) ) * (double)g_alPow10[ tRef.nDigits ]
                                 + (double)( tNew.lFraction - tRef.lFraction );
                if ( fabs( dApprox ) < 8589934592.0 )   // 2^33: exact in long long
                {
                    long long llTicks = ( (long long)( tNew.lOrdinal - tRef.lOrdinal ) * 86400LL + ( tNew.lSeconds - tRef.lSeconds ) ) * g_alPow10[ tRef.nDigits ]
                                        + ( tNew.lFraction - tRef.lFraction );
                    if ( ( llTicks < 0 ? -llTicks : llTicks ) <= (long long)CSjsonBitWriter::NYBBLE_MAX )
                    {
                        out.writeBool( true );
                        writeSigned( llTicks, out );
                        return true;
                    }
                }
            }
            std::vector<unsigned> aRef = SjsonUtil::codePoints( strRef ), aNew = SjsonUtil::codePoints( strNew );
            size_t nShared = 0;
            while ( nShared < aRef.size() && nShared < aNew.size() && aRef[ nShared ] == aNew[ nShared ] )
                nShared++;
            if ( nShared < 4 )
                return false;
            out.writeBool( false );
            out.writeNybble( nShared );
            SjsonCodec::encodeString( SjsonUtil::utf8( std::vector<unsigned>( aNew.begin() + (long)nShared, aNew.end() ) ), NULL, out );
            return true;
        }
        return false;
    }

    CJsonNode *CSjsonStreamState::decodeDelta( const CJsonNode &reference, CSjsonBitReader &in )
    {
        SjsonKind eKind = SjsonCodec::kindOf( &reference );
        if ( eKind == SJSON_KIND_FLOAT )
        {
            unsigned long long a = doubleBits( reference.doubleValue() );
            if ( in.readBool() )
            {
                bool bNegative = in.readBool();
                unsigned nLength = (unsigned)in.readBits( 6 ) + 1;
                unsigned long long ullMagnitude = ( 1ULL << ( nLength - 1 ) ) + ( nLength > 1 ? in.readBits( nLength - 1 ) : 0 );
                bool bNegA = a >= SIGN;
                long long sa = (long long)( bNegA ? ~( a - SIGN ) : a );
                    // The result must stay within [-2^63, 2^63 - 1].
                if ( bNegative ? ( sa < 0 && ullMagnitude > (unsigned long long)sa - (unsigned long long)LLONG_MIN )
                                    || ( sa >= 0 && ullMagnitude > (unsigned long long)sa + 9223372036854775808ULL )
                               : ( sa >= 0 && ullMagnitude > 9223372036854775807ULL - (unsigned long long)sa )
                                    || ( sa < 0 && ullMagnitude > 9223372036854775808ULL + (unsigned long long)( -( sa + 1 ) ) ) )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid float delta" );
                long long sb = (long long)( bNegative ? (unsigned long long)sa - ullMagnitude : (unsigned long long)sa + ullMagnitude );
                unsigned long long b = sb >= 0 ? (unsigned long long)sb : ~(unsigned long long)sb + SIGN;
                return new CNumberNode( bitsDouble( b ) );
            }
            unsigned nLeading = (unsigned)in.readBits( 5 );
            unsigned nMeaningful = (unsigned)in.readBits( 6 ) + 1;
            if ( nLeading + nMeaningful > 64 )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid float delta" );
            unsigned long long x = in.readBits( nMeaningful ) << ( 64 - nLeading - nMeaningful );
            return new CNumberNode( bitsDouble( a ^ x ) );
        }
        if ( eKind == SJSON_KIND_INT )
        {
            long long llDelta = readSigned( in );
            return SjsonCodec::makeInteger( SjsonUtil::decimalAdd( SjsonCodec::integerText( reference ), std::to_string( llDelta ) ) );
        }
        if ( eKind == SJSON_KIND_STRING )
        {
            std::string strRef = SjsonCodec::stringText( reference );
            if ( ! in.readBool() )
            {
                unsigned long long nShared = in.readNybble();
                std::vector<unsigned> aRef = SjsonUtil::codePoints( strRef );
                if ( nShared > aRef.size() )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid string delta" );
                std::string strOut = SjsonUtil::utf8( std::vector<unsigned>( aRef.begin(), aRef.begin() + (long)nShared ) );
                strOut += SjsonCodec::decodeString( in, NULL );
                return new CTextNode( strOut );
            }
            Timestamp t;
            if ( ! parseTimestamp( strRef, t ) )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "timestamp delta against a non-timestamp" );
            long long llDelta = readSigned( in );
            long long llScale = g_alPow10[ t.nDigits ];
            long long llFraction = t.lFraction + llDelta;
            long long llCarry = llFraction >= 0 ? llFraction / llScale : -( ( -llFraction + llScale - 1 ) / llScale );
            llFraction -= llCarry * llScale;
            long long llSeconds = t.lSeconds + llCarry;
            long long llDays = llSeconds >= 0 ? llSeconds / 86400 : -( ( -llSeconds + 86399 ) / 86400 );
            llSeconds -= llDays * 86400;
            t.lOrdinal += (long)llDays;
            t.lSeconds = (long)llSeconds;
            t.lFraction = (long)llFraction;
            std::string strOut;
            if ( t.lOrdinal < 1 || ! formatTimestamp( t, strOut ) )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid timestamp delta" );
            return new CTextNode( strOut );
        }
        throw CSjsonException( SJSON_ERROR_MALFORMED, "delta against a value that has no delta form" );
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
