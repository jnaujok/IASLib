/**
 * HTTP Handler Factory class
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/Handlers/HttpHandlerFactory.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpHandlerFactory, CObject );

    CHttpHandlerFactory::CHttpHandlerFactory( ) : m_aHandlers()
    {
    }

    CHttpHandlerFactory::~CHttpHandlerFactory( void )
    {
        m_aHandlers.DeleteAll();
    }

    CHttpHandler *CHttpHandlerFactory::getHandler( CHttpRequest *request, CUUID erid )
    {
        return getHandler( request->getRequestType(), request, erid );
    }

    CHttpHandler *CHttpHandlerFactory::getHandler( const CString &method, CHttpRequest *request, CUUID erid )
    {
        for ( size_t nX = 0; nX < m_aHandlers.Count(); nX++ )
        {
            CHttpHandler *prototype = (CHttpHandler *)m_aHandlers.Get( nX );
            if ( prototype->getMethod() == method )
            {
                CHttpHandler *pRetVal = prototype->generateHandler( request->getUri(), request->getVersion() );
                if ( pRetVal )
                {
                    pRetVal->setErid( erid );
                }
                return pRetVal;
            }
        }
        return NULL;
    }

    void CHttpHandlerFactory::registerHandler( CHttpHandler *handlerPrototype )
    {
        if ( handlerPrototype == NULL )
        {
            return;
        }

        CString method = handlerPrototype->getMethod();
        for ( size_t nX = 0; nX < m_aHandlers.Count(); nX++ )
        {
            CHttpHandler *existing = (CHttpHandler *)m_aHandlers.Get( nX );
            if ( existing->getMethod() == method )
            {
                m_aHandlers.Set( nX, handlerPrototype );
                delete existing;
                return;
            }
        }
        m_aHandlers.Append( handlerPrototype );
    }

    bool CHttpHandlerFactory::isValidHandler( const char *method )
    {
        CString strMethod( method );
        for ( size_t nX = 0; nX < m_aHandlers.Count(); nX++ )
        {
            if ( ((CHttpHandler *)m_aHandlers.Get( nX ))->getMethod() == strMethod )
            {
                return true;
            }
        }
        return false;
    }

    CStringArray CHttpHandlerFactory::getMethods( void )
    {
        CStringArray retVal;
        for ( size_t nX = 0; nX < m_aHandlers.Count(); nX++ )
        {
            retVal.Append( ((CHttpHandler *)m_aHandlers.Get( nX ))->getMethod() );
        }
        return retVal;
    }

    CString CHttpHandlerFactory::getAllowedList( void )
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
