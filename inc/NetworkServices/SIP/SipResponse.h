/**
 * SIP Response class
 *
 * This class encapsulates a SIP based response.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
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

            CSipResponse( CStream &responseStream );

            virtual ~CSipResponse( void );

            virtual void toStream( CStream *pStream );

            virtual CString toString( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPRESPONSE_H__

#endif // IASLIB_NETWORKING__
