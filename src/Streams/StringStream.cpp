/*
 * String Stream Class
 *
 *      This class provides the basis for streaming data into a CString
 * object. This allows us to use streaming operations to create a string
 * that can be used just like any other string.
 *      Note that this class can also be used to /read/ from a string,
 * just like any other stream.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/08/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "StringStream.h"
#include "StreamException.h"

namespace IASLib
{
    CStringStream::CStringStream( void )
    {
        m_strString = "";
        m_strOutputString = "";
        m_nCurrentPosition = 0;
    }

    CStringStream::CStringStream( const CString &strSource )
    {
        m_strString = strSource;
        m_strOutputString = "";
        m_nCurrentPosition = 0;
    }

    CStringStream::~CStringStream( void )
    {
        m_strString = "";
        m_strOutputString = "";
        m_nCurrentPosition = 0;
        m_bIsOpen = false;
    }

    CString CStringStream::GetLine( void )
    {
        CString strRetVal;

        if ( m_nCurrentPosition >= m_strString.GetLength() )
        {
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        }

        size_t nFoundAt = m_strString.IndexOf( '\n', m_nCurrentPosition );

        if ( nFoundAt == IASLib::NOT_FOUND )
        {
            strRetVal = m_strString.Substring( m_nCurrentPosition );
            m_nCurrentPosition = m_strString.GetLength();
        }
        else
        {
            strRetVal = m_strString.Substring( m_nCurrentPosition, (int)(nFoundAt - m_nCurrentPosition) );
            m_nCurrentPosition = nFoundAt + 1;
        }

        return strRetVal;
    }

    char CStringStream::GetChar( void )
    {
        if ( m_nCurrentPosition >= m_strString.GetLength() )
        {
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        }

        char chRetVal = m_strString[ m_nCurrentPosition ];
        m_nCurrentPosition++;

        return chRetVal;
    }

    void CStringStream::PutChar( const char chPut )
    {
        m_strOutputString += chPut;
    }
	
    void CStringStream::PutLine( const CString &strOutput )
    {
        m_strOutputString += strOutput;
    }

    unsigned char CStringStream::GetUChar( void )
    {
        if ( m_nCurrentPosition >= m_strString.GetLength() )
        {
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        }

        unsigned char chRetVal = (unsigned char)m_strString[ m_nCurrentPosition ];
        m_nCurrentPosition++;

        return chRetVal;
    }

    void CStringStream::PutChar( const unsigned char chPut )
    {
        m_strOutputString += chPut;
    }

    int CStringStream::PutBuffer( const char *achBuffer, int nLength )
    {
        CString strOutput( achBuffer, nLength );

        m_strOutputString += strOutput;
        return (int)strOutput.Length();
    }
	
    int CStringStream::GetBuffer( char *achBuffer, int nLength )
    {
        const char *pstrData = (const char *)m_strString;
        int         nReceived = 0;

        if ( (m_nCurrentPosition + (size_t)nLength) > m_strString.GetLength() )
        {
            nReceived = (int)(m_strString.GetLength() - m_nCurrentPosition);
            memcpy( achBuffer, pstrData + m_nCurrentPosition, nReceived );
        }
        else
        {
            nReceived = nLength;
            memcpy( achBuffer, pstrData + m_nCurrentPosition, nLength );
        }
        m_nCurrentPosition += nReceived;

        return nReceived;
    }

} // namespace IASLib

