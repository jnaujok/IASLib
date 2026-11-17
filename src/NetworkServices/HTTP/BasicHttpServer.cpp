/**
 *  Basic HTTP Server Class
 *
 *      A ready-to-run HTTP/1.1 server. See BasicHttpServer.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/BasicHttpServer.h"
#include "NetworkServices/HTTP/HttpListener.h"
#include "Sockets/ServerSocket.h"
#include "Logging/LogSink.h"
#ifdef IASLIB_TLS__
#include "Sockets/SecureSocket.h"
#endif

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace IASLib
{
    namespace
    {
        const int POLL_INTERVAL_MS = 200;

        struct CHttpConnection
        {
            CSocket            *socket;     // Owned by the listener once it runs.
            std::thread         thread;
            std::atomic<bool>   finished;

            CHttpConnection( void ) : socket( NULL ), finished( false ) {}
        };
    }

    struct CBasicHttpServerState
    {
        std::atomic<bool>               stopping;
        std::atomic<int>                port;

        std::mutex                      readyMutex;
        std::condition_variable         readyCondition;
        bool                            ready;
        bool                            failed;
        CString                         startupError;

        std::mutex                      connectionMutex;
        std::list< std::shared_ptr<CHttpConnection> > connections;

        std::shared_ptr<CTlsContext>    tlsContext;         // set for HTTPS
        std::atomic<int>                handshakeTimeoutMs;

        CBasicHttpServerState( void ) : stopping( false ), port( 0 ), ready( false ), failed( false ), handshakeTimeoutMs( 10000 ) {}
    };

    IMPLEMENT_OBJECT( CBasicHttpServer, CHttpServer );

    CBasicHttpServer::CBasicHttpServer( int nPort, size_t nMaxConnections ) : CHttpServer( nPort, false )
    {
        m_nMaxConnections = ( nMaxConnections > 0 ) ? nMaxConnections : 1;
        m_pState = new CBasicHttpServerState();
    }

    CBasicHttpServer::~CBasicHttpServer( void )
    {
            // The server thread uses our members, so it must finish first.
        Stop();
        Wait();
        delete m_pState;
        m_pState = NULL;
    }

    void CBasicHttpServer::Stop( void )
    {
        m_pState->stopping = true;
        CHttpServer::Stop();
    }

    bool CBasicHttpServer::waitUntilReady( int nTimeoutMs )
    {
        std::unique_lock<std::mutex> lock( m_pState->readyMutex );
        m_pState->readyCondition.wait_for( lock, std::chrono::milliseconds( nTimeoutMs ),
                                           [this] { return m_pState->ready || m_pState->failed; } );
        return m_pState->ready;
    }

    CString CBasicHttpServer::getStartupError( void )
    {
        std::lock_guard<std::mutex> lock( m_pState->readyMutex );
        return m_pState->startupError;
    }

    int CBasicHttpServer::getListeningPort( void )
    {
        return m_pState->port;
    }

    size_t CBasicHttpServer::getConnectionCount( void )
    {
        std::lock_guard<std::mutex> lock( m_pState->connectionMutex );
        size_t nCount = 0;
        for ( std::list< std::shared_ptr<CHttpConnection> >::iterator it = m_pState->connections.begin(); it != m_pState->connections.end(); ++it )
        {
            if ( ! (*it)->finished )
                nCount++;
        }
        return nCount;
    }

    bool CBasicHttpServer::setTlsContext( const std::shared_ptr<CTlsContext> &pContext )
    {
#ifdef IASLIB_TLS__
        if ( ! pContext || pContext->getRole() != CTlsContext::SERVER || ! pContext->isReady() )
        {
            ERROR_LOG( "HTTPS needs a server TLS context with a certificate and its private key." );
            return false;
        }
        m_pState->tlsContext = pContext;
        return true;
#else
        (void)pContext;
        ERROR_LOG( "HTTPS is not available: the library was built without OpenSSL." );
        return false;
#endif
    }

    bool CBasicHttpServer::isSecure( void )
    {
        return (bool)m_pState->tlsContext;
    }

    void CBasicHttpServer::setHandshakeTimeout( int nMilliseconds )
    {
        m_pState->handshakeTimeoutMs = nMilliseconds;
    }

    void CBasicHttpServer::reapConnections( bool bAll )
    {
        std::list< std::shared_ptr<CHttpConnection> > finished;

        {
            std::lock_guard<std::mutex> lock( m_pState->connectionMutex );
            std::list< std::shared_ptr<CHttpConnection> >::iterator it = m_pState->connections.begin();
            while ( it != m_pState->connections.end() )
            {
                if ( bAll || (*it)->finished )
                {
                    finished.push_back( *it );
                    it = m_pState->connections.erase( it );
                }
                else
                {
                    ++it;
                }
            }
        }

        for ( std::list< std::shared_ptr<CHttpConnection> >::iterator it = finished.begin(); it != finished.end(); ++it )
        {
            if ( (*it)->thread.joinable() )
            {
                (*it)->thread.join();
            }
        }
    }

    void *CBasicHttpServer::Run( void )
    {
            // Stopped before it ever ran (for example, destroyed unstarted).
        if ( m_pState->stopping )
        {
            return NULL;
        }

        CServerSocket *pListenSocket = NULL;
        try
        {
            pListenSocket = new CServerSocket( m_nPort, 128 );
        }
        catch ( ... )
        {
            pListenSocket = NULL;
        }

        if ( pListenSocket == NULL || ! pListenSocket->IsConnected() )
        {
            CString error = CString::FormatString( "Could not open TCP port %d.", m_nPort );
            ERROR_LOG( "%s", (const char *)error );
            if ( pListenSocket )
            {
                delete pListenSocket;
            }
            {
                std::lock_guard<std::mutex> lock( m_pState->readyMutex );
                m_pState->failed = true;
                m_pState->startupError = error;
            }
            m_pState->readyCondition.notify_all();
            return NULL;
        }

        m_pState->port = pListenSocket->GetPort();
        INFO_LOG( "HTTP%s server listening on TCP port %d.", m_pState->tlsContext ? "S" : "", (int)m_pState->port );

        {
            std::lock_guard<std::mutex> lock( m_pState->readyMutex );
            m_pState->ready = true;
        }
        m_pState->readyCondition.notify_all();

        while ( ! m_pState->stopping && ! IsShutdown() )
        {
            CSocket *pSocket = pListenSocket->Accept( POLL_INTERVAL_MS );
            reapConnections( false );

            if ( pSocket == NULL )
            {
                continue;
            }

            if ( getConnectionCount() >= m_nMaxConnections )
            {
                    // Over the limit: say so and close. (Over TLS that would
                    // need a handshake first, so the connection is just closed.)
                if ( ! m_pState->tlsContext )
                {
                    CString busy = errorResponse( 503 );
                    try
                    {
                        pSocket->Send( (const char *)busy, (int)busy.GetLength() );
                    }
                    catch ( ... )
                    {
                    }
                }
                pSocket->Close();
                delete pSocket;
                continue;
            }

#ifdef IASLIB_TLS__
            bool bSecure = (bool)m_pState->tlsContext;
            if ( bSecure )
            {
                    // Takes the accepted connection over; the handshake happens
                    // on the connection's thread.
                pSocket = new CSecureSocket( pSocket, m_pState->tlsContext );
            }
#else
            bool bSecure = false;
#endif

            std::shared_ptr<CHttpConnection> connection = std::make_shared<CHttpConnection>();
            connection->socket = pSocket;
            CHttpConnection *pConnection = connection.get();

            std::lock_guard<std::mutex> lock( m_pState->connectionMutex );
            m_pState->connections.push_back( connection );
            connection->thread = std::thread( [this, pSocket, pConnection, bSecure]
            {
#ifdef IASLIB_TLS__
                if ( bSecure )
                {
                    CSecureSocket *pSecure = static_cast<CSecureSocket *>( pSocket );
                    if ( ! pSecure->Handshake( m_pState->handshakeTimeoutMs ) )
                    {
                        DEBUG_LOG( "TLS handshake with %s failed: %s", (const char *)pSecure->getInternetAddress().toStringWithPort(),
                                   (const char *)pSecure->GetTlsError() );
                        {
                            std::lock_guard<std::mutex> socketLock( m_pState->connectionMutex );
                            pConnection->socket = NULL;
                        }
                        pSecure->Close();
                        delete pSecure;
                        pConnection->finished = true;
                        return;
                    }
                }
#else
                (void)bSecure;
#endif
                {
                    CHttpListener listener( this, pSocket );
                    try
                    {
                        listener.Run();
                    }
                    catch ( ... )
                    {
                        ERROR_LOG( "Unexpected exception in an HTTP connection." );
                    }

                        // Forget the socket under the lock before the listener
                        // closes and deletes it, so that shutdown never touches
                        // a closed socket.
                    std::lock_guard<std::mutex> socketLock( m_pState->connectionMutex );
                    pConnection->socket = NULL;
                }
                pConnection->finished = true;
            } );
        }
        m_pState->stopping = true;

            // Wake the connections (they also check IsShutdown()), then wait.
        {
            std::lock_guard<std::mutex> lock( m_pState->connectionMutex );
            for ( std::list< std::shared_ptr<CHttpConnection> >::iterator it = m_pState->connections.begin(); it != m_pState->connections.end(); ++it )
            {
                if ( (*it)->socket )
                {
                    (*it)->socket->Shutdown();
                }
            }
        }
        reapConnections( true );

        delete pListenSocket;
        m_pState->port = 0;
        {
            std::lock_guard<std::mutex> lock( m_pState->readyMutex );
            m_pState->ready = false;
        }
        return NULL;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
