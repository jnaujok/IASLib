/**
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
    class CTextPlainEntity : public CEntity
    {
        private:
            CString m_strBody;

        public:
            DEFINE_OBJECT( CTextPlainEntity );
            IASLIB_SET_ENTITY( "text/plain" );

            CTextPlainEntity( void );
            CTextPlainEntity( const CString &body );

            virtual ~CTextPlainEntity( void );

            virtual CString toString( void );
            virtual void setText( const CString &body ) { m_strBody = body; }

        protected:
            virtual CEntity *generateEntity( const CString &body );
    };
} // namespace IASLib

#endif // IASLIB_TEXTPLAINENTITY_H__

#endif // IASLIB_NETWORKING__
