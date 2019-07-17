/**
 *  UDP Socket Class
 *
 *      This base class provides platform independent support for UDP
 * sockets. This is the base class for both the UDP server and client socket
 * classes.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 7/11/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "UDPSocket.h"
#include "SocketException.h"

namespace IASLib
{
        // Server port constructor
    CUDPSocket::CUDPSocket( int nPort )
    {
        m_addrIPAddress = NULL;
        int     nOption = 1;

    #ifdef IASLIB_WIN32__
        if ( ! m_bInitialized )
        {
            WSAStartup( 2, &m_wsaData );
            m_bInitialized = true;
        }
    #endif
	    struct sockaddr_in	listen_addr;

        m_hSocket = socket( AF_INET, SOCK_DGRAM, 0 );

        if ( m_hSocket != INVALID_SOCKET )
        {
                // We have to set the re-use address so that we can re-start the server without waiting
                // for the TIME_WAIT to run out. This is usually about 5 minutes, which is a big deal.
    #ifndef IASLIB_WIN32__
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&nOption , sizeof( nOption )  );
    #else
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nOption , sizeof( nOption )  );
    #endif

            listen_addr.sin_family = AF_INET;           // Listen for Internet (TCP/IP) connections
            listen_addr.sin_port = htons( (u_short)nPort );	    // Assign the Port in Network byte order
            listen_addr.sin_addr.s_addr = INADDR_ANY;	// Allow connection on any valid IP for this machine

            if ( bind( m_hSocket,(struct sockaddr *)&listen_addr, sizeof(struct sockaddr_in) ) < 0 )
            {
    #ifdef IASLIB_WIN32__
                closesocket( m_hSocket );
    #else
                close( m_hSocket );
    #endif
                m_hSocket = NULL_SOCKET;
                IASLIB_THROW_SOCKET_EXCEPTION(errno);
            }

            m_nPort = nPort;
            m_addrIPAddress = new CRemoteAddress( "0.0.0.0", nPort, SOCK_DGRAM );
                // Now that we've bound the correct socket, we need to un-set the reuse-address option,
                // because otherwise the accept call creates a socket that matches this one, apparently,
                // that includes re-using the same address, which is really, really bad. We can get
                // two requests on the same *bleep*ing socket, which basically makes the socket library
                // go off on a quest to use all of the CPU. So, we un-do the option here.
            nOption = 0;
    #ifdef IASLIB_WIN32__
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&nOption , sizeof( nOption )  );
    #else
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&nOption , sizeof( nOption )  );
    #endif
            m_strAddress = m_addrIPAddress->toStringWithPort();
        }
        else
        {
            m_hSocket = NULL_SOCKET;
            IASLIB_THROW_SOCKET_EXCEPTION(errno);
        }

    }
    CUDPSocket::CUDPSocket( )
    {
        int     nOption = 1;

    #ifdef IASLIB_WIN32__
        if ( ! m_bInitialized )
        {
            WSAStartup( 2, &m_wsaData );
            m_bInitialized = true;
        }
    #endif
	    struct sockaddr	listen_addr;
        socklen_t addrLen = 0;

        m_hSocket = socket( AF_INET, SOCK_DGRAM, 0 );

        if ( m_hSocket != INVALID_SOCKET )
        {
                // We have to set the re-use address so that we can re-start the server without waiting
                // for the TIME_WAIT to run out. This is usually about 5 minutes, which is a big deal.
    #ifndef IASLIB_WIN32__
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&nOption , sizeof( nOption )  );
    #else
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nOption , sizeof( nOption )  );
    #endif
            if ( getsockname( m_hSocket, (sockaddr *)&listen_addr, &addrLen ) != 0)
            {
                // We can't get data from the socket -- that's bad.
    #ifdef IASLIB_WIN32__
                closesocket( m_hSocket );
    #else
                close( m_hSocket );
    #endif
                m_hSocket = NULL_SOCKET;
                IASLIB_THROW_SOCKET_EXCEPTION(errno);
            }
            m_nPort = CRemoteAddress::getPort( &listen_addr );

            m_addrIPAddress = new CRemoteAddress( &listen_addr );
            m_strAddress = m_addrIPAddress->toStringWithPort();
        }
        else
        {
            m_hSocket = NULL_SOCKET;
            IASLIB_THROW_SOCKET_EXCEPTION(errno);
        }

    }
    CUDPSocket::~CUDPSocket()
    {
        if ( m_hSocket )
    #ifdef IASLIB_WIN32__
            closesocket( m_hSocket );
    #else
            close( m_hSocket );
    #endif

        if ( m_addrIPAddress )
            delete m_addrIPAddress;
    }

    IMPLEMENT_OBJECT( CUDPSocket, CObject )

    int CUDPSocket::Read( char *pchBuffer, int nBufferSize, CRemoteAddress &incomingAddress )
    {
        struct sockaddr remaddr;
        socklen_t len = sizeof( struct sockaddr);
        int n = recvfrom( m_hSocket, (char *)pchBuffer, nBufferSize,
                    MSG_WAITALL, (struct sockaddr *) &remaddr,
                    &len );
        pchBuffer[n] = '\0';
        incomingAddress.SetAddress(&remaddr);
    }

    int CUDPSocket::Send( const char *pchBuffer, int nBufferSize, const CRemoteAddress &targetAddress )
    {
        int n, len;

        sendto( m_hSocket, (const char *)pchBuffer, nBufferSize,
            MSG_CONFIRM, (const struct sockaddr *)targetAddress,
                sizeof(struct sockaddr));
    }

    void CUDPSocket::Close( void )
    {
        if ( m_hSocket )
        {
#ifdef IASLIB_WIN32__
            closesocket( m_hSocket );
#else
            close( m_hSocket );
#endif
        }
    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__
