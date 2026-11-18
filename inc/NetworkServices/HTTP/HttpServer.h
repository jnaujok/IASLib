/**
 * @file HttpServer.h
 * @brief The protocol core of an HTTP/1.1 origin server (CHttpServer).
 *
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
 *      Virtual hosts: AddVirtualHost( "www.example.com", factory ) gives a
 * host name its own factories. A request is for the host in its Host header
 * (or the authority of an absolute-form target); a request for a virtual
 * host uses only that host's factories, and any other request uses the
 * server's own. "*.example.com" matches any one label in its place; exact
 * names are tried first; names ignore case and port.
 *
 *      Over TLS, a request whose host is not the one the client asked for in
 * the TLS handshake (SNI) gets 421 Misdirected Request (RFC 9110 15.5.20)
 * and the connection is closed, so one site's certificate cannot be used
 * to reach another. Clients that asked for no name are not checked.
 * setRequireMatchingHost( false ) turns the check off.
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

#include <string>
#include <utility>
#include <vector>

namespace IASLib
{
    /// The protocol core of an HTTP/1.1 origin server (RFC 9110, RFC 9112).
    ///
    /// It owns no sockets and its Run() does nothing: a concrete server (see
    /// CBasicHttpServer) accepts connections and runs a CHttpListener on each,
    /// which frames the requests and passes each one to handleRequest(). That
    /// checks the request, finds a handler from the registered factories, and
    /// returns the serialized response with the standard headers.
    ///
    /// The server owns its handler factories, including those of virtual
    /// hosts. Add factories and virtual hosts and change the settings before
    /// the server starts; handleRequest() may then be called from several
    /// listener threads at once.
    /// @code
    /// CBasicHttpServer oServer( 8080 );
    /// oServer.AddHandlerFactory( new CMyHandlerFactory() );
    /// oServer.AddVirtualHost( "*.example.com", new CExampleHandlerFactory() );
    /// oServer.setServerName( "MyApp/1.0" );
    /// oServer.Start();
    /// @endcode
    class CHttpServer : public CGenericServer
    {
        protected:
            bool            m_bUseHTTP11;           ///< Set true and not used by the core.
            bool            m_bUseKeepalive;        ///< Whether persistent connections are allowed (true by default).
            bool            m_bSecure;              ///< Whether the server is meant to serve TLS.
            int             m_nPort;                ///< The port for a derived server to bind.
            CString         m_strBindAddress;       ///< The interface for a derived server to bind, or "".
            CArray          m_aHandlerFactories;    ///< The server's own CHttpHandlerFactory objects (owned), newest first.
            CString         m_strServerName;        ///< The Server header value ("IASLib/2.0" by default; "" for none).

            size_t          m_nMaxHeaderSize;       ///< The largest request header section (64 KB by default).
            size_t          m_nMaxBodySize;         ///< The largest request body (10 MB by default).
            int             m_nKeepAliveTimeoutMs;  ///< The idle time allowed between requests (30 s by default).
            int             m_nRequestTimeoutMs;    ///< The time allowed to receive a whole request (30 s by default).
            bool            m_bRequireMatchingHost; ///< Whether a TLS request's host must match the SNI name (true by default).
                /// The virtual hosts: a normalized name or "*.domain" pattern, and
                /// its factories (owned), newest first.
            std::vector< std::pair< std::string, std::vector<CHttpHandlerFactory *> > > m_aVirtualHosts;

        public:
                /// Creates the server core.
                ///
                /// The port and TLS flag are kept for derived servers that bind
                /// sockets; the core itself does not use them.
                /// @param nHTTPPort The port to serve.
                /// @param bSecure   Whether to serve TLS.
	                        CHttpServer( int nHTTPPort=80, bool bSecure = false );
	                        /// Same, also keeping the interface to bind to.
	                        /// @param boundInterface The local address to bind; NULL or "" for all.
	                        /// @param nHTTPPort      The port to serve.
	                        /// @param bSecure        Whether to serve TLS.
	                        CHttpServer( const char *boundInterface, int nHTTPPort=80, bool bSecure = false );

	            /// Deletes all handler factories, the virtual hosts' included.
	        virtual        ~CHttpServer( void );

                            DEFINE_OBJECT( CHttpServer )

                /// The base server does nothing when run and returns NULL.
                /// Concrete servers override this to accept connections.
            virtual void   *Run( void );

                /// Adds a handler factory for requests that are not for a
                /// virtual host.
                ///
                /// Factories are consulted newest first. The server takes
                /// ownership. NULL is ignored.
            virtual void    AddHandlerFactory( CHttpHandlerFactory *pFactory );
                /// Removes the factory and returns ownership of it to the caller.
                /// Does nothing if the factory was not added.
            virtual void    RemoveHandlerFactory( CHttpHandlerFactory *pFactory );
                /// Adds a handler factory for one host name.
                ///
                /// A request is for the host in its Host header (or the
                /// authority of an absolute-form target); a request for a
                /// virtual host uses only that host's factories, and any other
                /// request uses the server's own. "*.example.com" matches any
                /// one label in its place; exact names are tried first; names
                /// ignore case and port. Factories of a host are consulted
                /// newest first.
                ///
                /// Set up virtual hosts before the server starts. The server
                /// takes ownership; a NULL factory or an empty name is ignored
                /// (and the factory deleted).
            virtual void    AddVirtualHost( const char *pszHostName, CHttpHandlerFactory *pFactory );
                /// Removes a host, deleting its factories.
                /// @return false if there is no such host.
            virtual bool    RemoveVirtualHost( const char *pszHostName );
                /// The virtual host name (normalized, as stored) a host name
                /// falls under, or "" for the server's own factories.
            virtual CString findVirtualHost( const CString &strHostName );

                /// Sets whether an HTTPS request's host must be the one asked for
                /// in the TLS handshake (on by default).
                ///
                /// When on, a mismatch gets 421 Misdirected Request (RFC 9110
                /// 15.5.20) and the connection is closed, so one site's
                /// certificate cannot be used to reach another. Clients that
                /// asked for no name are not checked.
            virtual void    setRequireMatchingHost( bool bRequire ) { m_bRequireMatchingHost = bRequire; }
                /// Whether an HTTPS request's host must match the TLS server name.
            virtual bool    getRequireMatchingHost( void ) { return m_bRequireMatchingHost; }

                /// Returns a new handler for the request's method, or NULL.
                ///
                /// Same as the overload below, with the request's own method.
                /// The caller deletes the handler.
            virtual CHttpHandler   *GetHandler( CHttpRequest *request, CUUID erid );
                /// Returns a new handler for the method, or NULL.
                ///
                /// The factories that serve the request's host are asked in
                /// turn; the first handler offered is given this server and
                /// returned. The caller deletes it.
                /// @param method  The method to find a handler for.
                /// @param request The request (selects the virtual host).
                /// @param erid    The request's id, passed to the factory.
            virtual CHttpHandler   *GetHandler( const CString &method, CHttpRequest *request, CUUID erid );

                /// The methods of the server's own factories, for an Allow
                /// header ("GET, POST, HEAD"). HEAD is listed wherever GET is.
            virtual CString getAllowedList( void );
                /// The same, for the factories that serve this request's host.
            virtual CString getAllowedList( CHttpRequest *request );

                /// Processes one framed request (see CHttpUtils::frameRequest())
                /// and returns the serialized response.
                ///
                /// Malformed requests get 400, as does an HTTP/1.1 request
                /// without exactly one Host header; a major version other than
                /// 1 gets 505 and an Expect other than 100-continue 417. The
                /// method's handler is found, using the GET handler for HEAD
                /// when there is no HEAD handler; with none the answer is 405
                /// (known method, with Allow) or 501 (unknown method). "OPTIONS
                /// *" is answered here. A handler that fails, throws or sets an
                /// impossible status gives 500. Date, Server, Content-Length and
                /// Connection headers are added. Exceptions do not escape.
                /// @param message   The framed request.
                /// @param source    The client's address.
                /// @param keepAlive Set to whether the connection may be used for
                ///                  another request (HTTP/1.1 persistent by
                ///                  default, HTTP/1.0 only with "Connection:
                ///                  keep-alive", never after "close").
                /// @return The whole response to send.
            virtual CString handleRequest( const CString &message, const CInternetAddress &source, bool &keepAlive );
                /// The same for a request read from a connection.
                /// @param message          The framed request.
                /// @param source           The client's address.
                /// @param keepAlive        Set to whether the connection may be reused.
                /// @param bSecure          Whether the connection uses TLS.
                /// @param strTlsServerName The host name asked for in the TLS
                ///                         handshake ("" if none).
                /// @return The whole response to send.
            virtual CString handleRequest( const CString &message, const CInternetAddress &source, bool &keepAlive,
                                           bool bSecure, const CString &strTlsServerName );

                /// A complete response for a request that could not be framed or
                /// timed out, with the standard reason phrase and headers. It
                /// always closes the connection.
            virtual CString errorResponse( int nStatus );

                /// Sets the value of the Server header, or "" for none.
            virtual void    setServerName( const char *name ) { m_strServerName = name; }
                /// The value of the Server header ("IASLib/2.0" by default).
            virtual CString getServerName( void ) { return m_strServerName; }

                /// Sets whether persistent connections are allowed (on by default).
            virtual void    setKeepAlive( bool bKeepAlive ) { m_bUseKeepalive = bKeepAlive; }

                /// Sets the largest request header section listeners accept
                /// (default 64 KB); a larger one gets 431.
            virtual void    setMaxHeaderSize( size_t nBytes ) { m_nMaxHeaderSize = nBytes; }
                /// The largest request header section accepted, in bytes.
            virtual size_t  getMaxHeaderSize( void ) { return m_nMaxHeaderSize; }
                /// Sets the largest request body listeners accept (default
                /// 10 MB); a larger one gets 413.
            virtual void    setMaxBodySize( size_t nBytes ) { m_nMaxBodySize = nBytes; }
                /// The largest request body accepted, in bytes.
            virtual size_t  getMaxBodySize( void ) { return m_nMaxBodySize; }
                /// Sets how long a listener waits between requests on an idle
                /// connection, in milliseconds (default 30 s).
            virtual void    setKeepAliveTimeout( int nMilliseconds ) { m_nKeepAliveTimeoutMs = nMilliseconds; }
                /// The idle time allowed between requests, in milliseconds.
            virtual int     getKeepAliveTimeout( void ) { return m_nKeepAliveTimeoutMs; }
                /// Sets how long a listener waits for a whole request once it
                /// has started, in milliseconds (default 30 s).
            virtual void    setRequestTimeout( int nMilliseconds ) { m_nRequestTimeoutMs = nMilliseconds; }
                /// The time allowed to receive a whole request, in milliseconds.
            virtual int     getRequestTimeout( void ) { return m_nRequestTimeoutMs; }

                /// Whether the server was created to serve TLS.
            virtual bool    isSecure( void ) { return m_bSecure; }
                /// The port given at construction.
            virtual int     getPort( void ) { return m_nPort; }
                /// The interface given at construction, or "".
            virtual CString getBindAddress( void ) { return m_strBindAddress; }

        protected:
                /// Fills in the response to a parsed request; see handleRequest().
                /// @param request   The request.
                /// @param parseOk   Whether the request parsed (400 if not).
                /// @param response  The response to fill.
                /// @param keepAlive Set to whether the connection may be reused.
            virtual void    processRequest( CHttpRequest *request, bool parseOk, CHttpResponse *response, bool &keepAlive );

                /// Adds Date, Server (unless already set) and Connection
                /// ("close", or "keep-alive" for HTTP/1.0) to a response.
            virtual void    addStandardHeaders( CHttpResponse *response, bool keepAlive, bool http10 );

        private:
                /// Sets the defaults; shared by the constructors.
            void            init( void );
                /// The factories that serve a request: its virtual host's, or
                /// (for other hosts, and with no request) the server's own.
            std::vector<CHttpHandlerFactory *> factoriesFor( CHttpRequest *request );
                /// The Allow list for a set of factories.
            CString         allowedFrom( const std::vector<CHttpHandlerFactory *> &factories );
    };
} // namespace IASLib

#endif // IASLIB_HTTPSERVER_H__

#endif // IASLIB_NETWORKING__
