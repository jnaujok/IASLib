/**
 * @file Socket.h
 * @brief CSocket, the base TCP/IP socket class, and the socket handle definitions.
 *
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
 * is then 0: use getInternetAddress()). Servers listen on IPv6 and IPv4
 * at once unless given an address; an IPv4 peer of such a socket is
 * reported as plain IPv4, not \::ffff:a.b.c.d.
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
            /// The value socket() returns on failure (as Winsock names it).
        #define INVALID_SOCKET -1
    #endif
    #ifndef SOCKET_ERROR
            /// The value socket calls return on failure (as Winsock names it).
        #define SOCKET_ERROR -1
    #endif
        /// A socket handle (a descriptor outside Windows, as Winsock names it).
    typedef int SOCKET;
#endif

    /// The handle of no socket.
    ///
    /// It was 0 outside Windows, which is a real descriptor: a program with
    /// stdin closed could be given it.
const SOCKET NULL_SOCKET = INVALID_SOCKET;

#include "../BaseTypes/String_.h"
#include "SocketConfig.h"

    /// The option for rebinding a listening address.
    ///
    /// On POSIX systems SO_REUSEADDR only lets a restarted server bind while
    /// old connections linger in TIME_WAIT; on Windows it would let another
    /// socket take over a port in use, so the exclusive option is used there
    /// instead (a second server then fails to bind, as on POSIX).
#ifdef IASLIB_WIN32__
    #define IASLIB_REUSE_ADDRESS SO_EXCLUSIVEADDRUSE
#else
    #define IASLIB_REUSE_ADDRESS SO_REUSEADDR
#endif

namespace IASLib
{
    // Forward definition
    class CInternetAddress;

    /// A TCP/IP socket: a connection to another host, or a bound (listening) address.
    ///
    /// This is the base class for CClientSocket, CServerSocket and the TLS
    /// sockets. The object owns its handle and closes it when destroyed.
    ///
    /// Errors: the constructors that bind throw a CSocketException when
    /// they cannot; the ones that connect leave the socket unconnected
    /// (IsConnected() is false). Read() and Send() throw a CSocketException
    /// (by value) on an error; ReadWithTimeout() never throws. Error numbers
    /// are errno values on every platform (Winsock's are translated).
    ///
    /// Addresses: GetPort() and GetAddress() are the far end's; the near
    /// end's are GetLocalPort() and GetAddressString( false ). Host names
    /// are looked up with getaddrinfo(), so IPv6 hosts connect too
    /// (GetAddress() is then 0: use getInternetAddress()). Servers listen on
    /// IPv6 and IPv4 at once unless given an address; an IPv4 peer of such a
    /// socket is reported as plain IPv4, not \::ffff:a.b.c.d.
    ///
    /// Threads: one thread may read while another sends; Shutdown() from
    /// another thread wakes a blocked reader. Other calls need a lock.
    ///
    /// @code
    /// CSocket oSocket( "example.com", 80 );
    /// if ( oSocket.IsConnected() )
    /// {
    ///     oSocket.Send( "HEAD / HTTP/1.0\r\n\r\n", 19 );
    ///     char achBuffer[512];
    ///     int nRead = oSocket.ReadWithTimeout( achBuffer, sizeof( achBuffer ), 5000 );
    /// }
    /// @endcode
    class CSocket : public CObject
    {
        protected:
            SOCKET          m_hSocket;              ///< The handle, or NULL_SOCKET.
            int             m_nPort;                ///< The far end's port.
            unsigned long   m_addrIPAddress;        ///< The far end's address (IPv4, host order; 0 for IPv6).
            int             m_nLocalPort;           ///< The near end's port.
            unsigned long   m_addrLocalIPAddress;   ///< The near end's address (IPv4, host order; 0 for IPv6).
            CString         m_strAddress;           ///< "ip:port" of the far end.
            CString         m_strLocalAddress;      ///< "ip:port" of the near end.
            CString         m_strRemoteIP;          ///< The far end's address alone.
            CString         m_strLocalIP;           ///< The near end's address alone.
            CString         m_strAddressBuffer;     ///< What GetAddressString() returns.
            CString         m_strSocketName;        ///< A name for the socket (informational only).
            bool            m_bBlocking;            ///< Whether calls block.
            CInternetAddress *m_internetAddress;    ///< The far end's address, once known (owned).

        public:
                /// Connects to strConnectTo:nPort with the configuration's
                /// buffer sizes, timeout, keep-alive, linger and no-delay
                /// settings.
                ///
                /// Each of the host's addresses is tried in turn. If none
                /// answers, the socket is left unconnected (IsConnected() is
                /// false); this does not throw.
                /// @param config  The settings to use.
                /// @param strConnectTo  A host name, or an IPv4 or IPv6 address.
                /// @param nPort  The port, from 1 to 65535.
                            CSocket( CSocketConfig config, const char *strConnectTo, int nPort );
                /// Binds nBindPort on strBindIP, with the configuration's settings.
                ///
                /// NULL (or "::") is every address, IPv6 and IPv4 on one
                /// socket (IPv4 alone on a machine without IPv6); "0.0.0.0" is
                /// every IPv4 address only.
                /// @param config  The settings to use.
                /// @param nBindPort  The port; 0 for any free port.
                /// @param strBindIP  The address to bind, or NULL for every address.
                /// @throws CSocketException if the address cannot be bound.
                            CSocket( CSocketConfig config, int nBindPort, const char *strBindIP = NULL );
                /// Wraps a connected handle (as accept() gives), taking it over.
                /// @param hSocket  The handle; NULL_SOCKET leaves the socket unconnected.
                /// @param strSockName  A name for the socket (informational only), or NULL.
                /// @param AddressIn  The peer's sockaddr, if known (it is read
                ///                   from the socket when it can be).
                            CSocket( SOCKET hSocket, const char *strSockName, void *AddressIn = NULL );
                /// Binds nPort on every address (as above).
                /// @param nPort  The port; 0 for any free port.
                /// @param bBlocking  Whether calls on the socket block.
                /// @throws CSocketException if the port cannot be bound.
	                        CSocket( int nPort, bool bBlocking = true );
                /// Connects to strConnectTo:nPort (unconnected if it cannot; see above).
                /// @param strConnectTo  A host name, or an IPv4 or IPv6 address.
                /// @param nPort  The port, from 1 to 65535.
                            CSocket( const char *strConnectTo, int nPort );
                /// Closes the handle.
	        virtual        ~CSocket();

                            DEFINE_OBJECT( CSocket )

                /// Whether the socket has a handle (connected, or bound).
            virtual bool            IsConnected( void ) { return m_hSocket != NULL_SOCKET; }
                /// Whether the connection is ready for data; for a plain socket,
                /// the same as IsConnected(). CSecureSocket overrides it.
            virtual bool            IsHandshakeComplete( void ) { return IsConnected(); }

                /// Reads one byte.
                /// @return The byte.
                /// @throws CSocketException if the connection closes or fails.
            virtual unsigned char   Read( void );
                /// Reads up to nBufferSize bytes, waiting for at least one.
                /// @param pchBuffer  Where to put them.
                /// @param nBufferSize  The most to read.
                /// @return How many were read, or 0 if the far end has closed
                ///         (or the socket is not connected).
                /// @throws CSocketException on an error.
            virtual int             Read( char *pchBuffer, int nBufferSize );
                /// Sends one byte.
                /// @param chSend  The byte.
                /// @return 1, or 0 if the socket is not connected.
                /// @throws CSocketException on an error.
            virtual int             Send( unsigned char chSend );
                /// Sends all of the bytes.
                /// @param pchBuffer  The bytes.
                /// @param nBufferSize  How many.
                /// @return nBufferSize, or 0 if there was nothing to send or
                ///         the socket is not connected.
                /// @throws CSocketException on an error.
                /// @note The Windows code used to send the whole buffer again
                ///       from the start on every pass, and counted a partial
                ///       send as complete.
            virtual int             Send( const char *pchBuffer, int nBufferSize );

                /// The far end's port (for a listening socket, the port it is
                /// bound to).
            virtual int             GetPort( void ) { return m_nPort; }
                /// The far end's IPv4 address, in host order (0 for an IPv6 peer).
            virtual unsigned long   GetAddress( void ) { return m_addrIPAddress; }
                /// The near end's port.
            int                     GetLocalPort( void ) const { return m_nLocalPort; }
                /// The far end's address (or, with bInternetAddress false, the
                /// near end's), as text.
                /// @param bInternetAddress  true for the far end, false for the near end.
                /// @param bIncludePort  Add ":port" ("[ip]:port" for IPv6).
                /// @return The text ("0.0.0.0" if unknown); it is good until
                ///         the next call.
            virtual const char     *GetAddressString( bool bInternetAddress=true, bool bIncludePort=false );
                /// The address of the remote end.
                /// @return An invalid address if it is not known (the reference
                ///         stays good while the socket lives).
            virtual CInternetAddress &getInternetAddress( void );

                /// Closes the handle.
            virtual void            Close( void );
                /// Makes calls on the socket non-blocking, or blocking again.
                /// @param bDontBlock  true for non-blocking.
            virtual void            SetNonBlocking( bool bDontBlock );
                /// Whether calls on the socket block.
            bool                    IsBlocking( void ) const { return m_bBlocking; }
                /// Whether data can be read now, without waiting.
            virtual bool            HasData( void );

                /// Reads up to nBufferSize bytes, waiting at most nTimeoutMs
                /// milliseconds. Unlike Read(), this never throws.
                /// @param pchBuffer  Where to put them.
                /// @param nBufferSize  The most to read.
                /// @param nTimeoutMs  The longest wait; negative waits forever.
                /// @return The number of bytes read, 0 if the peer closed the
                ///         connection, READ_TIMEOUT if nothing arrived in time,
                ///         or READ_ERROR.
            virtual int             ReadWithTimeout( char *pchBuffer, int nBufferSize, int nTimeoutMs );
                /// Shuts down both directions of a connected socket, which wakes
                /// up any thread blocked reading from it. The handle stays open.
            virtual void            Shutdown( void );
                /// Gives up the connection: returns the handle and leaves this
                /// object closed, without closing the connection. Used to hand an
                /// accepted connection to another socket class (CSecureSocket).
                /// @return The handle, which the caller now owns.
            SOCKET                  ReleaseHandle( void );
                /// Whether the connection is protected by TLS. Overridden by CSecureSocket.
            virtual bool            IsSecure( void ) { return false; }
                /// The host name the client asked for in the TLS handshake ("" if
                /// none, or not TLS). Overridden by CSecureSocket.
            virtual CString         GetTlsServerName( void ) { return CString( "" ); }

            static const int        READ_TIMEOUT = -1;  ///< ReadWithTimeout(): nothing arrived in time.
            static const int        READ_ERROR = -2;    ///< ReadWithTimeout(): the read failed.

                /// htons(): a 16-bit value from host to network order.
            static unsigned short Htons( unsigned short ushValue ) { return htons( ushValue ); }
                /// htonl(): a 32-bit value from host to network order.
            static unsigned long  Htonl( unsigned long ulValue ) { return htonl( (uint32_t)ulValue ); }
                /// ntohs(): a 16-bit value from network to host order.
            static unsigned short Ntohs( unsigned short ushValue ) { return ntohs( ushValue ); }
                /// ntohl(): a 32-bit value from network to host order.
            static unsigned long  Ntohl( unsigned long ulValue ) { return ntohl( (uint32_t)ulValue ); }
                /// The system's inet_ntop(): an address (in network order) as text.
                /// @param iAddrFamily  AF_INET or AF_INET6.
                /// @param addrConvert  The address (an in_addr or in6_addr).
                /// @param strBuffer  Where to write the text.
                /// @param nMaxLen  The buffer's size.
                /// @return strBuffer, or NULL on failure.
                /// @note A file-level replacement used inet_ntoa(), which shares
                ///       one buffer between threads; the system's inet_ntop() is
                ///       used everywhere now.
            static const char *inet_ntop( int iAddrFamily, unsigned long *addrConvert, char *strBuffer, int nMaxLen );

                /// Starts the platform's networking (Winsock), once; every
                /// socket class calls it.
            static void             InitializeNetworking( void );
                /// The last socket error, as an errno value.
            static int              LastError( void );
                /// Closes a handle (nothing for NULL_SOCKET).
                /// @param hSocket  The handle.
            static void             CloseHandle( SOCKET hSocket );
                /// Settings every new handle gets (no SIGPIPE on macOS; not
                /// inherited by child programs).
                /// @param hSocket  The handle.
            static void             PrepareHandle( SOCKET hSocket );
                /// A handle of nType bound as the constructors bind (see
                /// above), or INVALID_SOCKET with the error in nError.
                /// @param strBindIP  The address, or NULL (or "") for every address.
                /// @param nPort  The port, from 0 to 65535.
                /// @param nType  SOCK_STREAM or SOCK_DGRAM.
                /// @param nProtocol  IPPROTO_TCP or IPPROTO_UDP.
                /// @param bReuseAddress  Set IASLIB_REUSE_ADDRESS.
                /// @param pConfig  Settings to apply before binding, or NULL.
                /// @param nError  Set to the errno value on failure, 0 on success.
                /// @return The bound handle, which the caller owns, or INVALID_SOCKET.
            static SOCKET           BindHandle( const char *strBindIP, int nPort, int nType, int nProtocol,
                                                bool bReuseAddress, const CSocketConfig *pConfig, int &nError );
                /// Whether this machine can use IPv6 (has an IPv6 loopback).
                /// The answer is found once and kept.
            static bool             IsIPv6Available( void );

        private:
                /// Sets the members to an unconnected socket.
            void                    Init( void );
                /// Connects to the first of the host's addresses that answers.
                /// @note It used gethostbyname(), which is neither thread-safe nor
                ///       IPv6-aware, and copied 8 bytes out of a 4-byte address; it
                ///       never checked socket().
            void                    Connect( const char *strConnectTo, int nPort, CSocketConfig *pConfig );
                /// Binds a listening address; throws CSocketException if it cannot.
            void                    Bind( const char *strBindIP, int nPort, CSocketConfig *pConfig );
                /// Reads the near and far ends' addresses from the handle.
            void                    ReadAddresses( const struct sockaddr *pKnownPeer = NULL );

                                    CSocket( const CSocket & );        ///< Not copyable.
            CSocket                &operator =( const CSocket & );    ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SOCKET_H__
