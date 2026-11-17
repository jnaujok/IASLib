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

#include "NetworkServices/Entities/TextPlainEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CTextPlainEntity, CEntity );

    CTextPlainEntity::CTextPlainEntity( void )
    {
    }

    CTextPlainEntity::CTextPlainEntity( const CString &body ) : m_strBody( body )
    {
    }

    CTextPlainEntity::~CTextPlainEntity( void )
    {
    }

    CString CTextPlainEntity::toString( void )
    {
        return m_strBody;
    }

    CEntity *CTextPlainEntity::generateEntity( const CString &body )
    {
        return new CTextPlainEntity( body );
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
