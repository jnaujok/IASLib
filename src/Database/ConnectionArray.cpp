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

#include <stdlib.h>
#include "ConnectionArray.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    IMPLEMENT_OBJECT( CConnectionArray, CObject );

    CConnectionArray::CConnectionArray( int nScale )
    {
        if ( nScale > 0 )
            m_nScale = nScale;
        else
            m_nScale = 4;
        m_nSize = 0;
        m_nElements = 0;
        m_apElements = NULL;
    }

    CConnectionArray::~CConnectionArray( void )
    {
        DeleteAll();
    }

    void CConnectionArray::Resize( bool bIncrease )
    {
        if ( bIncrease )
        {
            if ( ( m_nSize % m_nScale ) == 0 )
            {
                CConnection **aTemp = new CConnection *[ m_nSize + m_nScale ];

                for ( int nCount = 0; nCount < m_nSize ; nCount++ )
                {
                    aTemp[ nCount ] = m_apElements[ nCount ];
                }

                delete m_apElements;
                m_apElements = aTemp;
                m_nSize += m_nScale;
            }
        }
        else
        {
            if ( ( m_nSize - m_nElements ) > m_nScale )
            {
                CConnection **aTemp = new CConnection *[ m_nSize - m_nScale ];

                for ( int nCount = 0; nCount < m_nElements ; nCount++ )
                {
                    aTemp[ nCount ] = m_apElements[ nCount ];
                }

                delete m_apElements;
                m_apElements = aTemp;
                m_nSize -= m_nScale;
            }
        }
    }

    int CConnectionArray::Push( CConnection *pNew )
    {
        Resize( true );
        m_apElements[ m_nElements ] = pNew;
        m_nElements++;

        return ( m_nElements - 1 );
    }

    CConnection *CConnectionArray::Get( int nCount )
    {
        if ( ( nCount >= 0 ) && ( nCount < m_nElements ) )
        {
            return m_apElements[ nCount ];
        }
        return NULL;
    }

    CConnection *CConnectionArray::operator []( int nCount )
    {
        return Get( nCount );
    }

    int CConnectionArray::Insert( int nCount, CConnection *pNew )
    {
        if ( nCount >= m_nElements )
        {
            return Push( pNew );
        }

        Resize( true );

        if ( nCount < 0 )
            nCount = 0;

        for ( int nIndex = m_nElements; nIndex > nCount ; nIndex-- )
        {
            m_apElements[ nIndex ] = m_apElements[ nIndex - 1 ];
        }
        m_apElements[ nCount ] = pNew;
        m_nElements++;

        return nCount;
    }

    bool CConnectionArray::Delete( int nCount )
    {
        if ( ( nCount >= m_nElements ) || ( nCount < 0 ) )
            return false;

        for ( int nIndex = nCount; nIndex < (m_nElements - 1) ; nIndex++ )
        {
            m_apElements[ nIndex ] = m_apElements[ nIndex + 1 ];
        }
        m_nElements--;
        Resize( false );
        return true;
    }

    void CConnectionArray::DeleteAll( void )
    {
        if ( m_nSize )
        {
            for ( int nCount = 0; nCount < m_nElements ; nCount++ )
            {
                delete m_apElements[ nCount ];
            }
            delete m_apElements;
            m_nElements = 0;
            m_nSize = 0;
            m_apElements = NULL;
        }
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
