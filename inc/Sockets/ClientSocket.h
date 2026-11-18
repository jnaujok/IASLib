/**
 * @file ClientSocket.h
 * @brief CClientSocket, a TCP connection to a server.
 *
 *  Client Socket Class
 *
 *      This class provides an interface for connecting
 * to a remote server through a socket connection.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/02/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CLIENTSOCKET_H__
#define IASLIB_CLIENTSOCKET_H__

#ifdef IASLIB_NETWORKING__

#include "Socket.h"

namespace IASLib
{
    /// A TCP connection to a server; a CSocket made by connecting.
    ///
    /// The constructors do not throw: check IsConnected().
    ///
    /// @code
    /// CClientSocket oSocket( "localhost", 8080 );
    /// if ( ! oSocket.IsConnected() )
    ///     ...;
    /// @endcode
    class CClientSocket : public CSocket
    {
        public:
                            /// Connects to strConnectTo:nPort; IsConnected()
                            /// says whether it worked.
                            /// @param strConnectTo  A host name, or an IPv4 or IPv6 address.
                            /// @param nPort  The port.
	                    CClientSocket( const char *strConnectTo, int nPort );
                            /// The same, with the configuration's options.
                            /// @param config  The settings to use.
                            /// @param strConnectTo  A host name, or an IPv4 or IPv6 address.
                            /// @param nPort  The port.
                        CClientSocket( CSocketConfig config, const char *strConnectTo, int nPort );
                            /// Closes the connection.
	        virtual    ~CClientSocket();

                        DEFINE_OBJECT( CClientSocket )

                /// Whether the connection uses TLS: false.
                /// @note Not the same as CSocket::IsSecure() (note the case).
            virtual bool isSecure( void ) { return false; }
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_CLIENTSOCKET_H__
