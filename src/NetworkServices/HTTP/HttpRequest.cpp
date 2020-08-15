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

#include "HttpRequest.h"

namespace IASLib
{
    const char *CHttpRequest::METHOD_GET = "GET";
    const char *CHttpRequest::METHOD_PUT = "PUT";
    const char *CHttpRequest::METHOD_HEAD = "HEAD";
    const char *CHttpRequest::METHOD_POST = "POST";
    const char *CHttpRequest::METHOD_OPTIONS = "OPTIONS";
    const char *CHttpRequest::METHOD_TRACE = "TRACE";
    const char *CHttpRequest::METHOD_DELETE = "DELETE";

    CHttpRequest::CHttpRequest( CInternetAddress &internetAddress ) : CGenericRequest( internetAddress )
    {
        
    }

    CHttpRequest::CHttpRequest( const char *strMethod, const char *uri )
    {
        
    }

    CHttpRequest::~CHttpRequest( void )
    {

    }

    IMPLEMENT_OBJECT( CHttpRequest, CGenericRequest )

                    // GET, PUT, HEAD, POST, OPTIONS, TRACE, DELETE
    void CHttpRequest::setRequestType( const char *requestType )
    {

    }

    CString CHttpRequest::getRequestType( void )
    {

    }

    CString CHttpRequest::toString( void )
    {

    }

    void CHttpRequest::toStream( CStream *pStream )
    {

    } 

}; // namespace IASLib

#endif // IASLIB_NETWORKING__
