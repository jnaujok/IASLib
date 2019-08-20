/*
 * CString Class
 *
 *      This class provides an interface to a string class which mimics
 * an intrinsic string type for C++. This class handles memory management
 * and dynamic re-allocation of string storage and the like. It also
 * provides a very intrinsic acting interface for using strings.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/02/1992
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "String_.h"

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

#ifdef IASLIB_STD_STREAM_SUPPORT__
    #ifdef IASLIB_WIN32__
        #if ( _MSC_VER >= 1300 )
            #include <ostream>
            #include <istream>
        #else
            #include <iostream.h>
        #endif
    #else
        #include <iostream>
    #endif
#endif

#if ( _MSC_VER >= 1300 )
#pragma warning( default:4995 )
#endif

#ifdef IASLIB_MULTI_THREADED__
#pragma message( "Multi-Threaded Set" )
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

static int stricmp( IASLibChar__ *pszString1, IASLibChar__ *pszString2 )
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

static int strnicmp( IASLibChar__ *pszString1, IASLibChar__ *pszString2, size_t nLength )
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

#ifdef IASLIB_MULTI_THREADED__
static IASLib::CMutex  mutexStringProtect;
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CString, CObject );

        // Default constructor. Creates a zero length string.
    CString::CString( void )
    {
        m_pStubData = NULL;
    }

        // String Constructor - Takes a char * and converts
        // it to a CString object.
    CString::CString( const IASLibChar__ *strSource, int nLength )
    {
        if ( ( strSource == NULL ) || ( nLength == 0 ) )
        {
            m_pStubData = NULL;
        }
        else
        {
            size_t nStrLen = strlen( strSource );
            if ( ( nLength > (int)nStrLen ) || ( nLength < 0 ) )
                nLength = (int)nStrLen;

            if ( nLength > 0 )
            {
                m_pStubData = new CStringStub( strSource, nLength );
                m_pStubData->AddRef();
            }
            else
            {
                m_pStubData = NULL;
            }
        }
    }

        // String Constructor - Takes a char * and converts
        // it to a CString object.
    CString::CString( const IASLibChar__ *strSource, size_t nLength )
    {
        if ( ( strSource == NULL ) || ( nLength == 0 ) )
        {
            m_pStubData = NULL;
        }
        else
        {
            size_t nStrLen = strlen( strSource );
            if ( nLength > nStrLen )
                nLength = nStrLen;

            if ( nLength > 0 )
            {
                m_pStubData = new CStringStub( strSource, nLength );
                m_pStubData->AddRef();
            }
            else
            {
                m_pStubData = NULL;
            }
        }
    }

        // String Constructor - Takes a fixed memory buffer
        //  with a maximum size and creates a non-movable
        //  fixed StringStub that points at it. The fixed
        //  CString violates a lot of the rules for other
        //  string types, in that no matter how many people
        //  have a copy, it never spawns a new string stub
        //  and everyone points at the same string. So if
        //  it changes, everyone's copy changes.
        //  Note: if you pass NULL, then this constructor
        //  will create a fixed stub of the size specified.
    CString::CString( IASLibChar__ *strSource, int nLength, int nSize )
    {
        if ( nLength > nSize )
        {
            nLength = nSize;
        }

        if ( strSource == NULL )
        {
            IASLibChar__ *strData;
#ifdef IASLIB_MEMORY_MANAGER__
    #ifdef IASLIB_DEBUG__
            strData = (IASLibChar__ *)CMemoryManager::AllocateDebug( nSize + 1, __FILE__, __LINE__ );
    #else
            strData = (IASLibChar__ *)CMemoryManager::Allocate( nSize + 1 );
    #endif
#else
            strData = (IASLibChar__ *)malloc( nSize + 1 );
#endif
            memset( strData, 0, nLength );
            m_pStubData = new CStringStub( strData, nLength, nSize, true );
            m_pStubData->AddRef();
        }
        else
        {
            size_t nStrLen = strlen( strSource );
            if ( ( nLength > (int)nStrLen ) || ( nLength < 0 ) )
                nLength = (int)nStrLen;

            if ( nLength > nSize )
                nLength = nSize;

            m_pStubData = new CStringStub( strSource, nLength, nSize );
            m_pStubData->AddRef();
        }
    }

        // Copy Constructor - Takes another CString object
        // and creates a new string object that's an exact
        // duplicate of the source object.
    CString::CString( const CString &strSource )
    {
        m_pStubData = strSource.m_pStubData;
        if ( m_pStubData )
            m_pStubData->AddRef();
    }


        // Destructor - Destroys a string obejct
    CString::~CString( void )
    {
        if ( m_pStubData )
            m_pStubData->RemoveRef();
    }

                // Operator Methods

    CString::operator const IASLibChar__ *() const
    {
        if ( m_pStubData )
            return (const IASLibChar__ *)m_pStubData->m_strData;
        return (const IASLibChar__ *)"";
    }

            // Concatenation Operators (+) - Combines two strings
    CString operator +( const CString &strSource1, const int nSource2 )
    {
        CString strReturn( (const IASLibChar__ *)strSource1 );
        CString strTemp;
        strTemp.Format( "%i", nSource2 );
        strReturn += strTemp;

        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CString operator +( const int nSource1, const CString &strSource2 )
    {
        CString strTemp;
        strTemp.Format( "%i", nSource1 );
        CString strReturn( strTemp );
        strReturn += strSource2;
        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CString operator +( const CString &strSource1, const long nSource2 )
    {
        CString strReturn( (const IASLibChar__ *)strSource1 );
        CString strTemp;
        strTemp.Format( "%ld", nSource2 );
        strReturn += strTemp;

        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CString operator +( const long nSource1, const CString &strSource2 )
    {
        CString strTemp;
        strTemp.Format( "%ld", nSource1 );
        CString strReturn( strTemp );
        strReturn += strSource2;
        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CString operator +( const CString &strSource1, const short nSource2 )
    {
        CString strReturn( (const IASLibChar__ *)strSource1 );
        CString strTemp;
        strTemp.Format( "%d", nSource2 );
        strReturn += strTemp;

        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CString operator +( const short nSource1, const CString &strSource2 )
    {
        CString strTemp;
        strTemp.Format( "%d", nSource1 );
        CString strReturn( strTemp );
        strReturn += strSource2;
        return strReturn;
    }

            // Concatenation Operators (+) - Combines two strings
    CString operator +( const CString &strSource1, const CString &strSource2 )
    {
        CString strReturn( (const IASLibChar__ *)strSource1 );
        strReturn += strSource2;
        return strReturn;
    }

    CString operator +( const CString &strSource1, const IASLibChar__ *strSource2 )
    {
        CString strReturn( (const IASLibChar__ *)strSource1 );
        strReturn += strSource2;
        return strReturn;
    }

    CString operator +( const IASLibChar__ *strSource1, const CString &strSource2 )
    {
        CString strReturn( strSource1 );
        strReturn += strSource2;
        return strReturn;
    }


    CString operator +( const CString &strSource1, const IASLibChar__ chSource2 )
    {
        CString strReturn( (const IASLibChar__ *)strSource1 );
        strReturn += chSource2;
        return strReturn;
    }

    CString operator +( const IASLibChar__ chSource1, const CString &strSource2 )
    {
        CString strReturn;
        strReturn = chSource1;
        strReturn += strSource2;
        return strReturn;
    }

                    // Concatenate / Assign - Concatenates the string onto the
                    // current string and assigns the result
    CString &CString::operator +=( const CString &strSource )
    {
        if ( strSource.m_pStubData )
        {
            if ( m_pStubData )
            {
                ChangeStub();
                size_t nLength = m_pStubData->m_nLength;

                if ( m_pStubData->m_bFixedStub )
                {
                    size_t nCopy = strSource.GetLength();
                    size_t nTemp = nLength + nCopy;

                    if ( nTemp > m_pStubData->m_nSize )
                    {
                        nCopy = (m_pStubData->m_nSize - nLength) - 1;
                    }
                    memcpy( m_pStubData->m_strData + nLength, strSource.m_pStubData->m_strData, nCopy );
                    m_pStubData->m_strData[ m_pStubData->m_nLength ] = 0;
                }
                else
                {
                    m_pStubData->ChangeSize( nLength + strSource.GetLength() );
                    memcpy( m_pStubData->m_strData + nLength, strSource.m_pStubData->m_strData, strSource.GetLength() );
                    m_pStubData->m_strData[ m_pStubData->m_nLength ] = 0;
                }
            }
            else
            {
                m_pStubData = strSource.m_pStubData;
                m_pStubData->AddRef();
            }
        }
        return *this;
    }

    CString &CString::operator +=( const IASLibChar__ *strSource )
    {
        if ( strSource == NULL )
            return *this;

        if ( m_pStubData )
        {
            size_t nLen = strlen( strSource );
            size_t nTemp = m_pStubData->m_nLength + nLen;
            size_t nLength = m_pStubData->m_nLength;
            if ( nTemp )
            {
                    // We need to create a new copy of the string (if it's used by more than one
                    // CString object.
                ChangeStub();
                m_pStubData->ChangeSize( nTemp );

                if ( m_pStubData->m_bFixedStub )
                {
                    size_t nCopy = nLen;

                    if ( nTemp > m_pStubData->m_nLength )
                    {
                        nCopy = m_pStubData->m_nLength - nLength;
                    }
                    memcpy( m_pStubData->m_strData + nLength, strSource, nCopy );
                    m_pStubData->m_strData[ m_pStubData->m_nLength ] = 0;
                }
                else
                {
                    memcpy( m_pStubData->m_strData + nLength, strSource, nLen );
                    m_pStubData->m_strData[ m_pStubData->m_nLength ] = 0;
                }
            }
        }
        else
        {
            if ( strlen( strSource ) != 0 )
            {
                m_pStubData = new CStringStub( strSource, -1 );
                m_pStubData->AddRef();
            }
        }

        // The combined lengths could only be zero if we're concatenating two empty strings,
        // thus, they're still empty, and no change has occured.

        return *this;
    }

    CString &CString::operator +=( const IASLibChar__ chSource )
    {
        if ( chSource == 0 )
        {
            return *this;
        }

        if ( ! m_pStubData  )
        {
            m_pStubData = new CStringStub( 1 );
            m_pStubData->AddRef();
            if ( m_pStubData->m_strData )
            {
                m_pStubData->m_strData[0] = chSource;
                m_pStubData->m_strData[1] = 0;
            }
        }
        else
        {
            size_t nLength = m_pStubData->m_nLength;
            ChangeStub();
            m_pStubData->ChangeSize( nLength + 1 );

            if ( m_pStubData->m_nLength > nLength )
            {
                m_pStubData->m_strData[ nLength ] = chSource;
                m_pStubData->m_strData[ m_pStubData->m_nLength ] = 0;
            }
        }

        return *this;
    }

    CString &CString::operator +=( const int nSource )
    {
        CString strSource;
        strSource.Format( "%i", nSource );

        if ( ! m_pStubData  )
        {
            *this = strSource;
        }
        else
        {
            size_t nLength = m_pStubData->m_nLength;
            ChangeStub();
            m_pStubData->ChangeSize( nLength + strSource.m_pStubData->m_nLength );
            if ( m_pStubData->m_bFixedStub )
            {
                size_t nCopy = m_pStubData->m_nLength - nLength;

                if ( strSource.m_pStubData->m_nLength < nCopy )
                    nCopy = strSource.m_pStubData->m_nLength;
                memcpy( m_pStubData->m_strData + nLength, strSource.m_pStubData->m_strData, nCopy );
            }
            else
            {
                memcpy( m_pStubData->m_strData + nLength, strSource.m_pStubData->m_strData, strSource.m_pStubData->m_nLength );
                m_pStubData->m_strData[ m_pStubData->m_nLength ] = 0;
            }
        }

        return *this;
    }

    CString &CString::operator +=( const long nSource )
    {
        CString strSource;
        strSource.Format( "%ld", nSource );

        if ( ! m_pStubData  )
        {
            *this = strSource;
        }
        else
        {
            size_t nLength = m_pStubData->m_nLength;
            ChangeStub();
            m_pStubData->ChangeSize( nLength + strSource.m_pStubData->m_nLength );
            if ( m_pStubData->m_bFixedStub )
            {
                size_t nCopy = m_pStubData->m_nLength - nLength;

                if ( strSource.m_pStubData->m_nLength < nCopy )
                    nCopy = strSource.m_pStubData->m_nLength;
                memcpy( m_pStubData->m_strData + nLength, strSource.m_pStubData->m_strData, nCopy );
            }
            else
            {
                memcpy( m_pStubData->m_strData + nLength, strSource.m_pStubData->m_strData, strSource.m_pStubData->m_nLength );
                m_pStubData->m_strData[ m_pStubData->m_nLength ] = 0;
            }
        }

        return *this;
    }

    CString &CString::operator +=( const short nSource )
    {
        CString strSource;
        strSource.Format( "%d", nSource );

        if ( ! m_pStubData )
        {
            *this = strSource;
        }
        else
        {
            size_t nLength = m_pStubData->m_nLength;
            ChangeStub();
            m_pStubData->ChangeSize( nLength + strSource.m_pStubData->m_nLength );
            if ( m_pStubData->m_bFixedStub )
            {
                size_t nCopy = m_pStubData->m_nLength - nLength;

                if ( strSource.m_pStubData->m_nLength < nCopy )
                    nCopy = strSource.m_pStubData->m_nLength;
                memcpy( m_pStubData->m_strData + nLength, strSource.m_pStubData->m_strData, nCopy );
            }
            else
            {
                memcpy( m_pStubData->m_strData + nLength, strSource.m_pStubData->m_strData, strSource.m_pStubData->m_nLength );
                m_pStubData->m_strData[ m_pStubData->m_nLength ] = 0;
            }
        }

        return *this;
    }

                    // Assignment Operators (=) - Assigns a string
    CString &CString::operator =( const CString &strSource )
    {
        if ( this != &strSource )
        {
            if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
            {
                m_pStubData->ChangeSize( strSource.m_pStubData->m_nLength );
                memcpy( m_pStubData->m_strData, strSource.m_pStubData->m_strData, m_pStubData->m_nLength );
            }
            else
            {
                if ( m_pStubData )
                    m_pStubData->RemoveRef();
                m_pStubData = strSource.m_pStubData;

                if ( m_pStubData )
                    m_pStubData->AddRef();
            }
        }

        return *this;
    }

    CString &CString::operator =( const IASLibChar__ *strSource )
    {
        if ( strSource )
        {
            if ( (m_pStubData) && ( m_pStubData->m_strData != strSource ) )
            {
                if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
                {
                    m_pStubData->ChangeSize( (int)strlen( strSource ) );
                    memcpy( m_pStubData->m_strData, strSource, m_pStubData->m_nLength );
                }
                else
                {
                    if ( m_pStubData )
                        m_pStubData->RemoveRef();
                    m_pStubData = new CStringStub( strSource, (int)strlen( strSource ) );
                    m_pStubData->AddRef();
                }
            }
            else
            {
                if ( ! m_pStubData )
                {
                    m_pStubData = new CStringStub( strSource, (int)strlen( strSource ) );
                    m_pStubData->AddRef();
                }
            }
        }
        else
        {
            if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
            {
                m_pStubData->ChangeSize( 0 );
            }
            else
            {
                if ( m_pStubData )
                    m_pStubData->RemoveRef();
                m_pStubData = NULL;
            }
        }

        return *this;
    }

    CString &CString::operator =( const IASLibChar__ chSource )
    {
        if ( chSource != '\0' )
        {
            if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
            {
                m_pStubData->ChangeSize( 1 );
                m_pStubData->m_strData[0] = chSource;
            }
            else
            {
                IASLibChar__ strTemp[2];
                strTemp[0] = chSource;
                strTemp[1] = 0;

                if ( m_pStubData )
                {
                    m_pStubData->RemoveRef();
                }

                m_pStubData = new CStringStub( strTemp, 1 );
                m_pStubData->AddRef();
            }
        }
        else
        {
            if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
            {
                m_pStubData->ChangeSize( 0 );
            }
            else
            {
                if ( m_pStubData )
                    m_pStubData->RemoveRef();
                m_pStubData = NULL;
            }
        }

        return *this;
    }

    CString &CString::operator =( const int nSource )
    {
        CString strSource;
        strSource.Format( "%i", nSource );

        if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
        {
            m_pStubData->ChangeSize( strSource.GetLength() );
            memcpy( m_pStubData->m_strData, strSource.m_pStubData->m_strData, m_pStubData->m_nLength );
        }
        else
        {
            if ( m_pStubData )
            {
                m_pStubData->RemoveRef();
            }

            m_pStubData = strSource.m_pStubData;
            m_pStubData->AddRef();
        }
        return *this;
    }

    CString &CString::operator =( const long nSource )
    {
        CString strSource;
        strSource.Format( "%ld", nSource );

        if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
        {
            m_pStubData->ChangeSize( strSource.GetLength() );
            memcpy( m_pStubData->m_strData, strSource.m_pStubData->m_strData, m_pStubData->m_nLength );
        }
        else
        {
            if ( m_pStubData )
            {
                m_pStubData->RemoveRef();
            }

            m_pStubData = strSource.m_pStubData;
            m_pStubData->AddRef();
        }
        return *this;
    }

    CString &CString::operator =( const short nSource )
    {
        CString strSource;
        strSource.Format( "%d", nSource );

        if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
        {
            m_pStubData->ChangeSize( strSource.GetLength() );
            memcpy( m_pStubData->m_strData, strSource.m_pStubData->m_strData, m_pStubData->m_nLength );
        }
        else
        {
            if ( m_pStubData )
            {
                m_pStubData->RemoveRef();
            }

            m_pStubData = strSource.m_pStubData;
            m_pStubData->AddRef();
        }
        return *this;
    }

    CString &CString::operator =( const float nSource )
    {
        CString strSource;
        strSource.Format( "%f", (double)nSource );

        if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
        {
            m_pStubData->ChangeSize( strSource.GetLength() );
            memcpy( m_pStubData->m_strData, strSource.m_pStubData->m_strData, m_pStubData->m_nLength );
        }
        else
        {
            if ( m_pStubData )
            {
                m_pStubData->RemoveRef();
            }

            m_pStubData = strSource.m_pStubData;
            m_pStubData->AddRef();
        }
        return *this;
    }

    CString &CString::operator =( const double nSource )
    {
        CString strSource;
        strSource.Format( "%f", nSource );

        if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
        {
            m_pStubData->ChangeSize( strSource.GetLength() );
            memcpy( m_pStubData->m_strData, strSource.m_pStubData->m_strData, m_pStubData->m_nLength );
        }
        else
        {
            if ( m_pStubData )
            {
                m_pStubData->RemoveRef();
            }

            m_pStubData = strSource.m_pStubData;
            m_pStubData->AddRef();
        }
        return *this;
    }

    CString &CString::operator =( const bool bSource )
    {
        CString strSource;
        strSource = ( bSource ) ? "true" : "false";

        if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
        {
            m_pStubData->ChangeSize( strSource.GetLength() );
            memcpy( m_pStubData->m_strData, strSource.m_pStubData->m_strData, m_pStubData->m_nLength );
        }
        else
        {
            if ( m_pStubData )
            {
                m_pStubData->RemoveRef();
            }

            m_pStubData = strSource.m_pStubData;
            m_pStubData->AddRef();
        }
        return *this;
    }


        // Equality Operators (==) - Compare a string
    bool CString::operator ==( const CString &strString )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), (const IASLibChar__ *)strString ) == 0 )
            return true;
        return false;
    }

    bool CString::operator ==( const IASLibChar__ *strString2 )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), strString2 ) == 0 )
            return true;
        return false;
    }

    bool operator ==( IASLibChar__ *strString1, const CString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) == 0 )
            return true;
        return false;
    }


                            // Inequality Operators (!=) - Compare a string
    bool CString::operator !=( const CString &strString )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), (const IASLibChar__ *)strString ) == 0 )
            return false;
        return true;
    }

    bool CString::operator !=( const IASLibChar__ *strString2 )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), strString2 ) != 0 )
            return true;
        return false;
    }

    bool operator !=( IASLibChar__ *strString1, const CString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) != 0 )
            return true;
        return false;
    }


                            // Greater than / Equal To Operators (>=) - Compare a string
    bool CString::operator >=( const CString &strString )
    {
        if ( m_pStubData )
        {
            if ( strcmp( m_pStubData->m_strData, strString ) >= 0 )
                return true;
        }
        else
        {
            if ( strString.GetLength() == 0 )
                return true;
        }
        return false;
    }

    bool CString::operator >=( const IASLibChar__ *strString2 )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), strString2 ) >= 0 )
            return true;
        return false;
    }

    bool operator >=( IASLibChar__ *strString1, const CString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) >= 0 )
            return true;
        return false;
    }

                            // Less than / Equal To Operators (<=) - Compare a string
    bool CString::operator <=( const CString &strString )
    {
        if ( m_pStubData )
        {
            if ( strcmp( m_pStubData->m_strData, strString ) <= 0 )
                return true;
        }
        else
        {
            if ( strString.GetLength() == 0 )
                return true;
        }
        return false;
    }

    bool CString::operator <=( const IASLibChar__ *strString2 )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), strString2 ) <= 0 )
            return true;
        return false;
    }

    bool operator <=( IASLibChar__ *strString1, const CString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) <= 0 )
            return true;
        return false;
    }

                            // Greater than Operators (>) - Compare a string
    bool CString::operator >( const CString &strString )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), strString ) > 0 )
            return true;
        return false;
    }

    bool CString::operator >( const IASLibChar__ *strString2 )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), strString2 ) > 0 )
            return true;
        return false;
    }

    bool operator >( IASLibChar__ *strString1, const CString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) > 0 )
            return true;
        return false;
    }


                            // Less than Operators (<) - Compare a string
    bool CString::operator <( const CString &strString )
    {
        if ( m_pStubData )
        {
            if ( strcmp( (const IASLibChar__ *)(*this), strString ) < 0 )
                return true;
        }
        return false;
    }

    bool CString::operator <( const IASLibChar__ *strString2 )
    {
        if ( strcmp( (const IASLibChar__ *)(*this), strString2 ) < 0 )
            return true;
        return false;
    }

    bool operator <( IASLibChar__ *strString1, const CString &strString2 )
    {
        if ( strcmp( strString1, strString2 ) < 0 )
            return true;
        return false;
    }


            // Lexical Comparison - Compares two strings and returns
            // 0 if equal, -1 if this is less than the comparison, and
            // +1 if this is greater than the comparison.
    int CString::Compare( const CString &strCompare, bool bCaseSensitive ) const
    {
        if ( bCaseSensitive )
        {
            if ( m_pStubData )
            {
                return strcmp( m_pStubData->m_strData, strCompare );
            }
            else
            {
                if ( ( strCompare.m_pStubData == NULL ) || ( strCompare. m_pStubData->m_nLength == 0 ) )
                    return 0;

                    // A Zero length string is always less than any non-zero length string
                return -1;
            }
        }
        else
        {
            CString strTemp1 = (const IASLibChar__ *)(*this);
            CString strTemp2 = (const IASLibChar__ *)strCompare;
            strTemp1.ToUpperCase();
            strTemp2.ToUpperCase();

            if ( strTemp1.m_pStubData )
            {
                return strcmp( strTemp1. m_pStubData->m_strData, strTemp2 );
            }
            else
            {
                if ( ( strTemp2.m_pStubData == NULL ) || ( strTemp2. m_pStubData->m_nLength == 0 ) )
                    return 0;

                    // A Zero length string is always less than any non-zero length string
                return -1;
            }
        }
    }

    int CString::Compare( const IASLibChar__ *strCompare, bool bCaseSensitive ) const
    {
        if ( bCaseSensitive )
        {
            if ( ( strCompare ) && ( m_pStubData ) )
            {
                return strcmp( m_pStubData->m_strData, strCompare );
            }
            else
            {
                if ( ( strCompare == NULL ) || ( strlen( strCompare ) == 0 ) )
                    return 0;

                    // A Zero length string is always less than any non-zero length string
                return -1;
            }
        }
        else
        {
            CString strTemp1 = (const IASLibChar__ *)(*this);
            CString strTemp2 = (const IASLibChar__ *)strCompare;
            strTemp1.ToUpperCase();
            strTemp2.ToUpperCase();

            if ( strTemp1.m_pStubData )
            {
                return strcmp( strTemp1.m_pStubData->m_strData, (const IASLibChar__ *)strTemp2 );
            }
            else
            {
                if ( ( ! strTemp2.m_pStubData ) || ( strTemp2.m_pStubData->m_nLength == 0 ) )
                    return 0;

                    // A Zero length string is always less than any non-zero length string
                return -1;
            }
        }
    }

            // Array Access Operator - Allows us to access the characters
            // of the string as if they were a IASLibChar__[] buffer.
    IASLibChar__ &CString::operator []( size_t nIndex ) const
    {
        static IASLibChar__ chDummy;

        if ( (m_pStubData) && ( (size_t)nIndex < m_pStubData->m_nLength ) )
            return m_pStubData->m_strData[ (size_t)nIndex ];

        chDummy = '\0';
        return chDummy;
    }

            // Array Access Operator - Allows us to access the characters
            // of the string as if they were a char[] buffer.
    IASLibChar__ &CString::operator []( int nIndex ) const
    {
        static IASLibChar__ chDummy;

        if ( (m_pStubData) && ( nIndex >= 0 ) && ( (size_t)nIndex < m_pStubData->m_nLength ) )
            return m_pStubData->m_strData[ nIndex ];

        chDummy = '\0';
        return chDummy;
    }

                // Standard Methods

            // Get a sub-string of the string
    CString CString::Substring( size_t nStartIndex, int nLength ) const
    {
        size_t nFixedLength;

        if ( m_pStubData )
        {
            if (  nStartIndex < m_pStubData->m_nLength )
            {
                if ( ( nLength == - 1 ) || ( nLength > (int)((int) m_pStubData->m_nLength - (int)nStartIndex ) ) )
                {
                    nFixedLength = m_pStubData->m_nLength - nStartIndex;
                }
                else
                {
                    nFixedLength = (size_t)nLength;
                }

                CString strReturn( m_pStubData->m_strData + nStartIndex, nFixedLength );
                return strReturn;
            }
        }
        CString strEmpty;

        return strEmpty;
    }

            // Convert a string to upper case
    void CString::ToUpperCase( void )
    {
        size_t nCount;

        if ( m_pStubData )
        {
            ChangeStub();

            for (nCount = 0; nCount < m_pStubData->m_nLength; nCount++ )
            {
                if ( islower( m_pStubData->m_strData[ nCount ] ) )
                {
                    m_pStubData->m_strData[ nCount ] = (IASLibChar__)toupper( m_pStubData->m_strData[ nCount ] );
                }
            }
        }
    }

            // Convert a string to lower case
    void CString::ToLowerCase( void )
    {
        size_t nCount;

        if ( m_pStubData )
        {
            ChangeStub();

            for (nCount = 0; nCount < m_pStubData->m_nLength; nCount++ )
            {
                if ( isupper( m_pStubData->m_strData[ nCount ] ) )
                {
                    m_pStubData->m_strData[ nCount ] = (IASLibChar__)tolower( m_pStubData->m_strData[ nCount ] );
                }
            }
        }
    }

            // Convert a string to Hexadecimal (Used for Oracle)
    void CString::ToHex( void )
    {
        static const IASLibChar__ *strHex = "0123456789ABCDEF";
        size_t nCount;

        if ( m_pStubData )
        {
            ChangeStub();

            size_t nLength = m_pStubData->m_nLength;

            m_pStubData->ChangeSize( nLength * 2 );

            for (nCount = nLength; nCount != NOT_FOUND; nCount-- )
            {
               m_pStubData->m_strData[ nCount * 2 + 1 ] = strHex[ m_pStubData->m_strData[ nCount ] % 16 ];
               m_pStubData->m_strData[ nCount * 2 ]     = strHex[ m_pStubData->m_strData[ nCount ] / 16 ];
            }
        }
    }

            // Convert a string From Hexadecimal (Used for Oracle)
    void CString::FromHex( void )
    {
        size_t nCount;

        if ( m_pStubData )
        {
            ChangeStub();

            size_t nLength = m_pStubData->m_nLength;

            for (nCount = 0; nCount < ( nLength / 2 ); nCount++ )
            {
               IASLibChar__ chTempLow = (IASLibChar__)(m_pStubData->m_strData[ nCount * 2 + 1 ]);
               IASLibChar__ chTempHigh = m_pStubData->m_strData[ nCount * 2 ];

               if ( ( chTempLow >= '0' ) && ( chTempLow <= '9' ) )
               {
                  chTempLow = chTempLow - '0';
               }
               else
               {
                  if ( ( chTempLow >= 'A' ) && ( chTempLow <= 'F' ) )
                  {
                     chTempLow = 10 + ( chTempLow - 'A' );
                  }
                  else
                  {
                     if ( ( chTempLow >= 'a' ) && ( chTempLow <= 'f' ) )
                     {
                        chTempLow = 10 + ( chTempLow - 'a' );
                     }
                     else
                     {
                        chTempLow = 0;
                     }
                  }
               }

               if ( ( chTempHigh >= '0' ) && ( chTempHigh <= '9' ) )
               {
                  chTempHigh = chTempHigh - '0';
               }
               else
               {
                  if ( ( chTempHigh >= 'A' ) && ( chTempHigh <= 'F' ) )
                  {
                     chTempHigh = 10 + ( chTempHigh - 'A' );
                  }
                  else
                  {
                     if ( ( chTempHigh >= 'a' ) && ( chTempHigh <= 'f' ) )
                     {
                        chTempHigh = 10 + ( chTempHigh - 'a' );
                     }
                     else
                     {
                        chTempHigh = 0;
                     }
                  }
               }

               m_pStubData->m_strData[ nCount ] = chTempLow + ( chTempHigh * 16 );
               if ( m_pStubData->m_strData[ nCount ] == 0 )
               {
                  m_pStubData->m_strData[ nCount ] = '?';
               }
            }

            m_pStubData->ChangeSize( nLength / 2 );
        }
    }

            // Trim leading white-space from a string
    CString &CString::LeftTrim( const IASLibChar__ *pszWhiteSpace )
    {
        if ( m_pStubData )
        {
            CString strWhiteSpace = pszWhiteSpace;

            size_t nCount;

            for (nCount = 0; ( nCount < 65535 ) && ( nCount < m_pStubData->m_nLength ) && ( strWhiteSpace.IndexOf( m_pStubData->m_strData[ nCount ] ) != IASLib::NOT_FOUND ); nCount++ );

            if ( nCount != 0 )
            {
                *this = Substring( nCount );
            }
        }
        return *this;
    }

            // Trim trailing white-space from a string
    CString &CString::RightTrim( const IASLibChar__ *pszWhiteSpace )
    {
        if ( m_pStubData )
        {
            CString strWhiteSpace = pszWhiteSpace;

            size_t nCount;

            for ( nCount = m_pStubData->m_nLength - 1;( nCount != NOT_FOUND ) && ( strWhiteSpace.IndexOf( m_pStubData->m_strData[ nCount ] ) != IASLib::NOT_FOUND ); nCount-- );

            if ( nCount != NOT_FOUND )
            {
                if ( nCount != (m_pStubData->m_nLength - 1) )
                {
                    *this = Substring( 0, (int)nCount + 1 );
                }
            }
            else
            {
                *this = "";
            }
        }
        return *this;
    }

        // Trim leading/trailing white-space from a string
    CString &CString::Trim( const IASLibChar__ *pszWhiteSpace )
    {
        if ( m_pStubData )
        {
            CString strWhiteSpace = pszWhiteSpace;

            size_t nCount;

            for (nCount = 0; ( nCount < 65535 ) && ( nCount < m_pStubData->m_nLength ) && ( strWhiteSpace.IndexOf( m_pStubData->m_strData[ nCount ] ) != IASLib::NOT_FOUND ); nCount++ );

            if ( nCount != 0 )
            {
                *this = Substring( nCount );
            }

            if ( ! m_pStubData )
                return *this;

            for ( nCount = m_pStubData->m_nLength - 1;( nCount != NOT_FOUND ) && ( strWhiteSpace.IndexOf( m_pStubData->m_strData[ nCount ] ) != IASLib::NOT_FOUND ); nCount-- );

            if ( nCount != (m_pStubData->m_nLength - 1) )
            {
                *this = Substring( 0, (int)nCount + 1 );
            }
        }
        return *this;
    }

            // Assign this string to a printf formatted string
    const IASLibChar__ *CString::Format( const IASLibChar__ *strFormat, ... )
    {
#ifdef IASLIB_MULTI_THREADED__
        mutexStringProtect.Lock();
#endif
        va_list       vaArgList;
        static IASLibChar__   szBuffer[65536];

        /* format buf using fmt and arguments contained in ap */
        va_start( vaArgList, strFormat );
#ifdef IASLIB_WIN32__
        StringCbVPrintf( szBuffer, 65536, strFormat, vaArgList );
#else
        vsprintf( szBuffer, strFormat, vaArgList );
#endif
        va_end( vaArgList );

        if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
        {
            m_pStubData->ChangeSize( (int)strlen( szBuffer ) );
            memcpy( m_pStubData->m_strData, szBuffer, m_pStubData->m_nLength );
        }
        else
        {
            if ( m_pStubData )
            {
                m_pStubData->RemoveRef();
            }

            m_pStubData = new CStringStub( szBuffer, -1 );
            m_pStubData->AddRef();
        }

#ifdef IASLIB_MULTI_THREADED__
        mutexStringProtect.Unlock();
#endif
        return m_pStubData->m_strData;
    }

            // Find the occurance of a substring in a string
    size_t CString::IndexOf( const CString &strSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        return IndexOf( (const IASLibChar__ *)strSearch, nStart, bCaseInsensitive );
    }

    size_t CString::IndexOf( const IASLibChar__ *strSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        size_t     nReturn = IASLib::NOT_FOUND;

        if ( m_pStubData )
        {
            if ( m_pStubData->m_strData )
            {
                if ( nStart == IASLib::NOT_FOUND )
                {
                    nStart = 0;
                }

                if ( nStart > m_pStubData->m_nLength )
                {
                    nStart = m_pStubData->m_nLength;
                }

                IASLibChar__ *pFound;

                if ( bCaseInsensitive )
                {
                    CString strCopy = m_pStubData->m_strData;
                    CString strSearchCopy = strSearch;

                    strCopy.ToLowerCase();
                    strSearchCopy.ToLowerCase();
                    pFound = strstr( &(strCopy.m_pStubData->m_strData[nStart] ), (const IASLibChar__ *)strSearchCopy );
                    if ( pFound != NULL )
                    {
                        nReturn = (size_t)(pFound - strCopy.m_pStubData->m_strData);
                    }
                }
                else
                {
                    pFound = strstr( &(m_pStubData->m_strData[nStart] ), strSearch );
                    if ( pFound != NULL )
                    {
                        nReturn = (size_t)(pFound - m_pStubData->m_strData);
                    }
                }
            }
        }

        return nReturn;
    }

    size_t CString::IndexOf( const IASLibChar__ chSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        size_t nCount = IASLib::NOT_FOUND;
        IASLibChar__ chMatch;

        if ( m_pStubData )
        {
            if ( nStart == IASLib::NOT_FOUND )
                nStart = 0;

            if ( bCaseInsensitive )
            {
                chMatch = (IASLibChar__)toupper( chSearch );

                for ( nCount = nStart; ( nCount < m_pStubData->m_nLength ) && ( toupper( m_pStubData->m_strData[ nCount ] ) != chMatch ); nCount++ );
            }
            else
            {
                for ( nCount = nStart; ( nCount < m_pStubData->m_nLength ) && ( m_pStubData->m_strData[ nCount ] != chSearch ); nCount++ );
            }
            if ( nCount == m_pStubData->m_nLength )
                nCount = IASLib::NOT_FOUND;
        }
        return nCount;
    }

    size_t CString::LastIndexOf( const CString &strSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        return LastIndexOf( (const IASLibChar__ *)strSearch, nStart, bCaseInsensitive );
    }

    size_t CString::LastIndexOf( const IASLibChar__ *strSearch, size_t nStart,  bool bCaseInsensitive ) const
    {
        size_t     nReturn = IASLib::NOT_FOUND;
        IASLibChar__   *pLast;
        IASLibChar__   *pFound;
        size_t     pPos = IASLib::NOT_FOUND;

        if ( m_pStubData )
        {
            pFound = m_pStubData->m_strData;
            if ( ( nStart > m_pStubData->m_nLength ) || ( nStart == IASLib::NOT_FOUND ) )
            {
                nStart = m_pStubData->m_nLength;
            }

            if ( m_pStubData->m_strData )
            {

                if ( bCaseInsensitive )
                {
                    CString strCopy = m_pStubData->m_strData;
                    CString strSearchCopy = strSearch;
                    strCopy.ToLowerCase();
                    strSearchCopy.ToLowerCase();
                    pFound = strCopy.m_pStubData->m_strData;
                    do
                    {
                        pLast = pFound;
                        pFound = strstr( pLast + 1, (const IASLibChar__ *)strSearchCopy );

                        if ( pFound )
                            pPos = (size_t)(pFound - strCopy.m_pStubData->m_strData);

                    } while ( ( pFound ) && (pPos != NOT_FOUND) && ( pPos < nStart ) );

                    if ( pLast != strCopy.m_pStubData->m_strData )
                    {
                        nReturn = (size_t)(pLast - strCopy.m_pStubData->m_strData);
                    }
                }
                else
                {
                    do
                    {
                        pLast = pFound;
                        pFound = strstr( pLast + 1, strSearch );

                        if ( pFound )
                            pPos = (int)(pFound - m_pStubData->m_strData);

                    } while ( ( pFound ) && ( pPos < nStart ) );

                    if ( pLast != m_pStubData->m_strData )
                    {
                        nReturn = (int)(pLast - m_pStubData->m_strData);
                    }
                }
            }
        }

        return nReturn;
    }

    size_t CString::LastIndexOf( const IASLibChar__ chSearch, size_t nStart,  bool bCaseInsensitive ) const
    {
        size_t nCount;

        if ( m_pStubData )
        {
            if ( ( nStart > m_pStubData->m_nLength ) || ( nStart == NOT_FOUND ) )
            {
                nStart = m_pStubData->m_nLength;
            }

            for ( nCount = nStart; nCount != NOT_FOUND; nCount-- )
            {
                if ( bCaseInsensitive )
                {
                    if ( ( m_pStubData->m_strData[ nCount ] == chSearch ) || ( ( toupper( m_pStubData->m_strData[ nCount ] ) == toupper( chSearch ) ) ) )
                        return (int)nCount;
                }
                else
                {
                    if ( m_pStubData->m_strData[ nCount ] == chSearch )
                        return (int)nCount;
                }
            }
        }
        return IASLib::NOT_FOUND;
    }

#ifdef IASLIB_STD_STREAM_SUPPORT__
            // Stream Methods

                            // Output a string to a stream derived object.
    std::ostream  &operator <<( std::ostream &oOutputStream, const CString &strSource )
    {
        size_t nCount;
//        long nOutputLength = (long)strSource.GetLength();

//        oOutputStream << nOutputLength;
        for ( nCount = 0; nCount < strSource.GetLength(); nCount++ )
        {
            oOutputStream << strSource[ nCount ];
        }

        return oOutputStream;
    }

                            // Input a string from a stream derived object.
    std::istream  &operator >>( std::istream &oInputStream, CString &strTarget )
    {
        size_t nCount;
        unsigned long nInputLength;

        oInputStream >> nInputLength;

        if ( strTarget.m_pStubData )
        {
            strTarget.m_pStubData->RemoveRef();
        }

        strTarget.m_pStubData = new CStringStub( nInputLength );
        strTarget.m_pStubData->AddRef();

        for ( nCount = 0; nCount < strTarget.m_pStubData->m_nLength; nCount++ )
        {
            oInputStream >> strTarget.m_pStubData->m_strData[ nCount ];
        }

        return oInputStream;
    }

#endif

    void CString::Unescape( void )
    {
        if ( m_pStubData )
        {
            IASLibChar__   *strRetVal = (IASLibChar__ *)malloc( m_pStubData->m_nLength + 1 ); // This is safe because unescape can never make the string longer
            size_t  nCount = 0;
            size_t  nInsert = 0;
            IASLibChar__    strHexCode[5];
            IASLibChar__    nResolved = 0;
            strHexCode[0] = '0';
            strHexCode[1] = 'x';
            strHexCode[4] = 0;

            while ( nCount < m_pStubData->m_nLength )
            {
                if ( m_pStubData->m_strData[ nCount ] == '%' )
                {
                    if ( m_pStubData->m_strData[ nCount + 1 ] == '%' )
                    {
                        strRetVal[ nInsert ] = '%';
                        nInsert++;
                        nCount += 2;
                        continue;
                    }
                    else
                    {
                        strHexCode[2] = m_pStubData->m_strData[ nCount + 1];
                        strHexCode[3] = m_pStubData->m_strData[ nCount + 2];

                        nResolved = (IASLibChar__)strtol(strHexCode, NULL, 0 );

                        strRetVal[ nInsert ] = nResolved;
                        nCount += 3;
                        nInsert++;
                        continue;
                    }
                }
                else
                {
                    if ( m_pStubData->m_strData[ nCount ] == '+' )
                    {
                        strRetVal[ nInsert ] = ' ';
                        nInsert++;
                        nCount++;
                    }
                    else
                    {
                        strRetVal[ nInsert ] = m_pStubData->m_strData[ nCount ];
                        nInsert++;
                        nCount++;
                    }
                }
            }
            strRetVal[ nInsert ] = 0;

            if ( ! m_pStubData->m_bFixedStub )
            {
                m_pStubData->RemoveRef();
                m_pStubData = new CStringStub( strRetVal, nInsert );
                m_pStubData->AddRef();
            }
            else
            {
                m_pStubData->ChangeSize( nInsert );
                memcpy( m_pStubData->m_strData, strRetVal, m_pStubData->m_nLength );
            }

            free( strRetVal );
        }
    }

    void CString::Escape( void )
    {
        CString strEscapedVersion;
        CString strHexChars = "0123456789ABCDEF";
        CString strEscapableChars = "\"<>#%{}|\\~^[]`'";
        size_t  nCount = 0;

        if ( m_pStubData )
        {
            for ( nCount = 0; nCount < m_pStubData->m_nLength ; nCount++ )
            {
                if ( ( m_pStubData->m_strData[ nCount ] < (IASLibChar__)32 ) ||
                     ( m_pStubData->m_strData[nCount] > 126 ) ||
                     ( strEscapableChars.IndexOf( m_pStubData->m_strData[ nCount ] ) != NOT_FOUND ) )
                {
                    strEscapedVersion += "%";
                    strEscapedVersion += strHexChars[ m_pStubData->m_strData[nCount] / 16 ];
                    strEscapedVersion += strHexChars[ m_pStubData->m_strData[nCount] % 16 ];
                }
                else
                {
                        // Space is a special case
                    if ( m_pStubData->m_strData[ nCount ] == ' ' )
                    {
                        strEscapedVersion += '+';
                    }
                    else
                    {
                        strEscapedVersion += m_pStubData->m_strData[ nCount ];
                    }
                }
            }

            *this = strEscapedVersion;
        }
    }

    void CString::Replace( const IASLibChar__ chReplace, const IASLibChar__ chTo )
    {
        ChangeStub();

        if ( m_pStubData )
        {
            for ( size_t nCount = 0; nCount < m_pStubData->m_nLength ; nCount++ )
            {
                if ( m_pStubData->m_strData[ nCount ] == chReplace )
                    m_pStubData->m_strData[ nCount ] = chTo;
            }
        }
    }

    void CString::Replace( const IASLibChar__ *strReplace, const IASLibChar__ *strTo )
    {
        size_t nReplacePoint;
        size_t nReplaceLength = strlen( strReplace );
        CString strTemp;

        while ( ( nReplacePoint = IndexOf( strReplace ) ) != IASLib::NOT_FOUND )
        {
            strTemp = Substring( 0, (int)nReplacePoint );
            strTemp += strTo;
            strTemp += Substring( nReplacePoint + nReplaceLength );
            *this = strTemp;
        }
    }

    void CString::Remove( const IASLibChar__ chRemove )
    {
        size_t nCount = 0;
        size_t nInsert = 0;
        CString strTemp;

        if ( m_pStubData )
        {
            CStringStub *pTemp = new CStringStub( m_pStubData->m_nLength );
            pTemp->AddRef();
            while ( nCount < m_pStubData->m_nLength )
            {
                if ( m_pStubData->m_strData[ nCount ] != chRemove )
                {
                    pTemp->m_strData[ nInsert ] = m_pStubData->m_strData[ nCount ];
                    nInsert++;
                }
                nCount++;
            }

            m_pStubData->RemoveRef();
            m_pStubData = pTemp;
        }
    }

    void CString::Remove( const IASLibChar__ *strRemove )
    {
        size_t nRemovePoint;
        size_t nRemoveLength = strlen( strRemove );
        CString strTemp;

        while ( ( nRemovePoint = IndexOf( strRemove ) ) != IASLib::NOT_FOUND )
        {
            strTemp = Substring( 0, (int)nRemovePoint );
            strTemp += Substring( nRemovePoint + nRemoveLength );
            *this = strTemp;
        }
    }


    bool CString::WildcardCompare( const IASLibChar__ *strPattern, bool bCaseSensitive )
    {
        if ( strPattern == NULL )
            return false;

        if ( strcmp( strPattern, "*" ) == 0 )
            return true;

        if ( strlen( strPattern ) == 0 )
            return true;

        if ( m_pStubData == NULL )
            return false;

        size_t  nCount = 0;
        int     nPattern = 0;
        CString strSubString;
        CString strSubPattern;
        int     nTemp;
        bool    bMatched;
        size_t  nPatternSize;
        int     nMinimumSize;

        while ( ( nCount < m_pStubData->m_nLength ) && ( nPattern < (int)strlen( strPattern ) ) )
        {
            switch ( strPattern[nPattern] )
            {
                case '*':
                    nTemp = (int)nCount;
                    bMatched = false;

                    nPattern++;
                    strSubPattern = strPattern;
                    strSubPattern = strSubPattern.Substring( nPattern );
                    nPatternSize = strlen( strPattern ) - nPattern;
                    nMinimumSize = (int)(m_pStubData->m_nLength - nPatternSize);
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
                        if ( m_pStubData->m_strData[ nCount ] != strPattern[ nPattern ] )
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if ( toupper( m_pStubData->m_strData[ nCount ] ) != toupper( strPattern[ nPattern ] ) )
                        {
                            return false;
                        }
                    }
                    nCount++;
                    nPattern++;
                    break;
            }
        }

        if ( ( nCount == m_pStubData->m_nLength ) && ( nPattern < (int)strlen( strPattern ) ) )
        {
            return false;
        }

        if ( nCount < m_pStubData->m_nLength )
        {
            return false;
        }

        return true;
    }

    void CString::OracleEscape( void )
    {
        CString strTemp;
        size_t  nCount;

        if ( m_pStubData )
        {
            for ( nCount = 0; nCount < m_pStubData->m_nLength ; nCount++ )
            {
                if ( m_pStubData->m_strData[nCount] == '\'')
                {
                    strTemp += '\'';
                }

                strTemp += m_pStubData->m_strData[nCount];
            }

            if ( m_pStubData->m_bFixedStub )
            {
                memcpy( m_pStubData->m_strData, strTemp.m_pStubData->m_strData, m_pStubData->m_nLength );
            }
            else
            {
                m_pStubData->RemoveRef();
                m_pStubData = strTemp.m_pStubData;
                m_pStubData->AddRef();
            }
        }
    }

    void CString::OracleUnescape( void )
    {
        CString strTemp;
        size_t  nCount;

        if ( m_pStubData )
        {
            for ( nCount = 0; nCount < m_pStubData->m_nLength ; nCount++ )
            {
                if ( ( m_pStubData->m_strData[nCount] == '\'') && ( m_pStubData->m_strData[nCount + 1] == '\'' ) )
                {
                    nCount++;
                }

                strTemp += m_pStubData->m_strData[nCount];
            }

            if ( m_pStubData->m_bFixedStub )
            {
                memcpy( m_pStubData->m_strData, strTemp.m_pStubData->m_strData, m_pStubData->m_nLength );
            }
            else
            {
                m_pStubData->RemoveRef();
                m_pStubData = strTemp.m_pStubData;
                m_pStubData->AddRef();
            }
        }
    }

    CString CString::Pad( size_t nLength, IASLibChar__ chPadChar, bool bLeftAlign )
    {
        int     nRemain = (int)nLength - (int)GetLength();
        IASLibChar__   *achPadding;
        CString strRetVal;

        if ( nRemain > 0 )
        {
            achPadding = new IASLibChar__[ nRemain + 1 ];
            memset( achPadding, chPadChar, nRemain );
            achPadding[ nRemain ] = 0;

            if ( bLeftAlign )
            {
                strRetVal = (*this);
                strRetVal += achPadding;
            }
            else
            {
                strRetVal = achPadding;
                strRetVal += (*this);
            }

            delete achPadding;
        }
        else
        {
            strRetVal = *this;
        }

        return strRetVal;
    }

    bool  CString::Contains (const CString &strSearch) const
    {
        bool bRetVal = false;

        if ( IndexOf( strSearch) != NOT_FOUND )
            bRetVal = true;

        return bRetVal;
    }

    bool  CString::Contains (const IASLibChar__ *strSearch) const
    {
        bool bRetVal = false;

        if ( IndexOf( strSearch) != NOT_FOUND )
            bRetVal = true;

        return bRetVal;
    }

    bool  CString::Contains (const IASLibChar__ chSearch) const
    {
        bool bRetVal = false;

        if ( IndexOf( chSearch) != NOT_FOUND )
            bRetVal = true;

        return bRetVal;
    }

    int CString::ConvertToToken( const IASLibChar__ *strToken, const IASLibChar__ *astrTokenList[], int nTokenList[], int nTokens, bool bCaseSensitive, bool bUseWildcards )
    {
        int nCount = 0;
        int nRetVal = -9999;
        bool bDone = false;

        if ( nTokens == -1 )
        {
            nTokens = 65535;
        }


        CString strCompare = strToken;
        CString strWork;

        do
        {
            strWork = astrTokenList[ nCount ];

                // Look for the end code in the token list
            if ( strWork == "**" )
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

    void CString::Copy( const IASLibChar__ *strSource, int nLength )
    {
        if ( ( m_pStubData ) && ( strSource != m_pStubData->m_strData ) )
        {
            if ( ( m_pStubData ) && ( m_pStubData->m_bFixedStub ) )
            {
                m_pStubData->ChangeSize( nLength );
                memcpy( m_pStubData->m_strData, strSource, m_pStubData->m_nLength );
            }
            else
            {
                if ( m_pStubData )
                {
                    m_pStubData->RemoveRef();
                }

                if ( ( strSource == NULL ) || ( nLength == 0 ) )
                {
                    m_pStubData = NULL;
                }
                else
                {
                    m_pStubData = new CStringStub( strSource, nLength );
                    m_pStubData->AddRef();
                }
            }
        }
    }

    void CString::Clear( void )
    {
        if ( ( m_pStubData ) && ( ! m_pStubData->m_bFixedStub ) )
        {
            m_pStubData->RemoveRef();
            m_pStubData = NULL;
        }
        else
        {
                // If there is a stub, then it's a fixed stub and
                // clearing it takes on a different meaning.
            if ( m_pStubData )
            {
                m_pStubData->m_nLength = 0;
                m_pStubData->m_strData[0] = 0;
            }
        }
    }

    void CString::ChangeStub( void )
    {
        CStringStub    *stubTemp;

        if ( ( m_pStubData ) && ( ! m_pStubData->m_bFixedStub ) )
        {
            if ( 1 < m_pStubData->m_nReferences )
            {
                stubTemp = m_pStubData;
                m_pStubData = new CStringStub( stubTemp->m_strData, stubTemp->m_nLength );

                stubTemp->RemoveRef();

                m_pStubData->AddRef();
            }
        }
    }

    bool CString::IsDigits( void )
    {
        if ( ! m_pStubData )
            return false;

        size_t nCount = 0;

        while ( nCount < m_pStubData->m_nLength )
        {
            if ( ! ::isdigit( m_pStubData->m_strData[ nCount ] ) )
                return false;
            nCount++;
        }
        return true;
    }

    CString CString::FormatString( const char *fmt, ... )
    {
        CString ret;

        va_list ap;
        va_start(ap, fmt);

        ret.Format( fmt, ap );

        va_end(ap);

        return ret;
    }

    // For proper hashing of strings, we redefine the hashcode function so
    // that the hash of a string is equivalent to the hash of its contents.
    // Thus two strings with the same value, "Hello" and "Hello" will hash
    // to the same hashcode, even though they are different values.
    int CString::hashcode( void )
    {
        int result = 131;

        for ( size_t x = 0; x < m_pStubData->m_nLength; x++ )
        {
            result = 37 * result + m_pStubData->m_strData[x];
        }

        return result;
    }

    void CString::dumpStubMemory( void )
    {
        if ( m_pStubData )
        {
            char *p = (char *)m_pStubData;
            p -= 128;
            while ( p < (char *)m_pStubData )
            {
                char t = *p++;
                printf( "%c %02x\n", t, t & 0xff );
            }
        }
    }

} // namespace ISALib
