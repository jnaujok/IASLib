/**
 *  Http Client Class
 *
 *      This class provides an interface for connecting
 * to an Http server and requesting data from that server
 * using any of the 7 methods defined in the Http standard.
 * This class uses synchronous calls, so it is best to use
 * it in a multi-threaded environment.
 *      Since this class is derived directly from the client
 * socket class, it is also possible to use this class to do
 * direct I/O to the the Http connection. This is not
 * recommended as the Http protocol is very specific as to 
 * the order and formatting of data.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/16/2006
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPCLIENT_H__
#define IASLIB_HTTPCLIENT_H__

#include "NetworkServices/GenericClient.h"
#include "NetworkServices/HTTP/HttpRequest.h"
#include "NetworkServices/HTTP/HttpResponse.h"

namespace IASLib
{
    class CHttpClient : public CGenericClient  
    {
        protected:
            bool                        m_bUseHttp11;
            bool                        m_bUseKeepalive;
        public:
	                                    CHttpClient( const char *strRemoteHost, int nHttpPort=80, bool bSecure = false );
	        virtual                    ~CHttpClient( void );

                                        DEFINE_OBJECT( CHttpClient )

            virtual CGenericResponse   *executeRequest( CGenericRequest *request );
            virtual CHttpResponse      *executeRequest( CHttpRequest *request );
    };
} // namespace IASLib

#endif // IASLIB_HTTPCLIENT_H__

#endif // IASLIB_NETWORKING__
