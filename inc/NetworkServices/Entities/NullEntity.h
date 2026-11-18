/**
 * @file NullEntity.h
 * @brief The entity of a message with no body (CNullEntity).
 *
 * Null Entity class
 *
 * This is the class for an empty entity body.
 *
 * Author: Jeffrey R. Naujok
 * Created: August 23, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_NULLENTITY_H__
#define IASLIB_NULLENTITY_H__

#include "NetworkServices/Entity.h"
#include "BaseTypes/String_.h"

namespace IASLib
{
    /// The entity of a message with no body.
    ///
    /// CEntity::getEntity() returns one for an empty body, so a message always
    /// has an entity object. Its body is empty and its media type is the
    /// placeholder "none/none".
    class CNullEntity : public CEntity
    {
        public:
            DEFINE_OBJECT( CNullEntity );
                /// The media type, the placeholder "none/none".
            IASLIB_SET_ENTITY( "none/none" );

                /// Creates an empty entity.
            CNullEntity( void );
                /// Destroys the entity.
            virtual ~CNullEntity( void );

                /// Always an empty string.
            virtual CString toString( void );
                /// Always 0.
            virtual size_t getLength( void ) { return 0; }

        protected:
                /// Returns a new CNullEntity, whatever the body (caller owns it).
            virtual CEntity *generateEntity( const CString &body );
    };
}

#endif // IASLIB_NULLENTITY_H__

#endif // IASLIB_NETWORKING__
