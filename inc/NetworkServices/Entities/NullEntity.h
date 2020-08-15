/**
 * Null Entity class
 *
 * This is the class for returning an empty entity body.
 *
 * Author: Jeffrey R. Naujok
 * Created: August 23, 2019
 */


#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_NULLENTITY_H__
#define IASLIB_NULLENTITY_H__

#include "NetworkServices/Entity.h"
#include "BaseTypes/String_.h"
#include "Streams/Stream.h"

namespace IASLib
{
    class CNullEntity : public CEntity
    {
        public:
            DEFINE_OBJECT( CNullEntity );
            IASLIB_SET_ENTITY( "none/none" );

            CNullEntity( void );

            virtual ~CNullEntity( void );

            virtual CString toString( void );

        protected:
            virtual CEntity *generateEntity( CStream &stream, size_t nContentLength );        
    };
}

#endif // IASLIB_NULLENTITY_H__

#endif // IASLIB_NETWORKING__