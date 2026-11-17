/**
 * HTTP Utilities
 *
 * Helpers for the HTTP/1.1 server: framing requests on a connection
 * (RFC 9112 section 6), status reason phrases and the HTTP date format.
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

        CHttpFrame( void ) : status( NEED_MORE ), errorStatus( 0 ), consumed( 0 ), expectContinue( false ), headersComplete( false ) {}
    };

    class CHttpUtils
    {
        public:
                // Frames the first request in the buffer. A request with both
                // Transfer-Encoding and Content-Length, conflicting
                // Content-Length values or an invalid chunked body fails with
                // 400 (these are request-smuggling vectors); a transfer coding
                // other than chunked fails with 501; headers longer than
                // nMaxHeader fail with 431, and bodies longer than nMaxBody with
                // 413.
            static CHttpFrame frameRequest( const char *pBuffer, size_t nLength, size_t nMaxHeader, size_t nMaxBody );

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
