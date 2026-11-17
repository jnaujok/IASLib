/**
 * SIP Headers class
 *
 * This class encapsulates a set of SIP header fields in a SIP request or response.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPHEADERLIST_H__
#define IASLIB_SIPHEADERLIST_H__

#include "Collections/Hash.h"
#include "Streams/Stream.h"
#include "SipHeader.h"
#include "NetworkServices/HeaderList.h"

namespace IASLib
{
    class CSipHeaderList : public CHeaderList
    {
        public:
                                CSipHeaderList( CStream &oInputStream );
                                CSipHeaderList( void );
            virtual            ~CSipHeaderList( void );

                                DEFINE_OBJECT( CSipHeaderList );

                // Expands compact header forms (RFC 3261 section 7.3.3 and the
                // IANA registry), so that "v:" is stored and found as "Via".
            virtual CString     canonicalName( const CString &name );

                // Expands a compact form on its own; returns the name unchanged
                // if it is not a compact form.
            static CString      expandCompactForm( const CString &name );

        protected:
            virtual CHeader    *newHeader( const CString &name );
    }; // Class CSipHeaderList
}  // Namespace IASLib

#endif // IASLIB_SIPHEADERLIST_H__

#endif // IASLIB_NETWORKING__
