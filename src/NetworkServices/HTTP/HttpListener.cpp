/**
 * HTTP Listener class
 *
 * This class handles incoming HTTP calls into the server, based
 * on a threaded task model.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/HttpListener.h"
#include "Logging/LogSink.h"
#include "BaseTypes/Uuid.h"

#include "NetworkServices/HTTP/HttpServer.h"
#include "NetworkServices/HTTP/HttpRequest.h"
#include "NetworkServices/HTTP/HttpResponse.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpListener, CGenericListener );

    CHttpListener::CHttpListener( CHttpServer *server, CClientSocket *pSocket ) : CGenericListener( "HttpListener", pSocket )
    {
    }

    CHttpListener::CHttpListener( CHttpServer *server, CUDPSocket *pSocket, CInternetAddress &internetAddress, CString incomingData ) : CGenericListener( "HttpUDPListener", pSocket, internetAddress, incomingData )
    {
    }

    CHttpListener::~CHttpListener( void )
    {
    }

    IASLib::CObject *CHttpListener::Run( void )
    {
        CUUID erid;
        CHttpRequest *httpRequest = new CHttpRequest( m_internetAddress );
        httpRequest->parse( *m_pInStream );
        CHttpResponse *httpResponse = new CHttpResponse( *m_pOutStream, m_internetAddress );

        addResponseHeaders( httpResponse );

        CHttpHandler *httpHandler = m_pParentServer->GetHandler( httpRequest, erid );
        if ( httpHandler )
        {
            CDate start;
            httpHandler->process( httpRequest, httpResponse  );
            CDate end;
            int elapsed = end.Elapsed( start );
            INFO_LOG( "f=%s cip=%s erid=%s uri=%s from=\"%s\" to=\"%s\" rtt=%d sc=%d", 
                (const char *)httpHandler->getMethod(), 
                (const char *)httpRequest->getInternetAddress().toStringWithPort(), 
                (const char *)(erid.toString() ), 
                (const char *)httpRequest->getUri(), 
                (const char *)httpRequest->getHeaderValue("From"), 
                (const char *)httpRequest->getHeaderValue( "To" ), 
                elapsed,
                httpResponse->getStatusCode() );
        }
        else
        {
            httpResponse->setStatus( 400, "Bad Request" );
        }
        
        httpResponse->toStream( m_pOutStream );

        delete httpRequest;
        delete httpHandler;

        return httpResponse;
    }

    void CHttpListener::addResponseHeaders( CGenericResponse *response )
    {
        response->addHeader( "x-powered-by", "IASLib Core HTTP Library 1.0" );
    }


} // namespace IASLib

#endif // IASLIB_NETWORKING__
