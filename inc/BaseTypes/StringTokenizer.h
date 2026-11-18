/*
 * String Tokenizer
 *
 *	This class allows the breakdown of strings into their constituent
 * tokens. It can be used to do some rather complex string parsing.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/18/1994
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STRINGTOKENIZER_H__
#define IASLIB_STRINGTOKENIZER_H__

#include "Object.h"
#include "String_.h"

namespace IASLib
{
        // Splits a string into tokens separated by any of a set of delimiter
        // characters, in one of three ways:
        //
        //  - normally, a run of delimiters separates two tokens, and tokens
        //    are never empty: "a,,b," gives "a", "b";
        //  - with bReturnTokens, each delimiter is also a token of its own:
        //    "a,,b" gives "a", ",", ",", "b";
        //  - with bAllDelimitersValid, every delimiter separates two tokens,
        //    which may be empty (as in a CSV line): "a,,b," gives "a", "",
        //    "b", "" (and "" gives no tokens).
        //
        // With bIgnoreQuotedDelimiters, delimiters between double quotes are
        // part of the token (the quotes are kept). After the last token,
        // NextToken() returns "".
    class CStringTokenizer : public CObject
    {
        private:
            size_t      m_nCurrentPosition;
            CString     m_strWorkingString;
            CString     m_strDelimiters;
            bool        m_bReturnTokens;
            bool        m_bPreserveDelimiters;
            bool        m_bIgnoreQuoted;
            bool        m_bTokenDue;        // bAllDelimitersValid: a token (maybe empty) follows
        public:
	                    CStringTokenizer( const char *strParsed, const char *strDelimiters = " \t\n\r\f", const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
	                    CStringTokenizer( const CString &strParsed, const char *strDelimiters = " \t\n\r\f", const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
	                    CStringTokenizer( const CString &strParsed, const CString &strDelimiters, const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
	        virtual    ~CStringTokenizer();

                        DEFINE_OBJECT( CStringTokenizer );

            bool        HasMoreTokens( void ) const;
            CString     NextToken( void );
                // Changes the delimiters (for this and later tokens), then
                // returns the next token.
            CString     NextToken( const char *strDelimiters );
                // How many tokens are left.
            int         CountTokens( void ) const;

        private:
            void        Init( const CString &strParsed, const CString &strDelimiters, bool bReturnTokens, bool bAllDelimitersValid, bool bIgnoreQuoted );
            bool        IsDelimiter( size_t nPosition ) const;
                // The end of the token starting at nPosition.
            size_t      TokenEnd( size_t nPosition ) const;
            size_t      SkipDelimiters( size_t nPosition ) const;
    };
} // End of namespace IASLib

#endif // IASLIB_STRINGTOKENIZER_H__
