/**
 * SIP Request class
 *
 * This class encapsulates a SIP based request, including the request line,
 * headers and body. Headers are parsed with SIP rules, so compact forms
 * such as "v:" and "i:" are found under their full names.
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

            CSipRequest( const CInternetAddress &internetAddress );
            CSipRequest( void );

            virtual ~CSipRequest( void );

                // The request method, for example "INVITE".
            virtual CString getMethod( void ) { return getRequestType(); }

                // The topmost Via value: the first element of the first Via
                // header, which may hold a comma-separated list.
            virtual CString getTopVia( void );
                // Replaces the topmost Via value.
            virtual void    setTopVia( const CString &via );

                // The tag parameters of the From and To headers ("" if absent).
            virtual CString getFromTag( void );
            virtual CString getToTag( void );

            virtual CString toString( void );
            virtual void toStream( CStream *pStream );

            virtual CHeaderList *getHeaderList( CStream &requestStream ) { return new CSipHeaderList( requestStream ); }

        protected:
            virtual CHeaderList *newHeaderList( void ) { return new CSipHeaderList(); }
    };
} // namespace IASLib

#endif // IASLIB_SIPREQUEST_H__

#endif // IASLIB_NETWORKING__
