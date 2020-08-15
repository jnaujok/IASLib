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
 *	Created: October 2, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPSERVER_H__
#define IASLIB_SIPSERVER_H__

#include "Collections/Array.h"
#include "Sockets/ServerSocket.h"
#include "NetworkServices/GenericServer.h"
#include "NetworkServices/SIP/Handlers/SipHandler.h"
#include "NetworkServices/SIP/Handlers/SipHandlerFactory.h"

namespace IASLib
{
    class CSipServer : public CGenericServer  
    {
        protected:
            bool            m_bSecure;
            CArray          m_aHandlerFactories;
            bool            m_bIsUDP;

        public:
                                // Bind to a port on all interfaces Note Port 5060 is default for SIP, 5061 for SIPS
	                        CSipServer( int nSipPort=5060, bool bSecure = false, bool useUDP = false );
                                // Bind to a port on a specific interface
	                        CSipServer( const char *boundInterface, int nSipPort=5060, bool bSecure = false, bool useUDP = false );

	        virtual        ~CSipServer( void );

                            DEFINE_OBJECT( CSipServer )

            virtual void   *Run( void );

            virtual void    AddHandlerFactory( CSipHandlerFactory *pFactory );            
            virtual void    RemoveHandlerFactory( CSipHandlerFactory *pFactory );         

            virtual CSipHandler   *GetHandler( CSipRequest *request, CUUID erid );

            virtual bool    isUdp( void ) { return m_bIsUDP; }

            virtual bool    isSecure( void ) { return m_bSecure; }
    };
} // namespace IASLib

#endif // IASLIB_SIPSERVER_H__

#endif // IASLIB_NETWORKING__
