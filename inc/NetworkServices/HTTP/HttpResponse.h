/**
 *  HTTP Response Class
 *
 *      This class provides an interface for retrieving the data from
 * an HTTP request. The response is used by the HTTP Client to return
 * data from the response to the calling program.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 9/07/2006
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPRESPONSE_H__
#define IASLIB_HTTPRESPONSE_H__

#include "Streams/Stream.h"
#include "HttpHeaderList.h"
#include "NetworkServices/GenericResponse.h"

namespace IASLib
{
    class CHttpResponse : public CGenericResponse
    {
        protected:
            CInternetAddress    m_remoteAddress;
        public:
                                DEFINE_OBJECT( CHttpResponse );

                                CHttpResponse( void );
                                CHttpResponse( CStream &oStream, CInternetAddress &internetAddress );

            virtual            ~CHttpResponse( void );

            virtual CString     toString( void );
            virtual void        toStream( CStream *oStream );
    };
} // namespace IASLib

#endif // IASLIB_HTTPRESPONSE_H__

#endif // IASLIB_NETWORKING__
