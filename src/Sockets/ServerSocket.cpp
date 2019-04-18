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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#ifdef IASLIB_WIN32__
typedef int socklen_t;
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CServerSocket, CSocket );

    /**
     * Server Socket Constructor
     *
     * This constructor builds a server socket. It binds to a port
     * and can be used to wait for a connection to be made.
     *
     * @param nPort
     *      The port number to bind the server to. Only connections
     *      made to this port will connect to this server socket.
     * @param nMaxBacklog
     *      The maximum number of connections that can be waiting on
     *      this socket without being "accepted".
     */
    CServerSocket::CServerSocket( int nPort, int nMaxBacklog ) : CSocket( nPort )
    {
        if ( m_hSocket != NULL_SOCKET )
            listen( m_hSocket, nMaxBacklog );
    }

    /**
     * Server Socket Destructor
     *
     * The destructor cleans up the socket object.
     */
    CServerSocket::~CServerSocket( void )
    {
        int nOption = 1;
            // We have to set the re-use address so that we can re-start the server without waiting
            // for the TIME_WAIT to run out. This is usually about 5 minutes, which is a big deal.
            // We had to un-set this before we did an accept, so we re-set it here, just in case
            // there's a carry-over between processes, which there shouldn't be.

    #ifdef IASLIB_WIN32__
        setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nOption , sizeof( nOption )  );
    #else
        setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&nOption , sizeof( nOption )  );
    #endif
    }

    /**
     * Accept
     *
     * This method waits for a connection to be made from a client
     * to a server socket. It returns a new <code>CSocket</code>
     * object that contains the new connection to the client 
     * computer.
     */
    CSocket *CServerSocket::Accept( void )
    {
        SOCKET              hSocket;
        CSocket            *pRetVal = NULL;
        struct sockaddr_in *pSockAddr = new struct sockaddr_in;
        socklen_t          *pnAddrLen = new socklen_t;

        *pnAddrLen = sizeof( struct sockaddr_in );

        hSocket = accept( m_hSocket, (struct sockaddr *)pSockAddr, pnAddrLen );


        if ( hSocket == SOCKET_ERROR )
        {
            throw( new CSocketException( errno ) );
        }
        
        if ( ( hSocket != INVALID_SOCKET ) && ( hSocket != SOCKET_ERROR ) )
            pRetVal = new CSocket( hSocket, "New Socket", pSockAddr );

        delete pnAddrLen;
        delete pSockAddr;
 
        return pRetVal;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
