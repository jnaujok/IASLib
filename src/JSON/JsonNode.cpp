/**
 * JSON Node Class
 *
 *  The behaviour shared by every JSON node: navigation (path, at, the find
 * methods), and the writing helpers (strings, numbers, base64).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_JSON_SUPPORT__

#include "JsonNode.h"
#include "ObjectNode.h"
#include "MissingNode.h"

#include <climits>
#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace IASLib
{
    IMPLEMENT_OBJECT( CJsonNode, CObject );

    short CJsonNode::shortValue( void ) const
    {
        int nValue = intValue();
        if ( nValue > SHRT_MAX )
            return SHRT_MAX;
        if ( nValue < SHRT_MIN )
            return SHRT_MIN;
        return (short)nValue;
    }

    const std::string &CJsonNode::textString( void ) const
    {
        static const std::string *pEmpty = new std::string();
        return *pEmpty;
    }

    bool CJsonNode::binaryValue( std::vector<unsigned char> &aData ) const
    {
        if ( ! isTextual() )
            return false;
        return decodeBase64( textString(), aData );
    }

    CJsonNode &CJsonNode::path( int nIndex ) const
    {
        const CJsonNode *pNode = get( nIndex );
        return pNode ? *const_cast<CJsonNode *>( pNode ) : CMissingNode::getInstance();
    }

    CJsonNode &CJsonNode::path( const char *pszField ) const
    {
        const CJsonNode *pNode = get( pszField );
        return pNode ? *const_cast<CJsonNode *>( pNode ) : CMissingNode::getInstance();
    }

    CJsonNode &CJsonNode::path( const std::string &strField ) const
    {
        const CJsonNode *pNode = get( strField );
        return pNode ? *const_cast<CJsonNode *>( pNode ) : CMissingNode::getInstance();
    }

    bool CJsonNode::hasNonNull( int nIndex ) const
    {
        const CJsonNode *pNode = get( nIndex );
        return pNode != NULL && ! pNode->isNull();
    }

    bool CJsonNode::hasNonNull( const char *pszField ) const
    {
        const CJsonNode *pNode = get( pszField );
        return pNode != NULL && ! pNode->isNull();
    }

    bool CJsonNode::hasNonNull( const std::string &strField ) const
    {
        const CJsonNode *pNode = get( strField );
        return pNode != NULL && ! pNode->isNull();
    }

        // RFC 6901: "" is the whole document; otherwise "/"-separated reference
        // tokens, with "~1" for "/" and "~0" for "~". An array index is digits
        // without leading zeros ("-", the element after the last, never exists).
    CJsonNode &CJsonNode::at( const char *pszPointer ) const
    {
        CJsonNode *pNode = const_cast<CJsonNode *>( this );
        if ( pszPointer == NULL )
            return CMissingNode::getInstance();
        const char *p = pszPointer;
        if ( *p == '\0' )
            return *pNode;
        if ( *p != '/' )
            return CMissingNode::getInstance();

        while ( *p == '/' )
        {
            p++;
            std::string strToken;
            while ( *p != '\0' && *p != '/' )
            {
                if ( *p == '~' )
                {
                    if ( p[1] == '0' )
                        strToken += '~';
                    else if ( p[1] == '1' )
                        strToken += '/';
                    else
                        return CMissingNode::getInstance();
                    p += 2;
                }
                else
                {
                    strToken += *p++;
                }
            }

            CJsonNode *pNext = NULL;
            if ( pNode->isObject() )
            {
                pNext = pNode->field( strToken );
            }
            else if ( pNode->isArray() )
            {
                bool bDigits = ! strToken.empty() && strToken.size() <= 9 && ( strToken[0] != '0' || strToken.size() == 1 );
                for ( size_t n = 0; bDigits && n < strToken.size(); n++ )
                    bDigits = ( strToken[ n ] >= '0' && strToken[ n ] <= '9' );
                if ( bDigits )
                    pNext = pNode->element( atoi( strToken.c_str() ) );
            }
            if ( pNext == NULL )
                return CMissingNode::getInstance();
            pNode = pNext;
        }
        return *pNode;
    }

    bool CJsonNode::collect( const std::string &strField, bool bParents, bool bFirst, std::vector<CJsonNode *> &aFound ) const
    {
        (void)strField; (void)bParents; (void)bFirst; (void)aFound;
        return false;
    }

    CJsonNode *CJsonNode::findValue( const char *pszField ) const
    {
        std::vector<CJsonNode *> aFound;
        collect( pszField ? pszField : "", false, true, aFound );
        return aFound.empty() ? NULL : aFound[0];
    }

    CJsonNode &CJsonNode::findPath( const char *pszField ) const
    {
        CJsonNode *pFound = findValue( pszField );
        return pFound ? *pFound : CMissingNode::getInstance();
    }

    std::vector<CJsonNode *> CJsonNode::findValues( const char *pszField ) const
    {
        std::vector<CJsonNode *> aFound;
        collect( pszField ? pszField : "", false, false, aFound );
        return aFound;
    }

    CStringArray CJsonNode::findValuesAsText( const char *pszField ) const
    {
        std::vector<CJsonNode *> aFound = findValues( pszField );
        CStringArray aText;
        for ( size_t n = 0; n < aFound.size(); n++ )
            aText.Push( aFound[ n ]->asText() );
        return aText;
    }

    CObjectNode *CJsonNode::findParent( const char *pszField ) const
    {
        std::vector<CJsonNode *> aFound;
        collect( pszField ? pszField : "", true, true, aFound );
        return aFound.empty() ? NULL : static_cast<CObjectNode *>( aFound[0] );
    }

    std::vector<CObjectNode *> CJsonNode::findParents( const char *pszField ) const
    {
        std::vector<CJsonNode *> aFound;
        collect( pszField ? pszField : "", true, false, aFound );
        std::vector<CObjectNode *> aParents;
        for ( size_t n = 0; n < aFound.size(); n++ )
            aParents.push_back( static_cast<CObjectNode *>( aFound[ n ] ) );
        return aParents;
    }

    CString CJsonNode::toString( void ) const
    {
        std::string strJson = toJson( -1, false );
        return CString( strJson.c_str(), strJson.size() );
    }

    CString CJsonNode::toPrettyString( void ) const
    {
        std::string strJson = toJson( 2, false );
        return CString( strJson.c_str(), strJson.size() );
    }

    std::string CJsonNode::toJson( int nIndent, bool bAsciiOnly ) const
    {
        std::string strOut;
        writeJson( strOut, nIndent, 0, bAsciiOnly );
        return strOut;
    }

    void CJsonNode::newLine( std::string &strOut, int nIndent, int nLevel )
    {
        if ( nIndent < 0 )
            return;
        strOut += '\n';
        strOut.append( (size_t)nIndent * (size_t)nLevel, ' ' );
    }

    namespace
    {
            // The length of the valid UTF-8 sequence at p (RFC 3629: no overlong
            // forms, surrogates or values past U+10FFFF), or 0 if it is not one.
        size_t utf8Length( const unsigned char *p, const unsigned char *pEnd, unsigned int &nCode )
        {
            unsigned char c = p[0];
            size_t nLength;
            unsigned char lo = 0x80, hi = 0xBF;
            if ( c < 0x80 )
            {
                nCode = c;
                return 1;
            }
            else if ( c >= 0xC2 && c <= 0xDF ) { nLength = 2; nCode = c & 0x1F; }
            else if ( c == 0xE0 ) { nLength = 3; nCode = c & 0x0F; lo = 0xA0; }
            else if ( c >= 0xE1 && c <= 0xEC ) { nLength = 3; nCode = c & 0x0F; }
            else if ( c == 0xED ) { nLength = 3; nCode = c & 0x0F; hi = 0x9F; }
            else if ( c >= 0xEE && c <= 0xEF ) { nLength = 3; nCode = c & 0x0F; }
            else if ( c == 0xF0 ) { nLength = 4; nCode = c & 0x07; lo = 0x90; }
            else if ( c >= 0xF1 && c <= 0xF3 ) { nLength = 4; nCode = c & 0x07; }
            else if ( c == 0xF4 ) { nLength = 4; nCode = c & 0x07; hi = 0x8F; }
            else
                return 0;
            if ( (size_t)( pEnd - p ) < nLength )
                return 0;
            for ( size_t n = 1; n < nLength; n++ )
            {
                unsigned char b = p[ n ];
                if ( b < ( n == 1 ? lo : 0x80 ) || b > ( n == 1 ? hi : 0xBF ) )
                    return 0;
                nCode = ( nCode << 6 ) | ( b & 0x3F );
            }
            return nLength;
        }

        void appendEscape( std::string &strOut, unsigned int nUnit )
        {
            char achBuffer[ 8 ];
            snprintf( achBuffer, sizeof( achBuffer ), "\\u%04x", nUnit );
            strOut += achBuffer;
        }
    }

    void CJsonNode::writeString( std::string &strOut, const std::string &strText, bool bAsciiOnly )
    {
        strOut += '"';
        const unsigned char *p = (const unsigned char *)strText.data();
        const unsigned char *pEnd = p + strText.size();
        while ( p < pEnd )
        {
            unsigned char c = *p;
            if ( c >= 0x20 && c < 0x80 && c != '"' && c != '\\' )
            {
                    // A run of plain characters.
                const unsigned char *pStart = p;
                while ( p < pEnd && *p >= 0x20 && *p < 0x80 && *p != '"' && *p != '\\' )
                    p++;
                strOut.append( (const char *)pStart, p - pStart );
                continue;
            }
            switch ( c )
            {
                case '"':  strOut += "\\\""; p++; continue;
                case '\\': strOut += "\\\\"; p++; continue;
                case '\b': strOut += "\\b"; p++; continue;
                case '\f': strOut += "\\f"; p++; continue;
                case '\n': strOut += "\\n"; p++; continue;
                case '\r': strOut += "\\r"; p++; continue;
                case '\t': strOut += "\\t"; p++; continue;
                default: break;
            }
            if ( c < 0x20 )
            {
                appendEscape( strOut, c );
                p++;
                continue;
            }

            unsigned int nCode = 0;
            size_t nLength = utf8Length( p, pEnd, nCode );
            if ( nLength == 0 )
            {
                    // Not UTF-8: one byte becomes U+FFFD.
                if ( bAsciiOnly )
                    strOut += "\\ufffd";
                else
                    strOut += "\xEF\xBF\xBD";
                p++;
                continue;
            }
            if ( bAsciiOnly )
            {
                if ( nCode >= 0x10000 )
                {
                    nCode -= 0x10000;
                    appendEscape( strOut, 0xD800 + ( nCode >> 10 ) );
                    appendEscape( strOut, 0xDC00 + ( nCode & 0x3FF ) );
                }
                else
                {
                    appendEscape( strOut, nCode );
                }
            }
            else
            {
                strOut.append( (const char *)p, nLength );
            }
            p += nLength;
        }
        strOut += '"';
    }

        // The shortest text that reads back as the same double, always with a
        // fraction or exponent so it stays a floating point number. JSON has no
        // infinities or NaN; they are written as null. Only the digits and
        // exponent are taken from the C library, whose decimal point follows
        // the locale, so the result does not depend on it.
    std::string CJsonNode::formatDouble( double dValue )
    {
        if ( dValue != dValue || dValue > 1.7976931348623157e308 || dValue < -1.7976931348623157e308 )
            return "null";

            // The fewest significant digits that read back the same.
        char achBuffer[ 40 ];
        for ( int nPrecision = 1; nPrecision <= 17; nPrecision++ )
        {
            snprintf( achBuffer, sizeof( achBuffer ), "%.*e", nPrecision - 1, dValue );
            if ( strtod( achBuffer, NULL ) == dValue )
                break;
        }

            // Split "-d.ddde+XX" into sign, digits and exponent.
        std::string strExp( achBuffer );
        bool bNegative = ( strExp[0] == '-' );
        size_t nE = strExp.find_first_of( "eE" );
        std::string strDigits;
        for ( size_t n = bNegative ? 1 : 0; n < nE; n++ )
            if ( strExp[ n ] >= '0' && strExp[ n ] <= '9' )
                strDigits += strExp[ n ];
        int nExponent = atoi( strExp.c_str() + nE + 1 );
        while ( strDigits.size() > 1 && strDigits[ strDigits.size() - 1 ] == '0' )
            strDigits.erase( strDigits.size() - 1 );

            // Plain notation for exponents from -7 to 20, as JavaScript does;
            // otherwise d.ddde[+-]XX.
        std::string strText = bNegative ? "-" : "";
        int nDigits = (int)strDigits.size();
        if ( nExponent >= -7 && nExponent < 21 )
        {
            if ( nExponent < 0 )
            {
                strText += "0.";
                strText.append( (size_t)( -nExponent - 1 ), '0' );
                strText += strDigits;
            }
            else if ( nExponent + 1 >= nDigits )
            {
                strText += strDigits;
                strText.append( (size_t)( nExponent + 1 - nDigits ), '0' );
                strText += ".0";
            }
            else
            {
                strText += strDigits.substr( 0, nExponent + 1 );
                strText += '.';
                strText += strDigits.substr( nExponent + 1 );
            }
        }
        else
        {
            strText += strDigits[0];
            if ( nDigits > 1 )
            {
                strText += '.';
                strText += strDigits.substr( 1 );
            }
            strText += ( nExponent < 0 ) ? "e-" : "e+";
            strText += std::to_string( nExponent < 0 ? -nExponent : nExponent );
        }
        return strText;
    }

    namespace
    {
        const char *BASE64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        int base64Value( unsigned char c )
        {
            if ( c >= 'A' && c <= 'Z' ) return c - 'A';
            if ( c >= 'a' && c <= 'z' ) return c - 'a' + 26;
            if ( c >= '0' && c <= '9' ) return c - '0' + 52;
            if ( c == '+' || c == '-' ) return 62;     // standard or URL-safe
            if ( c == '/' || c == '_' ) return 63;
            return -1;
        }
    }

    size_t CJsonNode::decodeUtf8( const unsigned char *p, const unsigned char *pEnd, unsigned int &nCode )
    {
        if ( p >= pEnd )
            return 0;
        return utf8Length( p, pEnd, nCode );
    }

    bool CJsonNode::parseDouble( const std::string &strText, double &dValue )
    {
        if ( strText.empty() )
            return false;
        std::string strLocal( strText );
        const char *pszPoint = localeconv()->decimal_point;
        if ( pszPoint && *pszPoint && *pszPoint != '.' )
        {
            if ( strLocal.find( *pszPoint ) != std::string::npos )
                return false;       // not ours to read
            for ( size_t n = 0; n < strLocal.size(); n++ )
                if ( strLocal[ n ] == '.' )
                    strLocal[ n ] = *pszPoint;
        }
        char *pszEnd = NULL;
        dValue = strtod( strLocal.c_str(), &pszEnd );
        return pszEnd != NULL && *pszEnd == '\0' && pszEnd != strLocal.c_str();
    }

    std::string CJsonNode::encodeBase64( const std::vector<unsigned char> &aData )
    {
        std::string strOut;
        strOut.reserve( ( aData.size() + 2 ) / 3 * 4 );
        size_t n = 0;
        for ( ; n + 2 < aData.size(); n += 3 )
        {
            unsigned long v = ( (unsigned long)aData[ n ] << 16 ) | ( (unsigned long)aData[ n + 1 ] << 8 ) | aData[ n + 2 ];
            strOut += BASE64[ ( v >> 18 ) & 63 ];
            strOut += BASE64[ ( v >> 12 ) & 63 ];
            strOut += BASE64[ ( v >> 6 ) & 63 ];
            strOut += BASE64[ v & 63 ];
        }
        if ( n < aData.size() )
        {
            unsigned long v = (unsigned long)aData[ n ] << 16;
            if ( n + 1 < aData.size() )
                v |= (unsigned long)aData[ n + 1 ] << 8;
            strOut += BASE64[ ( v >> 18 ) & 63 ];
            strOut += BASE64[ ( v >> 12 ) & 63 ];
            strOut += ( n + 1 < aData.size() ) ? BASE64[ ( v >> 6 ) & 63 ] : '=';
            strOut += '=';
        }
        return strOut;
    }

        // Standard or URL-safe base64, with or without padding. False (and no
        // data) for anything else, including white space.
    bool CJsonNode::decodeBase64( const std::string &strText, std::vector<unsigned char> &aData )
    {
        aData.clear();
        size_t nLength = strText.size();
        if ( nLength > 0 && strText[ nLength - 1 ] == '=' )
        {
            nLength--;
            if ( nLength > 0 && strText[ nLength - 1 ] == '=' )
                nLength--;
            if ( strText.size() % 4 != 0 )
                return false;
        }
        if ( nLength % 4 == 1 )
            return false;

        unsigned long v = 0;
        int nBits = 0;
        for ( size_t n = 0; n < nLength; n++ )
        {
            int nValue = base64Value( (unsigned char)strText[ n ] );
            if ( nValue < 0 )
            {
                aData.clear();
                return false;
            }
            v = ( ( v << 6 ) | (unsigned long)nValue ) & 0xFFFFFF;
            nBits += 6;
            if ( nBits >= 8 )
            {
                nBits -= 8;
                aData.push_back( (unsigned char)( ( v >> nBits ) & 0xFF ) );
            }
        }
            // The bits left over must be zero (canonical encoding).
        if ( nBits > 0 && ( v & ( ( 1UL << nBits ) - 1 ) ) != 0 )
        {
            aData.clear();
            return false;
        }
        return true;
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
