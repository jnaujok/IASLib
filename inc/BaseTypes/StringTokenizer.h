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
    class CStringTokenizer : public CObject
    {
        private:
            size_t      m_nCurrentPosition;
            size_t      m_nMaxPosition;
            CString     m_strWorkingString;
            CString     m_strDelimiters;
            bool        m_bReturnTokens;
            bool        m_bPreserveDelimiters;
            bool        m_bLastTokenBlank;
            bool        m_bFirstToken;
            bool        m_bIgnoreQuoted;
        public:
	                    CStringTokenizer( const char *strParsed, const char *strDelimiters = " \t\n\r\f", const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
	                    CStringTokenizer( const CString &strParsed, const char *strDelimiters = " \t\n\r\f", const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
	                    CStringTokenizer( const CString &strParsed, const CString &strDelimiters, const bool bReturnTokens = false, const bool bAllDelimitersValid = false, const bool bIgnoreQuotedDelimiters = false );
	        virtual    ~CStringTokenizer();

                        DEFINE_OBJECT( CStringTokenizer );

            bool        HasMoreTokens( void );
            CString     NextToken( void );
            CString     NextToken( const char *strDelimiters );
            int         CountTokens( void );

        private:
            void        SkipDelimiters();
    };
} // End of namespace IASLib

#endif // IASLIB_STRINGTOKENIZER_H__
