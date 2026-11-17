/**
 * SIP Handler Factory class
 *
 * Author: Jeffrey R. Naujok
 * Created: December 22, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipHandlerFactory.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipHandlerFactory, CObject );

    CSipHandlerFactory::CSipHandlerFactory( ) : m_aHandlers()
    {
    }

    CSipHandlerFactory::~CSipHandlerFactory( void )
    {
        m_aHandlers.DeleteAll();
    }

    CSipHandler *CSipHandlerFactory::getHandler( CSipRequest *request, CUUID erid )
    {
        CString method = request->getRequestType();

        for ( size_t nX = 0; nX < m_aHandlers.Count(); nX++ )
        {
            CSipHandler *prototype = (CSipHandler *)m_aHandlers.Get( nX );
            if ( prototype->getMethod() == method )
            {
                CSipHandler *pRetVal = prototype->generateHandler( request->getUri(), request->getVersion() );
                if ( pRetVal )
                {
                    pRetVal->setErid( erid );
                }
                return pRetVal;
            }
        }
        return NULL;
    }

    void CSipHandlerFactory::registerHandler( CSipHandler *handlerPrototype )
    {
        if ( handlerPrototype == NULL )
        {
            return;
        }

        CString method = handlerPrototype->getMethod();
        for ( size_t nX = 0; nX < m_aHandlers.Count(); nX++ )
        {
            CSipHandler *existing = (CSipHandler *)m_aHandlers.Get( nX );
            if ( existing->getMethod() == method )
            {
                m_aHandlers.Set( nX, handlerPrototype );
                delete existing;
                return;
            }
        }
        m_aHandlers.Append( handlerPrototype );
    }

    bool CSipHandlerFactory::isValidHandler( const char *method )
    {
        CString strMethod( method );
        for ( size_t nX = 0; nX < m_aHandlers.Count(); nX++ )
        {
            if ( ((CSipHandler *)m_aHandlers.Get( nX ))->getMethod() == strMethod )
            {
                return true;
            }
        }
        return false;
    }

    CStringArray CSipHandlerFactory::getMethods( void )
    {
        CStringArray retVal;
        for ( size_t nX = 0; nX < m_aHandlers.Count(); nX++ )
        {
            retVal.Append( ((CSipHandler *)m_aHandlers.Get( nX ))->getMethod() );
        }
        return retVal;
    }

    CString CSipHandlerFactory::getAllowedList( void )
    {
        CStringArray methods = getMethods();
        CString retVal;
        for ( size_t nX = 0; nX < methods.Count(); nX++ )
        {
            if ( nX > 0 )
            {
                retVal += ", ";
            }
            retVal += methods[ nX ];
        }
        return retVal;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
