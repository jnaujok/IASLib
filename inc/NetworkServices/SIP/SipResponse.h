/**
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
    class CSipResponse : public CGenericResponse
    {
        public:
            DEFINE_OBJECT( CSipResponse );

            CSipResponse( void );
            CSipResponse( CStream &responseStream );

            virtual ~CSipResponse( void );

                // Sets the status code with its standard reason phrase.
            virtual void setStatus( int nCode );
            virtual void setStatus( int nCode, const char *desc ) { CGenericResponse::setStatus( nCode, desc ); }

            virtual void toStream( CStream *pStream );
            virtual CString toString( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPRESPONSE_H__

#endif // IASLIB_NETWORKING__
