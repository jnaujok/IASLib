/*
 * Socket Stream Class
 *
 *      A stream over a connected socket. See SocketStream.h.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "SocketStream.h"
#include "StreamException.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{
    IMPLEMENT_OBJECT( CSocketStream, CStream );

    CSocketStream::CSocketStream( void )
    {
        m_pSocket = NULL;
        m_bNoDelete = false;
        m_bIsPeeked = false;
        m_chPeeked = '\0';
        m_bClosed = true;
        m_bIsOpen = false;
    }

    CSocketStream::CSocketStream( CSocket *pSocket )
    {
        m_pSocket = pSocket;
        m_bNoDelete = false;
        m_bIsPeeked = false;
        m_chPeeked = '\0';
        m_bClosed = ( pSocket == NULL );
        m_bIsOpen = ( pSocket != NULL );
    }

    CSocketStream::~CSocketStream( void )
    {
        Close();
    }

    void CSocketStream::Close( void )
    {
        if ( m_pSocket != NULL && ! m_bNoDelete )
            delete m_pSocket;
        m_pSocket = NULL;
        m_bIsOpen = false;
        m_bIsPeeked = false;
        m_bClosed = true;
    }

    int CSocketStream::GetBuffer( char *achBuffer, int nLength )
    {
        if ( achBuffer == NULL || nLength <= 0 )
            return 0;

        int nReceived = 0;
        if ( m_bIsPeeked )
        {
            achBuffer[ nReceived++ ] = m_chPeeked;
            m_bIsPeeked = false;
        }
        while ( nReceived < nLength && ! m_bClosed && m_pSocket != NULL )
        {
            int nRead = 0;
            try
            {
                nRead = m_pSocket->Read( achBuffer + nReceived, nLength - nReceived );
            }
            catch ( ... )
            {
                nRead = -1;     // an error ends the stream, as a close does
            }
            if ( nRead <= 0 )
            {
                    // (It used to ask again for ever, and add -1 on errors.)
                m_bClosed = true;
                break;
            }
            nReceived += nRead;
        }
        return nReceived;
    }

    int CSocketStream::PutBuffer( const char *achBuffer, int nLength )
    {
        if ( m_pSocket == NULL || achBuffer == NULL || nLength <= 0 )
            return 0;

        int nSent = 0;
        while ( nSent < nLength )
        {
            int nWritten = 0;
            try
            {
                nWritten = m_pSocket->Send( achBuffer + nSent, nLength - nSent );
            }
            catch ( ... )
            {
                nWritten = -1;
            }
            if ( nWritten <= 0 )
                break;
            nSent += nWritten;
        }
        return nSent;
    }

    bool CSocketStream::IsEOS( void )
    {
        if ( m_bIsPeeked )
            return false;
        if ( m_bClosed || m_pSocket == NULL || ! m_pSocket->IsConnected() )
            return true;

            // Only reading tells whether the other side has closed: read a
            // byte (waiting for it) and keep it for the next read. (This
            // used to answer "true" exactly when data was waiting.)
        char chRead;
        if ( GetBuffer( &chRead, 1 ) != 1 )
            return true;
        m_chPeeked = chRead;
        m_bIsPeeked = true;
        return false;
    }

    char CSocketStream::PeekChar( void )
    {
            // A second peek returns the same byte (it used to take it).
        if ( IsEOS() )
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        return m_chPeeked;
    }
} // Namespace IASLib

#endif // IASLIB_NETWORKING__
