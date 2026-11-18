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

    int CConnectionArray::Find( CConnection *pConnection ) const
    {
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
        {
            if ( m_aElements.Get( n, NULL ) == pConnection )
                return (int)n;
        }
        return -1;
    }

    bool CConnectionArray::Remove( CConnection *pConnection )
    {
        int nAt = Find( pConnection );
        return ( nAt >= 0 ) ? Delete( nAt ) : false;
    }

        // (Deleting in place, while each connection's destructor removed it
        // from this array, skipped every other one: those leaked, holding a
        // pointer to the database being destroyed.)
    void CConnectionArray::DeleteAll( void )
    {
        while ( m_aElements.Length() > 0 )
        {
            size_t nLast = m_aElements.Length() - 1;
            CConnection *pConnection = m_aElements.Get( nLast, NULL );
            m_aElements.Remove( nLast );
            delete pConnection;
        }
        m_aElements.Compact();
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
