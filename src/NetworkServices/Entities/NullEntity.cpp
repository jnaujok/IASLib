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

#include "NetworkServices/Entities/NullEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CNullEntity, CEntity );

    CNullEntity::CNullEntity( void )
    {
    }

    CNullEntity::~CNullEntity( void )
    {
    }

    CString CNullEntity::toString( void )
    {
        return CString( "" );
    }

    CEntity *CNullEntity::generateEntity( const CString &body )
    {
        return new CNullEntity();
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING
