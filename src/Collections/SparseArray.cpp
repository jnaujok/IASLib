/*
 * CSparseArray Class
 *
 *  An array where only some indexes have objects.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "SparseArray.h"
#include "CollectionException.h"

namespace IASLib
{
    CObject *CSparseArrayIterator::Next( void )
    {
        if ( m_nCurrentPos >= m_pArray->m_aElements.Length() )
            return NULL;
        const TSparseArray<CObject *>::Entry &oEntry = m_pArray->m_aElements.GetEntry( m_nCurrentPos++ );
        m_nSparseNumber = oEntry.Index;
        return oEntry.Value;
    }

    CObject *CSparseArrayIterator::Prev( void )
    {
        if ( m_nCurrentPos == 0 || m_nCurrentPos > m_pArray->m_aElements.Length() )
            return NULL;
        const TSparseArray<CObject *>::Entry &oEntry = m_pArray->m_aElements.GetEntry( --m_nCurrentPos );
        m_nSparseNumber = oEntry.Index;
        return oEntry.Value;
    }

    bool CSparseArrayIterator::HasMore( void ) const
    {
        return m_nCurrentPos < m_pArray->m_aElements.Length();
    }

    CSparseArray::CSparseArray( size_t nGrowBy )
        : m_aElements( nGrowBy )
    {
    }

    CSparseArray::~CSparseArray( void )
    {
        DeleteAll();
    }

    size_t CSparseArray::Push( CObject * )
    {
        throw CCollectionException( "Sparse Array does not support Append/Prepend/Push operations.", CException::NORMAL );
    }

    size_t CSparseArray::Insert( size_t nIndex, CObject *pNew )
    {
        CObject **ppOld = m_aElements.Find( nIndex );
        if ( ppOld != NULL )
        {
            if ( *ppOld != pNew )
            {
                CObject *pOld = *ppOld;
                *ppOld = pNew;
                delete pOld;
            }
        }
        else
        {
            m_aElements.Set( nIndex, pNew );
        }
        return nIndex;
    }

    CObject *CSparseArray::Get( size_t nIndex ) const
    {
        return m_aElements.Get( nIndex, NULL );
    }

    CObject *CSparseArray::Remove( size_t nIndex )
    {
        CObject *pObject = m_aElements.Get( nIndex, NULL );
        m_aElements.Remove( nIndex );
        return pObject;
    }

    bool CSparseArray::Delete( size_t nIndex )
    {
        if ( ! m_aElements.Exists( nIndex ) )
            return false;
        delete Remove( nIndex );
        return true;
    }

    bool CSparseArray::Swap( size_t nSwap1, size_t nSwap2 )
    {
        if ( nSwap1 == nSwap2 || ( ! Exists( nSwap1 ) && ! Exists( nSwap2 ) ) )
            return false;
        CObject *p1 = Remove( nSwap1 );
        CObject *p2 = Remove( nSwap2 );
        if ( p2 != NULL )
            m_aElements.Set( nSwap1, p2 );
        if ( p1 != NULL )
            m_aElements.Set( nSwap2, p1 );
        return true;
    }

    void CSparseArray::DeleteAll( void )
    {
        m_aElements.DeleteAll();
        m_aElements.Compact();
    }

    void CSparseArray::EmptyAll( void )
    {
        m_aElements.Clear();
        m_aElements.Compact();
    }

    CIterator *CSparseArray::Enumerate( void )
    {
        return new CSparseArrayIterator( this );
    }
} // namespace IASLib
