/**
 *  Basic SIP Server Class
 *
 *      A ready-to-run SIP server over UDP and TCP. See BasicSipServer.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/BasicSipServer.h"
#include "NetworkServices/SIP/SipListener.h"
#include "NetworkServices/SIP/SipUtils.h"
#include "Sockets/ServerSocket.h"
#include "Sockets/UDPSocket.h"
#include "Logging/LogSink.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <list>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace IASLib
{
    namespace
    {
            // Most UDP SIP messages are well under this; it is the largest
            // datagram the server reads.
        const int UDP_BUFFER_SIZE = 65536;
            // Do not let a flood of datagrams queue without limit.
        const size_t MAX_QUEUED_TASKS = 10000;
            // How often blocking calls wake up to check for shutdown.
        const int POLL_INTERVAL_MS = 200;

        struct CSipConnection
        {
            CSocket            *socket;
            std::thread         thread;
            std::atomic<bool>   finished;

            CSipConnection( void ) : socket( NULL ), finished( false ) {}
        };
    }

    struct CBasicSipServerState
    {
        std::atomic<bool>               stopping;
        std::atomic<int>                udpPort;
        std::atomic<int>                tcpPort;

        std::mutex                      readyMutex;
        std::condition_variable         readyCondition;
        bool                            ready;
        bool                            failed;
        CString                         startupError;

        CUDPSocket                     *udpSocket;
        CServerSocket                  *tcpSocket;
        CSipUdpTransport               *udpTransport;

        std::mutex                      queueMutex;
        std::condition_variable         queueCondition;
        std::deque<CThreadTask *>       queue;
        bool                            stopWorkers;
        std::vector<std::thread>        workers;

        std::thread                     udpThread;
        std::thread                     tcpThread;

        std::mutex                      connectionMutex;
        std::list< std::shared_ptr<CSipConnection> > connections;

        CBasicSipServerState( void ) : stopping( false ), udpPort( 0 ), tcpPort( 0 ), ready( false ), failed( false ),
                                       udpSocket( NULL ), tcpSocket( NULL ), udpTransport( NULL ), stopWorkers( false )
        {
        }
    };

    IMPLEMENT_OBJECT( CBasicSipServer, CSipServer );

    CBasicSipServer::CBasicSipServer( int nPort, bool bUseUdp, bool bUseTcp, size_t nWorkerThreads ) : CSipServer( nPort, false, bUseUdp )
    {
        m_bUseUdp = bUseUdp;
        m_bUseTcp = bUseTcp;
        m_nWorkerThreads = ( nWorkerThreads > 0 ) ? nWorkerThreads : 1;
        m_nMaxMessageSize = 65535;
        m_nIdleTimeoutMs = 5 * 60 * 1000;
        m_pState = new CBasicSipServerState();
    }

    CBasicSipServer::~CBasicSipServer( void )
    {
            // The server thread uses our members, so it must finish first.
        Stop();
        Wait();
        delete m_pState;
        m_pState = NULL;
    }

    void CBasicSipServer::Stop( void )
    {
        m_pState->stopping = true;
        CSipServer::Stop();
    }

    bool CBasicSipServer::waitUntilReady( int nTimeoutMs )
    {
        std::unique_lock<std::mutex> lock( m_pState->readyMutex );
        m_pState->readyCondition.wait_for( lock, std::chrono::milliseconds( nTimeoutMs ),
                                           [this] { return m_pState->ready || m_pState->failed; } );
        return m_pState->ready;
    }

    CString CBasicSipServer::getStartupError( void )
    {
        std::lock_guard<std::mutex> lock( m_pState->readyMutex );
        return m_pState->startupError;
    }

    int CBasicSipServer::getUdpPort( void )
    {
        return m_pState->udpPort;
    }

    int CBasicSipServer::getTcpPort( void )
    {
        return m_pState->tcpPort;
    }

    void CBasicSipServer::queueTask( CThreadTask *pTask )
    {
        {
            std::lock_guard<std::mutex> lock( m_pState->queueMutex );
            if ( m_pState->stopWorkers || m_pState->queue.size() >= MAX_QUEUED_TASKS )
            {
                WARN_LOG( "SIP server is overloaded; dropping a message." );
                delete pTask;
                return;
            }
            m_pState->queue.push_back( pTask );
        }
        m_pState->queueCondition.notify_one();
    }

    void CBasicSipServer::workerLoop( void )
    {
        while ( true )
        {
            CThreadTask *pTask = NULL;
            {
                std::unique_lock<std::mutex> lock( m_pState->queueMutex );
                m_pState->queueCondition.wait( lock, [this] { return m_pState->stopWorkers || ! m_pState->queue.empty(); } );
                if ( m_pState->queue.empty() )
                {
                    return;     // Stopping and nothing left to do.
                }
                pTask = m_pState->queue.front();
                m_pState->queue.pop_front();
            }

            try
            {
                CObject *pResult = pTask->Run();
                if ( pResult )
                {
                    delete pResult;
                }
            }
            catch ( ... )
            {
                ERROR_LOG( "Unexpected exception in a SIP worker task." );
            }
            delete pTask;
        }
    }

    void CBasicSipServer::udpLoop( void )
    {
        std::vector<char> buffer( UDP_BUFFER_SIZE );
        CInternetAddress source;

        while ( ! m_pState->stopping )
        {
            int nRead = m_pState->udpSocket->Read( buffer.data(), UDP_BUFFER_SIZE, source, POLL_INTERVAL_MS );
            if ( nRead <= 0 )
            {
                continue;   // Timeout, error or empty datagram.
            }

            CString message( buffer.data(), (size_t)nRead );
            queueTask( new CSipListener( this, m_pState->udpTransport, source, message ) );
        }
    }

    void CBasicSipServer::tcpAcceptLoop( void )
    {
        while ( ! m_pState->stopping )
        {
            CSocket *pSocket = m_pState->tcpSocket->Accept( POLL_INTERVAL_MS );
            reapConnections( false );

            if ( pSocket == NULL )
            {
                continue;
            }

            std::shared_ptr<CSipConnection> connection = std::make_shared<CSipConnection>();
            connection->socket = pSocket;

            std::lock_guard<std::mutex> lock( m_pState->connectionMutex );
            m_pState->connections.push_back( connection );
            CSipConnection *pConnection = connection.get();
            connection->thread = std::thread( [this, pSocket, pConnection] { connectionLoop( pSocket, pConnection ); } );
        }
    }

    void CBasicSipServer::reapConnections( bool bAll )
    {
        std::list< std::shared_ptr<CSipConnection> > finished;

        {
            std::lock_guard<std::mutex> lock( m_pState->connectionMutex );
            std::list< std::shared_ptr<CSipConnection> >::iterator it = m_pState->connections.begin();
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

        for ( std::list< std::shared_ptr<CSipConnection> >::iterator it = finished.begin(); it != finished.end(); ++it )
        {
            if ( (*it)->thread.joinable() )
            {
                (*it)->thread.join();
            }
        }
    }

    void CBasicSipServer::connectionLoop( CSocket *pSocket, void *pConnectionPtr )
    {
        CSipConnection *pConnection = (CSipConnection *)pConnectionPtr;
        CSipStreamTransport transport( pSocket, false );
        CInternetAddress peer( pSocket->getInternetAddress() );

        std::string buffer;
        std::vector<char> chunk( 8192 );
        int nIdleMs = 0;
        bool bOpen = true;

        while ( bOpen && ! m_pState->stopping )
        {
            int nRead = pSocket->ReadWithTimeout( chunk.data(), (int)chunk.size(), POLL_INTERVAL_MS );
            if ( nRead == CSocket::READ_TIMEOUT )
            {
                nIdleMs += POLL_INTERVAL_MS;
                if ( nIdleMs >= m_nIdleTimeoutMs )
                {
                    break;
                }
                continue;
            }
            if ( nRead <= 0 )
            {
                break;      // Closed by the peer, or an error.
            }

            nIdleMs = 0;
            buffer.append( chunk.data(), (size_t)nRead );

            while ( ! buffer.empty() )
            {
                    // Keep-alives (RFC 5626 4.4.1): answer a double CRLF with a
                    // single CRLF.
                size_t nCrlf = 0;
                while ( nCrlf < buffer.size() && ( buffer[ nCrlf ] == '\r' || buffer[ nCrlf ] == '\n' ) )
                {
                    nCrlf++;
                }
                if ( nCrlf == buffer.size() )
                {
                    if ( buffer.find( "\r\n\r\n" ) != std::string::npos )
                    {
                        transport.send( CString( "\r\n" ), peer );
                        buffer.clear();
                    }
                    break;
                }

                size_t nLeading = 0;
                long nTotal = CSipUtils::frameMessage( buffer.data(), buffer.size(), nLeading, m_nMaxMessageSize );
                if ( nTotal < 0 )
                {
                    WARN_LOG( "Closing a SIP connection from %s: message too large or invalid Content-Length.", (const char *)peer.toStringWithPort() );
                    bOpen = false;
                    break;
                }
                if ( nTotal == 0 )
                {
                    break;  // Need more data.
                }

                CString message( buffer.data() + nLeading, (size_t)nTotal - nLeading );
                buffer.erase( 0, (size_t)nTotal );

                    // Messages on one connection are handled in order, here.
                handleMessage( message, &transport, peer );
            }
        }

        transportClosed( &transport );
        pSocket->Close();
        delete pSocket;
        pConnection->finished = true;
    }

    void *CBasicSipServer::Run( void )
    {
        CString error;

            // Stopped before it ever ran (for example, destroyed unstarted).
        if ( m_pState->stopping )
        {
            return NULL;
        }

        try
        {
            if ( m_bUseUdp )
            {
                m_pState->udpSocket = new CUDPSocket( m_nPort );
                m_pState->udpPort = m_pState->udpSocket->GetPort();
                m_pState->udpTransport = new CSipUdpTransport( m_pState->udpSocket );
            }
        }
        catch ( ... )
        {
            error = CString::FormatString( "Could not open UDP port %d.", m_nPort );
        }

        if ( error.GetLength() == 0 && m_bUseTcp )
        {
                // Use the same port number for TCP as for UDP when the system
                // picked the UDP one.
            int nTcpPort = ( m_nPort == 0 && m_bUseUdp ) ? (int)m_pState->udpPort : m_nPort;
            try
            {
                m_pState->tcpSocket = new CServerSocket( nTcpPort, 64 );
            }
            catch ( ... )
            {
                m_pState->tcpSocket = NULL;
            }

            if ( m_pState->tcpSocket == NULL && m_nPort == 0 )
            {
                try
                {
                    m_pState->tcpSocket = new CServerSocket( 0, 64 );
                }
                catch ( ... )
                {
                    m_pState->tcpSocket = NULL;
                }
            }

            if ( m_pState->tcpSocket == NULL || ! m_pState->tcpSocket->IsConnected() )
            {
                error = CString::FormatString( "Could not open TCP port %d.", nTcpPort );
            }
            else
            {
                m_pState->tcpPort = m_pState->tcpSocket->GetPort();
            }
        }

        if ( error.GetLength() == 0 && ! m_bUseUdp && ! m_bUseTcp )
        {
            error = "Neither UDP nor TCP is enabled.";
        }

        if ( error.GetLength() > 0 )
        {
            ERROR_LOG( "%s", (const char *)error );
            cleanup();
            {
                std::lock_guard<std::mutex> lock( m_pState->readyMutex );
                m_pState->failed = true;
                m_pState->startupError = error;
            }
            m_pState->readyCondition.notify_all();
            return NULL;
        }

        for ( size_t nX = 0; nX < m_nWorkerThreads; nX++ )
        {
            m_pState->workers.push_back( std::thread( [this] { workerLoop(); } ) );
        }
        if ( m_pState->udpSocket )
        {
            m_pState->udpThread = std::thread( [this] { udpLoop(); } );
        }
        if ( m_pState->tcpSocket )
        {
            m_pState->tcpThread = std::thread( [this] { tcpAcceptLoop(); } );
        }

        INFO_LOG( "SIP server listening on UDP port %d, TCP port %d.", (int)m_pState->udpPort, (int)m_pState->tcpPort );

        {
            std::lock_guard<std::mutex> lock( m_pState->readyMutex );
            m_pState->ready = true;
        }
        m_pState->readyCondition.notify_all();

        while ( ! m_pState->stopping && ! IsShutdown() )
        {
            processTimers();
            std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
        }
        m_pState->stopping = true;

            // Stop receiving, then close the connections, then finish the work.
        if ( m_pState->udpThread.joinable() )
            m_pState->udpThread.join();
        if ( m_pState->tcpThread.joinable() )
            m_pState->tcpThread.join();

        {
            std::lock_guard<std::mutex> lock( m_pState->connectionMutex );
            for ( std::list< std::shared_ptr<CSipConnection> >::iterator it = m_pState->connections.begin(); it != m_pState->connections.end(); ++it )
            {
                if ( ! (*it)->finished && (*it)->socket )
                {
                    (*it)->socket->Shutdown();
                }
            }
        }
        reapConnections( true );

        {
            std::lock_guard<std::mutex> lock( m_pState->queueMutex );
            m_pState->stopWorkers = true;
        }
        m_pState->queueCondition.notify_all();
        for ( size_t nX = 0; nX < m_pState->workers.size(); nX++ )
        {
            m_pState->workers[ nX ].join();
        }
        m_pState->workers.clear();

        cleanup();

        {
            std::lock_guard<std::mutex> lock( m_pState->readyMutex );
            m_pState->ready = false;
        }
        return NULL;
    }

    void CBasicSipServer::cleanup( void )
    {
        {
            std::lock_guard<std::mutex> lock( m_pState->queueMutex );
            while ( ! m_pState->queue.empty() )
            {
                delete m_pState->queue.front();
                m_pState->queue.pop_front();
            }
        }

        if ( m_pState->udpTransport )
        {
            transportClosed( m_pState->udpTransport );
            delete m_pState->udpTransport;
            m_pState->udpTransport = NULL;
        }
        if ( m_pState->udpSocket )
        {
            delete m_pState->udpSocket;
            m_pState->udpSocket = NULL;
        }
        if ( m_pState->tcpSocket )
        {
            delete m_pState->tcpSocket;
            m_pState->tcpSocket = NULL;
        }
        m_pState->udpPort = 0;
        m_pState->tcpPort = 0;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
