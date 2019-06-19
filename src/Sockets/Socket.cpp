/**
 *  Socket Class
 *
 *      This base class provides platform independent support for TCP/IP
 * sockets. This is the base class for both the server and client socket
 * classes.
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
#include "Socket.h"
#include "SocketException.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#ifdef IASLIB_WIN32__
namespace IASLib
{
    bool CSocket::m_bInitialized = false;
    WSADATA CSocket::m_wsaData;
}
    #define INET_ADDRSTRLEN 36

typedef int socklen_t;
#endif

#ifdef IASLIB_DEC__
    #define INET_ADDRSTRLEN 36
#endif

#ifndef IASLIB_SUN__
    #ifdef IASLIB_MULTI_THREADED__
    #include "Mutex.h"

    static IASLib::CMutex mutexNTOP;
#endif

static const char *inet_ntop( int iAddrFamily, unsigned long *addrConvert, char *strBuffer, int nMaxLen )
{
    if ( iAddrFamily == AF_INET )
    {
        struct in_addr  inaddrConvert;
        inaddrConvert.s_addr = *addrConvert;
#ifdef IASLIB_MULTI_THREADED__
        mutexNTOP.Lock();
#endif
        char *pResult = inet_ntoa( inaddrConvert );
        if ( pResult )
		{
#ifdef IASLIB_WIN32__
			strncpy_s( strBuffer, nMaxLen, pResult, nMaxLen );
#else
			strncpy( strBuffer, pResult, nMaxLen );
#endif
		}
        else
		{
#ifdef IASLIB_WIN32__
            strcpy_s( strBuffer, nMaxLen, "Unknown Address" );
#else
            strcpy( strBuffer, "Unknown Address" );
#endif
		}

#ifdef IASLIB_MULTI_THREADED__
        mutexNTOP.Unlock();
#endif
    }
    else
    {
        // DO IPv6 conversion
    }
    return strBuffer;
}
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CSocket, CObject );

    /**
     * Configured client socket Constructor
     *
     *      This constructor takes a configuration object and builds a
     * client-connected socket using the configuration settings passed in.
     *
     * @param config
     *          The configuration data to use when building the socket
     * @param strConnectTo
     *          The remote host, or IP address to connect to.
     * @param nPort
     *          The remote port number to attach to.
     */
    CSocket::CSocket( CSocketConfig config, const char *strConnectTo, int nPort )
    {
        m_nPort = 0;
        m_addrIPAddress = 0;
        m_addrLocalIPAddress = 0;
        m_nLocalPort = 0;

    #ifdef IASLIB_WIN32__
        if ( ! m_bInitialized )
        {
            WSAStartup( 2, &m_wsaData );
            m_bInitialized = true;
        }
    #endif

        struct hostent *pHost = gethostbyname( strConnectTo );
	    struct sockaddr_in connect_addr;

        m_hSocket = socket( AF_INET, SOCK_STREAM, 0 );

        connect_addr.sin_family = AF_INET;           // Connect to a TCP/IP Port
	    connect_addr.sin_port = htons( (u_short)nPort );	     // The Port Number to attach to.
        if ( pHost )
        {
	        connect_addr.sin_addr.s_addr = (unsigned long)*((unsigned long *)(pHost->h_addr_list[0]) );	// The address of the host machine.
            m_strAddress.Format( "%s:%d", inet_ntoa( connect_addr.sin_addr ), nPort );
        }
        else
        {
            connect_addr.sin_addr.s_addr = inet_addr( strConnectTo );
            m_strAddress.Format( "%s:%d", inet_ntoa( connect_addr.sin_addr ), nPort );
        }

        if ( connect( m_hSocket,(struct sockaddr *)&connect_addr, sizeof(struct sockaddr_in) ) == SOCKET_ERROR )
        {
    #ifdef IASLIB_WIN32__
            closesocket( m_hSocket );
    #else
            close( m_hSocket );
    #endif
            m_hSocket = NULL_SOCKET;
	    }
        else
        {
            socklen_t nNameSize;

            if ( getsockname( m_hSocket, (sockaddr *)&connect_addr, &nNameSize ) != SOCKET_ERROR )
            {
                m_nPort = ntohs( connect_addr.sin_port );
                m_addrIPAddress = ntohl( connect_addr.sin_addr.s_addr );
            }
            struct in_addr addrConvert;
            addrConvert.s_addr = htonl( m_addrIPAddress );
            m_strLocalAddress.Format( "%s:%d", inet_ntoa( addrConvert ), m_nPort );
            int temp = config.getSendBufferSize();
            setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, &temp, sizeof(int) );
            temp = config.getReceiveBufferSize();
            setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, &temp, sizeof(int) );
            temp = config.getTimeout();
            setsockopt( m_hSocket, SOL_SOCKET, SO_RCVTIMEO, &temp, sizeof(int));
            setsockopt( m_hSocket, SOL_SOCKET, SO_SNDTIMEO, &temp, sizeof(int));
            temp = config.isTcpNoDelay() ? 1:0;
            setsockopt( m_hSocket, IPPROTO_TCP, TCP_NODELAY, &temp, sizeof(int));
            temp = config.isLinger() ? 1:0;
            setsockopt( m_hSocket, SOL_SOCKET, SO_LINGER, &temp, sizeof(int));
            temp = config.isReuseSocket() ? 1:0;
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEPORT, &temp, sizeof(int));
        }
    }

    /**
     * Configured server socket Constructor
     *
     *      This constructor takes a configuration object and builds a
     * server socket using the configuration settings passed in.
     *
     * @param config
     *          The configuration data to use when building the socket
     * @param nPort
     *          The port to bind to.
     * @param strBoundAddress
     *          The address to bind to. By default, this is NULL, meaning to
     * attach to all available IP interfaces.
     */
    CSocket::CSocket( CSocketConfig config, int nPort, const char *strBoundAddress )
    {
        int     nOption = 1;
#ifndef IASLIB_NO_LINT__
        m_bBlocking = config.isBlocking();
#endif

        m_nPort = 0;
        m_addrIPAddress = 0;
        m_addrLocalIPAddress = 0;
        m_nLocalPort = 0;

    #ifdef IASLIB_WIN32__
        if ( ! m_bInitialized )
        {
            WSAStartup( 2, &m_wsaData );
            m_bInitialized = true;
        }
    #endif
	    struct sockaddr_in	listen_addr;

        m_hSocket = socket( AF_INET, SOCK_STREAM, 0 );

        if ( m_hSocket != INVALID_SOCKET )
        {
                // We have to set the re-use address so that we can re-start the server without waiting
                // for the TIME_WAIT to run out. This is usually about 5 minutes, which is a big deal.
    #ifndef IASLIB_WIN32__
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&nOption , sizeof( nOption )  );
    #else
            setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nOption , sizeof( nOption )  );
    #endif

            int temp = config.getSendBufferSize();
            setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, &temp, sizeof(int) );
            temp = config.getReceiveBufferSize();
            setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, &temp, sizeof(int) );
            temp = config.isTcpNoDelay() ? 1:0;
            setsockopt( m_hSocket, IPPROTO_TCP, TCP_NODELAY, &temp, sizeof(int));
            temp = config.isLinger() ? 1:0;
            setsockopt( m_hSocket, SOL_SOCKET, SO_LINGER, &temp, sizeof(int));
            temp = config.getBacklogSize();

            listen_addr.sin_family = AF_INET;           // Listen for Internet (TCP/IP) connections
            listen_addr.sin_port = htons( (u_short)nPort );	    // Assign the Port in Network byte order
            listen_addr.sin_addr.s_addr = INADDR_ANY;	// Allow connection on any valid IP for this machine

            if ( bind( m_hSocket,(struct sockaddr *)&listen_addr, sizeof(struct sockaddr_in) ) == SOCKET_ERROR )
            {
    #ifdef IASLIB_WIN32__
                closesocket( m_hSocket );
    #else
                close( m_hSocket );
    #endif
                m_hSocket = NULL_SOCKET;
                IASLIB_THROW_SOCKET_EXCEPTION(errno);
            }
            else
            {
                m_nPort = nPort;
                m_addrIPAddress = INADDR_ANY;
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
            }
            struct in_addr addrConvert;
            addrConvert.s_addr = htonl( m_addrIPAddress );
            m_strAddress.Format( "%s:%d", inet_ntoa( addrConvert ), m_nPort );
        }
        else
        {
            m_hSocket = NULL_SOCKET;
            IASLIB_THROW_SOCKET_EXCEPTION(errno);
        }
    }

    /**
     *  Pre-existing Socket Constructor
     *
     *      This constructor takes an already existing socket and wraps it
     * in a class. This is especially useful for sockets resulting from
     * accept calls.
     *
     * @param hSocket
     *          The handle of the socket that already exists.
     * @param strSockName
     *          A text name to assign to the socket. Informational only.
     * @param AddressIn
     *          A pointer to the sockaddr_in structure that includes the
     *          address information for the socket being attached.
     */
    CSocket::CSocket( SOCKET hSocket, const char *strSockName, void *AddressIn )
    {
        struct sockaddr_in *remote_addr = (struct sockaddr_in *)AddressIn;

        socklen_t nNameSize;

        m_nPort = 0;
        m_addrIPAddress = 0;
        m_addrLocalIPAddress = 0;
        m_nLocalPort = 0;

        m_strSocketName = strSockName;

    #ifdef IASLIB_WIN32__
        if ( ! m_bInitialized )
        {
            WSAStartup( 2, &m_wsaData );
            m_bInitialized = true;
        }
    #endif

        if ( ( hSocket != NULL_SOCKET ) && ( hSocket != INVALID_SOCKET ) )
        {
            struct sockaddr_in connect_addr;

            m_hSocket = hSocket;

            if ( AddressIn )
            {
                m_nPort = ntohs( remote_addr->sin_port );
                m_addrIPAddress = ntohl( remote_addr->sin_addr.s_addr );

                if ( getsockname( m_hSocket, (sockaddr *)&connect_addr, &nNameSize ) != SOCKET_ERROR )
                {
                    m_nLocalPort = ntohs( connect_addr.sin_port );
                    m_addrLocalIPAddress = ntohl( connect_addr.sin_addr.s_addr );
                }
            }
            else
            {
                if ( getsockname( m_hSocket, (sockaddr *)&connect_addr, &nNameSize ) != SOCKET_ERROR )
                {
                    m_nLocalPort = ntohs( connect_addr.sin_port );
                    m_addrLocalIPAddress = ntohl( connect_addr.sin_addr.s_addr );
                }

                if ( getpeername( m_hSocket, (sockaddr *)&connect_addr, &nNameSize ) != SOCKET_ERROR )
                {
                    m_nPort = ntohs( connect_addr.sin_port );
                    m_addrIPAddress = ntohl( connect_addr.sin_addr.s_addr );
                }
            }

            char strIPBuff[ INET_ADDRSTRLEN ];

            struct in_addr addrConvert;
            addrConvert.s_addr = htonl( m_addrIPAddress );
            m_strAddress.Format( "%s:%d", inet_ntop(AF_INET, (unsigned long *)&(addrConvert.s_addr), strIPBuff, INET_ADDRSTRLEN ), m_nPort );
            addrConvert.s_addr = htonl( m_addrLocalIPAddress );
            m_strLocalAddress.Format( "%s:%d", inet_ntop(AF_INET, (unsigned long *)&(addrConvert.s_addr), strIPBuff, INET_ADDRSTRLEN ), m_nLocalPort );
        }
        else
        {
            m_hSocket = NULL_SOCKET;
        }
    }

    /**
     *  Server Socket Constructor
     *
     *      This constructor creates and binds a port for listening and
     * accepting connections from a client.
     *
     * @param nPort
     *          The Berkeley Sockets port to connect to, from 0-65535.
     * @param bBlocking
     *          Is the port a blocking port, or does the call to listen
     *          return immediately.
     */
    CSocket::CSocket( int nPort, bool bBlocking )
    {
        int     nOption = 1;
#ifndef IASLIB_NO_LINT__
        bBlocking = bBlocking;
#endif

        m_nPort = 0;
        m_addrIPAddress = 0;
        m_addrLocalIPAddress = 0;
        m_nLocalPort = 0;

    #ifdef IASLIB_WIN32__
        if ( ! m_bInitialized )
        {
            WSAStartup( 2, &m_wsaData );
            m_bInitialized = true;
        }
    #endif
	    struct sockaddr_in	listen_addr;

        m_hSocket = socket( AF_INET, SOCK_STREAM, 0 );

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

            if ( bind( m_hSocket,(struct sockaddr *)&listen_addr, sizeof(struct sockaddr_in) ) == SOCKET_ERROR )
            {
    #ifdef IASLIB_WIN32__
                closesocket( m_hSocket );
    #else
                close( m_hSocket );
    #endif
                m_hSocket = NULL_SOCKET;
                IASLIB_THROW_SOCKET_EXCEPTION(errno);
            }
            else
            {
                m_nPort = nPort;
                m_addrIPAddress = INADDR_ANY;
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
            }
            struct in_addr addrConvert;
            addrConvert.s_addr = htonl( m_addrIPAddress );
            m_strAddress.Format( "%s:%d", inet_ntoa( addrConvert ), m_nPort );
        }
        else
        {
            m_hSocket = NULL_SOCKET;
            IASLIB_THROW_SOCKET_EXCEPTION(errno);
        }
    }

    /**
     *  Client Socket Constructor
     *
     *      This constructor takes a remote host and a port to connect to
     * and then attempts to connect to that host via that port.
     *
     * @param strConnectTo
     *          The host name or IP address to connect to.
     * @param nPort
     *          The remote port to connect to.
     */
    CSocket::CSocket( const char *strConnectTo, int nPort )
    {
        m_nPort = 0;
        m_addrIPAddress = 0;
        m_addrLocalIPAddress = 0;
        m_nLocalPort = 0;

    #ifdef IASLIB_WIN32__
        if ( ! m_bInitialized )
        {
            WSAStartup( 2, &m_wsaData );
            m_bInitialized = true;
        }
    #endif

        struct hostent *pHost = gethostbyname( strConnectTo );
	    struct sockaddr_in connect_addr;

        m_hSocket = socket( AF_INET, SOCK_STREAM, 0 );

        connect_addr.sin_family = AF_INET;           // Connect to a TCP/IP Port
	    connect_addr.sin_port = htons( (u_short)nPort );	     // The Port Number to attach to.
        if ( pHost )
        {
	        connect_addr.sin_addr.s_addr = (unsigned long)*((unsigned long *)(pHost->h_addr_list[0]) );	// The address of the host machine.
            m_strAddress.Format( "%s:%d", inet_ntoa( connect_addr.sin_addr ), nPort );
        }
        else
        {
            connect_addr.sin_addr.s_addr = inet_addr( strConnectTo );
            m_strAddress.Format( "%s:%d", inet_ntoa( connect_addr.sin_addr ), nPort );
        }

        if ( connect( m_hSocket,(struct sockaddr *)&connect_addr, sizeof(struct sockaddr_in) ) == SOCKET_ERROR )
        {
    #ifdef IASLIB_WIN32__
            closesocket( m_hSocket );
    #else
            close( m_hSocket );
    #endif
            m_hSocket = NULL_SOCKET;
	    }
        else
        {
            socklen_t nNameSize;

            if ( getsockname( m_hSocket, (sockaddr *)&connect_addr, &nNameSize ) != SOCKET_ERROR )
            {
                m_nPort = ntohs( connect_addr.sin_port );
                m_addrIPAddress = ntohl( connect_addr.sin_addr.s_addr );
            }
            struct in_addr addrConvert;
            addrConvert.s_addr = htonl( m_addrIPAddress );
            m_strLocalAddress.Format( "%s:%d", inet_ntoa( addrConvert ), m_nPort );
        }
    }

    /**
     *  Socket Destructor
     *
     *      This method is used to clean up a socket when the object is
     * being destroyed.
     */
    CSocket::~CSocket()
    {
        if ( m_hSocket != NULL_SOCKET )
        {
    #ifdef IASLIB_WIN32__
            closesocket( m_hSocket );
    #else
            close( m_hSocket );
    #endif
        }
        m_hSocket = NULL_SOCKET;
    }

    /**
     * Read Method
     *
     * This method reads a single unsigned character from the socket stream.
     */
    unsigned char CSocket::Read( void )
    {
        char pchBuf[2];

        if ( m_hSocket != NULL_SOCKET )
        {
            int nRetVal = recv( m_hSocket, pchBuf, 1, 0 );

            if ( nRetVal == SOCKET_ERROR )
                throw( new CSocketException( errno ) );

            if ( nRetVal == 0 )
                throw( new CSocketException( EPIPE ) );

            return (unsigned char)pchBuf[0];
        }
        return (unsigned char)'\0';
    }

    /**
     * Read Method
     *
     * This method reads from the socket stream up to the size of the
     * buffer. This method returns the actual number of bytes read into the
     * buffer.
     *
     * @param pchBuffer
     *          The buffer to store the data read from the socket in.
     * @param nBufferSize
     *          The size of the buffer for storing the data.
     */
    int CSocket::Read( char *pchBuffer, int nBufferSize )
    {
        int nRet;

        if ( m_hSocket != NULL_SOCKET )
        {
            if ( ( nRet = recv( m_hSocket, pchBuffer, nBufferSize, 0 ) ) != SOCKET_ERROR )
            {
                return nRet;
            }

            if ( nRet == SOCKET_ERROR )
            {
                throw( new CSocketException( errno ) );
            }
        }
        return 0;
    }

    /**
     * Send Method
     *
     * This method sends a single unsigned character to the socket.
     *
     * @param chSend
     *          The unsigned character to be written to the socket.
     */
    int CSocket::Send( unsigned char chSend )
    {
        char pchBuf[1];

        if ( m_hSocket != NULL_SOCKET )
        {
            pchBuf[0] = (char)chSend;
            if ( send( m_hSocket, pchBuf, 1, 0 ) != SOCKET_ERROR )
            {
                return 1;
            }
        }
        return 0;
    }

    /**
     * Send Method
     *
     * This method sends all of the data in the buffer, up to the size
     * specified by nBufferSize, onto the socket stream.
     * This method assures that all data is sent, even if it requires
     * multiple calls to the socket send routine.
     *
     * @param pchBuffer
     *          The buffer to be sent to the socket.
     * @param nBufferSize
     *          The length of the data in the buffer to be transmitted.
     */
    int CSocket::Send( const char *pchBuffer, int nBufferSize )
    {
        int nSent = 0;

        if ( m_hSocket != NULL_SOCKET )
        {
            int nRet;

            while ( nSent < nBufferSize )
            {
    #ifdef IASLIB_WIN32__
                nRet = send( m_hSocket, pchBuffer, nBufferSize, 0 );
                if ( nRet != SOCKET_ERROR )
                {
                    nSent = nBufferSize;
    #else
                nRet = send( m_hSocket, &(pchBuffer[ nSent ] ), nBufferSize - nSent, 0 );
                if ( nRet != SOCKET_ERROR )
                {
                    nSent += nRet;
    #endif

                }
                else
                {
                    throw( new CSocketException( errno ) );
                }
            }
        }
        return nSent;
    }

    /**
     * GetAddressString
     *
     * This method converts an IP address into a human readable string.
     *
     * @param bRemoteAddress
     *      Convert the remote address instead of the local address.
     * @param bIncludePort
     *      Include the port number on the end of the IP address.
     */
    const char *CSocket::GetAddressString( bool bRemoteAddress, bool bIncludePort )
    {
        struct in_addr addrConvert;
        char    strIPBuff[ INET_ADDRSTRLEN ];
        int     nPort = 0;

        if ( bRemoteAddress )
        {
            addrConvert.s_addr = htonl( m_addrIPAddress );
            nPort = m_nPort;
        }
        else
        {
            addrConvert.s_addr = htonl( m_addrLocalIPAddress );
            nPort = m_nLocalPort;
        }

        if ( bIncludePort )
        {
            m_strAddress.Format( "%s:%d", inet_ntop(AF_INET, (unsigned long *)&(addrConvert.s_addr), strIPBuff, INET_ADDRSTRLEN ), nPort );
        }
        else
        {
            m_strAddress.Format( "%s", inet_ntop(AF_INET, (unsigned long *)&(addrConvert.s_addr), strIPBuff, INET_ADDRSTRLEN ) );
        }

        return (const char *)m_strAddress;

    }

    /**
     *  Close
     *
     *      This method closes a socket.
     */
    void CSocket::Close( void )
    {
        if ( m_hSocket )
        {
    #ifdef IASLIB_WIN32__
            closesocket( m_hSocket );
    #else
            close( m_hSocket );
    #endif
            m_hSocket = NULL_SOCKET;
        }
        m_nPort = 0;
        m_addrIPAddress = 0;
    }

    /**
     * SetNonBlocking
     *
     * This method sets the blocking state of a socket. Note that
     * this has no effect on a Windows system, which only supports
     * blocking sockets.
     *
     * @param bDontBlock
     *      Sets whether the socket should block or not when a
     *      request is made.
     */
    void CSocket::SetNonBlocking( bool bDontBlock )
    {
        if ( m_hSocket )
        {
    #ifdef IASLIB_WIN32__

#ifndef IASLIB_NO_LINT__
            bDontBlock = bDontBlock;
#endif
            return;
    #else
            int nOldFlags = fcntl( m_hSocket, F_GETFL );

            if ( bDontBlock )
            {
                nOldFlags |= O_NONBLOCK;
            }
            else
            {
                nOldFlags &= ~O_NONBLOCK;
            }

            fcntl( m_hSocket, F_SETFL, nOldFlags );
    #endif
        }
    }

    /**
     *  HasData
     *
     * This method checks if there is data waiting on the socket to be
     * received. If there is, it returns true, otherwise it returns false.
     */
    bool CSocket::HasData( void )
    {
        fd_set  readSet;
        timeval tvTimeout;
        int     nRet;

        memset( &readSet, 0, sizeof( fd_set ) );
#ifndef IASLIB_NO_LINT__
#pragma warning(disable:4127)
#endif
        FD_SET( m_hSocket, &readSet );
#ifndef IASLIB_NO_LINT__
#pragma warning(default:4127)
#endif

        tvTimeout.tv_sec = 0;
        tvTimeout.tv_usec = 100;

        nRet = select( 1, &readSet, NULL, NULL, &tvTimeout );

        return ( nRet == 1 );
    }

    /**
     * inet_ntop
     *
     * Converts an internet address into a printable (ASCII)
     * address that's human readable.
     *
     * @param iAddrFamily
     *      The address family.
     * @param addrConvert
     *      The address to be converted.
     * @param strBuffer
     *      The buffer to store the result in.
     * @param nMaxLen
     *      The maximum length of the string that can be stored
     *      in the buffer.
     */
    const char *CSocket::inet_ntop( int iAddrFamily, unsigned long *addrConvert, char *strBuffer, int nMaxLen )
    {
        return ::inet_ntop( iAddrFamily, addrConvert, strBuffer, nMaxLen );
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
