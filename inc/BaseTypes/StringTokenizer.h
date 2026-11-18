/**
 * @file StringTokenizer.h
 * @brief CStringTokenizer, which splits a string into tokens.
 *
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
        /// Splits a string into tokens separated by any of a set of delimiter
        /// characters.
        ///
        /// It works in one of three ways:
        ///
        ///  - normally, a run of delimiters separates two tokens, and tokens
        ///    are never empty: "a,,b," gives "a", "b";
        ///  - with bReturnTokens, each delimiter is also a token of its own:
        ///    "a,,b" gives "a", ",", ",", "b";
        ///  - with bAllDelimitersValid, every delimiter separates two tokens,
        ///    which may be empty (as in a CSV line): "a,,b," gives "a", "",
        ///    "b", "" (and "" gives no tokens).
        ///
        /// With bIgnoreQuotedDelimiters, delimiters between double quotes are
        /// part of the token (the quotes are kept). After the last token,
        /// NextToken() returns "". The tokenizer works on its own copy of the
        /// string.
        ///
        /// @code
        /// CStringTokenizer tok( "a,,b,", ",", false, true );
        /// while ( tok.HasMoreTokens() )
        ///     printf( "[%s]", (const char *)tok.NextToken() );   // [a][][b][]
        /// @endcode
    class CStringTokenizer : public CObject
    {
        private:
            size_t      m_nCurrentPosition;     ///< Where the next token starts.
            CString     m_strWorkingString;     ///< The string being split.
            CString     m_strDelimiters;        ///< The delimiter characters.
            bool        m_bReturnTokens;        ///< Whether delimiters are tokens too.
            bool        m_bPreserveDelimiters;  ///< bAllDelimitersValid (and not bReturnTokens).
            bool        m_bIgnoreQuoted;        ///< Whether quoted delimiters are part of the token.
            bool        m_bTokenDue;            ///< bAllDelimitersValid: a token (maybe empty) follows.
        public:
                    /// A tokenizer over strParsed.
                    /// @param strParsed The string to split.
                    /// @param strDelimiters The delimiter characters (white space by default).
                    /// @param bReturnTokens Return each delimiter as a token too.
                    /// @param bAllDelimitersValid Every delimiter separates two
                    ///     tokens, which may be empty (ignored with bReturnTokens).
                    /// @param bIgnoreQuotedDelimiters Delimiters between double
                    ///     quotes are part of the token.
	                    CStringTokenizer( const char *strParsed, const char *strDelimiters = " \t\n\r\f", const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
                    /// @copydoc CStringTokenizer(const char*,const char*,const bool,const bool,const bool)
	                    CStringTokenizer( const CString &strParsed, const char *strDelimiters = " \t\n\r\f", const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
                    /// @copydoc CStringTokenizer(const char*,const char*,const bool,const bool,const bool)
	                    CStringTokenizer( const CString &strParsed, const CString &strDelimiters, const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
	        virtual    ~CStringTokenizer();

                        DEFINE_OBJECT( CStringTokenizer );

                /// Whether another token is left.
            bool        HasMoreTokens( void ) const;
                /// The next token, or "" after the last one.
            CString     NextToken( void );
                /// Changes the delimiters (for this and later tokens), then
                /// returns the next token.
            CString     NextToken( const char *strDelimiters );
                /// How many tokens are left (counted on a copy; the position
                /// does not change).
            int         CountTokens( void ) const;

        private:
                /// What the constructors share.
            void        Init( const CString &strParsed, const CString &strDelimiters, bool bReturnTokens, bool bAllDelimitersValid, bool bIgnoreQuoted );
                /// Whether the character at nPosition is a delimiter (false past the end).
            bool        IsDelimiter( size_t nPosition ) const;
                /// The end of the token starting at nPosition.
            size_t      TokenEnd( size_t nPosition ) const;
                /// The first position at or after nPosition that is not a delimiter.
            size_t      SkipDelimiters( size_t nPosition ) const;
    };
} // End of namespace IASLib

#endif // IASLIB_STRINGTOKENIZER_H__
