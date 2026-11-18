/**
 * @file BasicHttpServer.h
 * @brief A ready-to-run HTTP/1.1 and HTTPS server (CBasicHttpServer).
 *
 *  Basic HTTP Server Class
 *
 *      A ready-to-run HTTP/1.1 server built on the CHttpServer protocol
 * core. It accepts TCP connections and serves each one with a
 * CHttpListener on its own thread, up to a maximum number of connections;
 * further connections get 503 Service Unavailable.
 *
 *      Typical use:
 *
 * @code
 *          CBasicHttpServer server( 8080 );
 *          server.AddHandlerFactory( new CBaseHttpHandlerFactory() );
 *          server.AddHandlerFactory( new CMyHandlerFactory() );
 *          server.Start();
 *          if ( ! server.waitUntilReady( 5000 ) ) { ... getStartupError() ... }
 *          ...
 *          server.Stop();
 *          server.Wait();
 * @endcode
 *
 *      Port 0 asks the system for a free port; getListeningPort() reports
 * the one used.
 *
 *      HTTPS: give the server a TLS context (with its certificate and key)
 * before Start(), and every connection is TLS:
 *
 * @code
 *          std::shared_ptr<CTlsContext> tls = std::make_shared<CTlsContext>( CTlsContext::SERVER );
 *          tls->setCertificateFile( "server.crt" );
 *          tls->setPrivateKeyFile( "server.key" );
 *          CBasicHttpServer server( 443 );
 *          server.setTlsContext( tls );
 * @endcode
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

    /// A ready-to-run HTTP/1.1 server on one TCP port, with optional TLS.
    ///
    /// Run() binds the port and accepts connections, serving each with a
    /// CHttpListener on its own thread, up to the maximum number of
    /// connections; further connections get 503 Service Unavailable (over
    /// TLS they are just closed). Request handling is that of CHttpServer.
    ///
    /// Start() begins serving; Stop() and Wait() (also run by the destructor)
    /// shut down the listening socket and every connection and wait for their
    /// threads. Configure the server before Start().
    class CBasicHttpServer : public CHttpServer
    {
        protected:
            size_t                  m_nMaxConnections;  ///< The most connections served at once (at least 1).
            CBasicHttpServerState  *m_pState;           ///< Run-time state shared with the connection threads (owned).

        public:
                                /// Creates the server; nothing is bound until it runs.
                                /// @param nPort           The TCP port; 0 asks the system for a
                                ///                        free one (see getListeningPort()).
                                /// @param nMaxConnections The most connections served at once
                                ///                        (0 is taken as 1).
                                CBasicHttpServer( int nPort = 80, size_t nMaxConnections = 256 );
                                /// Stops the server and waits for it and its connections to finish.
            virtual            ~CBasicHttpServer( void );

                                DEFINE_OBJECT( CBasicHttpServer );

                /// Binds the socket, then serves until Stop() is called.
                ///
                /// If the port cannot be opened, the failure is reported through
                /// waitUntilReady() and getStartupError(). On the way out all
                /// connections are shut down and their threads joined.
                /// @return NULL.
            virtual void       *Run( void );

                /// Asks the server and its connections to stop. Does not wait;
                /// call Wait() for that.
            virtual void        Stop( void );

                /// Waits until the server is listening (true) or has failed to
                /// start (false; see getStartupError()). Call after Start().
                /// @param nTimeoutMs The longest wait, in milliseconds; false if
                ///                   it runs out first.
            virtual bool        waitUntilReady( int nTimeoutMs );
                /// Why the server failed to start, or "".
            virtual CString     getStartupError( void );

                /// The port in use once the server is ready (0 if not listening).
            virtual int         getListeningPort( void );

                /// The number of connections being served.
            virtual size_t      getConnectionCount( void );

                /// Serves HTTPS with this SERVER context (call before Start()).
                ///
                /// The handshake runs on the connection's own thread, limited by
                /// setHandshakeTimeout().
                /// @return false if the context is not a ready SERVER context
                ///         with a certificate and key, or the library was built
                ///         without TLS.
            virtual bool        setTlsContext( const std::shared_ptr<CTlsContext> &pContext );
                /// Whether a TLS context has been set, so every connection is HTTPS.
            virtual bool        isSecure( void );
                /// Sets how long a client has to complete the TLS handshake, in
                /// milliseconds (default 10 seconds).
            virtual void        setHandshakeTimeout( int nMilliseconds );

        private:
                /// Joins the threads of finished connections (all of them if bAll).
            void                reapConnections( bool bAll );
    };
} // namespace IASLib

#endif // IASLIB_BASICHTTPSERVER_H__

#endif // IASLIB_NETWORKING__
