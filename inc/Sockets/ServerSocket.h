/**
 * @file ServerSocket.h
 * @brief CServerSocket, a listening TCP socket.
 *
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

#ifndef IASLIB_SERVERSOCKET_H__
#define IASLIB_SERVERSOCKET_H__

#ifdef IASLIB_NETWORKING__

#include "Socket.h"

namespace IASLib
{
        /// A listening TCP socket. The constructors throw a CSocketException
        /// if the port cannot be bound or listened on.
        ///
        /// Each accepted connection is a new CSocket that the caller owns.
        /// To serve TLS, wrap it in a CSecureSocket.
        ///
        /// @code
        /// CServerSocket oListener( 8080 );
        /// CSocket *pClient = oListener.Accept( 1000 );
        /// if ( pClient )
        /// {
        ///     ...;
        ///     delete pClient;
        /// }
        /// @endcode
    class CServerSocket : public CSocket
    {
        protected:
            bool m_bSecure;             ///< Informational: whether connections will be wrapped in TLS.
            CSocketConfig config;       ///< The settings it was made with.

        public:
                            /// Binds nPort on bindAddress, with the
                            /// configuration's backlog and options (which
                            /// accepted connections inherit), and listens.
                            /// @param config  The settings to use.
                            /// @param nPort  The port; 0 for any free port.
                            /// @param bindAddress  The address to bind; NULL for every
                            ///                     address, IPv6 and IPv4 (see CSocket).
                            /// @throws CSocketException if it cannot bind or listen.
                        CServerSocket( CSocketConfig config, int nPort, const char *bindAddress = NULL );
                            /// Binds nPort on every address and listens.
                            /// @param nPort  The port; 0 for any free port.
                            /// @param nMaxBacklog  How many connections may wait to be
                            ///                     accepted (0 or less: the system's most).
                            /// @param bSecure  Informational: whether the connections
                            ///                 will be wrapped in TLS.
                            /// @throws CSocketException if it cannot bind or listen.
	                    CServerSocket( int nPort, int nMaxBacklog = 16, bool bSecure = false );
                            /// Closes the socket.
	        virtual    ~CServerSocket( void );

                        DEFINE_OBJECT( CServerSocket )

                /// Waits for a connection.
                /// @return The new connection, which the caller owns; on a
                ///         non-blocking socket, NULL if none is waiting.
                /// @throws CSocketException on an error (ENOTCONN if the socket
                ///         is closed).
            virtual CSocket    *Accept( void );
                /// Waits at most nTimeoutMs milliseconds for a connection.
                /// Never throws.
                /// @param nTimeoutMs  The longest wait; negative waits forever.
                /// @return The new connection, which the caller owns, or NULL if
                ///         none arrived in time or on error.
            virtual CSocket    *Accept( int nTimeoutMs );
                /// The bSecure flag given to the constructor.
            virtual bool        isSecure( void ) { return m_bSecure; }

        private:
                /// Starts listening; closes the socket and throws CSocketException if it cannot.
            void                Listen( int nBacklog );
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SERVERSOCKET_H__

