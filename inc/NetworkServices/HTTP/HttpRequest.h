/**
 * @file HttpRequest.h
 * @brief An HTTP request, received or to be sent (CHttpRequest).
 *
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
    /// An HTTP/1.1 request: an incoming one read by a server, or an outgoing
    /// one built for CHttpClient.
    ///
    /// Parsing and the header and body accessors come from CGenericRequest.
    /// This class adds the HTTP view of the request target (path, query and
    /// host) and the details of the connection a request arrived on.
    /// @code
    /// CHttpRequest oRequest( CHttpRequest::METHOD_GET, "/status?verbose=1" );
    /// oRequest.setHeaderValue( "Host", "example.com" );
    /// oRequest.setHeaderValue( "Accept", "application/json" );
    /// CString strWire = oRequest.toString();
    /// @endcode
    class CHttpRequest : public CGenericRequest
    {
        public:
            static const char *METHOD_GET;      ///< "GET".
            static const char *METHOD_PUT;      ///< "PUT".
            static const char *METHOD_HEAD;     ///< "HEAD".
            static const char *METHOD_POST;     ///< "POST".
            static const char *METHOD_OPTIONS;  ///< "OPTIONS".
            static const char *METHOD_TRACE;    ///< "TRACE".
            static const char *METHOD_DELETE;   ///< "DELETE".

        public:
                                /// An incoming request from the peer; fill it with parse().
                            CHttpRequest( const CInternetAddress &internetAddress );
                                /// An outgoing HTTP/1.1 request.
                                ///
                                /// A method that is not a token or a URI that is
                                /// empty or holds spaces or control characters
                                /// leaves the request invalid (isValid() is false),
                                /// and CHttpClient will not send it.
                                /// @note Both used to be taken as they came, so a
                                ///       CR LF in the URI could add header lines.
	                        CHttpRequest( const char *method, const char *uri );
	                            /// Destroys the request.
	        virtual        ~CHttpRequest( void );

                            DEFINE_OBJECT( CHttpRequest )

                /// The protocol version, such as "HTTP/1.1" (the same as getVersion()).
            CString         getTransferProtocol( void ) { return getVersion(); }

                /// The path of the request target, without the query string.
                ///
                /// For an absolute-form target ("http://host/path") the scheme
                /// and authority are removed ("/" if there is no path).
            virtual CString getPath( void );
                /// The query string (after '?', without any '#' fragment), or "".
            virtual CString getQuery( void );

                /// The host the request is for, in lower case and without a
                /// port.
                ///
                /// This is the authority of an absolute-form target (without any
                /// user information), else the Host header (RFC 9112 3.2.2).
                /// @return The host name, normalised as normalizeHostName() does,
                ///         or "" if there is none.
            virtual CString getHostName( void );

                /// Records how the request arrived. Set by the server for
                /// requests read from a connection.
                /// @param bSecure          Whether the connection uses TLS.
                /// @param strTlsServerName The host name the client asked for in
                ///                         the TLS handshake ("" if none).
            void            setConnection( bool bSecure, const CString &strTlsServerName ) { m_bSecureConnection = bSecure; m_strTlsServerName = strTlsServerName; }
                /// Whether the request arrived over TLS.
            bool            isSecureConnection( void ) const { return m_bSecureConnection; }
                /// The host name the client sent in the TLS handshake, or "".
            CString         getTlsServerName( void ) const { return m_strTlsServerName; }

                /// A host name in the form getHostName() gives: trimmed, lower
                /// case, without a port or a trailing dot. An IPv6 literal keeps
                /// its brackets ("[::1]:443" gives "[::1]").
            static CString  normalizeHostName( const CString &strHost );

                /// The whole request as it is sent on the wire.
                ///
                /// When there is neither a Content-Length nor a
                /// Transfer-Encoding header, Content-Length is added for a body,
                /// and for a POST or PUT even when the body is empty (RFC 9110
                /// 8.6). The stored headers are not changed.
            virtual CString toString( void );
                /// Writes toString() to the stream.
            virtual void    toStream( CStream *pStream );

                /// Reads the header lines into a new CHttpHeaderList (caller owns it).
            virtual CHeaderList *getHeaderList( CStream &requestStream ) { return new CHttpHeaderList( requestStream ); }

        protected:
                /// Creates an empty CHttpHeaderList (caller owns it).
            virtual CHeaderList *newHeaderList( void ) { return new CHttpHeaderList(); }

            bool            m_bSecureConnection;    ///< Whether the request arrived over TLS.
            CString         m_strTlsServerName;     ///< The host name from the TLS handshake, or "".
    };
} // namespace IASLib

#endif // IASLIB_HTTPREQUEST_H__

#endif // IASLIB_NETWORKING__
