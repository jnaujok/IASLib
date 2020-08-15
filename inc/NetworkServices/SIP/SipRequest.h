/**
 * SIP Request class
 *
 * This class encapsulates a SIP based request, including the request line,
 * headers, and a handle to the stream with the request.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPREQUEST_H__
#define IASLIB_SIPREQUEST_H__

#include "NetworkServices/SIP/SipHeaderList.h"
#include "NetworkServices/Entity.h"
#include "NetworkServices/GenericRequest.h"

namespace IASLib
{
    class CSipRequest : public CGenericRequest
    {
        public:
            DEFINE_OBJECT( CSipRequest );

            CSipRequest( CInternetAddress &internetAddress );
            CSipRequest( void );

            virtual ~CSipRequest( void );

            virtual CString toString( void );
            virtual void toStream( CStream *pStream );

            virtual CHeaderList *getHeaderList( CStream &requestStream ) { return new CSipHeaderList( requestStream ); }
    };
} // namespace IASLib

#endif // IASLIB_SIPREQUEST_H__

#endif // IASLIB_NETWORKING__