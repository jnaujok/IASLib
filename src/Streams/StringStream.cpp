/*
 * String Stream Class
 *
 *      A stream over memory. See StringStream.h.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "StringStream.h"
#include "StreamException.h"

namespace IASLib
{
    CStringStream::CStringStream( void )
        : m_nCurrentPosition( 0 )
    {
    }

    CStringStream::CStringStream( const CString &strSource )
        : m_strInput( strSource.c_str(), strSource.GetLength() ), m_nCurrentPosition( 0 )
    {
    }

    CStringStream::CStringStream( const char *pData, size_t nLength )
        : m_strInput( ( pData != NULL ) ? std::string( pData, nLength ) : std::string() ), m_nCurrentPosition( 0 )
    {
    }

    CStringStream::CStringStream( const std::string &strSource )
        : m_strInput( strSource ), m_nCurrentPosition( 0 )
    {
    }

    CStringStream::~CStringStream( void )
    {
        m_bIsOpen = false;
    }

    int CStringStream::GetBuffer( char *achBuffer, int nLength )
    {
        if ( achBuffer == NULL || nLength <= 0 || IsEOS() )
            return 0;
        size_t nCount = m_strInput.size() - m_nCurrentPosition;
        if ( nCount > (size_t)nLength )
            nCount = (size_t)nLength;
        memcpy( achBuffer, m_strInput.data() + m_nCurrentPosition, nCount );
        m_nCurrentPosition += nCount;
        return (int)nCount;
    }

    int CStringStream::PutBuffer( const char *achBuffer, int nLength )
    {
        if ( achBuffer == NULL || nLength <= 0 )
            return 0;
        m_strOutput.append( achBuffer, (size_t)nLength );
        return nLength;
    }

    char CStringStream::PeekChar( void )
    {
        if ( IsEOS() )
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        return m_strInput[ m_nCurrentPosition ];
    }

    char CStringStream::GetChar( void )
    {
        if ( IsEOS() )
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        return m_strInput[ m_nCurrentPosition++ ];
    }

    CString CStringStream::GetLine( void )
    {
        size_t nEnd = m_strInput.find( '\n', m_nCurrentPosition );
        size_t nNext = ( nEnd == std::string::npos ) ? m_strInput.size() : nEnd + 1;
        if ( nEnd == std::string::npos )
            nEnd = m_strInput.size();
        if ( m_nCurrentPosition > nEnd )
            return CString();

        size_t nLength = nEnd - m_nCurrentPosition;
        if ( nLength > 0 && m_strInput[ m_nCurrentPosition + nLength - 1 ] == '\r' )
            nLength--;
        CString strLine( m_strInput.data() + m_nCurrentPosition, nLength );
        m_nCurrentPosition = nNext;
        return strLine;
    }
} // namespace IASLib
