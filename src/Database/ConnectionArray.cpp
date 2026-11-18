/*
 *  Connection Array Class
 *
 *  This class is a specialization of the generic CArray class that is used
 * to store and maintain the list of connections on a database.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/26/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "ConnectionArray.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    IMPLEMENT_OBJECT( CConnectionArray, CObject );

    CConnectionArray::CConnectionArray( int nScale )
        : m_aElements( ( nScale > 0 ) ? (size_t)nScale : 4 )
    {
    }

    CConnectionArray::~CConnectionArray( void )
    {
        DeleteAll();
    }

    int CConnectionArray::Push( CConnection *pNew )
    {
        return (int)m_aElements.Push( pNew );
    }

    CConnection *CConnectionArray::Get( int nCount ) const
    {
        return ( nCount >= 0 ) ? m_aElements.Get( (size_t)nCount, NULL ) : NULL;
    }

    int CConnectionArray::Insert( int nCount, CConnection *pNew )
    {
        return (int)m_aElements.Insert( ( nCount > 0 ) ? (size_t)nCount : 0, pNew );
    }

    bool CConnectionArray::Delete( int nCount )
    {
        if ( nCount < 0 || (size_t)nCount >= m_aElements.Length() )
            return false;
        m_aElements.Remove( (size_t)nCount );
        return true;
    }

    void CConnectionArray::DeleteAll( void )
    {
        m_aElements.DeleteAll();
        m_aElements.Compact();
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
