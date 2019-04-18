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

#ifndef IASLIB_HTTPCLIENT_H__
#define IASLIB_HTTPCLIENT_H__

#ifdef IASLIB_NETWORKING__

#include "../Sockets/ClientSocket.h"
#include "HTTPHeader.h"
#include "HTTPHeaderList.h"

namespace IASLib
{
    class CHTTPClient : public CClientSocket  
    {
        protected:
            bool            m_bUseHTTP11;
            bool            m_bUseKeepalive;
            bool            m_bConnected;
            CHTTPHeaderList m_hlHeaders;
            int             m_nLastResponseCode;
        public:
	                        CHTTPClient( const char *strRemoteHost, int nHTTPPort=80 );
	        virtual        ~CHTTPClient( void );

                            DEFINE_OBJECT( CHTTPClient )

            void            AddHeader( const char *strName, const char *strValue );
            CHTTPHeader    *GetHeader( const char *strName );

            bool            Get( const char *strURI );
            bool            Put( const char *strURI, void *pData, size_t nLength );
            bool            Head( const char *strURI );
            bool            Post( const char *strURI, void *pData, size_t nLength );
            bool            Options( const char *strURI );
            bool            Trace( const char *strURI, void *pData, size_t nLength );
            bool            Delete( const char *strURI );
        private:
            CString         BuildRequest( const char *strType, const char *strURI, void *pData = NULL, size_t nLength = 0 );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_HTTPCLIENT_H__
