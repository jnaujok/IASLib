/**
 * @file RawEntity.h
 * @brief A body of an unregistered media type, kept as bytes (CRawEntity).
 *
 * Raw Entity class
 *
 * Holds a body of a media type that has no registered entity class, such
 * as multipart/mixed or application/isup. The bytes and the original
 * Content-Type value are kept unchanged so they can be inspected, logged
 * or forwarded.
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_RAWENTITY_H__
#define IASLIB_RAWENTITY_H__

#include "NetworkServices/Entity.h"
#include "BaseTypes/String_.h"

namespace IASLib
{
    /// A body of a media type that has no registered entity class.
    ///
    /// CEntity::getEntity() uses it so that no body is lost: the bytes and the
    /// full Content-Type value are kept exactly as received and are written
    /// out unchanged.
    class CRawEntity : public CEntity
    {
        private:
            CString m_strContentType;   ///< The Content-Type value, parameters included.
            CString m_strBody;          ///< The body bytes, unchanged.

        public:
            DEFINE_OBJECT( CRawEntity );

                /// Keeps a body and the Content-Type value it arrived with.
                /// @param contentType The full Content-Type value.
                /// @param body        The body bytes.
            CRawEntity( const CString &contentType, const CString &body );
                /// Destroys the entity.
            virtual ~CRawEntity( void );

                /// The Content-Type value the body arrived with, including any
                /// parameters.
            virtual const char *getMimeType( void ) { return (const char *)m_strContentType; }
                /// The body bytes, unchanged.
            virtual CString toString( void ) { return m_strBody; }

        protected:
                /// Returns a new CRawEntity with this one's Content-Type and the
                /// given body (caller owns it).
            virtual CEntity *generateEntity( const CString &body );
    };
}

#endif // IASLIB_RAWENTITY_H__

#endif // IASLIB_NETWORKING__
