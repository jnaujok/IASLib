/**
 * SIP Handler class
 *
 * This class handles a particular request type for a SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipHandler.h"
#include "Collections/Hash.h"
#include "Logging/LogSink.h"

namespace IASLib
{
    CHash CSipHandler::m_handlerMap( CHash::SMALL );
    size_t CSipHandler::handlerCount = 0;
    CSipHandler *CSipHandler::handlers[128];
    bool CSipHandler::m_initialized = false;

    IMPLEMENT_OBJECT( CSipHandler, CObject );

    CSipHandler::CSipHandler( CString uri, CString version )
    {
        m_strUri = uri;
        m_strVersion = version;
    }


    CSipHandler::~CSipHandler( void )
    {
    }

    CSipHandler *CSipHandler::getHandler( CSipRequest *request, CUUID erid )
    {
        initHash();
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
            pRetVal->setErid( erid );
        }

        if ( pRetVal == NULL )
        {
            DEBUG_LOG( "No Handler Found! request=[%s] URL=[%s] ip=[%s] erid=[%s]", 
                    (const char *)request->getRequestType(), 
                    (const char *)request->getUri(), 
                    (const char *)request->getInternetAddress().toStringWithPort(),
                    (const char *)erid.toString() );
            DEBUG_LOG( "Request:\n%s", (const char *)request->toString() );
        }

        return pRetVal;
    }

    void CSipHandler::registerHandler( CSipHandler *handlerFactory )
    {
        int nX = 0;
        while ( handlers[nX] )
        {
            nX++;
        }

        handlers[nX] = handlerFactory;
        handlerCount = nX;
    }

    bool CSipHandler::isValidHandler( const char *method )
    {
        initHash();
        CString strMethod(method);
        strMethod.ToUpperCase();
        if ( m_handlerMap.HasKey( strMethod ) )
            return true;

        return false;
    }

    void CSipHandler::initHash( void )
    {
        if ( m_initialized == false )
        {
            size_t x = 0;

            while ( handlers[x] )
            {
                m_handlerMap.Push( handlers[x]->getMethod(), handlers[x]);
                x++;
            }
            m_initialized = true;
        }
    }

    CString CSipHandler::getAllowedList( void )
    {
        CString allowList = "";
        int nX = 0;
        while ( handlers[ nX ] )
        {
            if ( nX > 0 )
                allowList += ", ";
            allowList += handlers[nX]->getMethod();
            nX++;
        }    

        return allowList;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
