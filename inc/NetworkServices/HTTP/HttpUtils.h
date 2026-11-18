/**
 * @file HttpUtils.h
 * @brief HTTP/1.1 message framing, reason phrases and dates (CHttpUtils, CHttpFrame).
 *
 * HTTP Utilities
 *
 * Helpers for the HTTP/1.1 server and client: framing requests and
 * responses on a connection (RFC 9112 section 6), status reason phrases and
 * the HTTP date format.
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPUTILS_H__
#define IASLIB_HTTPUTILS_H__

#include "BaseTypes/String_.h"

#include <string>

namespace IASLib
{
        /// The result of looking for one request (or response) at the front of a buffer.
        ///
        /// Returned by CHttpUtils::frameRequest() and CHttpUtils::frameResponse().
        /// Which fields are set depends on status, as each field says.
    struct CHttpFrame
    {
            /// How far framing got.
        enum Status {
            NEED_MORE,  ///< The buffer does not hold a whole message yet; read more and try again.
            COMPLETE,   ///< A whole message was found; see consumed and message.
            FAILED      ///< The message is not acceptable; answer with errorStatus.
        };

        Status          status;             ///< How far framing got.
            /// For FAILED: the status code to answer with (400, 413, 431, 501
            /// for requests; 502, 413, 431 for responses).
        int             errorStatus;
            /// For COMPLETE: the bytes of the buffer the message used.
        size_t          consumed;
            /// For COMPLETE: the message with its body de-chunked, a
            /// Content-Length header in place of Transfer-Encoding, and no
            /// leading blank lines.
        std::string     message;
            /// The headers are complete and ask for "Expect: 100-continue".
        bool            expectContinue;
            /// The headers are complete (the body may not be).
        bool            headersComplete;
            /// Responses: the status code.
        int             statusCode;
            /// Responses: whether the connection may carry another request
            /// afterwards (no "Connection: close", HTTP/1.1 or "keep-alive",
            /// and a body not delimited by the close).
        bool            keepAlive;
            /// Responses: whether the body ran to the end of the connection.
        bool            closeDelimited;

            /// A NEED_MORE result with every other field zero or false.
        CHttpFrame( void ) : status( NEED_MORE ), errorStatus( 0 ), consumed( 0 ), expectContinue( false ), headersComplete( false ),
                             statusCode( 0 ), keepAlive( false ), closeDelimited( false ) {}
    };

    /// Static helpers for HTTP/1.1 message framing (RFC 9112 section 6),
    /// reason phrases and dates.
    ///
    /// The framing functions take the bytes read from a connection so far and
    /// say whether they hold a whole message. They are strict where a lenient
    /// reading would let two parsers disagree on where a message ends (request
    /// smuggling). All functions are stateless and thread-safe.
    /// @code
    /// CHttpFrame frame = CHttpUtils::frameRequest( buffer.data(), buffer.size(), 16384, 1048576 );
    /// if ( frame.status == CHttpFrame::COMPLETE )
    /// {
    ///     oRequest.parse( CString::FromBytes( frame.message.data(), frame.message.size() ) );
    ///     buffer.erase( 0, frame.consumed );
    /// }
    /// @endcode
    class CHttpUtils
    {
        public:
                /// Frames the first request in the buffer.
                ///
                /// Empty lines before the request line are skipped. A request
                /// with neither Content-Length nor Transfer-Encoding has no
                /// body. The framed message always carries a Content-Length.
                ///
                /// A request with both Transfer-Encoding and Content-Length,
                /// conflicting Content-Length values or an invalid chunked body
                /// fails with 400 (these are request-smuggling vectors), as does
                /// a header section holding NUL or a CR that does not end a
                /// line; a transfer coding other than chunked fails with 501;
                /// headers longer than nMaxHeader fail with 431, and bodies
                /// longer than nMaxBody with 413.
                /// @param pBuffer    The bytes received so far.
                /// @param nLength    The number of bytes in pBuffer.
                /// @param nMaxHeader The largest header section accepted, in bytes.
                /// @param nMaxBody   The largest body accepted, in bytes.
                /// @note NUL and lone CRs used to be passed on, so the request
                ///       was parsed with its text cut at the NUL, and a lone CR
                ///       could hide a header from these checks.
            static CHttpFrame frameRequest( const char *pBuffer, size_t nLength, size_t nMaxHeader, size_t nMaxBody );

                /// Frames the first response in the buffer (RFC 9112 6.3).
                ///
                ///  - a response to HEAD (bHeadRequest), a 1xx, 204 or 304 has no
                ///    body;
                ///  - a chunked body is decoded, and reported with a
                ///    Content-Length in place of Transfer-Encoding;
                ///  - a body with a Content-Length is that many bytes;
                ///  - otherwise the body runs until the server closes the
                ///    connection, so it is COMPLETE only once bClosed is true.
                ///
                /// message holds the status line, headers, blank line and body.
                /// A header section holding NUL or a lone CR, a malformed status
                /// line or chunk, conflicting Content-Lengths, a transfer coding
                /// other than chunked or a connection closed early fails with
                /// 502; headers over nMaxHeader with
                /// 431, and a body over nMaxBody with 413.
                /// @param pBuffer      The bytes received so far.
                /// @param nLength      The number of bytes in pBuffer.
                /// @param bHeadRequest Whether the request was HEAD.
                /// @param bClosed      Whether the server has closed the connection.
                /// @param nMaxHeader   The largest header section accepted, in bytes.
                /// @param nMaxBody     The largest body accepted, in bytes.
            static CHttpFrame frameResponse( const char *pBuffer, size_t nLength, bool bHeadRequest, bool bClosed,
                                             size_t nMaxHeader, size_t nMaxBody );

                /// The standard reason phrase for a status code.
                ///
                /// An unlisted code gets the name of its class, such as
                /// "Client Error". Never NULL.
            static const char *reasonPhrase( int nStatusCode );

                /// The current time as an HTTP date (RFC 9110 5.6.7), in GMT,
                /// for example "Wed, 18 Nov 2026 09:30:00 GMT".
            static CString httpDate( void );

                /// True for the methods defined by RFC 9110 (and PATCH), used to
                /// choose between 405 and 501. The match is case-sensitive.
            static bool isKnownMethod( const CString &method );
    };
} // namespace IASLib

#endif // IASLIB_HTTPUTILS_H__

#endif // IASLIB_NETWORKING__
