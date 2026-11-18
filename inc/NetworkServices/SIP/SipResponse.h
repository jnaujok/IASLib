/**
 * @file SipResponse.h
 * @brief A SIP response (CSipResponse).
 *
 * SIP Response class
 *
 * This class encapsulates a SIP based response. Content-Length is always
 * written, as RFC 3261 requires for stream transports.
 *
 * Author: Jeffrey R. Naujok
 * Created: July 21, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPRESPONSE_H__
#define IASLIB_SIPRESPONSE_H__

#include "NetworkServices/SIP/SipHeaderList.h"
#include "NetworkServices/Entity.h"
#include "NetworkServices/GenericResponse.h"

namespace IASLib
{
    /// A SIP response (RFC 3261): status, headers and body.
    ///
    /// The status line says SIP/2.0. Content-Length is always written, as RFC
    /// 3261 requires for stream transports, and Content-Type is filled in
    /// from the body (see CGenericResponse::serialize()). A new response has
    /// status 0 "Unknown" until one is set.
    class CSipResponse : public CGenericResponse
    {
        public:
            DEFINE_OBJECT( CSipResponse );

                /// Creates a response with no stream.
            CSipResponse( void );
                /// Creates a response to be written to the stream (not owned).
            CSipResponse( CStream &responseStream );

                /// Destroys the response and its body.
            virtual ~CSipResponse( void );

                /// Sets the status code with its standard reason phrase (see
                /// CSipUtils::reasonPhrase()).
            virtual void setStatus( int nCode );
                /// Sets the status code with the given reason phrase (see
                /// CGenericResponse::setStatus()).
            virtual void setStatus( int nCode, const char *desc ) { CGenericResponse::setStatus( nCode, desc ); }

                /// Writes toString() to the stream.
            virtual void toStream( CStream *pStream );
                /// The whole response as it is sent on the wire. This sets
                /// Content-Length (and Content-Type) in the stored headers.
            virtual CString toString( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPRESPONSE_H__

#endif // IASLIB_NETWORKING__
