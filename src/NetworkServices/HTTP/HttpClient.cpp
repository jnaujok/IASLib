/**
 *  HTTP Client Class
 *
 *      This class provides an interface for connecting
 * to an HTTP server and requesting data from that server
 * using any of the 7 methods defined in the HTTP standard.
 * This class uses synchronous calls, so it is best to use
 * it in a multi-threaded environment.
 *      Since this class is derived directly from the client
 * socket class, it is also possible to use this class to do
 * direct I/O to the the HTTP connection. This is not
 * recommended as the HTTP protocol is very specific as to 
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

#include "HttpClient.h"

namespace IASLib
{
    IMPLEMENT_OBJECT(CHttpClient, CGenericClient);

    CHttpClient::CHttpClient( const char *strRemoteHost, int nHTTPPort, bool useSSL )
        : CGenericClient( strRemoteHost, nHTTPPort, useSSL )
    {
        
    }

    CHttpClient::~CHttpClient()
    {
    }

    CGenericResponse *CHttpClient::executeRequest( CGenericRequest *request )
    {
        return executeRequest( (CHttpRequest *)request );
    }   

    CHttpResponse *CHttpClient::executeRequest( CHttpRequest *request )
    {
        CHttpResponse *retVal = new CHttpResponse();

        // Make the actual HTTP Call
        CString strRequest = request->toString();
        

        return retVal;
    }   

} // namespace IASLib

#endif
