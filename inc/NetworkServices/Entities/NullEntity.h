/**
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
    class CNullEntity : public CEntity
    {
        public:
            DEFINE_OBJECT( CNullEntity );
            IASLIB_SET_ENTITY( "none/none" );

            CNullEntity( void );
            virtual ~CNullEntity( void );

            virtual CString toString( void );
            virtual size_t getLength( void ) { return 0; }

        protected:
            virtual CEntity *generateEntity( const CString &body );
    };
}

#endif // IASLIB_NULLENTITY_H__

#endif // IASLIB_NETWORKING__
