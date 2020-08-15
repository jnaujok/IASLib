/**
 *  HTTP Server Class
 *
 *      This class provides an interface for hosting an HTTP
 * server. It implements all the basic functionality, but 
 * allows any of the basic commands to be overridden by a 
 * derived class.
 *      This class works in both threaded and non-threaded
 * modes, but in threaded mode, it can either create a new
 * thread for each handler, or it can hand off each request
 * to a thread pool of handlers. In the case of a multi-
 * threaded handlers, each handler function will be called 
 * from a new thread.
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

#ifndef IASLIB_HTTPSERVER_H__
#define IASLIB_HTTPSERVER_H__

#include "Collections/Array.h"
#include "Sockets/ServerSocket.h"
#include "NetworkServices/GenericServer.h"
#include "NetworkServices/HTTP/Handlers/HttpHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpHandlerFactory.h"

namespace IASLib
{
    class CHttpServer : public CGenericServer  
    {
        protected:
            bool            m_bUseHTTP11;
            bool            m_bUseKeepalive;
            bool            m_bSecure;
            CArray          m_aHandlerFactories;
        public:
                                // Bind to a port on all interfaces
	                        CHttpServer( int nHTTPPort=80, bool bSecure = false );
                                // Bind to a port on a specific interface
	                        CHttpServer( const char *boundInterface, int nHTTPPort=80, bool bSecure = false );

	        virtual        ~CHttpServer( void );

                            DEFINE_OBJECT( CHttpServer )

            virtual void   *Run( void );

            virtual void    AddHandlerFactory( CHttpHandlerFactory *pFactory );            
            virtual void    RemoveHandlerFactory( CHttpHandlerFactory *pFactory );            

            virtual CHttpHandler   *GetHandler( CHttpRequest *request, CUUID erid );
    };
} // namespace IASLib

#endif // IASLIB_HTTPSERVER_H__

#endif // IASLIB_NETWORKING__
