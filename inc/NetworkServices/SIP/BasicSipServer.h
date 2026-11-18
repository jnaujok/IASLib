/**
 * @file BasicSipServer.h
 * @brief A ready-to-run SIP server over UDP, TCP and TLS (CBasicSipServer).
 *
 *  Basic SIP Server Class
 *
 *      A ready-to-run SIP server built on the CSipServer protocol core. It
 * listens on UDP and/or TCP, frames TCP messages by Content-Length
 * (RFC 3261 18.3), answers CRLF keep-alives on TCP (RFC 5626), runs the
 * transaction timers, and processes requests on a pool of worker threads.
 *
 *      Typical use:
 *
 * @code
 *          CBasicSipServer server( 5060 );
 *          server.AddHandlerFactory( new CBaseSipHandlerFactory() );
 *          server.AddHandlerFactory( new CMyHandlerFactory() );
 *          server.Start();
 *          if ( ! server.waitUntilReady( 5000 ) ) { ... getStartupError() ... }
 *          ...
 *          server.Stop();
 *          server.Wait();
 * @endcode
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

    /// A ready-to-run SIP server on UDP and/or TCP, with optional TLS.
    ///
    /// Run() binds the sockets and serves until Stop(). Datagrams are queued
    /// to a pool of worker threads (a full queue drops messages); each TCP or
    /// TLS connection has its own thread, which frames messages by
    /// Content-Length and handles them there. The transaction timers run on
    /// the server thread. Request handling is that of CSipServer.
    ///
    /// Configure the server before Start(). Stop() and Wait() (also run by
    /// the destructor) close the sockets and connections and wait for every
    /// thread.
    class CBasicSipServer : public CSipServer
    {
        protected:
            bool                    m_bUseUdp;          ///< Whether to listen on UDP.
            bool                    m_bUseTcp;          ///< Whether to listen on TCP.
            size_t                  m_nWorkerThreads;   ///< The size of the worker pool (at least 1).
            size_t                  m_nMaxMessageSize;  ///< The largest message accepted on TCP (65535 by default).
            int                     m_nIdleTimeoutMs;   ///< How long an idle TCP connection is kept (5 minutes by default).
            CBasicSipServerState   *m_pState;           ///< Run-time state shared with the threads (owned).

        public:
                                /// Creates the server; nothing is bound until it runs.
                                /// @param nPort          The UDP and TCP port; 0 asks the
                                ///                       system for a free one.
                                /// @param bUseUdp        Whether to listen on UDP.
                                /// @param bUseTcp        Whether to listen on TCP.
                                /// @param nWorkerThreads The size of the worker pool for
                                ///                       datagrams (0 is taken as 1).
                                CBasicSipServer( int nPort = 5060, bool bUseUdp = true, bool bUseTcp = true, size_t nWorkerThreads = 8 );
                                /// Stops the server and waits for all of its threads.
            virtual            ~CBasicSipServer( void );

                                DEFINE_OBJECT( CBasicSipServer );

                /// Binds the sockets, then serves until Stop() is called.
                ///
                /// A socket that cannot be opened is reported through
                /// waitUntilReady() and getStartupError().
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

                /// The UDP port in use once the server is ready (0 if not listening).
            virtual int         getUdpPort( void );
                /// The TCP port in use once the server is ready (0 if not listening).
            virtual int         getTcpPort( void );
                /// The TLS port in use once the server is ready (0 if not listening).
            virtual int         getTlsPort( void );

                /// Also serves SIP over TLS, with this SERVER context.
                ///
                /// Call before Start(). Responses go back over the same
                /// connection, and the transport is "TLS" in Via headers.
                /// @param pContext A ready SERVER context with a certificate and key.
                /// @param nTlsPort The TLS port (0 for any free port).
                /// @return false if the context is not a ready SERVER context,
                ///         or the library was built without TLS.
            virtual bool        setTlsContext( const std::shared_ptr<CTlsContext> &pContext, int nTlsPort = 5061 );
                /// Sets how long a client has to complete the TLS handshake, in
                /// milliseconds (default 10 seconds).
            virtual void        setHandshakeTimeout( int nMilliseconds );

                /// Sets the largest message accepted on TCP (default 65535
                /// bytes); a larger one closes the connection.
            virtual void        setMaxMessageSize( size_t nBytes ) { m_nMaxMessageSize = nBytes; }
                /// Sets how long an idle TCP connection is kept, in milliseconds
                /// (default 5 minutes).
            virtual void        setIdleTimeout( int nMilliseconds ) { m_nIdleTimeoutMs = nMilliseconds; }

        protected:
                /// Runs the task on a worker thread; the server deletes it after.
                ///
                /// The server takes ownership at once. When stopping, or when
                /// the queue is full, the task is deleted without being run.
            virtual void        queueTask( CThreadTask *pTask );

        private:
                /// The body of a worker thread: runs queued tasks until stopped.
            void                workerLoop( void );
                /// Receives datagrams and queues a CSipListener for each.
            void                udpLoop( void );
                /// Accepts TCP (or TLS) connections and starts a thread for each.
            void                acceptLoop( bool bSecure );
                /// Serves one connection: handshake, framing and handling.
            void                connectionLoop( CSocket *pSocket, void *pConnection, bool bSecure );
                /// Joins the threads of finished connections (all of them if bAll).
            void                reapConnections( bool bAll );
                /// Deletes queued tasks, the transports and the sockets.
            void                cleanup( void );
    };
} // namespace IASLib

#endif // IASLIB_BASICSIPSERVER_H__

#endif // IASLIB_NETWORKING__
