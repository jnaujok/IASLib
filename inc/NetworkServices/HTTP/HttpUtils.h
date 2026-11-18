/**
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
        // The result of looking for one request at the front of a buffer.
    struct CHttpFrame
    {
        enum Status { NEED_MORE, COMPLETE, FAILED };

        Status          status;
            // For FAILED: the status code to answer with (400, 413, 431, 501).
        int             errorStatus;
            // For COMPLETE: the bytes of the buffer the request used.
        size_t          consumed;
            // For COMPLETE: the request with its body de-chunked, a
            // Content-Length header in place of Transfer-Encoding, and no
            // leading blank lines.
        std::string     message;
            // The headers are complete and ask for "Expect: 100-continue".
        bool            expectContinue;
            // The headers are complete (the body may not be).
        bool            headersComplete;
            // Responses: the status code, whether the connection may carry
            // another request afterwards (no "Connection: close", HTTP/1.1 or
            // "keep-alive", and a body not delimited by the close), and
            // whether the body ran to the end of the connection.
        int             statusCode;
        bool            keepAlive;
        bool            closeDelimited;

        CHttpFrame( void ) : status( NEED_MORE ), errorStatus( 0 ), consumed( 0 ), expectContinue( false ), headersComplete( false ),
                             statusCode( 0 ), keepAlive( false ), closeDelimited( false ) {}
    };

    class CHttpUtils
    {
        public:
                // Frames the first request in the buffer. A request with both
                // Transfer-Encoding and Content-Length, conflicting
                // Content-Length values or an invalid chunked body fails with
                // 400 (these are request-smuggling vectors), as does a header
                // section holding NUL or a CR that does not end a line; a
                // transfer coding other than chunked fails with 501; headers
                // longer than nMaxHeader fail with 431, and bodies longer than
                // nMaxBody with 413.
            static CHttpFrame frameRequest( const char *pBuffer, size_t nLength, size_t nMaxHeader, size_t nMaxBody );

                // Frames the first response in the buffer (RFC 9112 6.3):
                //  - a response to HEAD (bHeadRequest), a 1xx, 204 or 304 has no
                //    body;
                //  - a chunked body is decoded, and reported with a
                //    Content-Length in place of Transfer-Encoding;
                //  - a body with a Content-Length is that many bytes;
                //  - otherwise the body runs until the server closes the
                //    connection, so it is COMPLETE only once bClosed is true.
                // message holds the status line, headers, blank line and body.
                // A header section holding NUL or a lone CR, a malformed status
                // line or chunk, conflicting Content-Lengths or a connection
                // closed early fails with 502; headers over nMaxHeader with
                // 431, and a body over nMaxBody with 413.
            static CHttpFrame frameResponse( const char *pBuffer, size_t nLength, bool bHeadRequest, bool bClosed,
                                             size_t nMaxHeader, size_t nMaxBody );

                // The standard reason phrase for a status code.
            static const char *reasonPhrase( int nStatusCode );

                // The current time as an HTTP date (RFC 9110 5.6.7), in GMT.
            static CString httpDate( void );

                // True for the methods defined by RFC 9110 (and PATCH), used to
                // choose between 405 and 501.
            static bool isKnownMethod( const CString &method );
    };
} // namespace IASLib

#endif // IASLIB_HTTPUTILS_H__

#endif // IASLIB_NETWORKING__
