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

#ifndef IASLIB_HTTPSERVER_H__
#define IASLIB_HTTPSERVER_H__

#ifdef IASLIB_NETWORKING__

#include "../Sockets/ServerSocket.h"
#include "Server.h"
#include "HTTPHeader.h"
#include "HTTPHeaderList.h"

namespace IASLib
{
    class CHTTPServer : public CServer  
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
