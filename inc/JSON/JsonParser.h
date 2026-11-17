/**
 * JSON Parser Class
 *
 *  Reads JSON text (RFC 8259) into a tree of JSON nodes. Parsing is strict
 * by default: exactly one JSON value, optionally surrounded by white space,
 * in valid UTF-8 (a leading byte order mark is skipped). Anything else is an
 * error, reported with its line and column.
 *
 *  Options relax this for hand-written files: comments (// and slash-star),
 * and a trailing comma after the last element or field. Duplicate field
 * names are allowed by default (the last one wins, in the place of the
 * first) and can be made an error. Nesting is limited (512 levels by
 * default), so hostile input cannot exhaust the stack.
 *
 *      CJsonNode *pRoot = CJsonParser::parse( strText );   // NULL on error
 *
 *      CJsonParser parser;
 *      parser.setAllowComments( true );
 *      CJsonNode *pRoot = parser.readTree( strText );
 *      if ( pRoot == NULL )
 *          printf( "%s\n", (const char *)parser.getError() );
 *
 *  The caller owns the tree returned and deletes it.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 9/13/2020
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_JSON_PARSER_H__
#define IASLIB_JSON_PARSER_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../Streams/Stream.h"
#include "JsonNode.h"

#include <string>

namespace IASLib
{
    class CJsonParser : public CObject
    {
        public:
            DEFINE_OBJECT( CJsonParser )

            enum { DEFAULT_MAX_DEPTH = 512 };

                                    CJsonParser( void );
            virtual                ~CJsonParser( void ) {}

                // Strict parsing with the default options; NULL on error.
            static CJsonNode       *parse( const char *pszText );
            static CJsonNode       *parse( const std::string &strText );
            static CJsonNode       *parse( CStream *pStream );

                // Parsing with this parser's options. NULL on error, with the
                // error available from getError() and the methods after it.
            CJsonNode              *readTree( const char *pszText, size_t nLength );
            CJsonNode              *readTree( const char *pszText );
            CJsonNode              *readTree( const std::string &strText );
                // Reads the rest of the stream.
            CJsonNode              *readTree( CStream *pStream );

                // Options.
            void                    setAllowComments( bool bAllow ) { m_bAllowComments = bAllow; }
            bool                    getAllowComments( void ) const { return m_bAllowComments; }
            void                    setAllowTrailingCommas( bool bAllow ) { m_bAllowTrailingCommas = bAllow; }
            bool                    getAllowTrailingCommas( void ) const { return m_bAllowTrailingCommas; }
            void                    setRejectDuplicateKeys( bool bReject ) { m_bRejectDuplicateKeys = bReject; }
            bool                    getRejectDuplicateKeys( void ) const { return m_bRejectDuplicateKeys; }
                // Arrays and objects nested deeper than this are an error.
            void                    setMaxDepth( int nDepth ) { m_nMaxDepth = ( nDepth < 1 ) ? 1 : nDepth; }
            int                     getMaxDepth( void ) const { return m_nMaxDepth; }

                // The last error, as "line 3, column 14: expected ',' or '}'"
                // (empty after a successful parse), and where it was: the line
                // and column (from 1; columns count characters) and the byte
                // offset (from 0).
            const CString          &getError( void ) const { return m_strError; }
            const CString          &getErrorMessage( void ) const { return m_strErrorMessage; }
            size_t                  getErrorLine( void ) const { return m_nErrorLine; }
            size_t                  getErrorColumn( void ) const { return m_nErrorColumn; }
            size_t                  getErrorOffset( void ) const { return m_nErrorOffset; }
            bool                    hasError( void ) const { return m_strError.GetLength() > 0; }

        private:
            bool                    m_bAllowComments;
            bool                    m_bAllowTrailingCommas;
            bool                    m_bRejectDuplicateKeys;
            int                     m_nMaxDepth;

            CString                 m_strError;
            CString                 m_strErrorMessage;
            size_t                  m_nErrorLine;
            size_t                  m_nErrorColumn;
            size_t                  m_nErrorOffset;

                // The text being parsed.
            const char             *m_pszText;
            size_t                  m_nLength;
            size_t                  m_nPos;
            bool                    m_bFailed;

            CJsonNode              *parseValue( int nDepth );
            CJsonNode              *parseObject( int nDepth );
            CJsonNode              *parseArray( int nDepth );
            bool                    parseString( std::string &strValue );
            CJsonNode              *parseNumber( void );
            CJsonNode              *parseLiteral( void );
            bool                    skipWhitespace( void );
            bool                    readHex4( unsigned int &nValue );

            int                     peek( void ) const { return ( m_nPos < m_nLength ) ? (unsigned char)m_pszText[ m_nPos ] : -1; }
            bool                    fail( const char *pszMessage, size_t nOffset );
            bool                    fail( const std::string &strMessage, size_t nOffset ) { return fail( strMessage.c_str(), nOffset ); }
            std::string             describeNext( void ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_JSON_PARSER_H__
