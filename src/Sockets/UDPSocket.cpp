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
#include "Socket.h"
#include "SocketWait.h"
#include <errno.h>
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CUDPSocket, CObject )

        // (Neither constructor checked much: the client one asked
        // getsockname() for an address with a length of 0 and read whatever
        // was on the stack.) The client is bound to port 0 so its port is
        // known straight away. SO_REUSEADDR is not set: on a UDP port it lets
        // a second socket bind the same port and steal datagrams.
    void CUDPSocket::Open( int nPort )
    {
        CSocket::InitializeNetworking();
        m_hSocket = NULL_SOCKET;
        m_nPort = 0;
        m_addrIPAddress = new CInternetAddress();

        if ( nPort < 0 || nPort > 65535 )
            IASLIB_THROW_SOCKET_EXCEPTION( EINVAL );

        SOCKET hSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if ( hSocket == INVALID_SOCKET )
            IASLIB_THROW_SOCKET_EXCEPTION( CSocket::LastError() );
        CSocket::PrepareHandle( hSocket );

        struct sockaddr_in listen_addr;
        memset( &listen_addr, 0, sizeof( listen_addr ) );
        listen_addr.sin_family = AF_INET;
        listen_addr.sin_port = htons( (unsigned short)nPort );
        listen_addr.sin_addr.s_addr = htonl( INADDR_ANY );

        if ( bind( hSocket, (struct sockaddr *)&listen_addr, sizeof( listen_addr ) ) == SOCKET_ERROR )
        {
            int nError = CSocket::LastError();
            CSocket::CloseHandle( hSocket );
            IASLIB_THROW_SOCKET_EXCEPTION( nError );
        }
        m_hSocket = hSocket;

        socklen_t nBoundLen = sizeof( listen_addr );
        if ( getsockname( m_hSocket, (struct sockaddr *)&listen_addr, &nBoundLen ) != 0 )
        {
            listen_addr.sin_port = htons( (unsigned short)nPort );
        }
        m_nPort = ntohs( listen_addr.sin_port );
        m_addrIPAddress->SetAddress( (struct sockaddr *)&listen_addr );
        m_strAddress = m_addrIPAddress->toStringWithPort();
    }

    CUDPSocket::CUDPSocket( int nPort )
    {
        m_addrIPAddress = NULL;
        try
        {
            Open( nPort );
        }
        catch ( ... )
        {
            delete m_addrIPAddress;
            throw;
        }
    }

    CUDPSocket::CUDPSocket( void )
    {
        m_addrIPAddress = NULL;
        try
        {
            Open( 0 );
        }
        catch ( ... )
        {
            delete m_addrIPAddress;
            throw;
        }
    }

    CUDPSocket::~CUDPSocket()
    {
        Close();
        delete m_addrIPAddress;
        m_addrIPAddress = NULL;
    }

    int CUDPSocket::Read( char *pchBuffer, int nBufferSize, CInternetAddress &incomingAddress )
    {
        if ( m_hSocket == NULL_SOCKET || pchBuffer == NULL || nBufferSize <= 0 )
        {
            return -1;
        }

        struct sockaddr_storage remaddr;
        int n;
        for ( ;; )
        {
            socklen_t len = sizeof( remaddr );
            memset( &remaddr, 0, sizeof( remaddr ) );
            n = (int)recvfrom( m_hSocket, pchBuffer, nBufferSize, 0, (struct sockaddr *)&remaddr, &len );
            if ( n >= 0 )
                break;
            int nError = CSocket::LastError();
            if ( nError == EINTR )
                continue;
            if ( nError == EMSGSIZE )
            {
                    // Windows fills the buffer and reports the rest as an
                    // error; POSIX systems just cut the datagram short.
                n = nBufferSize;
                break;
            }
            return -1;
        }

        if ( n < nBufferSize )
        {
            pchBuffer[n] = '\0';
        }
        incomingAddress.SetAddress( (struct sockaddr *)&remaddr );
        return n;
    }

    int CUDPSocket::Read( char *pchBuffer, int nBufferSize, CInternetAddress &incomingAddress, int nTimeoutMs )
    {
        if ( m_hSocket == NULL_SOCKET )
        {
            return -1;
        }
        int nReady = WaitForReadable( m_hSocket, nTimeoutMs );
        if ( nReady == 0 )
        {
            return CSocket::READ_TIMEOUT;
        }
        if ( nReady < 0 )
        {
            return -1;
        }
        return Read( pchBuffer, nBufferSize, incomingAddress );
    }

    int CUDPSocket::Send( const char *pchBuffer, int nBufferSize, const CInternetAddress &targetAddress )
    {
        if ( m_hSocket == NULL_SOCKET || pchBuffer == NULL || nBufferSize < 0 )
        {
            return -1;
        }

            // The socket is IPv4, so use the target's IPv4 address. (A name
            // such as "localhost" can resolve to ::1 first, which an IPv4
            // socket cannot send to.)
        const struct sockaddr_in *pTarget = targetAddress.GetIPv4();
        if ( pTarget == NULL )
        {
            return -1;
        }

        for ( ;; )
        {
            int nSent = (int)sendto( m_hSocket, pchBuffer, nBufferSize, 0, (const struct sockaddr *)pTarget, sizeof( struct sockaddr_in ) );
            if ( nSent >= 0 || CSocket::LastError() != EINTR )
                return ( nSent >= 0 ) ? nSent : -1;
        }
    }

        // (This closed the handle without forgetting it, so the destructor
        // closed it again, possibly after the number was reused.)
    void CUDPSocket::Close( void )
    {
        CSocket::CloseHandle( m_hSocket );
        m_hSocket = NULL_SOCKET;
    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__
