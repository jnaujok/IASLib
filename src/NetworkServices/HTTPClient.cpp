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

#include "HTTPClient.h"

namespace IASLib
{
    IMPLEMENT_OBJECT(CHTTPClient,CClientSocket);

    CHTTPClient::CHTTPClient( const char *strRemoteHost, int nHTTPPort )
        : CClientSocket( strRemoteHost, nHTTPPort )
    {
        
    }

    CHTTPClient::~CHTTPClient()
    {
    }

    CString CHTTPClient::BuildRequest( const char *strType, const char *strURI, void *pData, size_t nLength )
    {
        CString strRequest;
        CString strURL = strURI;

        strURL.Escape();

        strRequest = strType;
        strRequest += " ";
        strRequest += strURL;
        strRequest += " ";

        if ( m_bUseHTTP11 )
        {
            strRequest += "HTTP/1.1";
        }
        else
        {
            strRequest += "HTTP/1.0";
        }

        strRequest += "\r\n\r\n";

            // Need to do encoding or intelligent insert for non-strings.
        if ( nLength > 0 )
        {
            strRequest += (const char *)pData;
        }

        return strRequest;
    }
        
} // namespace IASLib

#endif
