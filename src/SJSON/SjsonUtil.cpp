/**
 * SJSON helpers. See SjsonUtil.h.
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

#include "SjsonUtil.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <cstring>

#ifdef IASLIB_SJSON_LZ4__
#include <lz4.h>
#endif

namespace IASLib
{
    namespace SjsonUtil
    {
        std::vector<unsigned> codePoints( const std::string &strUtf8 )
        {
            std::vector<unsigned> aOut;
            aOut.reserve( strUtf8.size() );
            const unsigned char *p = (const unsigned char *)strUtf8.data();
            size_t nLength = strUtf8.size();
            for ( size_t n = 0; n < nLength; )
            {
                unsigned char c = p[ n ];
                unsigned nCode = c;
                int nExtra = 0;
                if ( c >= 0xF0 && c < 0xF8 ) { nCode = c & 0x07; nExtra = 3; }
                else if ( c >= 0xE0 && c < 0xF0 ) { nCode = c & 0x0F; nExtra = 2; }
                else if ( c >= 0xC0 && c < 0xE0 ) { nCode = c & 0x1F; nExtra = 1; }
                bool bValid = n + nExtra < nLength;
                for ( int k = 1; bValid && k <= nExtra; k++ )
                    bValid = n + k < nLength && ( p[ n + k ] & 0xC0 ) == 0x80;
                if ( c >= 0x80 && ( nExtra == 0 || ! bValid ) )
                {
                    aOut.push_back( c );        // not UTF-8: take the byte
                    n++;
                    continue;
                }
                for ( int k = 1; k <= nExtra; k++ )
                    nCode = ( nCode << 6 ) | ( p[ n + k ] & 0x3F );
                aOut.push_back( nCode );
                n += nExtra + 1;
            }
            return aOut;
        }

        void appendUtf8( std::string &strOut, unsigned nCode )
        {
            if ( nCode < 0x80 )
                strOut += (char)nCode;
            else if ( nCode < 0x800 )
            {
                strOut += (char)( 0xC0 | ( nCode >> 6 ) );
                strOut += (char)( 0x80 | ( nCode & 0x3F ) );
            }
            else if ( nCode < 0x10000 )
            {
                strOut += (char)( 0xE0 | ( nCode >> 12 ) );
                strOut += (char)( 0x80 | ( ( nCode >> 6 ) & 0x3F ) );
                strOut += (char)( 0x80 | ( nCode & 0x3F ) );
            }
            else
            {
                strOut += (char)( 0xF0 | ( nCode >> 18 ) );
                strOut += (char)( 0x80 | ( ( nCode >> 12 ) & 0x3F ) );
                strOut += (char)( 0x80 | ( ( nCode >> 6 ) & 0x3F ) );
                strOut += (char)( 0x80 | ( nCode & 0x3F ) );
            }
        }

        std::string utf8( const std::vector<unsigned> &aCodePoints )
        {
            std::string strOut;
            for ( size_t n = 0; n < aCodePoints.size(); n++ )
                appendUtf8( strOut, aCodePoints[ n ] );
            return strOut;
        }

        unsigned crc32( const unsigned char *pData, size_t nLength )
        {
                // Built once, thread-safely (C++11 static initialization).
            struct Table
            {
                unsigned a[ 256 ];
                Table( void )
                {
                    for ( unsigned n = 0; n < 256; n++ )
                    {
                        unsigned c = n;
                        for ( int k = 0; k < 8; k++ )
                            c = ( c & 1 ) ? 0xEDB88320U ^ ( c >> 1 ) : c >> 1;
                        a[ n ] = c;
                    }
                }
            };
            static const Table s_table;
            unsigned nCrc = 0xFFFFFFFFU;
            for ( size_t n = 0; n < nLength; n++ )
                nCrc = s_table.a[ ( nCrc ^ pData[ n ] ) & 0xFF ] ^ ( nCrc >> 8 );
            return nCrc ^ 0xFFFFFFFFU;
        }

        static const char g_szBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string base64Encode( const std::string &strData )
        {
            std::string strOut;
            const unsigned char *p = (const unsigned char *)strData.data();
            size_t n = 0, nLength = strData.size();
            for ( ; n + 2 < nLength; n += 3 )
            {
                unsigned v = ( p[ n ] << 16 ) | ( p[ n + 1 ] << 8 ) | p[ n + 2 ];
                strOut += g_szBase64[ v >> 18 ];
                strOut += g_szBase64[ ( v >> 12 ) & 63 ];
                strOut += g_szBase64[ ( v >> 6 ) & 63 ];
                strOut += g_szBase64[ v & 63 ];
            }
            if ( nLength - n == 1 )
            {
                unsigned v = p[ n ] << 16;
                strOut += g_szBase64[ v >> 18 ];
                strOut += g_szBase64[ ( v >> 12 ) & 63 ];
                strOut += "==";
            }
            else if ( nLength - n == 2 )
            {
                unsigned v = ( p[ n ] << 16 ) | ( p[ n + 1 ] << 8 );
                strOut += g_szBase64[ v >> 18 ];
                strOut += g_szBase64[ ( v >> 12 ) & 63 ];
                strOut += g_szBase64[ ( v >> 6 ) & 63 ];
                strOut += '=';
            }
            return strOut;
        }

        bool base64Decode( const std::string &strText, std::string &strData )
        {
            strData.clear();
            if ( strText.size() % 4 != 0 )
                return false;
            unsigned nBuffer = 0;
            int nBits = 0;
            size_t nPad = 0;
            for ( size_t n = 0; n < strText.size(); n++ )
            {
                char c = strText[ n ];
                if ( c == '=' )
                {
                    nPad++;
                    continue;
                }
                if ( nPad )
                    return false;
                const char *pFound = c ? strchr( g_szBase64, c ) : NULL;
                if ( pFound == NULL )
                    return false;
                nBuffer = ( nBuffer << 6 ) | (unsigned)( pFound - g_szBase64 );
                nBits += 6;
                if ( nBits >= 8 )
                {
                    nBits -= 8;
                    strData += (char)( ( nBuffer >> nBits ) & 0xFF );
                }
            }
            return nPad <= 2;
        }

        double parseDouble( const std::string &strText, size_t *pnUsed )
        {
            std::string strLocal( strText );
            const char *pszPoint = localeconv()->decimal_point;
            if ( pszPoint && pszPoint[ 0 ] && pszPoint[ 0 ] != '.' && pszPoint[ 1 ] == '\0' )
                for ( size_t n = 0; n < strLocal.size(); n++ )
                    if ( strLocal[ n ] == '.' )
                        strLocal[ n ] = pszPoint[ 0 ];
            char *pEnd = NULL;
            double dValue = strtod( strLocal.c_str(), &pEnd );
            if ( pnUsed )
                *pnUsed = (size_t)( pEnd - strLocal.c_str() );
            return dValue;
        }

        std::string floatRepr( double dValue )
        {
            if ( std::isnan( dValue ) )
                return "nan";
            if ( std::isinf( dValue ) )
                return dValue > 0 ? "inf" : "-inf";
            if ( dValue == 0.0 )
                return std::signbit( dValue ) ? "-0.0" : "0.0";
            char szBuffer[ 64 ];
            for ( int nPrecision = 1; nPrecision <= 17; nPrecision++ )
            {
                    // snprintf and strtod agree on the C locale's decimal point,
                    // whatever it is; only the digits are taken below.
                snprintf( szBuffer, sizeof( szBuffer ), "%.*e", nPrecision - 1, dValue );
                if ( strtod( szBuffer, NULL ) == dValue )
                    break;
            }
                // szBuffer is [-]d[.ddd]e[+-]xx: take the digits and exponent.
            std::string strText( szBuffer );
            bool bNegative = strText[ 0 ] == '-';
            size_t nE = strText.find( 'e' );
            std::string strDigits;
            for ( size_t n = bNegative ? 1 : 0; n < nE; n++ )
                if ( strText[ n ] >= '0' && strText[ n ] <= '9' )     // skips the decimal point, '.' or ','
                    strDigits += strText[ n ];
            while ( strDigits.size() > 1 && strDigits[ strDigits.size() - 1 ] == '0' )
                strDigits.erase( strDigits.size() - 1 );
            int nExponent = atoi( strText.c_str() + nE + 1 );
            int nPoint = nExponent + 1;     // digits are 0.ddd x 10^nPoint
            std::string strOut = bNegative ? "-" : "";
            if ( nPoint > -4 && nPoint <= 16 )
            {
                if ( nPoint <= 0 )
                    strOut += "0." + std::string( (size_t)( -nPoint ), '0' ) + strDigits;
                else if ( (size_t)nPoint >= strDigits.size() )
                    strOut += strDigits + std::string( (size_t)nPoint - strDigits.size(), '0' ) + ".0";
                else
                    strOut += strDigits.substr( 0, (size_t)nPoint ) + "." + strDigits.substr( (size_t)nPoint );
            }
            else
            {
                strOut += strDigits.substr( 0, 1 );
                if ( strDigits.size() > 1 )
                    strOut += "." + strDigits.substr( 1 );
                char szExp[ 16 ];
                snprintf( szExp, sizeof( szExp ), "e%c%02d", nExponent < 0 ? '-' : '+', nExponent < 0 ? -nExponent : nExponent );
                strOut += szExp;
            }
            return strOut;
        }

        bool lz4Decompress( const std::string &strBlock, size_t nOutLength, std::string &strOut )
        {
            strOut.clear();
            strOut.reserve( nOutLength );
            const unsigned char *p = (const unsigned char *)strBlock.data();
            size_t nIn = strBlock.size(), n = 0;
            while ( n < nIn )
            {
                unsigned nToken = p[ n++ ];
                size_t nLiteral = nToken >> 4;
                if ( nLiteral == 15 )
                {
                    unsigned char c;
                    do
                    {
                        if ( n >= nIn ) return false;
                        c = p[ n++ ];
                        nLiteral += c;
                    } while ( c == 255 );
                }
                if ( n + nLiteral > nIn || strOut.size() + nLiteral > nOutLength )
                    return false;
                strOut.append( (const char *)p + n, nLiteral );
                n += nLiteral;
                if ( n >= nIn )
                    break;              // the last sequence has no match
                if ( n + 2 > nIn )
                    return false;
                size_t nOffset = p[ n ] | ( p[ n + 1 ] << 8 );
                n += 2;
                size_t nMatch = nToken & 15;
                if ( nMatch == 15 )
                {
                    unsigned char c;
                    do
                    {
                        if ( n >= nIn ) return false;
                        c = p[ n++ ];
                        nMatch += c;
                    } while ( c == 255 );
                }
                nMatch += 4;
                if ( nOffset == 0 || nOffset > strOut.size() || strOut.size() + nMatch > nOutLength )
                    return false;
                size_t nStart = strOut.size() - nOffset;
                for ( size_t k = 0; k < nMatch; k++ )
                    strOut += strOut[ nStart + k ];
            }
            return strOut.size() == nOutLength;
        }

        bool lz4Available( void )
        {
#ifdef IASLIB_SJSON_LZ4__
            return true;
#else
            return false;
#endif
        }

        bool lz4Compress( const std::string &strData, std::string &strBlock )
        {
#ifdef IASLIB_SJSON_LZ4__
            int nBound = LZ4_compressBound( (int)strData.size() );
            strBlock.resize( (size_t)nBound );
            int nSize = LZ4_compress_default( strData.data(), &strBlock[ 0 ], (int)strData.size(), nBound );
            if ( nSize <= 0 )
                return false;
            strBlock.resize( (size_t)nSize );
            return true;
#else
            (void)strData;
            (void)strBlock;
            return false;
#endif
        }

        namespace
        {
                // Magnitudes are digit strings without leading zeros ("0" for zero).
            std::string stripZeros( const std::string &str )
            {
                size_t n = str.find_first_not_of( '0' );
                return n == std::string::npos ? "0" : str.substr( n );
            }

            int compareMagnitude( const std::string &a, const std::string &b )
            {
                if ( a.size() != b.size() )
                    return a.size() < b.size() ? -1 : 1;
                return a < b ? -1 : ( a > b ? 1 : 0 );
            }

            std::string addMagnitude( const std::string &a, const std::string &b )
            {
                std::string strOut;
                int nCarry = 0;
                for ( size_t i = 0; i < a.size() || i < b.size() || nCarry; i++ )
                {
                    int nSum = nCarry;
                    if ( i < a.size() ) nSum += a[ a.size() - 1 - i ] - '0';
                    if ( i < b.size() ) nSum += b[ b.size() - 1 - i ] - '0';
                    strOut += (char)( '0' + nSum % 10 );
                    nCarry = nSum / 10;
                }
                return stripZeros( std::string( strOut.rbegin(), strOut.rend() ) );
            }

                // a - b with a >= b.
            std::string subtractMagnitude( const std::string &a, const std::string &b )
            {
                std::string strOut;
                int nBorrow = 0;
                for ( size_t i = 0; i < a.size(); i++ )
                {
                    int nDigit = a[ a.size() - 1 - i ] - '0' - nBorrow;
                    if ( i < b.size() ) nDigit -= b[ b.size() - 1 - i ] - '0';
                    nBorrow = nDigit < 0 ? 1 : 0;
                    strOut += (char)( '0' + nDigit + 10 * nBorrow );
                }
                return stripZeros( std::string( strOut.rbegin(), strOut.rend() ) );
            }

            void split( const std::string &str, bool &bNegative, std::string &strMagnitude )
            {
                bNegative = ! str.empty() && str[ 0 ] == '-';
                strMagnitude = stripZeros( bNegative ? str.substr( 1 ) : str );
                if ( strMagnitude == "0" )
                    bNegative = false;
            }

            std::string join( bool bNegative, const std::string &strMagnitude )
            {
                return ( bNegative && strMagnitude != "0" ? "-" : "" ) + strMagnitude;
            }
        }

        std::string decimalAdd( const std::string &strA, const std::string &strB )
        {
            bool bNegA, bNegB;
            std::string a, b;
            split( strA, bNegA, a );
            split( strB, bNegB, b );
            if ( bNegA == bNegB )
                return join( bNegA, addMagnitude( a, b ) );
            int nCompare = compareMagnitude( a, b );
            if ( nCompare == 0 )
                return "0";
            return nCompare > 0 ? join( bNegA, subtractMagnitude( a, b ) ) : join( bNegB, subtractMagnitude( b, a ) );
        }

        std::string decimalSubtract( const std::string &strA, const std::string &strB )
        {
            bool bNegB;
            std::string b;
            split( strB, bNegB, b );
            return decimalAdd( strA, join( ! bNegB, b ) );
        }

        std::string decimalFromBits( const std::vector<bool> &aBits )
        {
            std::string strOut = "0";
            for ( size_t n = 0; n < aBits.size(); n++ )
            {
                strOut = addMagnitude( strOut, strOut );
                if ( aBits[ n ] )
                    strOut = addMagnitude( strOut, "1" );
            }
            return strOut;
        }

        bool decimalToBits( const std::string &strDecimal, size_t nCount, std::vector<bool> &aBits )
        {
            bool bNegative;
            std::string strValue;
            split( strDecimal, bNegative, strValue );
            if ( bNegative )
                return false;
            std::vector<bool> aReversed;
            while ( strValue != "0" )
            {
                    // Divide by two.
                std::string strHalf;
                int nRest = 0;
                for ( size_t i = 0; i < strValue.size(); i++ )
                {
                    int nDigit = nRest * 10 + ( strValue[ i ] - '0' );
                    strHalf += (char)( '0' + nDigit / 2 );
                    nRest = nDigit % 2;
                }
                aReversed.push_back( nRest != 0 );
                strValue = stripZeros( strHalf );
            }
            if ( aReversed.size() > nCount )
                return false;
            aBits.assign( nCount, false );
            for ( size_t n = 0; n < aReversed.size(); n++ )
                aBits[ nCount - 1 - n ] = aReversed[ n ];
            return true;
        }

        bool decimalMagnitude( const std::string &strDecimal, unsigned long long &ullMagnitude, bool &bNegative )
        {
            std::string strValue;
            split( strDecimal, bNegative, strValue );
            if ( compareMagnitude( strValue, "18446744073709551615" ) > 0 )
                return false;
            ullMagnitude = strtoull( strValue.c_str(), NULL, 10 );
            return true;
        }

        static bool leapYear( int nYear )
        {
            return ( nYear % 4 == 0 && nYear % 100 != 0 ) || nYear % 400 == 0;
        }

        static int daysInMonth( int nYear, int nMonth )
        {
            static const int s_aDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            return nMonth == 2 && leapYear( nYear ) ? 29 : s_aDays[ nMonth - 1 ];
        }

        bool validDate( int nYear, int nMonth, int nDay )
        {
            return nYear >= 1 && nYear <= 9999 && nMonth >= 1 && nMonth <= 12 && nDay >= 1 && nDay <= daysInMonth( nYear, nMonth );
        }

        long dateOrdinal( int nYear, int nMonth, int nDay )
        {
            long y = nYear - 1;
            long lDays = y * 365 + y / 4 - y / 100 + y / 400;
            for ( int m = 1; m < nMonth; m++ )
                lDays += daysInMonth( nYear, m );
            return lDays + nDay;
        }

        bool dateFromOrdinal( long lOrdinal, int &nYear, int &nMonth, int &nDay )
        {
            if ( lOrdinal < 1 || lOrdinal > dateOrdinal( 9999, 12, 31 ) )
                return false;
                // Estimate the year, then correct it.
            int y = (int)( lOrdinal / 365.2425 ) + 1;
            while ( y > 1 && dateOrdinal( y, 1, 1 ) > lOrdinal )
                y--;
            while ( y < 9999 && dateOrdinal( y + 1, 1, 1 ) <= lOrdinal )
                y++;
            long lDay = lOrdinal - dateOrdinal( y, 1, 1 ) + 1;
            int m = 1;
            while ( lDay > daysInMonth( y, m ) )
            {
                lDay -= daysInMonth( y, m );
                m++;
            }
            nYear = y;
            nMonth = m;
            nDay = (int)lDay;
            return true;
        }
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
