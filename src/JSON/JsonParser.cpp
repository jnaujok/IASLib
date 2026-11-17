/**
 * JSON Parser Class Implementation
 *
 *  A recursive descent parser for JSON (RFC 8259). See JsonParser.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 9/13/2020
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

/**
 * @formatter:off
 *
 * The grammar (RFC 8259, as on json.org):
 *
 *   json       element
 *   element    ws value ws
 *   value      object | array | string | number | "true" | "false" | "null"
 *   object     '{' ws '}' | '{' member ( ',' member )* '}'
 *   member     ws string ws ':' element
 *   array      '[' ws ']' | '[' element ( ',' element )* ']'
 *   string     '"' character* '"'
 *   character  '0020' . '10FFFF' - '"' - '\'  |  '\' escape
 *   escape     '"' | '\' | '/' | 'b' | 'f' | 'n' | 'r' | 't' | 'u' hex hex hex hex
 *   number     '-'? ( '0' | onenine digit* ) ( '.' digit+ )? ( [eE] [+-]? digit+ )?
 *   ws         ( '0020' | '000A' | '000D' | '0009' )*
 *
 * A \u escape of a UTF-16 surrogate must be one half of a pair.
 *
 * @formatter:on
 */

#ifdef IASLIB_JSON_SUPPORT__

#include "JsonParser.h"
#include "ArrayNode.h"
#include "BooleanNode.h"
#include "NullNode.h"
#include "NumberNode.h"
#include "ObjectNode.h"
#include "TextNode.h"

#include <cstdio>
#include <cstring>

namespace IASLib
{
    IMPLEMENT_OBJECT( CJsonParser, CObject );

    CJsonParser::CJsonParser( void )
        : m_bAllowComments( false ), m_bAllowTrailingCommas( false ), m_bRejectDuplicateKeys( false ),
          m_nMaxDepth( DEFAULT_MAX_DEPTH ), m_nErrorLine( 0 ), m_nErrorColumn( 0 ), m_nErrorOffset( 0 ),
          m_pszText( NULL ), m_nLength( 0 ), m_nPos( 0 ), m_bFailed( false )
    {
    }

    CJsonNode *CJsonParser::parse( const char *pszText )
    {
        CJsonParser parser;
        return parser.readTree( pszText );
    }

    CJsonNode *CJsonParser::parse( const std::string &strText )
    {
        CJsonParser parser;
        return parser.readTree( strText );
    }

    CJsonNode *CJsonParser::parse( CStream *pStream )
    {
        CJsonParser parser;
        return parser.readTree( pStream );
    }

    CJsonNode *CJsonParser::readTree( const char *pszText )
    {
        return readTree( pszText ? pszText : "", pszText ? strlen( pszText ) : 0 );
    }

    CJsonNode *CJsonParser::readTree( const std::string &strText )
    {
        return readTree( strText.data(), strText.size() );
    }

    CJsonNode *CJsonParser::readTree( CStream *pStream )
    {
        std::string strText;
        if ( pStream == NULL || ! pStream->IsOpen() )
        {
            readTree( "", 0 );      // resets the state (and fails: no value)
            m_bFailed = false;
            fail( "the stream is not open", 0 );
            return NULL;
        }
        char achBuffer[ 8192 ];
        while ( ! pStream->IsEOS() )
        {
            int nRead = pStream->GetBuffer( achBuffer, (int)sizeof( achBuffer ) );
            if ( nRead <= 0 )
                break;
            strText.append( achBuffer, (size_t)nRead );
        }
        return readTree( strText );
    }

    CJsonNode *CJsonParser::readTree( const char *pszText, size_t nLength )
    {
        m_pszText = pszText ? pszText : "";
        m_nLength = pszText ? nLength : 0;
        m_nPos = 0;
        m_bFailed = false;
        m_strError = "";
        m_strErrorMessage = "";
        m_nErrorLine = m_nErrorColumn = m_nErrorOffset = 0;

            // A UTF-8 byte order mark may be ignored (RFC 8259 section 8.1).
        if ( m_nLength >= 3 && memcmp( m_pszText, "\xEF\xBB\xBF", 3 ) == 0 )
            m_nPos = 3;

        if ( ! skipWhitespace() )
            return NULL;
        if ( m_nPos >= m_nLength )
        {
            fail( "there is no JSON value", m_nPos );
            return NULL;
        }
        CJsonNode *pRoot = parseValue( 1 );
        if ( pRoot == NULL )
            return NULL;
        if ( skipWhitespace() && m_nPos < m_nLength )
            fail( "unexpected " + describeNext() + " after the JSON value", m_nPos );
        if ( m_bFailed )
        {
            delete pRoot;
            return NULL;
        }
        return pRoot;
    }

        // Records the first error, with its line and column (counting
        // characters, not bytes). Always returns false.
    bool CJsonParser::fail( const char *pszMessage, size_t nOffset )
    {
        if ( m_bFailed )
            return false;
        m_bFailed = true;
        if ( nOffset > m_nLength )
            nOffset = m_nLength;
        size_t nLine = 1;
        size_t nColumn = 1;
        for ( size_t n = 0; n < nOffset; n++ )
        {
            unsigned char c = (unsigned char)m_pszText[ n ];
            if ( c == '\n' )
            {
                nLine++;
                nColumn = 1;
            }
            else if ( ( c & 0xC0 ) != 0x80 )        // not a UTF-8 continuation byte
            {
                nColumn++;
            }
        }
        m_nErrorOffset = nOffset;
        m_nErrorLine = nLine;
        m_nErrorColumn = nColumn;
        m_strErrorMessage = pszMessage;
        char achPrefix[ 64 ];
        snprintf( achPrefix, sizeof( achPrefix ), "line %lu, column %lu: ", (unsigned long)nLine, (unsigned long)nColumn );
        m_strError = achPrefix;
        m_strError += pszMessage;
        return false;
    }

        // The next character, for error messages: "'x'", "end of input", or a
        // code for a control or non-ASCII byte.
    std::string CJsonParser::describeNext( void ) const
    {
        int c = peek();
        if ( c < 0 )
            return "end of input";
        char achBuffer[ 32 ];
        if ( c >= 0x20 && c < 0x7F )
            snprintf( achBuffer, sizeof( achBuffer ), "'%c'", c );
        else
            snprintf( achBuffer, sizeof( achBuffer ), "byte 0x%02X", c );
        return achBuffer;
    }

    bool CJsonParser::skipWhitespace( void )
    {
        while ( m_nPos < m_nLength )
        {
            char c = m_pszText[ m_nPos ];
            if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
            {
                m_nPos++;
                continue;
            }
            if ( c == '/' && m_bAllowComments && m_nPos + 1 < m_nLength )
            {
                if ( m_pszText[ m_nPos + 1 ] == '/' )
                {
                    while ( m_nPos < m_nLength && m_pszText[ m_nPos ] != '\n' )
                        m_nPos++;
                    continue;
                }
                if ( m_pszText[ m_nPos + 1 ] == '*' )
                {
                    size_t nStart = m_nPos;
                    m_nPos += 2;
                    while ( m_nPos + 1 < m_nLength && ! ( m_pszText[ m_nPos ] == '*' && m_pszText[ m_nPos + 1 ] == '/' ) )
                        m_nPos++;
                    if ( m_nPos + 1 >= m_nLength )
                        return fail( "unterminated comment", nStart );
                    m_nPos += 2;
                    continue;
                }
            }
            break;
        }
        return true;
    }

    CJsonNode *CJsonParser::parseValue( int nDepth )
    {
        int c = peek();
        switch ( c )
        {
            case '{':
                return parseObject( nDepth );
            case '[':
                return parseArray( nDepth );
            case '"':
            {
                std::string strValue;
                if ( ! parseString( strValue ) )
                    return NULL;
                return new CTextNode( strValue );
            }
            case '-': case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return parseNumber();
            case 't': case 'f': case 'n':
                return parseLiteral();
            default:
                fail( "expected a value but found " + describeNext(), m_nPos );
                return NULL;
        }
    }

    CJsonNode *CJsonParser::parseObject( int nDepth )
    {
        if ( nDepth > m_nMaxDepth )
        {
            fail( "arrays and objects are nested more than " + std::to_string( m_nMaxDepth ) + " deep", m_nPos );
            return NULL;
        }
        m_nPos++;       // '{'
        CObjectNode *pObject = new CObjectNode();
        if ( ! skipWhitespace() )
        {
            delete pObject;
            return NULL;
        }
        if ( peek() == '}' )
        {
            m_nPos++;
            return pObject;
        }

        while ( true )
        {
            if ( peek() != '"' )
            {
                fail( "expected a field name (a string) but found " + describeNext(), m_nPos );
                break;
            }
            size_t nKeyOffset = m_nPos;
            std::string strKey;
            if ( ! parseString( strKey ) || ! skipWhitespace() )
                break;
            if ( peek() != ':' )
            {
                fail( "expected ':' after the field name but found " + describeNext(), m_nPos );
                break;
            }
            m_nPos++;
            if ( ! skipWhitespace() )
                break;
            CJsonNode *pValue = parseValue( nDepth + 1 );
            if ( pValue == NULL )
                break;
            if ( m_bRejectDuplicateKeys && pObject->has( strKey ) )
            {
                delete pValue;
                fail( "the field name \"" + strKey + "\" is repeated", nKeyOffset );
                break;
            }
            pObject->set( strKey, pValue );
            if ( ! skipWhitespace() )
                break;

            int c = peek();
            if ( c == '}' )
            {
                m_nPos++;
                return pObject;
            }
            if ( c != ',' )
            {
                fail( "expected ',' or '}' but found " + describeNext(), m_nPos );
                break;
            }
            m_nPos++;
            if ( ! skipWhitespace() )
                break;
            if ( m_bAllowTrailingCommas && peek() == '}' )
            {
                m_nPos++;
                return pObject;
            }
        }
        delete pObject;
        return NULL;
    }

    CJsonNode *CJsonParser::parseArray( int nDepth )
    {
        if ( nDepth > m_nMaxDepth )
        {
            fail( "arrays and objects are nested more than " + std::to_string( m_nMaxDepth ) + " deep", m_nPos );
            return NULL;
        }
        m_nPos++;       // '['
        CArrayNode *pArray = new CArrayNode();
        if ( ! skipWhitespace() )
        {
            delete pArray;
            return NULL;
        }
        if ( peek() == ']' )
        {
            m_nPos++;
            return pArray;
        }

        while ( true )
        {
            CJsonNode *pValue = parseValue( nDepth + 1 );
            if ( pValue == NULL )
                break;
            pArray->add( pValue );
            if ( ! skipWhitespace() )
                break;

            int c = peek();
            if ( c == ']' )
            {
                m_nPos++;
                return pArray;
            }
            if ( c != ',' )
            {
                fail( "expected ',' or ']' but found " + describeNext(), m_nPos );
                break;
            }
            m_nPos++;
            if ( ! skipWhitespace() )
                break;
            if ( m_bAllowTrailingCommas && peek() == ']' )
            {
                m_nPos++;
                return pArray;
            }
        }
        delete pArray;
        return NULL;
    }

    bool CJsonParser::readHex4( unsigned int &nValue )
    {
        nValue = 0;
        for ( int n = 0; n < 4; n++ )
        {
            int c = peek();
            int nDigit;
            if ( c >= '0' && c <= '9' )
                nDigit = c - '0';
            else if ( c >= 'a' && c <= 'f' )
                nDigit = c - 'a' + 10;
            else if ( c >= 'A' && c <= 'F' )
                nDigit = c - 'A' + 10;
            else
                return fail( "expected four hexadecimal digits after \\u", m_nPos );
            nValue = ( nValue << 4 ) | (unsigned int)nDigit;
            m_nPos++;
        }
        return true;
    }

    namespace
    {
        void appendUtf8( std::string &strOut, unsigned int nCode )
        {
            if ( nCode < 0x80 )
            {
                strOut += (char)nCode;
            }
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
    }

    bool CJsonParser::parseString( std::string &strValue )
    {
        size_t nStart = m_nPos;
        m_nPos++;       // '"'
        strValue.clear();
        const unsigned char *pText = (const unsigned char *)m_pszText;

        while ( true )
        {
                // A run of plain characters.
            size_t nRun = m_nPos;
            while ( m_nPos < m_nLength && pText[ m_nPos ] >= 0x20 && pText[ m_nPos ] < 0x80 &&
                    pText[ m_nPos ] != '"' && pText[ m_nPos ] != '\\' )
                m_nPos++;
            strValue.append( m_pszText + nRun, m_nPos - nRun );

            if ( m_nPos >= m_nLength )
                return fail( "the string is not closed", nStart );
            unsigned char c = pText[ m_nPos ];
            if ( c == '"' )
            {
                m_nPos++;
                return true;
            }
            if ( c < 0x20 )
                return fail( "control characters must be escaped in strings", m_nPos );
            if ( c >= 0x80 )
            {
                unsigned int nCode = 0;
                size_t nLength = CJsonNode::decodeUtf8( pText + m_nPos, pText + m_nLength, nCode );
                if ( nLength == 0 )
                    return fail( "the text is not valid UTF-8", m_nPos );
                strValue.append( m_pszText + m_nPos, nLength );
                m_nPos += nLength;
                continue;
            }

                // An escape.
            size_t nEscape = m_nPos;
            m_nPos++;
            int e = peek();
            m_nPos++;
            switch ( e )
            {
                case '"':  strValue += '"'; break;
                case '\\': strValue += '\\'; break;
                case '/':  strValue += '/'; break;
                case 'b':  strValue += '\b'; break;
                case 'f':  strValue += '\f'; break;
                case 'n':  strValue += '\n'; break;
                case 'r':  strValue += '\r'; break;
                case 't':  strValue += '\t'; break;
                case 'u':
                {
                    unsigned int nCode = 0;
                    if ( ! readHex4( nCode ) )
                        return false;
                    if ( nCode >= 0xDC00 && nCode <= 0xDFFF )
                        return fail( "a \\u escape gives the second half of a surrogate pair without the first", nEscape );
                    if ( nCode >= 0xD800 && nCode <= 0xDBFF )
                    {
                        unsigned int nLow = 0;
                        if ( peek() != '\\' || m_nPos + 1 >= m_nLength || m_pszText[ m_nPos + 1 ] != 'u' )
                            return fail( "a \\u escape gives the first half of a surrogate pair without the second", nEscape );
                        m_nPos += 2;
                        if ( ! readHex4( nLow ) )
                            return false;
                        if ( nLow < 0xDC00 || nLow > 0xDFFF )
                            return fail( "a \\u escape gives the first half of a surrogate pair without the second", nEscape );
                        nCode = 0x10000 + ( ( nCode - 0xD800 ) << 10 ) + ( nLow - 0xDC00 );
                    }
                    appendUtf8( strValue, nCode );
                    break;
                }
                default:
                    m_nPos = nEscape + 1;
                    if ( e < 0 )
                        return fail( "the string is not closed", nStart );
                    if ( e >= 0x20 && e < 0x7F )
                        return fail( std::string( "\\" ) + (char)e + " is not a valid escape", nEscape );
                    return fail( "a backslash must be followed by a valid escape", nEscape );
            }
        }
    }

    CJsonNode *CJsonParser::parseNumber( void )
    {
        size_t nStart = m_nPos;
        if ( peek() == '-' )
            m_nPos++;
        int c = peek();
        if ( c == '0' )
        {
            m_nPos++;
            c = peek();
            if ( c >= '0' && c <= '9' )
            {
                fail( "numbers cannot have leading zeros", nStart );
                return NULL;
            }
        }
        else if ( c >= '1' && c <= '9' )
        {
            while ( peek() >= '0' && peek() <= '9' )
                m_nPos++;
        }
        else
        {
            fail( "expected a digit but found " + describeNext(), m_nPos );
            return NULL;
        }

        if ( peek() == '.' )
        {
            m_nPos++;
            if ( ! ( peek() >= '0' && peek() <= '9' ) )
            {
                fail( "expected a digit after the decimal point but found " + describeNext(), m_nPos );
                return NULL;
            }
            while ( peek() >= '0' && peek() <= '9' )
                m_nPos++;
        }

        if ( peek() == 'e' || peek() == 'E' )
        {
            m_nPos++;
            if ( peek() == '+' || peek() == '-' )
                m_nPos++;
            if ( ! ( peek() >= '0' && peek() <= '9' ) )
            {
                fail( "expected a digit in the exponent but found " + describeNext(), m_nPos );
                return NULL;
            }
            while ( peek() >= '0' && peek() <= '9' )
                m_nPos++;
        }

        CNumberNode *pNumber = CNumberNode::fromText( std::string( m_pszText + nStart, m_nPos - nStart ) );
        if ( pNumber == NULL )
            fail( "the number cannot be read", nStart );
        return pNumber;
    }

    CJsonNode *CJsonParser::parseLiteral( void )
    {
        static const struct { const char *pszText; size_t nLength; int nKind; } literals[] = {
            { "true", 4, 1 }, { "false", 5, 0 }, { "null", 4, 2 },
        };
        for ( size_t n = 0; n < sizeof( literals ) / sizeof( literals[0] ); n++ )
        {
            if ( m_nLength - m_nPos >= literals[ n ].nLength && memcmp( m_pszText + m_nPos, literals[ n ].pszText, literals[ n ].nLength ) == 0 )
            {
                m_nPos += literals[ n ].nLength;
                if ( literals[ n ].nKind == 2 )
                    return new CNullNode();
                return new CBooleanNode( literals[ n ].nKind == 1 );
            }
        }
        fail( "expected a value but found " + describeNext(), m_nPos );
        return NULL;
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
