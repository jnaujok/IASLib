/**
 * SIP Handler Factory class
 *
 * This class provides a factory for generating SIP Handlers. 
 * 
 * Factories have handlers registered to them, and then they are added
 * to the SIP Server. Once added to the server, they override any
 * factories previously added, allowing a chain of handlers to deal 
 * with any particular request.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 29, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipHandlerFactory.h"
#include "Logging/LogSink.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipHandlerFactory, CObject );

    CSipHandlerFactory::CSipHandlerFactory( ) : m_handlerMap( CHash::SMALL )
    {

    }

    CSipHandlerFactory::~CSipHandlerFactory( void )
    {
        m_handlerMap.DeleteAll();
    }

    CSipHandler *CSipHandlerFactory::getHandler( CSipRequest *request, CUUID erid )
    {
        CSipHandler *pRetVal = NULL;

        DEBUG_LOG( "Getting Handler request=[%s] URL=[%s] ip=[%s] erid=[%s]", 
                (const char *)request->getRequestType(), 
                (const char *)request->getUri(), 
                (const char *)request->getInternetAddress().toStringWithPort(),
                (const char *)erid.toString() );

        if ( m_handlerMap.HasKey( request->getRequestType() ) )
        {
            CSipHandler *temp = (CSipHandler *)m_handlerMap.Get( request->getRequestType() );
            pRetVal = temp->generateHandler( request->getUri(), request->getVersion() );
        }

        return pRetVal;
    }

    void CSipHandlerFactory::registerHandler( CSipHandler *handlerFactory )
    {
        m_handlerMap.Push( handlerFactory->getMethod(), handlerFactory );
    }

    bool CSipHandlerFactory::isValidHandler( const char *method )
    {
        CString strMethod(method);
        strMethod.ToUpperCase();
        return m_handlerMap.HasKey( strMethod );
    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__

