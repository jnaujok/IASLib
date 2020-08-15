/**
 *  SIP Server Class
 *
 *      This class provides an interface for hosting a SIP
 * server. It implements all the basic functionality, but 
 * allows any of the basic commands to be overridden by a 
 * derived class.
 *      This class works by handing off all requests to a 
 * thread pool of workers, running through a SipListener
 * derived class. In most cases, the generic SipListener is
 * adequate. 
 *      The Handler Factory will be used to find a handler
 * for any given request type and URI. if no handler is found
 * then an UNSUPPORTED error will be returned. 
 *      Handlers then process the request and fill the response
 * object. The Response object is finally streamed back in
 * response to the request. 
 *      By default the handler factory is set to the base handlers
 * that return default responses to most SIP commands.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: December 21, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipServer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipServer, CGenericServer );

        // Bind to a port on all interfaces Note Port 5060 is default for SIP, 5061 for SIPS
    CSipServer::CSipServer( int nSipPort, bool bSecure, bool useUDP ) : CGenericServer(), m_bIsUDP( useUDP ), m_bSecure( bSecure )
    {
        if ( isUdp() )
        {

        }
        else
        {

        }
    }

                                // Bind to a port on a specific interface
    CSipServer::CSipServer( const char *boundInterface, int nSipPort, bool bSecure, bool useUDP ) : CGenericServer(), m_bIsUDP( useUDP ), m_bSecure( bSecure )
    {

    }

    CSipServer::~CSipServer( void )
    {

    }

    void *CSipServer::Run( void )
    {

    }

    CSipHandler *CSipServer::GetHandler( CSipRequest *request, CUUID erid )
    {
        CSipHandlerFactory *pFactory = NULL;
        CSipHandler *pRetVal = NULL;

        for ( size_t nX = 0; nX < m_aHandlerFactories.GetCount(); nX++ )
        {
            pFactory = (CSipHandlerFactory *)m_aHandlerFactories[ nX ];
            pRetVal = pFactory->getHandler( request, erid );

            if ( pRetVal )
            {
                break;
            }
        }

        return pRetVal;
    }

    void CSipServer::AddHandlerFactory( CSipHandlerFactory *pFactory )
    {
        m_aHandlerFactories.Prepend( pFactory );
    }

    void CSipServer::RemoveHandlerFactory( CSipHandlerFactory *pFactory )
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
