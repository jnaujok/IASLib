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
    void CUDPSocket::Open( int nPort, const char *strBindAddress )
    {
        m_hSocket = NULL_SOCKET;
        m_nPort = 0;
        m_nFamily = AF_INET;
        m_addrIPAddress = new CInternetAddress();

        int nError = 0;
        SOCKET hSocket = CSocket::BindHandle( strBindAddress, nPort, SOCK_DGRAM, IPPROTO_UDP, false, NULL, nError );
        if ( hSocket == INVALID_SOCKET )
            IASLIB_THROW_SOCKET_EXCEPTION( nError );
        m_hSocket = hSocket;

        struct sockaddr_storage bound;
        socklen_t nBoundLen = sizeof( bound );
        memset( &bound, 0, sizeof( bound ) );
        if ( getsockname( m_hSocket, (struct sockaddr *)&bound, &nBoundLen ) == 0 )
        {
            m_nFamily = bound.ss_family;
            m_addrIPAddress->SetAddress( (struct sockaddr *)&bound );
            m_nPort = m_addrIPAddress->GetPort();
        }
        m_strAddress = m_addrIPAddress->toStringWithPort();
    }

    CUDPSocket::CUDPSocket( int nPort )
    {
        m_addrIPAddress = NULL;
        try
        {
            Open( nPort, NULL );
        }
        catch ( ... )
        {
            delete m_addrIPAddress;
            throw;
        }
    }

    CUDPSocket::CUDPSocket( int nPort, const char *strBindAddress )
    {
        m_addrIPAddress = NULL;
        try
        {
            Open( nPort, strBindAddress );
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
            Open( 0, NULL );
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

        const struct sockaddr *pTarget = NULL;
        socklen_t nTargetLength = 0;
        struct sockaddr_in6 mapped;

        if ( m_nFamily == AF_INET6 )
        {
                // Either family: an IPv4 target goes as ::ffff:a.b.c.d.
            const struct sockaddr *pFirst = targetAddress.GetAddress();
            if ( pFirst == NULL )
                return -1;
            if ( pFirst->sa_family == AF_INET6 )
            {
                pTarget = pFirst;
                nTargetLength = sizeof( struct sockaddr_in6 );
            }
            else if ( pFirst->sa_family == AF_INET )
            {
                CInternetAddress::mapToIPv6( (const struct sockaddr_in *)pFirst, mapped );
                pTarget = (const struct sockaddr *)&mapped;
                nTargetLength = sizeof( mapped );
            }
        }
        else
        {
                // An IPv4 socket can only use the target's IPv4 address. (A
                // name such as "localhost" can resolve to ::1 first.)
            pTarget = (const struct sockaddr *)targetAddress.GetIPv4();
            nTargetLength = sizeof( struct sockaddr_in );
        }
        if ( pTarget == NULL )
        {
            return -1;
        }

        for ( ;; )
        {
            int nSent = (int)sendto( m_hSocket, pchBuffer, nBufferSize, 0, pTarget, nTargetLength );
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
