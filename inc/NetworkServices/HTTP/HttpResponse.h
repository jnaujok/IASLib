/**
 * @file HttpResponse.h
 * @brief An HTTP/1.1 response, sent or received (CHttpResponse).
 *
 *  HTTP Response Class
 *
 *      An HTTP/1.1 response: status, headers and body. When serialized,
 * Content-Length is set from the body, except for 1xx, 204 and 304
 * responses (which have no body), and a response to HEAD keeps its
 * Content-Length but omits the body (RFC 9110 9.3.2).
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
    /// An HTTP/1.1 response: status, headers and body.
    ///
    /// A server handler fills one and the server writes it; CHttpClient
    /// returns one filled by parse(). When written, Content-Length is set from
    /// the body, except for 1xx, 204 and 304 responses (which have no body),
    /// and a response to HEAD keeps its Content-Length but omits the body
    /// (RFC 9110 9.3.2). The status line always says HTTP/1.1.
    /// @code
    /// CHttpResponse oResponse;
    /// oResponse.setStatus( 200 );                       // "200 OK"
    /// oResponse.addHeader( "Content-Type", "text/plain" );
    /// oResponse.SetEntity( new CTextPlainEntity( "hello" ) );
    /// CString strWire = oResponse.toString();
    /// @endcode
    class CHttpResponse : public CGenericResponse
    {
        protected:
            CInternetAddress    m_remoteAddress;    ///< The peer the response is for.
            bool                m_bHeadResponse;    ///< Whether this answers a HEAD request.
            CString             m_strVersion;       ///< The HTTP version of a received response, or "".

        public:
                                DEFINE_OBJECT( CHttpResponse );

                                /// Creates a response (500 until a status is set).
                                CHttpResponse( void );
                                /// Creates a response to be written to the stream for
                                /// the peer. The stream is not owned.
                                CHttpResponse( CStream &oStream, CInternetAddress &internetAddress );

                                /// Destroys the response and its body.
            virtual            ~CHttpResponse( void );

                /// Sets the status with its standard reason phrase (see
                /// CHttpUtils::reasonPhrase()).
            virtual void        setStatus( int nCode );
                /// Sets the status with the given reason phrase (see
                /// CGenericResponse::setStatus()).
            virtual void        setStatus( int nCode, const char *desc ) { CGenericResponse::setStatus( nCode, desc ); }

                /// Marks this as the response to a HEAD request: the body is not
                /// sent, but Content-Length still describes it.
            virtual void        setHeadResponse( bool bHead ) { m_bHeadResponse = bHead; }
                /// Whether this is the response to a HEAD request.
            virtual bool        isHeadResponse( void ) { return m_bHeadResponse; }

                /// The whole response as it is sent on the wire.
                ///
                /// This sets Content-Length (and Content-Type, from the body) in
                /// the stored headers, as CGenericResponse::serialize() does; for
                /// 1xx and 204 it removes Content-Length instead.
            virtual CString     toString( void );
                /// Writes toString() to the stream.
            virtual void        toStream( CStream *oStream );

                /// Fills this response from a received message.
                ///
                /// It reads the status line, the headers and, after the blank
                /// line, the body (all of the bytes that follow it). The message
                /// should already be framed, as CHttpUtils::frameResponse() does.
                /// Previous headers and body are discarded first.
                /// @return false, with whatever could be read kept, if the status
                ///         line or a header line is malformed or the header
                ///         section does not end.
            virtual bool        parse( const CString &message );
                /// The HTTP version of a received response ("HTTP/1.1"), or "".
            virtual CString     getVersion( void ) { return m_strVersion; }
    };
} // namespace IASLib

#endif // IASLIB_HTTPRESPONSE_H__

#endif // IASLIB_NETWORKING__
