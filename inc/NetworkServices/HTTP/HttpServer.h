/**
 *  HTTP Server Class
 *
 *      This class is the protocol core of an HTTP/1.1 origin server
 * (RFC 9110 and RFC 9112). It owns no sockets and its Run() method is an
 * empty stub: a concrete server (see CBasicHttpServer) accepts connections
 * and runs a CHttpListener on each, which frames the requests and passes
 * each one to handleRequest().
 *
 *      For each request the core:
 *        - answers malformed requests with 400, an HTTP/1.1 request without
 *          exactly one Host header with 400, a major version other than 1
 *          with 505, and an Expect other than 100-continue with 417;
 *        - finds the method's handler, using the GET handler for HEAD when
 *          there is no HEAD handler, and answers 405 (known method, with
 *          Allow) or 501 (unknown method) when there is none; "OPTIONS *"
 *          is answered by the core;
 *        - adds Date, Server, Content-Length and Connection headers;
 *        - decides whether the connection stays open (HTTP/1.1 persistent
 *          by default, HTTP/1.0 only with "Connection: keep-alive").
 *
 *      Handlers come from the handler factories added with
 * AddHandlerFactory(). The server owns its factories.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: September 24, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
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
            int             m_nPort;
            CString         m_strBindAddress;
            CArray          m_aHandlerFactories;
            CString         m_strServerName;

            size_t          m_nMaxHeaderSize;
            size_t          m_nMaxBodySize;
            int             m_nKeepAliveTimeoutMs;
            int             m_nRequestTimeoutMs;

        public:
                // Port and interface are kept for derived servers that bind
                // sockets; the core itself does not use them.
	                        CHttpServer( int nHTTPPort=80, bool bSecure = false );
	                        CHttpServer( const char *boundInterface, int nHTTPPort=80, bool bSecure = false );

	        virtual        ~CHttpServer( void );

                            DEFINE_OBJECT( CHttpServer )

                // The base server does nothing when run. Concrete servers
                // override this to accept connections.
            virtual void   *Run( void );

                // Factories are consulted newest first. The server takes ownership.
            virtual void    AddHandlerFactory( CHttpHandlerFactory *pFactory );
                // Removes the factory and returns ownership of it to the caller.
            virtual void    RemoveHandlerFactory( CHttpHandlerFactory *pFactory );
                // Returns a new handler for the request's method, or NULL.
            virtual CHttpHandler   *GetHandler( CHttpRequest *request, CUUID erid );
            virtual CHttpHandler   *GetHandler( const CString &method, CHttpRequest *request, CUUID erid );

                // The methods of all factories, for an Allow header.
            virtual CString getAllowedList( void );

                // Processes one framed request (see CHttpUtils::frameRequest) and
                // returns the serialized response. keepAlive is set to whether
                // the connection may be used for another request. Thread-safe.
            virtual CString handleRequest( const CString &message, const CInternetAddress &source, bool &keepAlive );

                // A complete response for a request that could not be framed or
                // timed out; it always closes the connection.
            virtual CString errorResponse( int nStatus );

                // The value of the Server header, or "" for none.
            virtual void    setServerName( const char *name ) { m_strServerName = name; }
            virtual CString getServerName( void ) { return m_strServerName; }

                // Whether persistent connections are allowed (on by default).
            virtual void    setKeepAlive( bool bKeepAlive ) { m_bUseKeepalive = bKeepAlive; }

                // Limits used by listeners: header section (default 64 KB),
                // body (default 10 MB), idle time between requests (default
                // 30 s) and time to receive a whole request (default 30 s).
            virtual void    setMaxHeaderSize( size_t nBytes ) { m_nMaxHeaderSize = nBytes; }
            virtual size_t  getMaxHeaderSize( void ) { return m_nMaxHeaderSize; }
            virtual void    setMaxBodySize( size_t nBytes ) { m_nMaxBodySize = nBytes; }
            virtual size_t  getMaxBodySize( void ) { return m_nMaxBodySize; }
            virtual void    setKeepAliveTimeout( int nMilliseconds ) { m_nKeepAliveTimeoutMs = nMilliseconds; }
            virtual int     getKeepAliveTimeout( void ) { return m_nKeepAliveTimeoutMs; }
            virtual void    setRequestTimeout( int nMilliseconds ) { m_nRequestTimeoutMs = nMilliseconds; }
            virtual int     getRequestTimeout( void ) { return m_nRequestTimeoutMs; }

            virtual bool    isSecure( void ) { return m_bSecure; }
            virtual int     getPort( void ) { return m_nPort; }
            virtual CString getBindAddress( void ) { return m_strBindAddress; }

        protected:
                // Fills in the response to a parsed request.
            virtual void    processRequest( CHttpRequest *request, bool parseOk, CHttpResponse *response, bool &keepAlive );

                // Adds Date, Server and Connection to every response.
            virtual void    addStandardHeaders( CHttpResponse *response, bool keepAlive, bool http10 );

        private:
            void            init( void );
    };
} // namespace IASLib

#endif // IASLIB_HTTPSERVER_H__

#endif // IASLIB_NETWORKING__
