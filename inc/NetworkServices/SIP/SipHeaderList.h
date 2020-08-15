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

            virtual CIterator  *Enumerate( void );

            virtual CHeader    *newHeader( const CString &name );
    }; // Class CSipHeaderList
}  // Namespace IASLib

#endif // IASLIB_SIPHEADERLIST_H__

#endif // IASLIB_NETWORKING__
