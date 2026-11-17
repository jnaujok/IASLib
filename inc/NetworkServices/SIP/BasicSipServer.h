/**
 *  Basic SIP Server Class
 *
 *      A ready-to-run SIP server built on the CSipServer protocol core. It
 * listens on UDP and/or TCP, frames TCP messages by Content-Length
 * (RFC 3261 18.3), answers CRLF keep-alives on TCP (RFC 5626), runs the
 * transaction timers, and processes requests on a pool of worker threads.
 *
 *      Typical use:
 *
 *          CBasicSipServer server( 5060 );
 *          server.AddHandlerFactory( new CBaseSipHandlerFactory() );
 *          server.AddHandlerFactory( new CMyHandlerFactory() );
 *          server.Start();
 *          if ( ! server.waitUntilReady( 5000 ) ) { ... getStartupError() ... }
 *          ...
 *          server.Stop();
 *          server.Wait();
 *
 *      Port 0 asks the system for a free port; getUdpPort() and
 * getTcpPort() report the ports actually used.
 *
 *      SIP over TLS (RFC 3261 section 26.2): give the server a TLS context
 * before Start() and it also listens for TLS connections, on port 5061 by
 * default. Responses go back over the same connection, and the transport
 * is "TLS" in Via headers. The handshake runs on the connection's own
 * thread, limited by setHandshakeTimeout() (10 seconds by default). UDP and
 * TCP can be turned off for a TLS-only server. Needs a library built with
 * OpenSSL (IASLIB_TLS__).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_BASICSIPSERVER_H__
#define IASLIB_BASICSIPSERVER_H__

#include "NetworkServices/SIP/SipServer.h"
#include "Threading/ThreadTask.h"

#include <memory>

namespace IASLib
{
    struct CBasicSipServerState;
    class CTlsContext;

    class CBasicSipServer : public CSipServer
    {
        protected:
            bool                    m_bUseUdp;
            bool                    m_bUseTcp;
            size_t                  m_nWorkerThreads;
            size_t                  m_nMaxMessageSize;
            int                     m_nIdleTimeoutMs;
            CBasicSipServerState   *m_pState;

        public:
                                CBasicSipServer( int nPort = 5060, bool bUseUdp = true, bool bUseTcp = true, size_t nWorkerThreads = 8 );
            virtual            ~CBasicSipServer( void );

                                DEFINE_OBJECT( CBasicSipServer );

                // Binds the sockets, then serves until Stop() is called.
            virtual void       *Run( void );

            virtual void        Stop( void );

                // Waits until the server is listening (true) or has failed to
                // start (false; see getStartupError()). Call after Start().
            virtual bool        waitUntilReady( int nTimeoutMs );
            virtual CString     getStartupError( void );

                // The ports in use once the server is ready (0 if not listening).
            virtual int         getUdpPort( void );
            virtual int         getTcpPort( void );
            virtual int         getTlsPort( void );

                // Also serves SIP over TLS, on nTlsPort (0 for any free port),
                // with this SERVER context. Call before Start(). False if the
                // context has no certificate and key, or the library was built
                // without TLS.
            virtual bool        setTlsContext( const std::shared_ptr<CTlsContext> &pContext, int nTlsPort = 5061 );
                // How long a client has to complete the TLS handshake.
            virtual void        setHandshakeTimeout( int nMilliseconds );

                // The largest message accepted on TCP (default 65535 bytes).
            virtual void        setMaxMessageSize( size_t nBytes ) { m_nMaxMessageSize = nBytes; }
                // Idle TCP connections are closed after this long (default 5 minutes).
            virtual void        setIdleTimeout( int nMilliseconds ) { m_nIdleTimeoutMs = nMilliseconds; }

        protected:
                // Runs the task on a worker thread; the server deletes it after.
            virtual void        queueTask( CThreadTask *pTask );

        private:
            void                workerLoop( void );
            void                udpLoop( void );
            void                acceptLoop( bool bSecure );
            void                connectionLoop( CSocket *pSocket, void *pConnection, bool bSecure );
            void                reapConnections( bool bAll );
            void                cleanup( void );
    };
} // namespace IASLib

#endif // IASLIB_BASICSIPSERVER_H__

#endif // IASLIB_NETWORKING__
