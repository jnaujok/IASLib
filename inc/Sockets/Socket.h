/**
 *  Socket Class
 *
 *      This base class provides platform independent support for TCP/IP
 * sockets. This is the base class for both the server and client socket
 * classes.
 *
 *      Errors: the constructors that bind throw a CSocketException when
 * they cannot; the ones that connect leave the socket unconnected
 * (IsConnected() is false). Read() and Send() throw a CSocketException
 * (by value) on an error; ReadWithTimeout() never throws. Error numbers
 * are errno values on every platform (Winsock's are translated).
 *
 *      Addresses: GetPort() and GetAddress() are the far end's; the near
 * end's are GetLocalPort() and GetAddressString( false ). Host names are
 * looked up with getaddrinfo(), so IPv6 hosts connect too (GetAddress()
 * is then 0: use getInternetAddress()).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/02/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SOCKET_H__
#define IASLIB_SOCKET_H__

#ifdef IASLIB_NETWORKING__

#ifdef IASLIB_WIN32__
    #include "../BaseTypes/Platform.h"
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <inttypes.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <arpa/inet.h>

    #ifndef INVALID_SOCKET
        #define INVALID_SOCKET -1
    #endif
    #ifndef SOCKET_ERROR
        #define SOCKET_ERROR -1
    #endif
    typedef int SOCKET;
#endif

    // The handle of no socket. (It was 0 outside Windows, which is a real
    // descriptor: a program with stdin closed could be given it.)
const SOCKET NULL_SOCKET = INVALID_SOCKET;

#include "../BaseTypes/String_.h"
#include "SocketConfig.h"

    // Rebinding a listening address. On POSIX systems SO_REUSEADDR only lets
    // a restarted server bind while old connections linger in TIME_WAIT; on
    // Windows it would let another socket take over a port in use, so the
    // exclusive option is used there instead (a second server then fails to
    // bind, as on POSIX).
#ifdef IASLIB_WIN32__
    #define IASLIB_REUSE_ADDRESS SO_EXCLUSIVEADDRUSE
#else
    #define IASLIB_REUSE_ADDRESS SO_REUSEADDR
#endif

namespace IASLib
{
    // Forward definition
    class CInternetAddress;

    class CSocket : public CObject
    {
        protected:
            SOCKET          m_hSocket;
            int             m_nPort;                // the far end
            unsigned long   m_addrIPAddress;        // the far end (IPv4, host order)
            int             m_nLocalPort;
            unsigned long   m_addrLocalIPAddress;
            CString         m_strAddress;           // "ip:port" of the far end
            CString         m_strLocalAddress;
            CString         m_strRemoteIP;          // the far end's address alone
            CString         m_strLocalIP;
            CString         m_strAddressBuffer;     // what GetAddressString() returns
            CString         m_strSocketName;
            bool            m_bBlocking;
            CInternetAddress *m_internetAddress;

        public:
                                // Connects to strConnectTo:nPort with the
                                // configuration's buffer sizes, timeout,
                                // keep-alive, linger and no-delay settings.
                            CSocket( CSocketConfig config, const char *strConnectTo, int nPort );
                                // Binds nBindPort (0: any free port) on
                                // strBindIP (NULL: every IPv4 address).
                            CSocket( CSocketConfig config, int nBindPort, const char *strBindIP = NULL );
                                // Wraps a connected handle (as accept() gives).
                            CSocket( SOCKET hSocket, const char *strSockName, void *AddressIn = NULL );
                                // Binds nPort on every IPv4 address.
	                        CSocket( int nPort, bool bBlocking = true );
                                // Connects to strConnectTo:nPort.
                            CSocket( const char *strConnectTo, int nPort );
	        virtual        ~CSocket();

                            DEFINE_OBJECT( CSocket )

            virtual bool            IsConnected( void ) { return m_hSocket != NULL_SOCKET; }
            virtual bool            IsHandshakeComplete( void ) { return IsConnected(); }

                // One byte (throws if the connection closes or fails).
            virtual unsigned char   Read( void );
                // Up to nBufferSize bytes: how many, or 0 if the far end has
                // closed (throws on an error).
            virtual int             Read( char *pchBuffer, int nBufferSize );
                // All of the bytes (throws on an error).
            virtual int             Send( unsigned char chSend );
            virtual int             Send( const char *pchBuffer, int nBufferSize );

                // The far end's port (for a listening socket, the port it is
                // bound to).
            virtual int             GetPort( void ) { return m_nPort; }
            virtual unsigned long   GetAddress( void ) { return m_addrIPAddress; }
            int                     GetLocalPort( void ) const { return m_nLocalPort; }
                // The far end's address (or, with bInternetAddress false, the
                // near end's), as text.
            virtual const char     *GetAddressString( bool bInternetAddress=true, bool bIncludePort=false );
                // The address of the remote end. An invalid address if it is
                // not known (the reference stays good while the socket lives).
            virtual CInternetAddress &getInternetAddress( void );

            virtual void            Close( void );
            virtual void            SetNonBlocking( bool bDontBlock );
            bool                    IsBlocking( void ) const { return m_bBlocking; }
                // Whether data can be read now, without waiting.
            virtual bool            HasData( void );

                // Reads up to nBufferSize bytes, waiting at most nTimeoutMs
                // milliseconds (a negative timeout waits forever). Returns the
                // number of bytes read, 0 if the peer closed the connection,
                // READ_TIMEOUT if nothing arrived in time, or READ_ERROR. Unlike
                // Read(), this never throws.
            virtual int             ReadWithTimeout( char *pchBuffer, int nBufferSize, int nTimeoutMs );
                // Shuts down both directions of a connected socket, which wakes
                // up any thread blocked reading from it.
            virtual void            Shutdown( void );
                // Gives up the connection: returns the handle and leaves this
                // object closed, without closing the connection. Used to hand an
                // accepted connection to another socket class (CSecureSocket).
            SOCKET                  ReleaseHandle( void );
                // Whether the connection is protected by TLS, and the host name
                // the client asked for in the TLS handshake ("" if none, or not
                // TLS). Overridden by CSecureSocket.
            virtual bool            IsSecure( void ) { return false; }
            virtual CString         GetTlsServerName( void ) { return CString( "" ); }

            static const int        READ_TIMEOUT = -1;
            static const int        READ_ERROR = -2;

            static unsigned short Htons( unsigned short ushValue ) { return htons( ushValue ); }
            static unsigned long  Htonl( unsigned long ulValue ) { return htonl( (uint32_t)ulValue ); }
            static unsigned short Ntohs( unsigned short ushValue ) { return ntohs( ushValue ); }
            static unsigned long  Ntohl( unsigned long ulValue ) { return ntohl( (uint32_t)ulValue ); }
            static const char *inet_ntop( int iAddrFamily, unsigned long *addrConvert, char *strBuffer, int nMaxLen );

                // Starts the platform's networking (Winsock), once; every
                // socket class calls it.
            static void             InitializeNetworking( void );
                // The last socket error, as an errno value.
            static int              LastError( void );
                // Closes a handle.
            static void             CloseHandle( SOCKET hSocket );
                // Settings every new handle gets (no SIGPIPE on macOS).
            static void             PrepareHandle( SOCKET hSocket );

        private:
            void                    Init( void );
            void                    Connect( const char *strConnectTo, int nPort, CSocketConfig *pConfig );
            void                    Bind( const char *strBindIP, int nPort, CSocketConfig *pConfig );
            void                    ReadAddresses( const struct sockaddr *pKnownPeer = NULL );

                                    CSocket( const CSocket & );
            CSocket                &operator =( const CSocket & );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SOCKET_H__
