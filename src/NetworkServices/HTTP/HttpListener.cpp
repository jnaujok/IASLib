/**
 * HTTP Listener class
 *
 * A thread task that serves one HTTP connection.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 24, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/HttpListener.h"
#include "NetworkServices/HTTP/HttpServer.h"
#include "NetworkServices/HTTP/HttpUtils.h"
#include "Logging/LogSink.h"

#include <chrono>
#include <string>
#include <vector>

namespace IASLib
{
    namespace
    {
        const int POLL_INTERVAL_MS = 200;

        unsigned long long nowMs( void )
        {
            return (unsigned long long)std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch() ).count();
        }
    }

    IMPLEMENT_OBJECT( CHttpListener, CThreadTask );

    CHttpListener::CHttpListener( CHttpServer *server, CSocket *pSocket ) :
        CThreadTask( "HttpListener" ), m_internetAddress( pSocket->getInternetAddress() )
    {
        m_pParentServer = server;
        m_pSocket = pSocket;
    }

    CHttpListener::~CHttpListener( void )
    {
        if ( m_pSocket )
        {
            m_pSocket->Close();
            delete m_pSocket;
            m_pSocket = NULL;
        }
    }

    bool CHttpListener::sendData( const CString &data )
    {
        try
        {
            m_pSocket->Send( (const char *)data, (int)data.GetLength() );
        }
        catch ( ... )
        {
            return false;
        }
        return true;
    }

    IASLib::CObject *CHttpListener::Run( void )
    {
        if ( m_pParentServer == NULL || m_pSocket == NULL )
        {
            return NULL;
        }

        std::string buffer;
        std::vector<char> chunk( 16384 );
        bool bSentContinue = false;
        unsigned long long nIdleSince = nowMs();
        unsigned long long nRequestStarted = 0;

        while ( ! m_pParentServer->IsShutdown() )
        {
            CHttpFrame frame = CHttpUtils::frameRequest( buffer.data(), buffer.size(),
                                                         m_pParentServer->getMaxHeaderSize(),
                                                         m_pParentServer->getMaxBodySize() );

            if ( frame.status == CHttpFrame::FAILED )
            {
                sendData( m_pParentServer->errorResponse( frame.errorStatus ) );
                break;
            }

            if ( frame.status == CHttpFrame::COMPLETE )
            {
                buffer.erase( 0, frame.consumed );
                bSentContinue = false;
                nRequestStarted = 0;

                bool bKeepAlive = false;
                    // FromBytes keeps a binary body whole. (The (text, length)
                    // string stopped at the first zero byte, cutting the body
                    // short of its Content-Length, so the request got 400.)
                CString response = m_pParentServer->handleRequest( CString::FromBytes( frame.message.data(), frame.message.size() ),
                                                                   m_internetAddress, bKeepAlive,
                                                                   m_pSocket->IsSecure(), m_pSocket->GetTlsServerName() );
                if ( ! sendData( response ) || ! bKeepAlive )
                {
                    break;
                }
                nIdleSince = nowMs();
                continue;   // There may be another (pipelined) request buffered.
            }

                // More data is needed.
            if ( frame.expectContinue && ! bSentContinue )
            {
                    // RFC 9110 10.1.1: let the client send the body.
                bSentContinue = true;
                if ( ! sendData( CString( "HTTP/1.1 100 Continue\r\n\r\n" ) ) )
                {
                    break;
                }
            }

            if ( ! buffer.empty() && nRequestStarted == 0 )
            {
                nRequestStarted = nowMs();
            }

            int nRead = m_pSocket->ReadWithTimeout( chunk.data(), (int)chunk.size(), POLL_INTERVAL_MS );
            if ( nRead == CSocket::READ_TIMEOUT )
            {
                unsigned long long nNow = nowMs();
                if ( buffer.empty() )
                {
                    if ( (long long)( nNow - nIdleSince ) >= m_pParentServer->getKeepAliveTimeout() )
                    {
                        break;      // Idle connection: close quietly.
                    }
                }
                else if ( (long long)( nNow - nRequestStarted ) >= m_pParentServer->getRequestTimeout() )
                {
                    sendData( m_pParentServer->errorResponse( 408 ) );
                    break;
                }
                continue;
            }
            if ( nRead <= 0 )
            {
                break;      // Closed by the client, or an error.
            }

            buffer.append( chunk.data(), (size_t)nRead );
        }

            // The destructor closes the socket.
        return NULL;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
