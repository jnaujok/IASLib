/**
 * @file JsonParser.h
 * @brief CJsonParser, which reads JSON text into a tree of JSON nodes.
 *
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
 * first) and can be made an error.
 *
 *  Limits: nesting is limited to 512 levels by default, and can be raised as
 * far as memory allows, since the parser keeps its own stack rather than
 * recursing (and the nodes' writing, copying, comparing and deleting do the
 * same). The size of the input and the length of any one string can also be
 * limited (by default they are not), so hostile input can be refused before
 * it uses much memory. Input past a limit is an error like any other.
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
        /// Reads JSON text (RFC 8259) into a tree of JSON nodes.
        ///
        /// Parsing is strict by default: exactly one JSON value, optionally
        /// surrounded by white space, in valid UTF-8 (a leading byte order mark
        /// is skipped). Anything else is an error, reported with its line and
        /// column. Options relax this for hand-written files (comments, a
        /// trailing comma), make duplicate field names an error (by default
        /// the last one wins, in the place of the first), and limit nesting,
        /// input size and string length so hostile input can be refused early.
        ///
        /// The parser keeps its own stack rather than recursing, so nesting is
        /// limited by setMaxDepth() and memory, not by the call stack.
        ///
        /// Ownership: the caller owns the tree returned and deletes it.
        /// Threading: one parser object must not be used by two threads at
        /// once; the static parse() methods use a parser of their own.
        ///
        /// @code
        ///     CJsonNode *pRoot = CJsonParser::parse( strText );   // NULL on error
        ///
        ///     CJsonParser parser;
        ///     parser.setAllowComments( true );
        ///     CJsonNode *pRoot = parser.readTree( strText );
        ///     if ( pRoot == NULL )
        ///         printf( "%s\n", (const char *)parser.getError() );
        ///     delete pRoot;
        /// @endcode
    class CJsonParser : public CObject
    {
        public:
            DEFINE_OBJECT( CJsonParser )

                /// The default limit on nesting (see setMaxDepth()).
            enum { DEFAULT_MAX_DEPTH = 512 /**< Levels of arrays and objects. */ };

                /// A parser with the default options: strict, with a depth limit
                /// of #DEFAULT_MAX_DEPTH and no size limits.
                                    CJsonParser( void );
                /// Destroys the parser (not any tree it returned).
            virtual                ~CJsonParser( void ) {}

                /// @name Parsing with the default options
                /// Strict parsing with the default options. The caller owns the
                /// tree returned. No error details are available: use a
                /// CJsonParser object and readTree() for those.
                ///@{

                /// Parses NUL-terminated text (NULL is an error).
                /// @return The tree, or NULL on error.
            static CJsonNode       *parse( const char *pszText );
                /// Parses a string, which may hold NULs.
                /// @return The tree, or NULL on error.
            static CJsonNode       *parse( const std::string &strText );
                /// Parses the rest of a stream.
                /// @return The tree, or NULL on error (or a NULL or closed stream).
            static CJsonNode       *parse( CStream *pStream );
                ///@}

                /// @name Parsing with this parser's options
                /// Each resets the error state, then parses. The caller owns the
                /// tree returned. On error the result is NULL, with the error
                /// available from getError() and the methods after it.
                ///@{

                /// Parses nLength bytes of text, which may hold NULs.
            CJsonNode              *readTree( const char *pszText, size_t nLength );
                /// Parses NUL-terminated text (NULL is an error).
            CJsonNode              *readTree( const char *pszText );
                /// Parses a string, which may hold NULs.
            CJsonNode              *readTree( const std::string &strText );
                /// Reads the rest of the stream and parses it. With a maximum
                /// input size the stream is read no further than just past it.
                /// A NULL or closed stream is an error.
            CJsonNode              *readTree( CStream *pStream );
                ///@}

                /// @name Options
                ///@{

                /// Allow `//` and slash-star comments wherever white space may go
                /// (default false).
            void                    setAllowComments( bool bAllow ) { m_bAllowComments = bAllow; }
                /// Whether comments are allowed.
            bool                    getAllowComments( void ) const { return m_bAllowComments; }
                /// Allow a comma after the last element of an array or field of
                /// an object (default false).
            void                    setAllowTrailingCommas( bool bAllow ) { m_bAllowTrailingCommas = bAllow; }
                /// Whether trailing commas are allowed.
            bool                    getAllowTrailingCommas( void ) const { return m_bAllowTrailingCommas; }
                /// Make a field name repeated in one object an error (default
                /// false: the last value wins, in the place of the first).
            void                    setRejectDuplicateKeys( bool bReject ) { m_bRejectDuplicateKeys = bReject; }
                /// Whether duplicate field names are an error.
            bool                    getRejectDuplicateKeys( void ) const { return m_bRejectDuplicateKeys; }
                /// Arrays and objects nested deeper than this are an error.
                /// Values below 1 are taken as 1. The default is
                /// #DEFAULT_MAX_DEPTH; it can be raised as far as memory allows.
            void                    setMaxDepth( int nDepth ) { m_nMaxDepth = ( nDepth < 1 ) ? 1 : nDepth; }
                /// The nesting limit.
            int                     getMaxDepth( void ) const { return m_nMaxDepth; }
                /// Input longer than this many bytes is an error (0, the
                /// default: no limit). A stream is read no further than that.
            void                    setMaxInputSize( size_t nBytes ) { m_nMaxInputSize = nBytes; }
                /// The input size limit in bytes (0: none).
            size_t                  getMaxInputSize( void ) const { return m_nMaxInputSize; }
                /// A string value or field name longer than this many bytes
                /// (once its escapes are decoded) is an error (0, the default:
                /// no limit).
            void                    setMaxStringLength( size_t nBytes ) { m_nMaxStringLength = nBytes; }
                /// The string length limit in bytes (0: none).
            size_t                  getMaxStringLength( void ) const { return m_nMaxStringLength; }
                ///@}

                /// @name The last error
                /// Empty or 0 after a successful parse.
                ///@{

                /// The last error with where it was, as
                /// "line 3, column 14: expected ',' or '}'".
            const CString          &getError( void ) const { return m_strError; }
                /// The last error without the line and column.
            const CString          &getErrorMessage( void ) const { return m_strErrorMessage; }
                /// The error's line, from 1.
            size_t                  getErrorLine( void ) const { return m_nErrorLine; }
                /// The error's column, from 1, counting characters (not bytes).
            size_t                  getErrorColumn( void ) const { return m_nErrorColumn; }
                /// The error's byte offset in the input, from 0.
            size_t                  getErrorOffset( void ) const { return m_nErrorOffset; }
                /// True if the last parse failed.
            bool                    hasError( void ) const { return m_strError.GetLength() > 0; }
                ///@}

        private:
            bool                    m_bAllowComments;           ///< Comments allowed.
            bool                    m_bAllowTrailingCommas;     ///< Trailing commas allowed.
            bool                    m_bRejectDuplicateKeys;     ///< Duplicate field names are an error.
            int                     m_nMaxDepth;                ///< Nesting limit (at least 1).
            size_t                  m_nMaxInputSize;            ///< Input size limit in bytes (0: none).
            size_t                  m_nMaxStringLength;         ///< String length limit in bytes (0: none).

            CString                 m_strError;                 ///< The last error, with line and column.
            CString                 m_strErrorMessage;          ///< The last error alone.
            size_t                  m_nErrorLine;               ///< Its line, from 1.
            size_t                  m_nErrorColumn;             ///< Its column, from 1.
            size_t                  m_nErrorOffset;             ///< Its byte offset, from 0.

            const char             *m_pszText;                  ///< The text being parsed (not owned).
            size_t                  m_nLength;                  ///< Its length in bytes.
            size_t                  m_nPos;                     ///< The current byte offset.
            bool                    m_bFailed;                  ///< An error has been recorded.

                /// One value, nested to any depth up to the limit, without
                /// recursion. NULL on error, with nothing left allocated.
            CJsonNode              *parseTree( void );
                /// A string, number or literal at the current position.
            CJsonNode              *parseScalar( void );
                /// A field name and the ':' after it, leaving the position at
                /// the value.
            bool                    parseFieldName( std::string &strKey, size_t &nKeyOffset );
                /// A quoted string, its escapes decoded.
            bool                    parseString( std::string &strValue );
                /// A number, checked against the JSON grammar.
            CJsonNode              *parseNumber( void );
                /// true, false or null.
            CJsonNode              *parseLiteral( void );
                /// Skips white space (and comments, if allowed). False for an
                /// unterminated comment.
            bool                    skipWhitespace( void );
                /// Four hexadecimal digits, as after `\u`.
            bool                    readHex4( unsigned int &nValue );

                /// The byte at the current position, or -1 at the end.
            int                     peek( void ) const { return ( m_nPos < m_nLength ) ? (unsigned char)m_pszText[ m_nPos ] : -1; }
                /// Records the first error, with its line and column. Always
                /// returns false.
            bool                    fail( const char *pszMessage, size_t nOffset );
                /// @copydoc fail(const char *,size_t)
            bool                    fail( const std::string &strMessage, size_t nOffset ) { return fail( strMessage.c_str(), nOffset ); }
                /// The next character, for error messages: "'x'", "end of
                /// input", or "byte 0xNN" for a control or non-ASCII byte.
            std::string             describeNext( void ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_JSON_PARSER_H__
