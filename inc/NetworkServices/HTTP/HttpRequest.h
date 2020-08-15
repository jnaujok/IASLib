/**
 *  HTTP Request Class
 *
 *      This class provides an interface for generating an HTTP Request to
 * a remote server, or an incoming request to a server from a remote client.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: September 17, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPREQUEST_H__
#define IASLIB_HTTPREQUEST_H__

#include "HttpHeader.h"
#include "HttpHeaderList.h"
#include "NetworkServices/GenericRequest.h"

namespace IASLib
{
    class CHttpRequest : public CGenericRequest
    {
        public:
            static const char *METHOD_GET;
            static const char *METHOD_PUT;
            static const char *METHOD_HEAD;
            static const char *METHOD_POST;
            static const char *METHOD_OPTIONS;
            static const char *METHOD_TRACE;
            static const char *METHOD_DELETE;

        protected:
            CString         m_strRemoteHost;
            int             m_nPort;
            bool            m_bSecure;

        public:
                            CHttpRequest( CInternetAddress &internetAddress );
	                        CHttpRequest( const char *method, const char *uri );
	        virtual        ~CHttpRequest( void );

                            DEFINE_OBJECT( CHttpRequest )

                    // GET, PUT, HEAD, POST, OPTIONS, TRACE, DELETE
            virtual void    setRequestType( const char *requestType );
            virtual CString getRequestType( void );

            CString         getTransferProtocol( void ) { return CString("HTTP/1.1"); }

            virtual CString toString( void );
            virtual void    toStream( CStream *pStream );

            virtual CHeaderList *getHeaderList( CStream &requestStream ) { return new CHttpHeaderList( requestStream ); }
    };
} // namespace IASLib

#endif // IASLIB_HTTPCLIENT_H__

#endif // IASLIB_NETWORKING__
