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
#include "InternetAddress.h"
#include "SocketWait.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mutex>
#ifdef IASLIB_WIN32__
#include "Platform.h"
#else
#include <fcntl.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#endif

    // Never raise SIGPIPE when the peer has closed the connection; send()
    // reports the error instead. (macOS has no MSG_NOSIGNAL; PrepareHandle()
    // sets SO_NOSIGPIPE there.)
#if defined( MSG_NOSIGNAL )
    #define IASLIB_SEND_FLAGS MSG_NOSIGNAL
#else
    #define IASLIB_SEND_FLAGS 0
#endif

namespace IASLib
{
    namespace
    {
        void setIntOption( SOCKET hSocket, int nLevel, int nOption, int nValue )
        {
            setsockopt( hSocket, nLevel, nOption, (const char *)&nValue, sizeof( nValue ) );
        }

            // (A timeout was passed as an int, which POSIX systems reject: they
            // take a timeval. Windows takes a DWORD of milliseconds.)
        void setTimeoutOption( SOCKET hSocket, int nOption, int nTimeoutMs )
        {
#ifdef IASLIB_WIN32__
            DWORD dwTimeout = (DWORD)nTimeoutMs;
            setsockopt( hSocket, SOL_SOCKET, nOption, (const char *)&dwTimeout, sizeof( dwTimeout ) );
#else
            struct timeval tvTimeout;
            tvTimeout.tv_sec = nTimeoutMs / 1000;
            tvTimeout.tv_usec = ( nTimeoutMs % 1000 ) * 1000;
            setsockopt( hSocket, SOL_SOCKET, nOption, (const char *)&tvTimeout, sizeof( tvTimeout ) );
#endif
        }

            // Everything in a configuration but blocking and the backlog.
        void applyConfig( SOCKET hSocket, const CSocketConfig &config )
        {
            if ( config.getSendBufferSize() > 0 )
                setIntOption( hSocket, SOL_SOCKET, SO_SNDBUF, config.getSendBufferSize() );
            if ( config.getReceiveBufferSize() > 0 )
                setIntOption( hSocket, SOL_SOCKET, SO_RCVBUF, config.getReceiveBufferSize() );
                // (Keep-alive was never applied.)
            setIntOption( hSocket, SOL_SOCKET, SO_KEEPALIVE, config.isKeepAlive() ? 1 : 0 );
            if ( config.getTimeout() > 0 )
            {
                setTimeoutOption( hSocket, SO_RCVTIMEO, config.getTimeout() );
                setTimeoutOption( hSocket, SO_SNDTIMEO, config.getTimeout() );
            }
            setIntOption( hSocket, IPPROTO_TCP, TCP_NODELAY, config.isTcpNoDelay() ? 1 : 0 );
                // (SO_LINGER takes a struct linger, not an int.)
            struct linger lingerOption;
            lingerOption.l_onoff = config.isLinger() ? 1 : 0;
            lingerOption.l_linger = (unsigned short)config.getLingerSeconds();
            setsockopt( hSocket, SOL_SOCKET, SO_LINGER, (const char *)&lingerOption, sizeof( lingerOption ) );
        }

            // The numeric address and port of a sockaddr, and its IPv4 address
            // in host order (0 for IPv6).
        bool describeAddress( const struct sockaddr *pAddress, CString &strIP, int &nPort, unsigned long &ulIPv4 )
        {
            strIP = CInternetAddress::addressToString( pAddress );
            nPort = CInternetAddress::getPort( pAddress );
            ulIPv4 = 0;
            if ( nPort < 0 )
            {
                nPort = 0;
                return false;
            }
            if ( pAddress->sa_family == AF_INET )
                ulIPv4 = ntohl( ((const struct sockaddr_in *)pAddress)->sin_addr.s_addr );
            return true;
        }

        void formatWithPort( CString &strOut, const CString &strIP, int nPort )
        {
            if ( strIP.IndexOf( ':' ) != NOT_FOUND )
                strOut.Format( "[%s]:%d", (const char *)strIP, nPort );
            else
                strOut.Format( "%s:%d", (const char *)strIP, nPort );
        }
    }

    IMPLEMENT_OBJECT( CSocket, CObject );

    void CSocket::InitializeNetworking( void )
    {
#ifdef IASLIB_WIN32__
            // (Each constructor started Winsock behind an unguarded static
            // flag, which two threads could race.)
        static std::once_flag onceStarted;
        std::call_once( onceStarted, []()
        {
            WSADATA wsaData;
            WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
        } );
#endif
    }

    int CSocket::LastError( void )
    {
#ifdef IASLIB_WIN32__
        int nError = WSAGetLastError();
        switch ( nError )
        {
            case WSAEINTR:              return EINTR;
            case WSAEBADF:              return EBADF;
            case WSAEACCES:             return EACCES;
            case WSAEINVAL:             return EINVAL;
            case WSAEMFILE:             return EMFILE;
            case WSAEWOULDBLOCK:        return EWOULDBLOCK;
            case WSAEINPROGRESS:        return EINPROGRESS;
            case WSAEALREADY:           return EALREADY;
            case WSAENOTSOCK:           return ENOTSOCK;
            case WSAEMSGSIZE:           return EMSGSIZE;
            case WSAEPROTONOSUPPORT:    return EPROTONOSUPPORT;
            case WSAEOPNOTSUPP:         return EOPNOTSUPP;
            case WSAEAFNOSUPPORT:       return EAFNOSUPPORT;
            case WSAEADDRINUSE:         return EADDRINUSE;
            case WSAEADDRNOTAVAIL:      return EADDRNOTAVAIL;
            case WSAENETUNREACH:        return ENETUNREACH;
            case WSAECONNABORTED:       return ECONNABORTED;
            case WSAECONNRESET:         return ECONNRESET;
            case WSAENOBUFS:            return ENOBUFS;
            case WSAENOTCONN:           return ENOTCONN;
            case WSAESHUTDOWN:          return EPIPE;
            case WSAETIMEDOUT:          return ETIMEDOUT;
            case WSAECONNREFUSED:       return ECONNREFUSED;
            case WSAEHOSTUNREACH:       return EHOSTUNREACH;
            default:                    return nError;
        }
#else
        return errno;
#endif
    }

    void CSocket::CloseHandle( SOCKET hSocket )
    {
        if ( hSocket == NULL_SOCKET )
            return;
#ifdef IASLIB_WIN32__
        closesocket( hSocket );
#else
        close( hSocket );
#endif
    }

    void CSocket::PrepareHandle( SOCKET hSocket )
    {
        if ( hSocket == NULL_SOCKET )
            return;
#if defined( SO_NOSIGPIPE )
        setIntOption( hSocket, SOL_SOCKET, SO_NOSIGPIPE, 1 );
#endif
#ifndef IASLIB_WIN32__
            // Not inherited by programs this one runs.
        fcntl( hSocket, F_SETFD, FD_CLOEXEC );
#endif
    }

    void CSocket::Init( void )
    {
        InitializeNetworking();
        m_hSocket = NULL_SOCKET;
        m_nPort = 0;
        m_addrIPAddress = 0;
        m_nLocalPort = 0;
        m_addrLocalIPAddress = 0;
        m_bBlocking = true;         // (was left uninitialised)
        m_internetAddress = NULL;
    }

        // Connects to the first of the host's addresses that answers. (It
        // used gethostbyname(), which is neither thread-safe nor IPv6-aware,
        // and copied 8 bytes out of a 4-byte address; it never checked
        // socket().)
    void CSocket::Connect( const char *strConnectTo, int nPort, CSocketConfig *pConfig )
    {
        if ( strConnectTo == NULL || nPort <= 0 || nPort > 65535 )
            return;

        m_strRemoteIP = strConnectTo;
        m_nPort = nPort;
        formatWithPort( m_strAddress, m_strRemoteIP, nPort );

        CString strService;
        strService.Format( "%d", nPort );

        struct addrinfo hints;
        memset( &hints, 0, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        struct addrinfo *pList = NULL;
        if ( getaddrinfo( strConnectTo, (const char *)strService, &hints, &pList ) != 0 || pList == NULL )
            return;

        for ( struct addrinfo *pAddr = pList; pAddr; pAddr = pAddr->ai_next )
        {
            SOCKET hSocket = socket( pAddr->ai_family, pAddr->ai_socktype, pAddr->ai_protocol );
            if ( hSocket == INVALID_SOCKET )
                continue;
            PrepareHandle( hSocket );
                // Before connecting: the buffer sizes set the TCP window, and
                // the send timeout bounds connect() on most systems.
            if ( pConfig )
                applyConfig( hSocket, *pConfig );
            if ( connect( hSocket, pAddr->ai_addr, (socklen_t)pAddr->ai_addrlen ) != SOCKET_ERROR )
            {
                m_hSocket = hSocket;
                break;
            }
            CloseHandle( hSocket );
        }
        freeaddrinfo( pList );

        if ( m_hSocket == NULL_SOCKET )
            return;

            // (The far end's port and address were set to the near end's.)
        ReadAddresses();
        if ( pConfig && ! pConfig->isBlocking() )
            SetNonBlocking( true );
    }

        // Binds a listening address. (The reuse option used to be cleared
        // again after binding, which is not needed, and errno was thrown on
        // Windows, where it is not the socket error.)
    void CSocket::Bind( const char *strBindIP, int nPort, CSocketConfig *pConfig )
    {
        if ( nPort < 0 || nPort > 65535 )
            IASLIB_THROW_SOCKET_EXCEPTION( EINVAL );

        struct sockaddr_storage address;
        socklen_t nAddressLength;
        memset( &address, 0, sizeof( address ) );

        if ( strBindIP == NULL || *strBindIP == '\0' )
        {
            struct sockaddr_in *pAny = (struct sockaddr_in *)&address;
            pAny->sin_family = AF_INET;
            pAny->sin_port = htons( (unsigned short)nPort );
            pAny->sin_addr.s_addr = htonl( INADDR_ANY );
            nAddressLength = (socklen_t)sizeof( struct sockaddr_in );
        }
        else
        {
            CInternetAddress bindAddress( strBindIP, nPort, SOCK_STREAM );
            if ( ! bindAddress.isValid() )
                IASLIB_THROW_SOCKET_EXCEPTION( EADDRNOTAVAIL );
            nAddressLength = bindAddress.GetAddressLength();
            memcpy( &address, bindAddress.GetAddress(), nAddressLength );
        }

        SOCKET hSocket = socket( address.ss_family, SOCK_STREAM, IPPROTO_TCP );
        if ( hSocket == INVALID_SOCKET )
            IASLIB_THROW_SOCKET_EXCEPTION( LastError() );
        PrepareHandle( hSocket );

            // Restart without waiting out TIME_WAIT (see IASLIB_REUSE_ADDRESS).
        setIntOption( hSocket, SOL_SOCKET, IASLIB_REUSE_ADDRESS, 1 );
        if ( pConfig )
        {
            applyConfig( hSocket, *pConfig );
#if defined( SO_REUSEPORT ) && ! defined( IASLIB_WIN32__ )
                // Several listeners sharing the port.
            if ( pConfig->isReuseSocket() )
                setIntOption( hSocket, SOL_SOCKET, SO_REUSEPORT, 1 );
#endif
        }

        if ( bind( hSocket, (struct sockaddr *)&address, nAddressLength ) == SOCKET_ERROR )
        {
            int nError = LastError();
            CloseHandle( hSocket );
            IASLIB_THROW_SOCKET_EXCEPTION( nError );
        }
        m_hSocket = hSocket;

            // The port actually bound (when asked for 0).
        struct sockaddr_storage bound;
        socklen_t nBoundLength = sizeof( bound );
        if ( getsockname( m_hSocket, (struct sockaddr *)&bound, &nBoundLength ) == 0 )
            describeAddress( (struct sockaddr *)&bound, m_strLocalIP, m_nLocalPort, m_addrLocalIPAddress );
        else
            describeAddress( (struct sockaddr *)&address, m_strLocalIP, m_nLocalPort, m_addrLocalIPAddress );
        formatWithPort( m_strLocalAddress, m_strLocalIP, m_nLocalPort );

            // A listening socket has no far end; it reports its own.
        m_nPort = m_nLocalPort;
        m_addrIPAddress = m_addrLocalIPAddress;
        m_strRemoteIP = m_strLocalIP;
        m_strAddress = m_strLocalAddress;

        if ( pConfig && ! pConfig->isBlocking() )
            SetNonBlocking( true );
    }

    void CSocket::ReadAddresses( const struct sockaddr *pKnownPeer )
    {
        struct sockaddr_storage address;
        socklen_t nLength = sizeof( address );

        if ( getsockname( m_hSocket, (struct sockaddr *)&address, &nLength ) == 0 )
        {
            describeAddress( (struct sockaddr *)&address, m_strLocalIP, m_nLocalPort, m_addrLocalIPAddress );
            formatWithPort( m_strLocalAddress, m_strLocalIP, m_nLocalPort );
        }

        const struct sockaddr *pPeer = NULL;
        nLength = sizeof( address );
        if ( getpeername( m_hSocket, (struct sockaddr *)&address, &nLength ) == 0 )
            pPeer = (struct sockaddr *)&address;
        else if ( pKnownPeer && CInternetAddress::addressLength( pKnownPeer ) )
            pPeer = pKnownPeer;

        if ( pPeer )
        {
            describeAddress( pPeer, m_strRemoteIP, m_nPort, m_addrIPAddress );
            formatWithPort( m_strAddress, m_strRemoteIP, m_nPort );
            delete m_internetAddress;
            m_internetAddress = new CInternetAddress( pPeer );
        }
    }

    /**
     * Configured client socket Constructor
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
        Init();
        Connect( strConnectTo, nPort, &config );
    }

    /**
     * Configured server socket Constructor
     *
     * @param config
     *          The configuration data to use when building the socket
     * @param nPort
     *          The port to bind to (0 for any free port).
     * @param strBoundAddress
     *          The address to bind to. By default, this is NULL, meaning to
     * attach to all available IPv4 interfaces.
     */
    CSocket::CSocket( CSocketConfig config, int nPort, const char *strBoundAddress )
    {
        Init();
        Bind( strBoundAddress, nPort, &config );
    }

    /**
     *  Pre-existing Socket Constructor
     *
     *      Wraps an already existing socket (especially one from accept()).
     *
     * @param hSocket
     *          The handle of the socket that already exists.
     * @param strSockName
     *          A text name to assign to the socket. Informational only.
     * @param AddressIn
     *          The peer's sockaddr, if known (it is read from the socket
     *          when it can be).
     */
    CSocket::CSocket( SOCKET hSocket, const char *strSockName, void *AddressIn )
    {
        Init();
        if ( strSockName )
            m_strSocketName = strSockName;

        if ( hSocket != NULL_SOCKET )
        {
            m_hSocket = hSocket;
            PrepareHandle( m_hSocket );
            ReadAddresses( (const struct sockaddr *)AddressIn );
        }
    }

    /**
     *  Server Socket Constructor
     *
     * @param nPort
     *          The port to bind, from 0-65535 (0 for any free port).
     * @param bBlocking
     *          Whether calls on the socket block. (This was ignored.)
     */
    CSocket::CSocket( int nPort, bool bBlocking )
    {
        Init();
        Bind( NULL, nPort, NULL );
        if ( ! bBlocking )
            SetNonBlocking( true );
    }

    /**
     *  Client Socket Constructor
     *
     * @param strConnectTo
     *          The host name or IP address to connect to.
     * @param nPort
     *          The remote port to connect to.
     */
    CSocket::CSocket( const char *strConnectTo, int nPort )
    {
        Init();
        Connect( strConnectTo, nPort, NULL );
    }

    CSocket::~CSocket()
    {
        CloseHandle( m_hSocket );
        m_hSocket = NULL_SOCKET;
        delete m_internetAddress;
        m_internetAddress = NULL;
    }

    unsigned char CSocket::Read( void )
    {
        unsigned char chRead = 0;
        int nRead = Read( (char *)&chRead, 1 );
        if ( nRead == 0 )
            IASLIB_THROW_SOCKET_EXCEPTION( IsConnected() ? EPIPE : ENOTCONN );
        return chRead;
    }

    int CSocket::Read( char *pchBuffer, int nBufferSize )
    {
        if ( m_hSocket == NULL_SOCKET || pchBuffer == NULL || nBufferSize <= 0 )
            return 0;

        for ( ;; )
        {
            int nRet = (int)recv( m_hSocket, pchBuffer, nBufferSize, 0 );
            if ( nRet != SOCKET_ERROR )
                return nRet;
            int nError = LastError();
            if ( nError != EINTR )
                IASLIB_THROW_SOCKET_EXCEPTION( nError );     // (threw a pointer)
        }
    }

    int CSocket::Send( unsigned char chSend )
    {
        char chBuffer = (char)chSend;
        return Send( &chBuffer, 1 );
    }

        // (The Windows branch sent the whole buffer again from the start on
        // every pass, and counted a partial send as complete.)
    int CSocket::Send( const char *pchBuffer, int nBufferSize )
    {
        if ( m_hSocket == NULL_SOCKET || pchBuffer == NULL || nBufferSize <= 0 )
            return 0;

        int nSent = 0;
        while ( nSent < nBufferSize )
        {
            int nRet = (int)send( m_hSocket, pchBuffer + nSent, nBufferSize - nSent, IASLIB_SEND_FLAGS );
            if ( nRet == SOCKET_ERROR )
            {
                int nError = LastError();
                if ( nError == EINTR )
                    continue;
                IASLIB_THROW_SOCKET_EXCEPTION( nError );
            }
            nSent += nRet;
        }
        return nSent;
    }

        // (This overwrote m_strAddress, so the next call with the other
        // arguments started from the wrong text.)
    const char *CSocket::GetAddressString( bool bInternetAddress, bool bIncludePort )
    {
        const CString &strIP = ( bInternetAddress ) ? m_strRemoteIP : m_strLocalIP;
        int nPort = ( bInternetAddress ) ? m_nPort : m_nLocalPort;
        const char *strAddress = ( strIP.GetLength() ) ? (const char *)strIP : "0.0.0.0";

        if ( bIncludePort )
            formatWithPort( m_strAddressBuffer, CString( strAddress ), nPort );
        else
            m_strAddressBuffer = strAddress;

        return (const char *)m_strAddressBuffer;
    }

    void CSocket::Close( void )
    {
        CloseHandle( m_hSocket );
        m_hSocket = NULL_SOCKET;
        m_nPort = 0;
        m_addrIPAddress = 0;
    }

    void CSocket::SetNonBlocking( bool bDontBlock )
    {
        if ( m_hSocket == NULL_SOCKET )
            return;
#ifdef IASLIB_WIN32__
            // (Windows sockets can be non-blocking too; this did nothing.)
        u_long ulMode = ( bDontBlock ) ? 1 : 0;
        if ( ioctlsocket( m_hSocket, FIONBIO, &ulMode ) == 0 )
            m_bBlocking = ! bDontBlock;
#else
        int nFlags = fcntl( m_hSocket, F_GETFL );
        if ( nFlags < 0 )
            return;
        nFlags = ( bDontBlock ) ? ( nFlags | O_NONBLOCK ) : ( nFlags & ~O_NONBLOCK );
        if ( fcntl( m_hSocket, F_SETFL, nFlags ) == 0 )
            m_bBlocking = ! bDontBlock;
#endif
    }

    bool CSocket::HasData( void )
    {
            // (It used to call select() with a descriptor count of 1, which
            // only ever looked at descriptor 0 outside Windows.)
        if ( m_hSocket == NULL_SOCKET )
            return false;
        return WaitForReadable( m_hSocket, 0 ) > 0;
    }

        // (A file-level replacement used inet_ntoa(), which shares one buffer
        // between threads; the system's inet_ntop() is used everywhere now.)
    const char *CSocket::inet_ntop( int iAddrFamily, unsigned long *addrConvert, char *strBuffer, int nMaxLen )
    {
        if ( addrConvert == NULL || strBuffer == NULL || nMaxLen <= 0 )
            return NULL;
#ifdef IASLIB_WIN32__
        return ::inet_ntop( iAddrFamily, (void *)addrConvert, strBuffer, (size_t)nMaxLen );
#else
        return ::inet_ntop( iAddrFamily, (const void *)addrConvert, strBuffer, (socklen_t)nMaxLen );
#endif
    }

    int CSocket::ReadWithTimeout( char *pchBuffer, int nBufferSize, int nTimeoutMs )
    {
        if ( m_hSocket == NULL_SOCKET || pchBuffer == NULL || nBufferSize <= 0 )
        {
            return READ_ERROR;
        }

        int nReady = WaitForReadable( m_hSocket, nTimeoutMs );
        if ( nReady == 0 )
        {
            return READ_TIMEOUT;
        }
        if ( nReady < 0 )
        {
            return READ_ERROR;
        }

        int nRet = (int)recv( m_hSocket, pchBuffer, nBufferSize, 0 );
        if ( nRet < 0 )
        {
            return SocketErrorIsTimeout() ? READ_TIMEOUT : READ_ERROR;
        }
        return nRet;
    }

    SOCKET CSocket::ReleaseHandle( void )
    {
        SOCKET hSocket = m_hSocket;
        m_hSocket = NULL_SOCKET;
        return hSocket;
    }

    void CSocket::Shutdown( void )
    {
        if ( m_hSocket != NULL_SOCKET )
        {
#ifdef IASLIB_WIN32__
            shutdown( m_hSocket, SD_BOTH );
#else
            shutdown( m_hSocket, SHUT_RDWR );
#endif
        }
    }

        // (It returned one static invalid address shared by every socket and
        // thread, which callers could change.)
    CInternetAddress &CSocket::getInternetAddress( void )
    {
        if ( m_internetAddress == NULL )
        {
            m_internetAddress = new CInternetAddress();
        }
        return *m_internetAddress;
    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__
