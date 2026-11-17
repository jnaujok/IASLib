/**
 *  Basic HTTP Server Class
 *
 *      A ready-to-run HTTP/1.1 server built on the CHttpServer protocol
 * core. It accepts TCP connections and serves each one with a
 * CHttpListener on its own thread, up to a maximum number of connections;
 * further connections get 503 Service Unavailable.
 *
 *      Typical use:
 *
 *          CBasicHttpServer server( 8080 );
 *          server.AddHandlerFactory( new CBaseHttpHandlerFactory() );
 *          server.AddHandlerFactory( new CMyHandlerFactory() );
 *          server.Start();
 *          if ( ! server.waitUntilReady( 5000 ) ) { ... getStartupError() ... }
 *          ...
 *          server.Stop();
 *          server.Wait();
 *
 *      Port 0 asks the system for a free port; getListeningPort() reports
 * the one used.
 *
 *      HTTPS: give the server a TLS context (with its certificate and key)
 * before Start(), and every connection is TLS:
 *
 *          std::shared_ptr<CTlsContext> tls = std::make_shared<CTlsContext>( CTlsContext::SERVER );
 *          tls->setCertificateFile( "server.crt" );
 *          tls->setPrivateKeyFile( "server.key" );
 *          CBasicHttpServer server( 443 );
 *          server.setTlsContext( tls );
 *
 *      The handshake runs on the connection's own thread, limited by
 * setHandshakeTimeout() (10 seconds by default), so slow or silent clients
 * hold nothing else up. Needs a library built with OpenSSL (IASLIB_TLS__).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_BASICHTTPSERVER_H__
#define IASLIB_BASICHTTPSERVER_H__

#include "NetworkServices/HTTP/HttpServer.h"

#include <memory>

namespace IASLib
{
    struct CBasicHttpServerState;
    class CTlsContext;

    class CBasicHttpServer : public CHttpServer
    {
        protected:
            size_t                  m_nMaxConnections;
            CBasicHttpServerState  *m_pState;

        public:
                                CBasicHttpServer( int nPort = 80, size_t nMaxConnections = 256 );
            virtual            ~CBasicHttpServer( void );

                                DEFINE_OBJECT( CBasicHttpServer );

                // Binds the socket, then serves until Stop() is called.
            virtual void       *Run( void );

            virtual void        Stop( void );

                // Waits until the server is listening (true) or has failed to
                // start (false; see getStartupError()). Call after Start().
            virtual bool        waitUntilReady( int nTimeoutMs );
            virtual CString     getStartupError( void );

                // The port in use once the server is ready (0 if not listening).
            virtual int         getListeningPort( void );

                // The number of connections being served.
            virtual size_t      getConnectionCount( void );

                // Serves HTTPS with this SERVER context (call before Start()).
                // False if the context has no certificate and key, or the
                // library was built without TLS.
            virtual bool        setTlsContext( const std::shared_ptr<CTlsContext> &pContext );
            virtual bool        isSecure( void );
                // How long a client has to complete the TLS handshake.
            virtual void        setHandshakeTimeout( int nMilliseconds );

        private:
            void                reapConnections( bool bAll );
    };
} // namespace IASLib

#endif // IASLIB_BASICHTTPSERVER_H__

#endif // IASLIB_NETWORKING__
