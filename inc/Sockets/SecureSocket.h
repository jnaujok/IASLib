/**
 *  Secure Socket Class
 *
 *      This class wraps a standard socket in an SSL/TLS security layer and
 * handles the handshaking between the ends of the conversation is such a way
 * as to make the use of a secure or non-secure socket transparent to the end
 * user of the library.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/22/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SECURE_SOCKET_H__
#define IASLIB_SECURE_SOCKET_H__

#ifdef IASLIB_NETWORKING__

#include "Socket.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace IASLib
{
    class CSecureSocket : public CSocket
    {
        protected:
            SSL_CTX         *m_pContext;
        public:
                            CSecureSocket( SOCKET hSocket, const char *strSockName, void *AddressIn=NULL );
	                        CSecureSocket( int nPort, bool bBlocking = true );
                            CSecureSocket( const char *strConnectTo, int nPort );
	        virtual        ~CSecureSocket();

                            DEFINE_OBJECT( CSecureSocket )

            virtual bool            IsConnected( void );
            virtual bool            IsHandshakeComplete( void );
            virtual unsigned char   Read( void );
            virtual int             Read( char *pchBuffer, int nBufferSize );
            virtual int             Send( unsigned char chSend );
            virtual int             Send( const char *pchBuffer, int nBufferSize );
            virtual int             GetPort( void ) { return m_nPort; }
            virtual unsigned long   GetAddress( void ) { return m_addrIPAddress; }
            virtual const char     *GetAddressString( bool bInternetAddress=true, bool bIncludePort=false );
            virtual void            Close( void );
            virtual void            SetNonBlocking( bool bDontBlock );
            virtual bool            HasData( void );

        private:
            void                    initSsl( void );
            SSL_CTX                *createContext();
            void                    configureContext( SSL_CTX *context );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SECURE_SOCKET_H__
