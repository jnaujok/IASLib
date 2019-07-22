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
#include "Exception.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CStringTokenizer, CObject );

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

        /**
        * /brief The standard CStringTokenizer constructor.
        * Constructs a string tokenizer for the specified string. All
        * characters in the <code>strDelimiter</code> argument are the delimiters
        * for separating tokens.
        * <p>
        * If the <code>bReturnTokens</code> flag is <code>true</code>, then
        * the delimiter characters are also returned as tokens. Each
        * delimiter is returned as a string of length one. If the flag is
        * <code>false</code>, the delimiter characters are skipped and only
        * serve as separators between tokens.
        * If the <code>bAllDelimtersValid</code> flag is <code>true</code>, then
        * every delimiter is treated as a delimiter, even when there are multiple
        * delimiters in a row. In this case, some tokens that are returned may
        * be empty. If this flag is <code>false</code> then multiple continuous
        * delimiters are treated as a single delimiter.
        *
        * @param   strParsed      a string to be parsed.
        * @param   strDelimiters  the delimiters.
        * @param   bReturnTokens  flag indicating whether to return the delimiters
        *                         as tokens.
        * @param   bAllDelimitersValid  flag indicating whether to treat multiple
        *                               continuous delimiters as multiple delimiters
        *                               (<code>true</code>) or treat them all as a
        *                               single delimiter (<code>false</code>).
        */
    CStringTokenizer::CStringTokenizer( const char *strParsed, const char *strDelimiters, const bool bReturnTokens, const bool bAllDelimitersValid, const bool bIgnoreQuotedDelimiters )
    {
	    m_nCurrentPosition = 0;
	    m_strWorkingString = strParsed;
	    m_nMaxPosition = m_strWorkingString.GetLength();
	    m_strDelimiters = strDelimiters;
	    m_bReturnTokens = bReturnTokens;
        m_bPreserveDelimiters = bAllDelimitersValid;
        m_bLastTokenBlank = false;
        m_bFirstToken = true;
        m_bIgnoreQuoted = bIgnoreQuotedDelimiters;
    }

        /**
        * /brief The CStringTokenizer constructor with delimiter importance flag.
        * Constructs a string tokenizer for the specified string. All
        * characters in the <code>strDelimiter</code> argument are the delimiters
        * for separating tokens.
        * <p>
        * If the <code>bReturnTokens</code> flag is <code>true</code>, then
        * the delimiter characters are also returned as tokens. Each
        * delimiter is returned as a string of length one. If the flag is
        * <code>false</code>, the delimiter characters are skipped and only
        * serve as separators between tokens.
        * If the <code>bAllDelimtersValid</code> flag is <code>true</code>, then
        * every delimiter is treated as a delimiter, even when there are multiple
        * delimiters in a row. In this case, some tokens that are returned may
        * be empty. If this flag is <code>false</code> then multiple continuous
        * delimiters are treated as a single delimiter.
        *
        * @param   strParsed      a CString object to be parsed.
        * @param   strDelimiters  the delimiters.
        * @param   bReturnTokens  flag indicating whether to return the delimiters
        *                         as tokens.
        * @param   bAllDelimitersValid  flag indicating whether to treat multiple
        *                               continuous delimiters as multiple delimiters
        *                               (<code>true</code>) or treat them all as a
        *                               single delimiter (<code>false</code>).
        */
    CStringTokenizer::CStringTokenizer( const CString &strParsed, const char *strDelimiters, const bool bReturnTokens, const bool bAllDelimitersValid, const bool bIgnoreQuotedDelimiters )
    {
	    m_nCurrentPosition = 0;
	    m_strWorkingString = strParsed;
	    m_nMaxPosition = m_strWorkingString.GetLength();
	    m_strDelimiters = strDelimiters;
	    m_bReturnTokens = bReturnTokens;
        m_bPreserveDelimiters = bAllDelimitersValid;
        m_bLastTokenBlank = false;
        m_bFirstToken = true;
        m_bIgnoreQuoted = bIgnoreQuotedDelimiters;
    }

        /**
        * /brief The CStringTokenizer constructor with delimiter importance flag.
        * Constructs a string tokenizer for the specified string. All
        * characters in the <code>strDelimiter</code> argument are the delimiters
        * for separating tokens.
        * <p>
        * If the <code>bReturnTokens</code> flag is <code>true</code>, then
        * the delimiter characters are also returned as tokens. Each
        * delimiter is returned as a string of length one. If the flag is
        * <code>false</code>, the delimiter characters are skipped and only
        * serve as separators between tokens.
        * If the <code>bAllDelimtersValid</code> flag is <code>true</code>, then
        * every delimiter is treated as a delimiter, even when there are multiple
        * delimiters in a row. In this case, some tokens that are returned may
        * be empty. If this flag is <code>false</code> then multiple continuous
        * delimiters are treated as a single delimiter.
        *
        * @param   strParsed      a CString object to be parsed.
        * @param   strDelimiters  a CString object of the delimiters.
        * @param   bReturnTokens  flag indicating whether to return the delimiters
        *                         as tokens.
        * @param   bAllDelimitersValid  flag indicating whether to treat multiple
        *                               continuous delimiters as multiple delimiters
        *                               (<code>true</code>) or treat them all as a
        *                               single delimiter (<code>false</code>).
        */
    CStringTokenizer::CStringTokenizer( const CString &strParsed, const CString &strDelimiters, const bool bReturnTokens, const bool bAllDelimitersValid, const bool bIgnoreQuotedDelimiters )
    {
	    m_nCurrentPosition = 0;
	    m_strWorkingString = strParsed;
	    m_nMaxPosition = m_strWorkingString.GetLength();
	    m_strDelimiters = strDelimiters;
	    m_bReturnTokens = bReturnTokens;
        m_bPreserveDelimiters = bAllDelimitersValid;
        m_bLastTokenBlank = false;
        m_bFirstToken = true;
        m_bIgnoreQuoted = bIgnoreQuotedDelimiters;
    }

        /**
        * Destructor for a string tokenizer. Basically allows the automatic
        * destruction of all it's member values.
        */
    CStringTokenizer::~CStringTokenizer()
    {

    }

        /**
        * Skips delimiters.
        */
    void CStringTokenizer::SkipDelimiters( void )
    {
	    while ( ( !m_bReturnTokens ) &&
	            ( m_nCurrentPosition < m_nMaxPosition ) &&
	            (  m_strDelimiters.IndexOf( m_strWorkingString[ m_nCurrentPosition ] ) != IASLib::NOT_FOUND ) )
        {
            if ( ( m_bFirstToken ) && ( m_nCurrentPosition == 0 ) && ( m_bPreserveDelimiters ) )
            {
                m_bFirstToken = false;
                break;
            }
	        m_nCurrentPosition++;
            m_bFirstToken = false;
            if ( m_bPreserveDelimiters )
                break;

            m_bLastTokenBlank = false;
        }
    }

        /**
        * Tests if there are more tokens available from this tokenizer's string.
        * If this method returns <tt>true</tt>, then a subsequent call to
        * <tt>NextToken</tt> with no argument will successfully return a token.
        *
        * @return  <code>true</code> if and only if there is at least one token
        *          in the string after the current position; <code>false</code>
        *          otherwise.
        */
    bool CStringTokenizer::HasMoreTokens( void )
    {
        if ( ! m_bPreserveDelimiters )
        {
            SkipDelimiters();
            return ( ( m_nCurrentPosition < m_nMaxPosition ) ? true:false );
        }

        return ( ( ( m_nCurrentPosition + 1 ) < m_nMaxPosition ) ? true:false );
    }

        /**
        * Returns the next token from this string tokenizer.
        *
        * @return     the next token from this string tokenizer.
        * @exception  CException  if there are no more tokens in this
        *               tokenizer's string.
        */
    CString CStringTokenizer::NextToken( void )
    {
	    SkipDelimiters();
        bool inQuote = false;

        m_bFirstToken = false;

	    if (m_nCurrentPosition >= m_nMaxPosition)
        {
            return CString( "" );
            //throw CException( "No such element!", CException::NORMAL );
	    }

            // This is the start of our next token
	    size_t nStart = m_nCurrentPosition;

            // Search for the next delimiter. Skip all text in-between
	    while ( ( m_nCurrentPosition < m_nMaxPosition ) &&
          ( inQuote || ( m_strDelimiters.IndexOf( m_strWorkingString[ m_nCurrentPosition ] ) == IASLib::NOT_FOUND ) ) )
        {
            if ( m_bIgnoreQuoted && ( m_strWorkingString[m_nCurrentPosition] == '"' ) )
            {
                inQuote = ! inQuote;
            }
	        m_nCurrentPosition++;
	    }

            // If we're preserving delimiters, then we can't go any further,
            // because an empty string is a valid return.
        if ( ( m_bPreserveDelimiters ) && ( ! m_bLastTokenBlank ) && ( nStart == m_nCurrentPosition ) )
        {
            m_bLastTokenBlank = true;
            return CString( "" );
        }

        m_bLastTokenBlank = false;

            // If we are returning delimiters,
        if ( ( m_bReturnTokens ) &&
            ( nStart == m_nCurrentPosition ) &&
            ( m_strDelimiters.IndexOf( m_strWorkingString[ m_nCurrentPosition ] ) != IASLib::NOT_FOUND ) )
        {
	        m_nCurrentPosition++;
	    }

        return m_strWorkingString.Substring( nStart, (int)(m_nCurrentPosition - nStart) );
    }

        /**
        * Returns the next token in this string tokenizer's string. First,
        * the set of characters considered to be delimiters by this
        * <tt>CStringTokenizer</tt> object is changed to be the characters in
        * the string <tt>strDelimiters</tt>. Then the next token in the string
        * after the current position is returned. The current position is
        * advanced beyond the recognized token.  The new delimiter set
        * remains the default after this call.
        *
        * @param      strDelimiters   the new delimiters.
        * @return     the next token, after switching to the new delimiter set.
        * @exception  CException  if there are no more tokens in this
        *               tokenizer's string.
        */
    CString CStringTokenizer::NextToken( const char *strDelimiters )
    {
        m_strDelimiters = strDelimiters;
        return NextToken();
    }

        /**
        * Calculates the number of times that this tokenizer's
        * <code>NextToken</code> method can be called before it generates an
        * exception. The current position is not advanced.
        *
        * @return  the number of tokens remaining in the string using the current
        *          delimiter set.
        * @see     CStringTokenizer#NextToken()
        */
    int CStringTokenizer::CountTokens( void )
    {
	    int nCount = 0;
	    size_t nCurPos = m_nCurrentPosition;
        bool bLast = m_bLastTokenBlank;
        bool bSaveFirst = m_bFirstToken;

	    while ( m_nCurrentPosition < m_nMaxPosition)
        {
            NextToken();
            nCount++;
        }

            // Restore State
        m_nCurrentPosition = nCurPos;
        m_bLastTokenBlank = bLast;
        m_bFirstToken = bSaveFirst;

	    return nCount;
    }
} // namespace IASLib
