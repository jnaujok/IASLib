/**
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
    class CHttpResponse : public CGenericResponse
    {
        protected:
            CInternetAddress    m_remoteAddress;
            bool                m_bHeadResponse;
            CString             m_strVersion;

        public:
                                DEFINE_OBJECT( CHttpResponse );

                                CHttpResponse( void );
                                CHttpResponse( CStream &oStream, CInternetAddress &internetAddress );

            virtual            ~CHttpResponse( void );

                // Sets the status with its standard reason phrase.
            virtual void        setStatus( int nCode );
            virtual void        setStatus( int nCode, const char *desc ) { CGenericResponse::setStatus( nCode, desc ); }

                // Marks this as the response to a HEAD request: the body is not
                // sent, but Content-Length still describes it.
            virtual void        setHeadResponse( bool bHead ) { m_bHeadResponse = bHead; }
            virtual bool        isHeadResponse( void ) { return m_bHeadResponse; }

            virtual CString     toString( void );
            virtual void        toStream( CStream *oStream );

                // Fills this response from a received message: the status line,
                // the headers and, after the blank line, the body (all of the
                // bytes that follow it). The message should already be framed,
                // as CHttpUtils::frameResponse() does. False, with whatever
                // could be read kept, if the status line or a header line is
                // malformed or the header section does not end.
            virtual bool        parse( const CString &message );
                // The HTTP version of a received response ("HTTP/1.1"), or "".
            virtual CString     getVersion( void ) { return m_strVersion; }
    };
} // namespace IASLib

#endif // IASLIB_HTTPRESPONSE_H__

#endif // IASLIB_NETWORKING__
