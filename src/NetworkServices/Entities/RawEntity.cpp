/**
 * Raw Entity class
 *
 * Holds a body of a media type that has no registered entity class.
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/Entities/RawEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CRawEntity, CEntity );

    CRawEntity::CRawEntity( const CString &contentType, const CString &body ) : m_strContentType( contentType ), m_strBody( body )
    {
    }

    CRawEntity::~CRawEntity( void )
    {
    }

    CEntity *CRawEntity::generateEntity( const CString &body )
    {
        return new CRawEntity( m_strContentType, body );
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
