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

        public:
                                // An incoming request; fill it with parse().
                            CHttpRequest( const CInternetAddress &internetAddress );
                                // An outgoing request.
	                        CHttpRequest( const char *method, const char *uri );
	        virtual        ~CHttpRequest( void );

                            DEFINE_OBJECT( CHttpRequest )

            CString         getTransferProtocol( void ) { return getVersion(); }

                // The path of the request target, without the query string. For
                // an absolute-form target ("http://host/path") the scheme and
                // authority are removed.
            virtual CString getPath( void );
                // The query string (after '?'), or "".
            virtual CString getQuery( void );

            virtual CString toString( void );
            virtual void    toStream( CStream *pStream );

            virtual CHeaderList *getHeaderList( CStream &requestStream ) { return new CHttpHeaderList( requestStream ); }

        protected:
            virtual CHeaderList *newHeaderList( void ) { return new CHttpHeaderList(); }
    };
} // namespace IASLib

#endif // IASLIB_HTTPREQUEST_H__

#endif // IASLIB_NETWORKING__
