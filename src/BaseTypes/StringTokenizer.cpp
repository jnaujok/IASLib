/**
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

#include "StringTokenizer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CStringTokenizer, CObject );

    CStringTokenizer::CStringTokenizer( const char *strParsed, const char *strDelimiters, const bool bReturnTokens, const bool bAllDelimitersValid, const bool bIgnoreQuotedDelimiters )
    {
        Init( strParsed, strDelimiters, bReturnTokens, bAllDelimitersValid, bIgnoreQuotedDelimiters );
    }

    CStringTokenizer::CStringTokenizer( const CString &strParsed, const char *strDelimiters, const bool bReturnTokens, const bool bAllDelimitersValid, const bool bIgnoreQuotedDelimiters )
    {
        Init( strParsed, strDelimiters, bReturnTokens, bAllDelimitersValid, bIgnoreQuotedDelimiters );
    }

    CStringTokenizer::CStringTokenizer( const CString &strParsed, const CString &strDelimiters, const bool bReturnTokens, const bool bAllDelimitersValid, const bool bIgnoreQuotedDelimiters )
    {
        Init( strParsed, strDelimiters, bReturnTokens, bAllDelimitersValid, bIgnoreQuotedDelimiters );
    }

    CStringTokenizer::~CStringTokenizer()
    {
    }

    void CStringTokenizer::Init( const CString &strParsed, const CString &strDelimiters, bool bReturnTokens, bool bAllDelimitersValid, bool bIgnoreQuoted )
    {
        m_nCurrentPosition = 0;
        m_strWorkingString = strParsed;
        m_strDelimiters = strDelimiters;
        m_bReturnTokens = bReturnTokens;
            // Returning delimiters already shows where every one was.
        m_bPreserveDelimiters = bAllDelimitersValid && ! bReturnTokens;
        m_bIgnoreQuoted = bIgnoreQuoted;
        m_bTokenDue = ! m_strWorkingString.IsEmpty();
    }

    bool CStringTokenizer::IsDelimiter( size_t nPosition ) const
    {
        return nPosition < m_strWorkingString.GetLength() && m_strDelimiters.IndexOf( m_strWorkingString[ nPosition ] ) != NOT_FOUND;
    }

    size_t CStringTokenizer::TokenEnd( size_t nPosition ) const
    {
        size_t nLength = m_strWorkingString.GetLength();
        bool bInQuote = false;
        while ( nPosition < nLength && ( bInQuote || ! IsDelimiter( nPosition ) ) )
        {
            if ( m_bIgnoreQuoted && m_strWorkingString[ nPosition ] == '"' )
            {
                bInQuote = ! bInQuote;
            }
            nPosition++;
        }
        return nPosition;
    }

    size_t CStringTokenizer::SkipDelimiters( size_t nPosition ) const
    {
        while ( IsDelimiter( nPosition ) )
        {
            nPosition++;
        }
        return nPosition;
    }

    bool CStringTokenizer::HasMoreTokens( void ) const
    {
        if ( m_bPreserveDelimiters )
        {
            return m_bTokenDue;
        }
        size_t nPosition = m_bReturnTokens ? m_nCurrentPosition : SkipDelimiters( m_nCurrentPosition );
        return nPosition < m_strWorkingString.GetLength();
    }

    CString CStringTokenizer::NextToken( void )
    {
        size_t nLength = m_strWorkingString.GetLength();

        if ( m_bPreserveDelimiters )
        {
            if ( ! m_bTokenDue )
            {
                return CString();
            }
            size_t nEnd = TokenEnd( m_nCurrentPosition );
            CString strToken = m_strWorkingString.Substring( m_nCurrentPosition, (int)( nEnd - m_nCurrentPosition ) );
                // Stopped at a delimiter: another token follows it.
            m_bTokenDue = ( nEnd < nLength );
            m_nCurrentPosition = m_bTokenDue ? nEnd + 1 : nEnd;
            return strToken;
        }

        if ( ! m_bReturnTokens )
        {
            m_nCurrentPosition = SkipDelimiters( m_nCurrentPosition );
        }
        if ( m_nCurrentPosition >= nLength )
        {
            return CString();
        }

        if ( m_bReturnTokens && IsDelimiter( m_nCurrentPosition ) )
        {
            return m_strWorkingString.Substring( m_nCurrentPosition++, 1 );
        }

        size_t nEnd = TokenEnd( m_nCurrentPosition );
        CString strToken = m_strWorkingString.Substring( m_nCurrentPosition, (int)( nEnd - m_nCurrentPosition ) );
        m_nCurrentPosition = nEnd;
        return strToken;
    }

    CString CStringTokenizer::NextToken( const char *strDelimiters )
    {
        m_strDelimiters = strDelimiters;
        return NextToken();
    }

    int CStringTokenizer::CountTokens( void ) const
    {
        CStringTokenizer copy( *this );
        int nCount = 0;
        while ( copy.HasMoreTokens() )
        {
            copy.NextToken();
            nCount++;
        }
        return nCount;
    }
} // namespace IASLib
