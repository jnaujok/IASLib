/**
 * @file TextPlainEntity.h
 * @brief A text/plain body (CTextPlainEntity).
 *
 * Text/Plain Entity class
 *
 * This is the class for a body encoded using text/plain encoding.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 13, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_TEXTPLAINENTITY_H__
#define IASLIB_TEXTPLAINENTITY_H__

#include "NetworkServices/Entity.h"
#include "BaseTypes/String_.h"

namespace IASLib
{
    /// A text/plain body, kept as the text it holds.
    ///
    /// The text is stored and written unchanged; no charset conversion is done.
    class CTextPlainEntity : public CEntity
    {
        private:
            CString m_strBody;  ///< The text.

        public:
            DEFINE_OBJECT( CTextPlainEntity );
                /// The media type, "text/plain".
            IASLIB_SET_ENTITY( "text/plain" );

                /// Creates an empty text body.
            CTextPlainEntity( void );
                /// Creates a body holding the text.
            CTextPlainEntity( const CString &body );

                /// Destroys the entity.
            virtual ~CTextPlainEntity( void );

                /// The text, unchanged.
            virtual CString toString( void );
                /// Replaces the text.
            virtual void setText( const CString &body ) { m_strBody = body; }

        protected:
                /// Returns a new CTextPlainEntity holding the body (caller owns it).
            virtual CEntity *generateEntity( const CString &body );
    };
} // namespace IASLib

#endif // IASLIB_TEXTPLAINENTITY_H__

#endif // IASLIB_NETWORKING__
