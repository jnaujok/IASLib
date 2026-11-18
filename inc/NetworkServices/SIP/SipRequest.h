/**
 * @file SipRequest.h
 * @brief A SIP request, received or to be sent (CSipRequest).
 *
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
    /// A SIP request (RFC 3261): an incoming one read by a server, or an
    /// outgoing one built to send.
    ///
    /// Parsing and the header and body accessors come from CGenericRequest.
    /// Headers are parsed with SIP rules, so compact forms such as "v:" and
    /// "i:" are found under their full names. This class adds access to the
    /// top Via and the From and To tags.
    /// @code
    /// CSipRequest oRequest;                     // version "SIP/2.0"
    /// oRequest.setRequestType( "OPTIONS" );
    /// oRequest.setUri( "sip:bob@example.com" );
    /// oRequest.setHeaderValue( "Via", "SIP/2.0/UDP 192.0.2.1;branch=z9hG4bK776" );
    /// CString strWire = oRequest.toString();
    /// @endcode
    class CSipRequest : public CGenericRequest
    {
        public:
            DEFINE_OBJECT( CSipRequest );

                /// An incoming request from the peer; fill it with parse().
            CSipRequest( const CInternetAddress &internetAddress );
                /// An empty outgoing request with version "SIP/2.0".
            CSipRequest( void );

                /// Destroys the request.
            virtual ~CSipRequest( void );

                /// The request method, for example "INVITE".
            virtual CString getMethod( void ) { return getRequestType(); }

                /// The topmost Via value: the first element of the first Via
                /// header, which may hold a comma-separated list. "" if there is
                /// no Via.
            virtual CString getTopVia( void );
                /// Replaces the topmost Via value, keeping the rest of the list.
                /// With no Via header yet, adds one. A value holding CR, LF or
                /// NUL is not stored.
            virtual void    setTopVia( const CString &via );

                /// The tag parameter of the From header ("" if absent).
            virtual CString getFromTag( void );
                /// The tag parameter of the To header ("" if absent).
            virtual CString getToTag( void );

                /// The whole request as it is sent on the wire. A Content-Length
                /// header is added (without changing the stored headers) when
                /// there is none.
            virtual CString toString( void );
                /// Writes toString() to the stream.
            virtual void toStream( CStream *pStream );

                /// Reads the header lines into a new CSipHeaderList (caller owns it).
            virtual CHeaderList *getHeaderList( CStream &requestStream ) { return new CSipHeaderList( requestStream ); }

        protected:
                /// Creates an empty CSipHeaderList (caller owns it).
            virtual CHeaderList *newHeaderList( void ) { return new CSipHeaderList(); }
    };
} // namespace IASLib

#endif // IASLIB_SIPREQUEST_H__

#endif // IASLIB_NETWORKING__
