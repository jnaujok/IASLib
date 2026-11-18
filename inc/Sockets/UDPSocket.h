/**
 * @file UDPSocket.h
 * @brief CUDPSocket, a UDP socket for sending and receiving datagrams.
 *
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

#ifndef IASLIB_UDPSOCKET_H__
#define IASLIB_UDPSOCKET_H__

#ifdef IASLIB_NETWORKING__

#include "Socket.h"
#include "InternetAddress.h"

namespace IASLib
{
        /// A UDP socket. By default it takes IPv6 and IPv4 on one socket (IPv4
        /// alone on a machine without IPv6). Errors are thrown only by the
        /// constructors (a CSocketException); Read() and Send() return -1.
        ///
        /// The object owns its handle and closes it when destroyed.
        /// SO_REUSEADDR is not set, so a second socket cannot bind the same
        /// port and steal datagrams.
        ///
        /// @code
        /// CUDPSocket oSocket( 5060 );
        /// CInternetAddress oFrom;
        /// char achBuffer[1500];
        /// int nRead = oSocket.Read( achBuffer, sizeof( achBuffer ), oFrom, 1000 );
        /// if ( nRead > 0 )
        ///     oSocket.Send( achBuffer, nRead, oFrom );
        /// @endcode
    class CUDPSocket : public CObject
    {
        protected:
            SOCKET          m_hSocket;              ///< The handle, or NULL_SOCKET.
            int             m_nPort;                ///< The port it is bound to.
            CInternetAddress *m_addrIPAddress;      ///< The address it is bound to (owned; never NULL).
            CString         m_strAddress;           ///< The bound address with its port, as text.
            CString         m_strSocketName;        ///< A name for the socket (see setName()).
            int             m_nFamily;              ///< AF_INET or AF_INET6.

                /// Binds the socket (the constructors call it).
                /// @throws CSocketException if it cannot bind.
            void            Open( int nPort, const char *strBindAddress );

        public:
                            /// Server socket, bound to nPort on every address.
                            /// @param nPort  The port; 0 for any free port.
                            /// @throws CSocketException if it cannot bind.
	                        CUDPSocket( int nPort );
                            /// Bound to one address: "0.0.0.0" for every IPv4
                            /// address only, "::1", "127.0.0.1", a name...
                            /// @param nPort  The port; 0 for any free port.
                            /// @param strBindAddress  The address, or NULL for every address.
                            /// @throws CSocketException if it cannot bind.
                            CUDPSocket( int nPort, const char *strBindAddress );
                            /// Client socket, bound to a free port (so its port
                            /// is known straight away).
                            /// @throws CSocketException if it cannot bind.
                            CUDPSocket( void );
                            /// Closes the socket.
	        virtual        ~CUDPSocket( void );

                            DEFINE_OBJECT( CUDPSocket )

                /// Sets a name for the socket (informational only).
            virtual void setName( const char *name ) { m_strSocketName = name; }
                /// The socket's name.
            virtual CString getName( void ) { return m_strSocketName; }
                /// Whether the socket has a handle (is open).
            virtual bool            IsConnected( void ) { return (m_hSocket != NULL_SOCKET ) ? true:false; }
                /// Reads one datagram (blocking).
                ///
                /// A datagram longer than the buffer is cut short (on every
                /// platform). The data is NUL-terminated when it fits in the
                /// buffer.
                /// @param pchBuffer  Where to put it.
                /// @param nBufferSize  The buffer's size.
                /// @param incomingAddress  Set to the sender's address.
                /// @return Its length, or -1 on error.
            virtual int             Read( char *pchBuffer, int nBufferSize, CInternetAddress &incomingAddress );
                /// As above, but waits at most nTimeoutMs milliseconds.
                /// @param pchBuffer  Where to put it.
                /// @param nBufferSize  The buffer's size.
                /// @param incomingAddress  Set to the sender's address.
                /// @param nTimeoutMs  The longest wait; negative waits forever.
                /// @return Its length, CSocket::READ_TIMEOUT if nothing arrived
                ///         in time, or -1 on error.
            virtual int             Read( char *pchBuffer, int nBufferSize, CInternetAddress &incomingAddress, int nTimeoutMs );
                /// Sends one datagram to the target (its first address; an
                /// IPv4-only socket uses its IPv4 address).
                /// @param pchBuffer  The data.
                /// @param nBufferSize  Its length.
                /// @param targetAddress  Where to send it.
                /// @return The bytes sent, or -1.
            virtual int             Send( const char *pchBuffer, int nBufferSize, const CInternetAddress &targetAddress );
                /// The port the socket is bound to.
            virtual int             GetPort( void ) { return m_nPort; }
                /// Whether the socket is IPv6 (and so takes both families).
            bool                    IsIPv6( void ) const { return m_nFamily == AF_INET6; }
                /// The address the socket is bound to.
            virtual const CInternetAddress &GetAddress( void ) const { return *m_addrIPAddress; }
                /// Closes the handle.
                /// @note It used to close the handle without forgetting it, so
                ///       the destructor closed it again, possibly after the
                ///       number was reused.
            virtual void            Close( void );

        private:
                                    CUDPSocket( const CUDPSocket & );      ///< Not copyable.
            CUDPSocket             &operator =( const CUDPSocket & );      ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_UDPSOCKET_H__
