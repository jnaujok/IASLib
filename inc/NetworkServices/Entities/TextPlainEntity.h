/**
 * Text/Plain Entity class
 *
 * This is the class for returning a body encoded using text/plain encoding.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_TEXTPLAINENTITY_H__
#define IASLIB_TEXTPLAINENTITY_H__

#include "NetworkServices/Entity.h"
#include "BaseTypes/String_.h"
#include "Streams/Stream.h"

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
            CTextPlainEntity( CStream &stream, size_t nContentLength );

            virtual ~CTextPlainEntity( void );

            virtual CString toString( void );

        protected:
            CEntity *generateEntity( CStream &stream, size_t nContentLength );
    };
} // namespace IASLib

#endif // IASLIB_TEXTPLAINENTITY_H__

#endif // IASLIB_NETWORKING__
