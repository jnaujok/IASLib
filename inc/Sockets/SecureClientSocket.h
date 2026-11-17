/**
 *  Secure Client Socket Class
 *
 *      A TLS connection to a server: CSecureSocket's client side, with a
 * default context that trusts the system's certificate authorities and
 * checks the server's host name.
 *
 *          CSecureClientSocket socket( "example.com", 443 );
 *          if ( ! socket.Handshake( 10000 ) ) ... socket.GetTlsError() ...
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/23/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SECURECLIENTSOCKET_H__
#define IASLIB_SECURECLIENTSOCKET_H__

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_TLS__

#include "SecureSocket.h"

namespace IASLib
{
    class CSecureClientSocket : public CSecureSocket
    {
        public:
                // Connects with the given context, or the default client one.
	                    CSecureClientSocket( const char *strConnectTo, int nPort,
                                             const std::shared_ptr<CTlsContext> &pContext = DefaultContext() );
	        virtual    ~CSecureClientSocket();

                        DEFINE_OBJECT( CSecureClientSocket )

            virtual bool isSecure( void ) { return true; }

                // A client context using the system's trusted certificates,
                // shared by every CSecureClientSocket made without one.
            static std::shared_ptr<CTlsContext> DefaultContext( void );
    };
}

#endif // IASLIB_TLS__
#endif // IASLIB_NETWORKING__
#endif // IASLIB_SECURECLIENTSOCKET_H__
