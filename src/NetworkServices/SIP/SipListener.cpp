/**
 * SIP Listener class
 *
 * This class handles incoming SIP calls into the server, based
 * on a threaded task model.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipListener.h"
#include "Logging/LogSink.h"
#include "BaseTypes/Uuid.h"

#include "NetworkServices/SIP/SipServer.h"
#include "NetworkServices/SIP/SipRequest.h"
#include "NetworkServices/SIP/SipResponse.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipListener, CGenericListener );

    CSipListener::CSipListener( CSipServer *server, CClientSocket *pSocket ) : CGenericListener( "SipListener", pSocket )
    {
        m_pParentServer = server;
    }

    CSipListener::CSipListener( CSipServer *server, CUDPSocket *pSocket, CInternetAddress &internetAddress, CString incomingData ) : CGenericListener( "SipUDPListener", pSocket, internetAddress, incomingData )
    {
        m_pParentServer = server;
    }

    CSipListener::~CSipListener( void )
    {
    }

    IASLib::CObject *CSipListener::Run( void )
    {
        CUUID erid;
        CSipRequest *sipRequest = new CSipRequest( m_internetAddress );
        sipRequest->parse( *m_pInStream );
        CSipResponse *sipResponse = new CSipResponse( *m_pOutStream );

        addResponseHeaders( sipResponse );

        CSipHandler *sipHandler = m_pParentServer->GetHandler( sipRequest, erid );
        if ( sipHandler )
        {
            CDate start;
            sipHandler->process( sipRequest, sipResponse  );
            CDate end;
            int elapsed = end.Elapsed( start );
            INFO_LOG( "f=%s cip=%s erid=%s uri=%s from=\"%s\" to=\"%s\" rtt=%d sc=%d", 
                (const char *)sipHandler->getMethod(), 
                (const char *)sipRequest->getInternetAddress().toStringWithPort(), 
                (const char *)(erid.toString() ), 
                (const char *)sipRequest->getUri(), 
                (const char *)sipRequest->getHeaderValue("From"), 
                (const char *)sipRequest->getHeaderValue( "To" ), 
                elapsed,
                sipResponse->getStatusCode() );
        }
        else
        {
            sipResponse->setStatus( 400, "Bad Request" );
        }
        
        sipResponse->toStream( m_pOutStream );

        delete sipRequest;
        delete sipHandler;

        return sipResponse;
    }

    void CSipListener::addResponseHeaders( CGenericResponse *response )
    {
        response->addHeader( "x-powered-by", "IASLib Core SIP Library 1.0" );
    }


} // namespace IASLib

#endif // IASLIB_NETWORKING__
