/*
 * CCString Class
 *
 *      This class provides an interface to a string class which mimics
 * an intrinsic string type for C++. This class handles memory management
 * and dynamic re-allocation of string storage and the like. It also
 * provides a very intrinsic acting interface for using strings.
 *      This class performs deep copies of the string every time it is
 * duplicated. Each instance of the class has its own copy of the string.
 * It is thus totally safe, but somewhat slow. If speed is of the essence,
 * then the CCString class should be used instead.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/01/1992
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "String_Copy.h"

#include <stdio.h>

#include <string>
#include <memory>
#include <iostream>

#ifndef IASLIB_WIN32__
#include <stdarg.h>
#else
#include <strsafe.h>
#endif

#include "StringException.h"

#if ( _MSC_VER >= 1300 )
#pragma warning( disable:4995 )
#endif

#ifdef IASLIB_STREAM_SUPPORT__
#if ( _MSC_VER >= 1300 )
#include <ostream>
#include <istream>
#else
#include <iostream.h>
#endif
#endif

#if ( _MSC_VER >= 1300 )
#pragma warning( default:4995 )
#endif

#ifdef IASLIB_MULTI_THREADED__
#include "Mutex.h"
#endif

#ifdef IASLIB_DEBUG_MEMORY__
#include <crtdbg.h>

#define malloc(x)   _malloc_dbg( x, _CLIENT_BLOCK, __FILE__, __LINE__ )
#define realloc(p,x)   _realloc_dbg( p,x, _CLIENT_BLOCK, __FILE__, __LINE__ )
#define calloc(n,x)   _calloc_dbg( n,x, _CLIENT_BLOCK, __FILE__, __LINE__ )
#define free(p)       _free_dbg(p,_CLIENT_BLOCK)
#endif

#ifdef IASLIB_MAC__

/*************************************************************************
*   stricmp Function                                                     *
*                                                                        *
*       Description:  This function is not included in the Macintosh     *
*                     standard run-time libraries, and must therefore be *
*                     defined within the program.                        *
*                                                                        *
*            Author:  Jeffrey R. Naujok                                  *
*              Date:  Tuesday, May 30, 1995                              *
*                                                                        *
*************************************************************************/

static int stricmp( char *pszString1, char *pszString2 )
{
  while ( ( *pszString1 ) && ( *pszString2 ) && ( toupper( *pszString1 ) == toupper( *pszString2 ) ) )
  {
    pszString1++;
    pszString2++;
  }
  if ( ( *pszString1 == '\0' ) && ( *pszString2 == '\0' ) )
    return 0;
    
  if ( toupper( *pszString1 ) < toupper( *pszString2 ) )
    return -1;
    
  return 1;
}

/*************************************************************************
*   strnicmp Function                                                    *
*                                                                        *
*       Description:  This function is not included in the Macintosh     *
*                     standard run-time libraries, and must therefore be *
*                     defined within the program.                        *
*                                                                        *
*            Author:  Jeffrey R. Naujok                                  *
*              Date:  Tuesday, May 30, 1995                              *
*                                                                        *
*************************************************************************/

static int strnicmp( char *pszString1, char *pszString2, size_t nLength )
{
  size_t nCount = nLength;
  
  while ( ( nCount ) && ( *pszString1 ) && ( *pszString2 ) && ( toupper( *pszString1 ) == toupper( *pszString2 ) ) )
  {
    pszString1++;
    pszString2++;
    nCount--;
  }
  
  if ( nCount == 0 )
    return 0;
    
  if ( ( *pszString1 == '\0' ) && ( *pszString2 == '\0' ) )
    return 0;
    
  if ( toupper( *pszString1 ) < toupper( *pszString2 ) )
    return -1;
    
  return 1;
}

#endif // ifdef IASLIB_MAC__

#ifdef IASLIB_THREADSAFE__
static CMutex  mutexStringProtect;
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CCString, CObject );

	const char *CCString::END_OF_TOKENS_MARKER = "**";

        // Default constructor. Creates a zero length string.
    CCString::CCString( void )
    {
        m_strData = NULL;
        m_nLength = 0;
    }

        // String Constructor - Takes a char * and converts
        // it to a CCString object.
    CCString::CCString( const char *strSource, int nLength )
    {
        if ( strSource == NULL )
        {
            m_nLength = 0;
            m_strData = NULL;
        }
        else
        {
            if ( nLength == -1 )
            {
                m_nLength = (int)strlen( strSource );
                if ( m_nLength > 0 )
                {
                    m_strData = (char *)malloc( m_nLength + 1 );
                    memcpy( m_strData, strSource, m_nLength );
                    m_strData[ m_nLength ] = 0;
                }
                else
                {
                    m_strData = NULL;
                    m_nLength = 0;
                }
            }
            else
            {
                int nStrLen = (int)strlen( strSource );
                if ( ( nLength > nStrLen ) || ( nLength < 0 ) )
                    nLength = nStrLen;
                m_nLength = nLength;
                if ( m_nLength > 0 )
                {
                    m_strData = (char *)malloc( m_nLength + 1 );
                    memcpy( m_strData, strSource, m_nLength );
                    m_strData[ m_nLength ] = 0;
                }
                else
                {
                    m_strData = NULL;
                    m_nLength = 0;
                }
            }
        }
    }

        // Copy Constructor - Takes another CCString object
        // and creates a new string object that's an exact
        // duplicate of the source object.
    CCString::CCString( const CCString &strSource )
    {
        m_nLength = strSource.m_nLength;
        if ( m_nLength > 0 )
        {
            m_strData = (char *)malloc( m_nLength + 1 );
            memcpy( m_strData, strSource.m_strData, m_nLength );
            m_strData[ m_nLength ] = 0;
        }
        else
        {
            m_strData = NULL;
            m_nLength = 0;
        }
    }

        // Destructor - Destroys a string obejct
    CCString::~CCString( void )
    {
        if ( m_strData )
            free( m_strData );
        m_strData = NULL;
        m_nLength = 0;
    }

                // Operator Methods

    CCString::operator const char *() const
    {
        if ( m_strData )
            return (const char *)m_strData;
        return (const char *)"";
    }

            // Concatenation Operators (+) - Combines two strings
    CCString operator +( const CCString &strSource1, const int nSource2 )
    {
        CCString strReturn( strSource1 );
        CCString strTemp;
        strTemp.Format( "%i", nSource2 );
        strReturn += strTemp;

        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CCString operator +( const int nSource1, const CCString &strSource2 )
    {
        CCString strTemp;
        strTemp.Format( "%i", nSource1 );
        CCString strReturn( strTemp );
        strReturn += strSource2;
        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CCString operator +( const CCString &strSource1, const long nSource2 )
    {
        CCString strReturn( strSource1 );
        CCString strTemp;
        strTemp.Format( "%ld", nSource2 );
        strReturn += strTemp;

        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CCString operator +( const long nSource1, const CCString &strSource2 )
    {
        CCString strTemp;
        strTemp.Format( "%ld", nSource1 );
        CCString strReturn( strTemp );
        strReturn += strSource2;
        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CCString operator +( const CCString &strSource1, const short nSource2 )
    {
        CCString strReturn( strSource1 );
        CCString strTemp;
        strTemp.Format( "%d", nSource2 );
        strReturn += strTemp;

        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CCString operator +( const short nSource1, const CCString &strSource2 )
    {
        CCString strTemp;
        strTemp.Format( "%d", nSource1 );
        CCString strReturn( strTemp );
        strReturn += strSource2;
        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CCString operator +( const CCString &strSource1, const CCString &strSource2 )
    {
        CCString strReturn( strSource1 );
        strReturn += strSource2;
        return strReturn;
    }

    CCString operator +( const CCString &strSource1, const char *strSource2 )
    {
        CCString strReturn( strSource1 );
        strReturn += strSource2;
        return strReturn;
    }

    CCString operator +( const char *strSource1, const CCString &strSource2 )
    {
        CCString strReturn( strSource1 );
        strReturn += strSource2;
        return strReturn;
    }


    CCString operator +( const CCString &strSource1, const char chSource2 )
    {
        CCString strReturn( strSource1 );
        strReturn += chSource2;
        return strReturn;
    }

    CCString operator +( const char chSource1, const CCString &strSource2 )
    {
        CCString strReturn;
        strReturn = chSource1;
        strReturn += strSource2;
        return strReturn;
    }

                    // Concatenate / Assign - Concatenates the string onto the
                    // current string and assigns the result
    CCString &CCString::operator +=( const CCString &strSource )
    {
        int nTemp = m_nLength + strSource.m_nLength;

        if ( nTemp )
        {
            char *strTemp = (char *)realloc( m_strData, nTemp + 1 );
            if ( strTemp )
            {
                m_strData = strTemp;
                memcpy( m_strData + m_nLength, strSource.m_strData, strSource.m_nLength );
                m_nLength = nTemp;
                m_strData[ m_nLength ] = 0;
            }
        }

        // The combined lengths could only be zero if we're concatenating two empty strings,
        // thus, they're still empty, and no change has occured.

        return *this;
    }

    CCString &CCString::operator +=( const char *strSource )
    {
        if ( strSource == NULL )
            return *this;

        int nLen = (int)strlen( strSource );
        int nTemp = m_nLength + nLen;
        if ( nTemp )
        {
            char *strTemp = (char *)realloc( m_strData, nTemp + 1 );
            if ( strTemp )
            {
                m_strData = strTemp;
                memcpy( m_strData + m_nLength, strSource, nLen );
                m_nLength = nTemp;
                m_strData[ m_nLength ] = 0;
            }
        }

        // The combined lengths could only be zero if we're concatenating two empty strings,
        // thus, they're still empty, and no change has occured.

        return *this;
    }

    CCString &CCString::operator +=( const char chSource )
    {
        if ( chSource == 0 )
        {
            return *this;
        }

        if ( m_nLength == 0 )
        {
            m_strData = (char *)malloc( 2 );
            if ( m_strData )
            {
                m_strData[0] = chSource;
                m_strData[1] = 0;
                m_nLength = 1;
            }
        }
        else
        {
            char *strTemp = (char *)realloc( m_strData, m_nLength + 2 );
            if ( strTemp )
            {
                m_strData = strTemp;
                m_strData[ m_nLength ] = chSource;
                m_nLength++;
                m_strData[ m_nLength ] = 0;
            }
        }

        return *this;
    }

    CCString &CCString::operator +=( const int nSource )
    {
        CCString strSource;
        strSource.Format( "%i", nSource );

        if ( m_nLength == 0 )
        {
            m_strData = (char *)malloc( strSource.m_nLength + 1 );
            if ( m_strData )
            {
                memcpy( m_strData, strSource.m_strData, strSource.m_nLength + 1 );
                m_nLength = strSource.m_nLength;
                m_strData[ m_nLength ] = 0;
            }
        }
        else
        {
            char *strTemp = (char *)realloc( m_strData, m_nLength + strSource.m_nLength + 1 );
            if ( strTemp )
            {
                m_strData = strTemp;
                memcpy( m_strData + m_nLength, strSource.m_strData, strSource.m_nLength );
            }
            m_nLength += strSource.m_nLength;
            m_strData[ m_nLength ] = 0;
        }

        return *this;
    }

    CCString &CCString::operator +=( const long nSource )
    {
        CCString strSource;
        strSource.Format( "%ld", nSource );

        if ( m_nLength == 0 )
        {
            m_strData = (char *)malloc( strSource.m_nLength + 1 );
            if ( m_strData )
            {
                memcpy( m_strData, strSource.m_strData, strSource.m_nLength + 1 );
                m_nLength = strSource.m_nLength;
                m_strData[ m_nLength ] = 0;
            }
        }
        else
        {
            char *strTemp = (char *)realloc( m_strData, m_nLength + strSource.m_nLength + 1 );
            if ( strTemp )
            {
                m_strData = strTemp;
                memcpy( m_strData + m_nLength, strSource.m_strData, strSource.m_nLength );
            }
            m_nLength += strSource.m_nLength;
            m_strData[ m_nLength ] = 0;
        }

        return *this;
    }

    CCString &CCString::operator +=( const short nSource )
    {
        CCString strSource;
        strSource.Format( "%d", nSource );

        if ( m_nLength == 0 )
        {
            m_strData = (char *)malloc( strSource.m_nLength + 1 );
            if ( m_strData )
            {
                memcpy( m_strData, strSource.m_strData, strSource.m_nLength + 1 );
                m_nLength = strSource.m_nLength;
                m_strData[ m_nLength ] = 0;
            }
        }
        else
        {
            char *strTemp = (char *)realloc( m_strData, m_nLength + strSource.m_nLength + 1 );
            if ( strTemp )
            {
                m_strData = strTemp;
                memcpy( m_strData + m_nLength, strSource.m_strData, strSource.m_nLength );
            }
            m_nLength += strSource.m_nLength;
            m_strData[ m_nLength ] = 0;
        }

        return *this;
    }

                    // Assignment Operators (=) - Assigns a string
    CCString &CCString::operator =( const CCString &strSource )
    {
        if ( this != &strSource )
        {
            if ( strSource.m_nLength )
            {
                char *strTemp = (char *)realloc( m_strData, strSource.m_nLength + 1 );
                
                if ( strTemp )
                {
                    m_strData = strTemp;
                    m_nLength = strSource.m_nLength;
                    memcpy( m_strData, strSource.m_strData, m_nLength );
                    m_strData[ m_nLength ] = 0;
                }
            }
            else
            {
                if ( m_strData )
                {
                    free( m_strData );
                    m_strData = NULL;
                    m_nLength = 0;
                }
            }
        }

        return *this;
    }

    CCString &CCString::operator =( const char *strSource )
    {
        if ( strSource != m_strData )
        {
            if ( ( strSource == NULL ) || ( ( m_nLength = (int)strlen( strSource ) ) == 0 ) )
            {
                if ( m_strData )
                    free( m_strData );
                m_nLength = 0;
                m_strData = NULL;
            }
            else
            {
                char *strTemp = (char *)realloc( m_strData, m_nLength + 1 );
                if ( strTemp )
                {
                    m_strData = strTemp;
                    memcpy( m_strData, strSource, m_nLength + 1 );
                    m_strData[ m_nLength ] = 0;
                }
            }
        }

        return *this;
    }

    CCString &CCString::operator =( const char chSource )
    {
        if ( chSource != '\0' )
        {
            char *strTemp = (char *)realloc( m_strData, 2 );
            if ( strTemp )
            {
                m_strData = strTemp;
                m_strData[ 0 ] = chSource;
                m_strData[ 1 ] = '\0';
                m_nLength = 1;
            }
        }
        else
        {
            m_nLength = 0;
            if ( m_strData )
                free( m_strData );
            m_strData = NULL;
        }

        return *this;
    }

    CCString &CCString::operator =( const int nSource )
    {
        CCString strSource;
        strSource.Format( "%i", nSource );

        m_nLength = strSource.m_nLength;
        char *strTemp = (char *)realloc( m_strData, m_nLength + 1 );
        if ( strTemp )
        {
            m_strData = strTemp;
            memcpy( m_strData, strSource.m_strData, m_nLength + 1 );
            m_strData[ m_nLength ] = 0;
        }
        return *this;
    }

    CCString &CCString::operator =( const long nSource )
    {
        CCString strSource;
        strSource.Format( "%ld", nSource );

        m_nLength = strSource.m_nLength;
        char *strTemp = (char *)realloc( m_strData, m_nLength + 1 );
        if ( strTemp )
        {
            m_strData = strTemp;
            memcpy( m_strData, strSource.m_strData, m_nLength + 1 );
            m_strData[ m_nLength ] = 0;
        }
        return *this;
    }

    CCString &CCString::operator =( const short nSource )
    {
        CCString strSource;
        strSource.Format( "%d", nSource );

        m_nLength = strSource.m_nLength;
        char *strTemp = (char *)realloc( m_strData, m_nLength + 1 );
        if ( strTemp )
        {
            m_strData = strTemp;
            memcpy( m_strData, strSource.m_strData, m_nLength + 1 );
            m_strData[ m_nLength ] = 0;
        }
        return *this;
    }

                            // Equality Operators (==) - Compare a string
    bool CCString::operator ==( const CCString &strString )
    {
        if ( strcmp( (const char *)(*this), (const char *)strString ) == 0 )
            return true;
        return false;
    }

    bool CCString::operator ==( const char *strString2 )
    {
        if ( strcmp( (const char *)(*this), strString2 ) == 0 )
            return true;
        return false;
    }

    bool operator ==( char *strString1, const CCString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) == 0 )
            return true;
        return false;
    }


                            // Inequality Operators (!=) - Compare a string
    bool CCString::operator !=( const CCString &strString )
    {
        if ( strcmp( (const char *)(*this), (const char *)strString ) == 0 )
            return false;
        return true;
    }

    bool CCString::operator !=( const char *strString2 )
    {
        if ( strcmp( (const char *)(*this), strString2 ) != 0 )
            return true;
        return false;
    }

    bool operator !=( char *strString1, const CCString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) != 0 )
            return true;
        return false;
    }


                            // Greater than / Equal To Operators (>=) - Compare a string
    bool CCString::operator >=( const CCString &strString )
    {
        if ( m_strData )
        {
            if ( strcmp( m_strData, strString ) >= 0 )
                return true;
        }
        else
        {
            if ( strString.m_nLength == 0 )
                return true;
        }
        return false;
    }

    bool CCString::operator >=( const char *strString2 )
    {
        if ( strcmp( (const char *)(*this), strString2 ) >= 0 )
            return true;
        return false;
    }

    bool operator >=( char *strString1, const CCString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) >= 0 )
            return true;
        return false;
    }

                            // Less than / Equal To Operators (<=) - Compare a string
    bool CCString::operator <=( const CCString &strString )
    {
        if ( m_strData )
        {
            if ( strcmp( m_strData, strString ) <= 0 )
                return true;
        }
        else
        {
            if ( strString.m_nLength == 0 )
                return true;
        }
        return false;
    }

    bool CCString::operator <=( const char *strString2 )
    {
        if ( strcmp( (const char *)(*this), strString2 ) <= 0 )
            return true;
        return false;
    }

    bool operator <=( char *strString1, const CCString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) <= 0 )
            return true;
        return false;
    }

                            // Greater than Operators (>) - Compare a string
    bool CCString::operator >( const CCString &strString )
    {
        if ( m_strData )
        {
            if ( strcmp( m_strData, strString ) > 0 )
                return true;
        }
        return false;
    }

    bool CCString::operator >( const char *strString2 )
    {
        if ( strcmp( (const char *)(*this), strString2 ) > 0 )
            return true;
        return false;
    }

    bool operator >( char *strString1, const CCString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) > 0 )
            return true;
        return false;
    }


                            // Less than Operators (<) - Compare a string
    bool CCString::operator <( const CCString &strString )
    {
        if ( m_strData )
        {
            if ( strcmp( m_strData, strString ) < 0 )
                return true;
        }
        return false;
    }

    bool CCString::operator <( const char *strString2 )
    {
        if ( strcmp( (const char *)(*this), strString2 ) < 0 )
            return true;
        return false;
    }

    bool operator <( char *strString1, const CCString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) < 0 )
            return true;
        return false;
    }


            // Lexical Comparison - Compares two strings and returns
            // 0 if equal, -1 if this is less than the comparison, and
            // +1 if this is greater than the comparison.
    int CCString::Compare( const CCString &strCompare, bool bCaseSensitive ) const
    {
        if ( bCaseSensitive )
        {
            if ( m_strData )
            {
                return strcmp( m_strData, strCompare );
            }
            else
            {
                if ( strCompare.m_nLength == 0 )
                    return 0;

                    // A Zero length string is always less than any non-zero length string
                return -1;
            }
        }
        else
        {
            CCString strTemp1 = *this;
            CCString strTemp2 = strCompare;
            strTemp1.ToUpperCase();
            strTemp2.ToUpperCase();

            if ( strTemp1.m_strData )
            {
                return strcmp( strTemp1.m_strData, strTemp2 );
            }
            else
            {
                if ( strTemp2.m_nLength == 0 )
                    return 0;

                    // A Zero length string is always less than any non-zero length string
                return -1;
            }
        }
    }

    int CCString::Compare( const char *strCompare, bool bCaseSensitive ) const
    {
        if ( bCaseSensitive )
        {
            if ( m_strData )
            {
                return strcmp( m_strData, strCompare );
            }
            else
            {
                if ( strlen( strCompare ) == 0 )
                    return 0;

                    // A Zero length string is always less than any non-zero length string
                return -1;
            }
        }
        else
        {
            CCString strTemp1 = *this;
            CCString strTemp2 = strCompare;
            strTemp1.ToUpperCase();
            strTemp2.ToUpperCase();

            if ( strTemp1.m_strData )
            {
                return strcmp( strTemp1.m_strData, strTemp2 );
            }
            else
            {
                if ( strTemp2.m_nLength == 0 )
                    return 0;

                    // A Zero length string is always less than any non-zero length string
                return -1;
            }
        }
    }

            // Array Access Operator - Allows us to access the characters
            // of the string as if they were a char[] buffer.
    char &CCString::operator []( int nIndex ) const
    {
        static char chDummy;

        if ( ( nIndex >= 0 ) && ( nIndex < m_nLength ) )
            return m_strData[ nIndex ];

        chDummy = '\0';
        return chDummy;
    }

                // Standard Methods

            // Get a sub-string of the string
    CCString CCString::Substring( int nStartIndex, int nLength ) const
    {
        int nFixedLength;

        if ( ( nStartIndex >= 0 ) && ( nStartIndex < m_nLength ) )
        {
            if ( ( nLength == - 1 ) || ( nLength > ( m_nLength - nStartIndex ) ) )
            {
                nFixedLength = m_nLength - nStartIndex;
            }
            else
            {
                nFixedLength = nLength;
            }

            CCString strReturn( m_strData + nStartIndex, nFixedLength );
            return strReturn;
        }

        CCString strEmpty;

        return strEmpty;
    }

            // Convert a string to upper case
    void CCString::ToUpperCase( void )
    {
        int nCount;

        for (nCount = 0; nCount < m_nLength; nCount++ )
        {
            if ( islower( m_strData[ nCount ] ) )
            {
                m_strData[ nCount ] = (char)toupper( m_strData[ nCount ] );
            }
        }
    }

            // Convert a string to lower case
    void CCString::ToLowerCase( void )
    {
        int nCount;

        for (nCount = 0; nCount < m_nLength; nCount++ )
        {
            if ( isupper( m_strData[ nCount ] ) )
            {
                m_strData[ nCount ] = (char)tolower( m_strData[ nCount ] );
            }
        }
    }

            // Trim leading/trailing white-space from a string
    CCString &CCString::Trim( const char *pszWhiteSpace )
    {
        CCString strWhiteSpace = pszWhiteSpace;

        int nCount;

        for (nCount = 0; ( nCount < 65535 ) && ( nCount < m_nLength ) && ( strWhiteSpace.IndexOf( m_strData[ nCount ] ) != -1 ); nCount++ );

        if ( nCount != 0 )
        {
            *this = Substring( nCount );
        }

        if ( m_nLength == 0 )
            return *this;

        for ( nCount = m_nLength - 1;( nCount >= 0 ) && ( strWhiteSpace.IndexOf( m_strData[ nCount ] ) != -1 ); nCount-- );

        if ( nCount != (m_nLength - 1) )
        {
            *this = Substring( 0, nCount + 1 );
        }

        return *this;
    }

            // Assign this string to a printf formatted string
    const char *CCString::Format( const char *strFormat, ... )
    {
#ifdef IASLIB_THREADSAFE__
		mutexStringProtect.Lock();
#endif
        va_list       vaArgList;
        static char   szBuffer[65536];

        /* format buf using fmt and arguments contained in ap */
        va_start( vaArgList, strFormat );
#ifdef IASLIB_WIN32__
        vsprintf_s( szBuffer, 65536, strFormat, vaArgList );
#else
        vsprintf( szBuffer, strFormat, vaArgList );
#endif
        va_end( vaArgList );

        m_nLength = (int)strlen( szBuffer );
        m_strData = (char *)realloc( m_strData, m_nLength + 1 );
        memcpy( m_strData, szBuffer, m_nLength + 1 );
        m_strData[ m_nLength ] = 0;

#ifdef IASLIB_THREADSAFE__
        mutexStringProtect.Unlock();
#endif

        return m_strData;
    }
                
            // Find the occurance of a substring in a string
    int CCString::IndexOf( const CCString &strSearch, int nStart, bool bCaseInsensitive ) const
    {
        return IndexOf( (const char *)strSearch, nStart, bCaseInsensitive );
    }

    int CCString::IndexOf( const char *strSearch, int nStart, bool bCaseInsensitive ) const
    {
        int     nReturn = -1;

        if ( m_strData )
        {
            if ( nStart < 0 )
            {
                nStart = 0;
            }

            if ( nStart > m_nLength )
            {
                nStart = m_nLength;
            }

            char *pFound;

            if ( bCaseInsensitive )
            {
                CCString strCopy = m_strData;
                CCString strSearchCopy = strSearch;

                strCopy.ToLowerCase();
                strSearchCopy.ToLowerCase();
                pFound = strstr( &(strCopy.m_strData[nStart] ), (const char *)strSearchCopy );
                if ( pFound != NULL )
                {
                    nReturn = (int)(pFound - strCopy.m_strData);
                }
            }
            else
            {
                pFound = strstr( &(m_strData[nStart] ), strSearch );        
                if ( pFound != NULL )
                {
                    nReturn = (int)(pFound - m_strData);
                }
            }
        }

        return nReturn;
    }

    int CCString::IndexOf( const char chSearch, int nStart, bool bCaseInsensitive ) const
    {
        int nCount;
        char chMatch;

        if ( nStart < 0 )
            nStart = 0;

        if ( bCaseInsensitive )
        {
            chMatch = (char)toupper( chSearch );

            for ( nCount = nStart; ( nCount < m_nLength ) && ( toupper( m_strData[ nCount ] ) != chMatch ); nCount++ );
            if ( nCount == m_nLength )
                nCount = -1;
        }
        else
        {
            for ( nCount = nStart; ( nCount < m_nLength ) && ( m_strData[ nCount ] != chSearch ); nCount++ );
            if ( nCount == m_nLength )
                nCount = -1;
        }

        return nCount;
    }

    int CCString::LastIndexOf( const CCString &strSearch, int nStart, bool bCaseInsensitive ) const
    {
        return LastIndexOf( (const char *)strSearch, nStart, bCaseInsensitive );
    }

    int CCString::LastIndexOf( const char *strSearch, int nStart,  bool bCaseInsensitive ) const
    {
        int     nReturn = -1;
        char   *pLast;
        char   *pFound = m_strData;
        int     pPos = -1;

        if ( ( nStart > m_nLength ) || ( nStart < 0 ) )
        {
            nStart = m_nLength;
        }

        if ( m_strData )
        {

            if ( bCaseInsensitive )
            {
                CCString strCopy = m_strData;
                CCString strSearchCopy = strSearch;
                strCopy.ToLowerCase();
                strSearchCopy.ToLowerCase();
                pFound = strCopy.m_strData;
                do
                {
                    pLast = pFound;
                    pFound = strstr( pLast + 1, (const char *)strSearchCopy );

                    if ( pFound )
                        pPos = (int)(pFound - strCopy.m_strData);

                } while ( ( pFound ) && ( pPos < nStart ) );

                if ( pLast != strCopy.m_strData )
                {
                    nReturn = (int)(pLast - strCopy.m_strData);
                }
            }
            else
            {
                do
                {
                    pLast = pFound;
                    pFound = strstr( pLast + 1, strSearch );

                    if ( pFound )
                        pPos = (int)(pFound - m_strData);

                } while ( ( pFound ) && ( pPos < nStart ) );

                if ( pLast != m_strData )
                {
                    nReturn = (int)(pLast - m_strData);
                }
            }
        }

        return nReturn;
    }

    int CCString::LastIndexOf( const char chSearch, int nStart,  bool bCaseInsensitive ) const
    {
        int nCount;

        if ( ( nStart > m_nLength ) || ( nStart < 0 ) )
        {
            nStart = m_nLength;
        }

        for ( nCount = nStart; nCount >= 0; nCount-- )
        {
            if ( bCaseInsensitive )
            {
                if ( ( m_strData[ nCount ] == chSearch ) || ( ( toupper( m_strData[ nCount ] ) == toupper( chSearch ) ) ) )
                    return nCount;
            }
            else
            {
                if ( m_strData[ nCount ] == chSearch )
                    return nCount;
            }
        }

        return -1;
    }

#ifdef IASLIB_STREAM_SUPPORT__

                // Stream Methods

                            // Output a string to a stream derived object.
    ostream  &operator <<( ostream &oOutputStream, const CCString &strSource )
    {
        int nCount;
        long nOutputLength = strSource.m_nLength;

        oOutputStream << nOutputLength;
        for ( nCount = 0; nCount < strSource.m_nLength; nCount++ )
        {
            oOutputStream << strSource.m_strData[ nCount ];
        }

        return oOutputStream;
    }

                            // Input a string from a stream derived object.
    istream  &operator >>( istream &oInputStream, CCString &strTarget )
    {
        int nCount;
        long nInputLength;

        oInputStream >> nInputLength;

        strTarget.m_strData = (char *)realloc( strTarget.m_strData, nInputLength + 1 );
        strTarget.m_nLength = (int)nInputLength;

        for ( nCount = 0; nCount < strTarget.m_nLength; nCount++ )
        {
            oInputStream >> strTarget.m_strData[ nCount ];
        }

        strTarget.m_strData[ strTarget.m_nLength ] = '\0';

        return oInputStream;
    }
#endif // IASLIB_STREAM_SUPPORT__

    void CCString::Unescape( void )
    {
        // Regular expression: s/%([0-9A-Fa-f]{2})/chr(hex($1))/eg;

        char   *strRetVal = (char *)malloc( m_nLength + 1 ); // This is safe because unescape can never make the string longer
        int     nCount = 0;
        int     nInsert = 0;
        char    strHexCode[5];
        char    nResolved = 0;
        strHexCode[0] = '0';
        strHexCode[1] = 'x';
        strHexCode[4] = 0;

        while ( nCount < m_nLength )
        {
            if ( m_strData[ nCount ] == '%' )
            {
                if ( m_strData[ nCount + 1 ] == '%' )
                {
                    strRetVal[ nInsert ] = '%';
                    nInsert++;
                    nCount += 2;
                    continue;
                }
                else
                {
                    strHexCode[2] = m_strData[ nCount + 1];
                    strHexCode[3] = m_strData[ nCount + 2];

                    nResolved = (char)strtol(strHexCode, NULL, 0 );

                    strRetVal[ nInsert ] = nResolved;
                    nCount += 3;
                    nInsert++;
                    continue;
                }
            }
            else
            {
                if ( m_strData[ nCount ] == '+' )
                {
                    strRetVal[ nInsert ] = ' ';
                    nInsert++;
                    nCount++;
                }
                else
                {
                    strRetVal[ nInsert ] = m_strData[ nCount ];
                    nInsert++;
                    nCount++;
                }
            }
        }
        strRetVal[ nInsert ] = 0;
        m_nLength = nInsert;
        m_strData = (char *)realloc( m_strData, m_nLength + 1 );
        memcpy( m_strData, strRetVal, m_nLength + 1 );
        m_strData[ m_nLength ] = 0;
        free( strRetVal );
    }

    void CCString::Escape( void )
    {
        CCString strEscapedVersion;
        CCString strHexChars = "0123456789ABCDEF";
        CCString strEscapableChars = "\"<>#%{}|\\~^[]`";
        int     nCount = 0;

        if ( m_strData )
        {
            for ( nCount = 0; nCount < m_nLength ; nCount++ )
            {
                if ( ( m_strData[ nCount ] < (char)32 ) || ( m_strData[nCount] > 126 ) || ( strEscapableChars.IndexOf( m_strData[ nCount ] ) >= 0 ) )
                {
                    strEscapedVersion += "%";
                    strEscapedVersion += strHexChars[ m_strData[nCount] / 16 ];
                    strEscapedVersion += strHexChars[ m_strData[nCount] % 16 ];
                }
                else
                {
                        // Space is a special case
                    if ( m_strData[ nCount ] == ' ' )
                    {
                        strEscapedVersion += '+';
                    }
                    else
                    {
                        strEscapedVersion += m_strData[ nCount ];
                    }
                }
            }

            *this = strEscapedVersion;
        }
    }

    void CCString::Replace( const char chReplace, const char chTo )
    {
        for ( int nCount = 0; nCount < m_nLength ; nCount++ )
        {
            if ( m_strData[ nCount ] == chReplace )
                m_strData[ nCount ] = chTo;
        }
    }

    void CCString::Replace( const char *strReplace, const char *strTo )
    {
        int nReplacePoint;
        int nReplaceLength = (int)strlen( strReplace );
        CCString strTemp;

        while ( ( nReplacePoint = IndexOf( strReplace ) ) != -1 )
        {
            strTemp = Substring( 0, nReplacePoint );
            strTemp += strTo;
            strTemp += Substring( nReplacePoint + nReplaceLength );
            *this = strTemp;
        }
    }

    void CCString::Remove( const char chRemove )
    {
        int nCount = 0;
        CCString strTemp;

        while ( nCount < m_nLength )
        {
            if ( m_strData[ nCount ] == chRemove )
            {
                strTemp = Substring( 0, nCount );
                strTemp += Substring( nCount + 1 );
                *this = strTemp;
            }
            else
            {
                nCount++;
            }
        }
    }

    void CCString::Remove( const char *strRemove )
    {
        int nRemovePoint;
        int nRemoveLength = (int)strlen( strRemove );
        CCString strTemp;

        while ( ( nRemovePoint = IndexOf( strRemove ) ) != -1 )
        {
            strTemp = Substring( 0, nRemovePoint );
            strTemp += Substring( nRemovePoint + nRemoveLength );
            *this = strTemp;
        }
    }


    bool CCString::WildcardCompare( const char *strPattern, bool bCaseSensitive )
    {
        if ( strcmp( strPattern, "*" ) == 0 )
            return true;

        if ( strlen( strPattern ) == 0 )
            return true;

        if ( m_nLength == 0 )
            return false;

        int     nCount = 0;
        int     nPattern = 0;
        CCString strSubString;
        CCString strSubPattern;
        int     nTemp;
        bool    bMatched;
        int     nPatternSize;
        int     nMinimumSize;

        while ( ( nCount < m_nLength ) && ( nPattern < (int)strlen( strPattern ) ) )
        {
            switch ( strPattern[nPattern] )
            {
                case '*':
                    nTemp = nCount;
                    bMatched = false;

                    nPattern++;
                    strSubPattern = strPattern;
                    strSubPattern = strSubPattern.Substring( nPattern );
                    nPatternSize = (int)strlen( strPattern ) - nPattern;
                    nMinimumSize = m_nLength - nPatternSize;
                    while ( nTemp <= nMinimumSize )
                    {
                        strSubString = Substring( nTemp );
                        if ( strSubString.WildcardCompare( strSubPattern, bCaseSensitive ) )
                        {
                            bMatched = true;
                            break;
                        }
                        nTemp++;
                    }
                    return bMatched;

                case '?':
                        // Skip the next character
                    nCount++;
                    nPattern++;
                    break;
                    
                default:
                    if ( bCaseSensitive )
                    {
                        if ( m_strData[ nCount ] != strPattern[ nPattern ] )
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if ( toupper( m_strData[ nCount ] ) != toupper( strPattern[ nPattern ] ) )
                        {
                            return false;
                        }
                    }
                    nCount++;
                    nPattern++;
                    break;
            }
        }

        if ( ( nCount == m_nLength ) && ( nPattern < (int)strlen( strPattern ) ) )
        {
            return false;
        }

        if ( nCount < m_nLength )
        {
            return false;
        }

        return true;
    }

    void CCString::OracleEscape( void )
    {
        CCString strTemp;
        int     nCount;

        for ( nCount = 0; nCount < m_nLength ; nCount++ )
        {
            if ( m_strData[nCount] == '\'')
            {
                strTemp += '\'';
            }

            strTemp += m_strData[nCount];
        }

        *this = strTemp;
    }

    void CCString::OracleUnescape( void )
    {
        CCString strTemp;
        int     nCount;

        for ( nCount = 0; nCount < m_nLength ; nCount++ )
        {
            if ( ( m_strData[nCount] == '\'') && ( m_strData[nCount + 1] == '\'' ) )
            {
                nCount++;
            }

            strTemp += m_strData[nCount];
        }

        *this = strTemp;
    }

    CCString CCString::Pad( int nLength, char chPadChar, bool bLeftAlign )
    {
        int     nRemain = nLength - m_nLength;
        char   *achPadding;
        CCString strRetVal;

        if ( nRemain > 0 )
        {
            achPadding = new char[ nRemain + 1 ];
            memset( achPadding, chPadChar, nRemain );
            achPadding[ nRemain ] = 0;

            if ( bLeftAlign )
            {
                strRetVal = m_strData;
                strRetVal += achPadding;
            }
            else
            {
                strRetVal = achPadding;
                strRetVal += m_strData;
            }
        }
        else
        {
            strRetVal = m_strData;
        }

        return strRetVal;
    }

    bool  CCString::Contains (const CCString &strSearch) const
    {
        bool bRetVal = false;

        if ( IndexOf( strSearch) >= 0 ) 
            bRetVal = true;
        
        return bRetVal;
    }

    bool  CCString::Contains (const char *strSearch) const
    {
        bool bRetVal = false;

        if ( IndexOf( strSearch) >= 0 ) 
            bRetVal = true;
        
        return bRetVal;
    }

    bool  CCString::Contains (const char chSearch) const
    {
        bool bRetVal = false;

        if ( IndexOf( chSearch) >= 0 ) 
            bRetVal = true;
        
        return bRetVal;
    }

    int CCString::ConvertToToken( const char *strToken, const char *astrTokenList[], int nTokenList[], int nTokens, bool bCaseSensitive, bool bUseWildcards )
    {
        int nCount = 0;
        int nRetVal = -9999;
        bool bDone = false;

        if ( nTokens == -1 )
        {
            nTokens = 65535;
        }


        CCString strCompare = strToken;
        CCString strWork;

        do
        {
            strWork = astrTokenList[ nCount ];

                // Look for the end code in the token list
            if ( strWork == END_OF_TOKENS_MARKER )
            {
                bDone = true;
                nRetVal = nTokenList[ nCount ];
            }
            else
            {
                if ( bUseWildcards )
                {
                    if ( strCompare.WildcardCompare( strWork, bCaseSensitive ) )
                    {
                        bDone = true;
                        nRetVal = nTokenList[ nCount ];
                    }
                }
                else
                {
                    if ( strCompare.Compare( strWork, bCaseSensitive ) == 0 )
                    {
                        bDone = true;
                        nRetVal = nTokenList[ nCount ];
                    }
                }
            }

            nCount++;
        } while ( ( ! bDone ) && ( nCount < nTokens ) );

        return nRetVal;
    }

    void CCString::Copy( const char *strSource, int nLength )
    {
        if ( ( strSource == NULL ) || ( nLength == 0 ) )
        {
            if ( m_strData )
                free( m_strData );
            m_nLength = 0;
            m_strData = NULL;
        }
        else
        {
            m_nLength = nLength;
            char *strTemp = (char *)realloc( m_strData, m_nLength + 1 );
            if ( strTemp )
            {
                m_strData = strTemp;
                memcpy( m_strData, strSource, m_nLength + 1 );
                m_strData[ m_nLength ] = 0;
            }
        }
    }

} // namespace IASLib
