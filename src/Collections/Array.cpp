/*
 * CArray Class
 *
 *  An array of CObject pointers, which the array owns.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Array.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CArray, CCollection );

#ifdef IASLIB_MULTI_THREADED__
    #define IASLIB_ARRAY_LOCK__     CMutexLock oLock__( m_mutex )
#else
    #define IASLIB_ARRAY_LOCK__     do { } while ( 0 )
#endif

    CObject *CArrayIterator::Next( void )
    {
        CObject *pRetVal = m_pArray->Get( m_nCurrentPos );
        if ( m_nCurrentPos < m_pArray->GetLength() )
            m_nCurrentPos++;
        return pRetVal;
    }

    CObject *CArrayIterator::Prev( void )
    {
        if ( m_nCurrentPos == 0 )
            return NULL;
        m_nCurrentPos--;
        return m_pArray->Get( m_nCurrentPos );
    }

    bool CArrayIterator::HasMore( void ) const
    {
        return m_nCurrentPos < m_pArray->GetLength();
    }

    CArray::CArray( size_t nScale )
        : m_aElements( ( nScale > 0 ) ? nScale : 4 )
    {
    }

    CArray::~CArray( void )
    {
        DeleteAll();
    }

    size_t CArray::GetLength( void ) const
    {
        IASLIB_ARRAY_LOCK__;
        return m_aElements.Length();
    }

    size_t CArray::Push( CObject *pNew )
    {
        IASLIB_ARRAY_LOCK__;
        return m_aElements.Push( pNew );
    }

    size_t CArray::Insert( size_t nIndex, CObject *pNew )
    {
        IASLIB_ARRAY_LOCK__;
        return m_aElements.Insert( nIndex, pNew );
    }

    size_t CArray::Set( size_t nIndex, CObject *pSet )
    {
        IASLIB_ARRAY_LOCK__;
        return m_aElements.Set( nIndex, pSet );
    }

    CObject *CArray::Get( size_t nIndex ) const
    {
        IASLIB_ARRAY_LOCK__;
        return m_aElements.Get( nIndex, NULL );
    }

    CObject *CArray::operator []( size_t nIndex ) const
    {
        return Get( nIndex );
    }

    size_t CArray::Find( const CObject *pFind ) const
    {
        IASLIB_ARRAY_LOCK__;
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
            if ( m_aElements[n] == pFind )
                return n;
        return NOT_FOUND;
    }

    CObject *CArray::Remove( size_t nIndex )
    {
        IASLIB_ARRAY_LOCK__;
        if ( nIndex >= m_aElements.Length() )
            return NULL;
        return m_aElements.Remove( nIndex );
    }

    bool CArray::Delete( size_t nIndex )
    {
        CObject *pObject = NULL;
        {
            IASLIB_ARRAY_LOCK__;
            if ( nIndex >= m_aElements.Length() )
                return false;
            pObject = m_aElements.Remove( nIndex );
        }
            // Deleted outside the lock: its destructor may use the array.
        delete pObject;
        return true;
    }

    void CArray::DeleteAll( void )
    {
        TArray<CObject *> aTaken;
        {
            IASLIB_ARRAY_LOCK__;
            aTaken.Swap( m_aElements );
            m_aElements.SetScale( aTaken.GetScale() );
        }
        aTaken.DeleteAll();
    }

    void CArray::EmptyAll( void )
    {
        IASLIB_ARRAY_LOCK__;
        m_aElements.Clear();
        m_aElements.Compact();
    }

    void CArray::Sort( int (*fnCompare)(const CObject *, const CObject *, void *), void *pCallback )
    {
        if ( fnCompare == NULL )
            return;
        IASLIB_ARRAY_LOCK__;
        m_aElements.StableSort( [fnCompare, pCallback]( const CObject *a, const CObject *b ) { return fnCompare( a, b, pCallback ) < 0; } );
    }

    void CArray::Reserve( size_t nCapacity )
    {
        IASLIB_ARRAY_LOCK__;
        m_aElements.Reserve( nCapacity );
    }

    void CArray::Compact( void )
    {
        IASLIB_ARRAY_LOCK__;
        m_aElements.Compact();
    }

    bool CArray::Swap( size_t nSwap1, size_t nSwap2 )
    {
        IASLIB_ARRAY_LOCK__;
        if ( nSwap1 >= m_aElements.Length() || nSwap2 >= m_aElements.Length() || nSwap1 == nSwap2 )
            return false;
        return m_aElements.Swap( nSwap1, nSwap2 );
    }

    CIterator *CArray::Enumerate( void )
    {
        return new CArrayIterator( this );
    }
} // namespace IASLib
