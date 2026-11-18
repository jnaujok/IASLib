/**
 *  Server Socket Class
 *
 *      This class provides an interface for creating a server
 * socket connection.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/02/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "ServerSocket.h"
#include "SocketException.h"
#include "SocketWait.h"
#include <errno.h>
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CServerSocket, CSocket );

        // (listen() was never checked.)
    void CServerSocket::Listen( int nBacklog )
    {
        if ( m_hSocket == NULL_SOCKET )
            return;
        if ( nBacklog <= 0 )
            nBacklog = SOMAXCONN;
        if ( listen( m_hSocket, nBacklog ) == SOCKET_ERROR )
        {
            int nError = LastError();
            Close();
            IASLIB_THROW_SOCKET_EXCEPTION( nError );
        }
    }

    /**
     * Server Socket Constructor
     *
     * @param nPort
     *      The port number to bind the server to (0 for any free port).
     * @param nMaxBacklog
     *      The maximum number of connections that can be waiting on
     *      this socket without being "accepted".
     * @param bSecure
     *      Informational: whether the connections will be wrapped in TLS.
     */
    CServerSocket::CServerSocket( int nPort, int nMaxBacklog, bool bSecure ) : CSocket( nPort )
    {
        m_bSecure = bSecure;
        Listen( nMaxBacklog );
    }

        // (This was declared but never defined.)
    CServerSocket::CServerSocket( CSocketConfig oConfig, int nPort, const char *bindAddress )
        : CSocket( oConfig, nPort, bindAddress ), config( oConfig )
    {
        m_bSecure = false;
        Listen( config.getBacklogSize() );
    }

        // (It set the reuse option on the handle about to be closed, which
        // has no effect.)
    CServerSocket::~CServerSocket( void )
    {
    }

        // (The address length was allocated but its value never set on some
        // paths, the error was thrown as a pointer, and errno is not the
        // socket error on Windows.)
    CSocket *CServerSocket::Accept( void )
    {
        if ( m_hSocket == NULL_SOCKET )
            IASLIB_THROW_SOCKET_EXCEPTION( ENOTCONN );

        for ( ;; )
        {
            struct sockaddr_storage sockAddr;
            socklen_t nAddrLen = sizeof( sockAddr );
            memset( &sockAddr, 0, sizeof( sockAddr ) );

            SOCKET hSocket = accept( m_hSocket, (struct sockaddr *)&sockAddr, &nAddrLen );
            if ( hSocket != INVALID_SOCKET )
                return new CSocket( hSocket, "Accepted Socket", &sockAddr );

            int nError = LastError();
            if ( nError == EINTR || nError == ECONNABORTED )
                continue;           // a signal, or a client that gave up
            if ( ! m_bBlocking && ( nError == EWOULDBLOCK || nError == EAGAIN ) )
                return NULL;
            IASLIB_THROW_SOCKET_EXCEPTION( nError );
        }
    }

    CSocket *CServerSocket::Accept( int nTimeoutMs )
    {
        if ( m_hSocket == NULL_SOCKET )
        {
            return NULL;
        }

        if ( WaitForReadable( m_hSocket, nTimeoutMs ) <= 0 )
        {
            return NULL;
        }

        struct sockaddr_storage sockAddr;
        socklen_t nAddrLen = sizeof( sockAddr );
        memset( &sockAddr, 0, sizeof( sockAddr ) );

        SOCKET hSocket = accept( m_hSocket, (struct sockaddr *)&sockAddr, &nAddrLen );
        if ( hSocket == INVALID_SOCKET )
        {
            return NULL;
        }

        return new CSocket( hSocket, "Accepted Socket", &sockAddr );
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
