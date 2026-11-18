/**
 * @file SipHeaderList.h
 * @brief The header fields of a SIP message, with compact forms expanded (CSipHeaderList).
 *
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
    /// The header fields of a SIP request or response.
    ///
    /// It uses the generic CHeaderList rules and, in addition, expands the
    /// one-letter compact header forms, so that "v:" is stored and found as
    /// "Via" and "i" finds "Call-ID". Headers are stored as CSipHeader objects.
    class CSipHeaderList : public CHeaderList
    {
        public:
                                /// Parses header lines from the stream, up to and
                                /// including the blank line that ends them, with
                                /// compact forms expanded (see CHeaderList::parse()).
                                CSipHeaderList( CStream &oInputStream );
                                /// Creates an empty list.
                                CSipHeaderList( void );
                                /// Deletes the headers.
            virtual            ~CSipHeaderList( void );

                                DEFINE_OBJECT( CSipHeaderList );

                /// Expands compact header forms (RFC 3261 section 7.3.3 and the
                /// IANA registry), so that "v:" is stored and found as "Via".
            virtual CString     canonicalName( const CString &name );

                /// Expands a compact form on its own.
                ///
                /// A one-letter name (either case) with a registered meaning
                /// gives the full name, such as "Call-ID" for "i".
                /// @return The full name, or the name unchanged if it is not a
                ///         compact form.
            static CString      expandCompactForm( const CString &name );

        protected:
                /// Creates a CSipHeader for a new header.
            virtual CHeader    *newHeader( const CString &name );
    }; // Class CSipHeaderList
}  // Namespace IASLib

#endif // IASLIB_SIPHEADERLIST_H__

#endif // IASLIB_NETWORKING__
