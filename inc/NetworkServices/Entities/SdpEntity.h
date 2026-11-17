/**
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
    class CSDPEntity : public CEntity
    {
        private:
            CArray      m_SessionValues;

        public:
                        DEFINE_OBJECT( CSDPEntity );
                        IASLIB_SET_ENTITY( "application/sdp" );

                        CSDPEntity( const CString &body );
                        CSDPEntity( void );

            virtual    ~CSDPEntity( void );

                // Appends a line. The caller is responsible for adding lines in
                // the order RFC 4566 requires (v, o, s, ..., c, ..., t, ..., m).
            void        addAttribute( const char *name, const char *value );

                // Returns the value of the first line of this type, or "".
            CString     getAttribute( const char *name );

            size_t      getAttributeCount( void ) { return m_SessionValues.Count(); }

            virtual CString     toString( void );

        protected:
            virtual CEntity *generateEntity( const CString &body );
    };
} // namespace IASLib

#endif // IASLIB_SDPENTITY_H__

#endif // IASLIB_NETWORKING__
