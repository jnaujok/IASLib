/**
 *  HTTP Server Class
 *
 *      This class provides an interface for hosting an HTTP
 * server. It implements all the basic functionality, but 
 * allows any of the basic commands to be overridden by a 
 * derived class.
 *      This class works by creating a thread pool for doing the
 * actual work of the class. When in practical use, the standard
 * HTTP Handler Factory can be overridden, and unique handlers
 * assigned for any function or URI. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/16/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "HttpServer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpServer, CGenericServer );

    CHttpServer::CHttpServer( int nHttpPort, bool bSecure ) : CGenericServer(), m_aHandlerFactories()
    {
        m_bUseHTTP11 = true;
        m_bUseKeepalive = true;
        m_bSecure = bSecure;
    }

        // Bind to a port on a specific interface
    CHttpServer::CHttpServer( const char *boundInterface, int nHttpPort, bool bSecure ) : CGenericServer(), m_aHandlerFactories()
    {
        m_bUseHTTP11 = true;
        m_bUseKeepalive = true;
        m_bSecure = bSecure;
    }

    CHttpServer::~CHttpServer( void )
    {
        m_aHandlerFactories.DeleteAll();
    }

    void *CHttpServer::Run( void )
    {

    }

    CHttpHandler *CHttpServer::GetHandler( CHttpRequest *request, CUUID erid )
    {
        CHttpHandlerFactory *pFactory = NULL;
        CHttpHandler *pRetVal = NULL;

        for ( size_t nX = 0; nX < m_aHandlerFactories.GetCount(); nX++ )
        {
            pFactory = (CHttpHandlerFactory *)m_aHandlerFactories[ nX ];
            pRetVal = pFactory->getHandler( request, erid );

            if ( pRetVal )
            {
                break;
            }
        }

        return pRetVal;
    }

    void CHttpServer::AddHandlerFactory( CHttpHandlerFactory *pFactory )
    {
        m_aHandlerFactories.Prepend( pFactory );
    }

    void CHttpServer::RemoveHandlerFactory( CHttpHandlerFactory *pFactory )
    {
        for ( size_t nX = 0; nX < m_aHandlerFactories.GetCount(); nX++ )
        {
            if ( m_aHandlerFactories[ nX ] == pFactory )
            {
                m_aHandlerFactories.Remove( nX );
                break;
            }
        }
    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__
