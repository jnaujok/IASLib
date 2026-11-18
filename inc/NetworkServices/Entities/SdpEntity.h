/**
 * @file SdpEntity.h
 * @brief A Session Description Protocol body (CSDPEntity).
 *
 * SDP Entity class
 *
 * This class encapsulates a Session Description Protocol (RFC 4566) body
 * as an entity within a request or response. It keeps the description as
 * an ordered list of "type=value" lines; the order of SDP lines is
 * significant and is preserved.
 *
 * Author: Jeffrey R. Naujok
 * Created: July 30, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SDPENTITY_H__
#define IASLIB_SDPENTITY_H__

#include "NetworkServices/Entity.h"
#include "BaseTypes/String_.h"
#include "Collections/Array.h"

namespace IASLib
{
    /// A Session Description Protocol (RFC 4566) body.
    ///
    /// The description is kept as an ordered list of "type=value" lines, since
    /// the order of SDP lines matters. Lines are split at the first '=' only,
    /// so values such as "fmtp:101 0-15;mode=20" are kept whole. No other
    /// checking of the SDP is done.
    /// @code
    /// CSDPEntity oSdp;
    /// oSdp.addAttribute( "v", "0" );
    /// oSdp.addAttribute( "o", "- 1 1 IN IP4 192.0.2.1" );
    /// oSdp.addAttribute( "s", "-" );
    /// CString strBody = oSdp.toString();   // "v=0\r\no=- 1 1 IN IP4 192.0.2.1\r\ns=-\r\n"
    /// @endcode
    class CSDPEntity : public CEntity
    {
        private:
            CArray      m_SessionValues;    ///< The lines, as owned attribute objects, in order.

        public:
                        DEFINE_OBJECT( CSDPEntity );
                        /// The media type, "application/sdp".
                        IASLIB_SET_ENTITY( "application/sdp" );

                        /// Parses a body into lines.
                        ///
                        /// Lines may end in CRLF or LF; empty lines are skipped. A
                        /// line without '=' is kept with the whole line as its type
                        /// and an empty value.
                        CSDPEntity( const CString &body );
                        /// Creates an empty description.
                        CSDPEntity( void );

                /// Deletes the lines.
            virtual    ~CSDPEntity( void );

                /// Appends a line.
                ///
                /// The caller is responsible for adding lines in the order RFC
                /// 4566 requires (v, o, s, ..., c, ..., t, ..., m).
                /// @param name  The line type, such as "v" or "a".
                /// @param value The text after the '='.
            void        addAttribute( const char *name, const char *value );

                /// Returns the value of the first line of this type, or "".
            CString     getAttribute( const char *name );

                /// The number of lines.
            size_t      getAttributeCount( void ) { return m_SessionValues.Count(); }

                /// The description as `"type=value\r\n"` lines, in order.
            virtual CString     toString( void );

        protected:
                /// Returns a new CSDPEntity parsed from the body (caller owns it).
            virtual CEntity *generateEntity( const CString &body );
    };
} // namespace IASLib

#endif // IASLIB_SDPENTITY_H__

#endif // IASLIB_NETWORKING__
