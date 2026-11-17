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
 * the one used. TLS is not supported yet.
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

namespace IASLib
{
    struct CBasicHttpServerState;

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

        private:
            void                reapConnections( bool bAll );
    };
} // namespace IASLib

#endif // IASLIB_BASICHTTPSERVER_H__

#endif // IASLIB_NETWORKING__
