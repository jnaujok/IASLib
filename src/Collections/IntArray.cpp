/*
 * CIntArray Class
 *
 *  An array of ints.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "IntArray.h"

#include <functional>

namespace IASLib
{
    namespace
    {
        class CIntArrayIterator : public CIterator
        {
            protected:
                const CIntArray    *m_pArray;
                size_t              m_nCurrentPos;
            public:
                                    CIntArrayIterator( const CIntArray *pArray ) { m_pArray = pArray; m_nCurrentPos = 0; }
                                    DECLARE_OBJECT( CIntArrayIterator, CIterator )
                virtual            ~CIntArrayIterator( void ) {}
                virtual int         NextInt( void ) { return HasMore() ? m_pArray->Get( m_nCurrentPos++ ) : 0; }
                virtual CObject    *Next( void ) { if ( HasMore() ) m_nCurrentPos++; return NULL; }
                virtual CObject    *Prev( void ) { if ( m_nCurrentPos > 0 ) m_nCurrentPos--; return NULL; }
                virtual void        Reset( void ) { m_nCurrentPos = 0; }
                virtual bool        HasMore( void ) const { return m_nCurrentPos < m_pArray->GetLength(); }
        };
    }

    IMPLEMENT_OBJECT( CIntArray, CCollection );

    CIntArray::CIntArray( size_t nScale )
        : m_aElements( ( nScale > 0 ) ? nScale : 4 ), m_nScratch( -1 )
    {
    }

    CIntArray::~CIntArray( void )
    {
    }

    size_t CIntArray::Push( int nValue )
    {
        return m_aElements.Push( nValue );
    }

    int CIntArray::Get( size_t nIndex ) const
    {
        return m_aElements.Get( nIndex, 0 );
    }

    int &CIntArray::operator []( size_t nIndex )
    {
        if ( nIndex < m_aElements.Length() )
            return m_aElements[ nIndex ];
        m_nScratch = -1;
        return m_nScratch;
    }

    size_t CIntArray::Insert( size_t nIndex, int nValue )
    {
        return m_aElements.Insert( nIndex, nValue );
    }

    bool CIntArray::Delete( size_t nIndex )
    {
        if ( nIndex >= m_aElements.Length() )
            return false;
        m_aElements.Remove( nIndex );
        return true;
    }

    void CIntArray::Sort( bool bAscending )
    {
        if ( bAscending )
            m_aElements.Sort();
        else
            m_aElements.Sort( std::greater<int>() );
    }

    void CIntArray::DeleteAll( void )
    {
        m_aElements.Clear();
        m_aElements.Compact();
    }

    void CIntArray::EmptyAll( void )
    {
        DeleteAll();
    }

    CIterator *CIntArray::Enumerate( void )
    {
        return new CIntArrayIterator( this );
    }
} // namespace IASLib
