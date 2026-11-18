/*
 * CStringArray Class
 *
 *  An array of strings.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "StringArray.h"

#include <algorithm>

namespace IASLib
{
    namespace
    {
        class CStringArrayIterator : public CIterator
        {
            protected:
                const CStringArray *m_pArray;
                size_t              m_nCurrentPos;
            public:
                                    CStringArrayIterator( const CStringArray *pArray ) { m_pArray = pArray; m_nCurrentPos = 0; }
                                    DECLARE_OBJECT( CStringArrayIterator, CIterator )
                virtual            ~CStringArrayIterator( void ) {}
                virtual CString     NextString( void ) { return HasMore() ? m_pArray->Get( m_nCurrentPos++ ) : CString(); }
                virtual CObject    *Next( void ) { if ( HasMore() ) m_nCurrentPos++; return NULL; }
                virtual CObject    *Prev( void ) { if ( m_nCurrentPos > 0 ) m_nCurrentPos--; return NULL; }
                virtual void        Reset( void ) { m_nCurrentPos = 0; }
                virtual bool        HasMore( void ) const { return m_nCurrentPos < m_pArray->GetLength(); }
        };
    }

    IMPLEMENT_OBJECT( CStringArray, CCollection );

    CStringArray::CStringArray( size_t nScale )
        : m_aElements( ( nScale > 0 ) ? nScale : 4 )
    {
    }

    CStringArray::CStringArray( const CStringArray &oSource )
        : CCollection( oSource ), m_aElements( oSource.m_aElements )
    {
    }

    CStringArray &CStringArray::operator =( const CStringArray &oSource )
    {
        if ( &oSource != this )
            m_aElements = oSource.m_aElements;
        return *this;
    }

    CStringArray::~CStringArray( void )
    {
    }

    size_t CStringArray::Push( const CString &strNew )
    {
        return m_aElements.Push( strNew );
    }

    CString CStringArray::Get( size_t nIndex ) const
    {
        return m_aElements.Get( nIndex );
    }

    CString &CStringArray::operator []( size_t nIndex )
    {
        if ( nIndex < m_aElements.Length() )
            return m_aElements[ nIndex ];
        m_strScratch.Clear();
        return m_strScratch;
    }

    const CString &CStringArray::operator []( size_t nIndex ) const
    {
        static const CString strEmpty;
        if ( nIndex < m_aElements.Length() )
            return m_aElements[ nIndex ];
        return strEmpty;
    }

    size_t CStringArray::Insert( size_t nIndex, const CString &strNew )
    {
        return m_aElements.Insert( nIndex, strNew );
    }

    bool CStringArray::Delete( size_t nIndex )
    {
        if ( nIndex >= m_aElements.Length() )
            return false;
        m_aElements.Remove( nIndex );
        return true;
    }

    size_t CStringArray::Find( const char *strFind, bool bCaseSensitive ) const
    {
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
            if ( m_aElements[n].Compare( strFind, bCaseSensitive ) == 0 )
                return n;
        return NOT_FOUND;
    }

    CString CStringArray::Join( const char *strSeparator ) const
    {
        CString strResult;
        for ( size_t n = 0; n < m_aElements.Length(); n++ )
        {
            if ( n > 0 && strSeparator != NULL )
                strResult += strSeparator;
            strResult += m_aElements[n];
        }
        return strResult;
    }

    bool CStringArray::Sort( bool bAscending )
    {
        if ( ! m_aElements.IsEmpty() )
            QuickSort( 0, m_aElements.Length() - 1, bAscending );
        return true;
    }

    void CStringArray::QuickSort( size_t nLow, size_t nHigh, bool bAscending )
    {
        if ( m_aElements.IsEmpty() || nLow >= m_aElements.Length() || nLow >= nHigh )
            return;
        if ( nHigh >= m_aElements.Length() )
            nHigh = m_aElements.Length() - 1;

        CString *pFirst = m_aElements.Data() + nLow;
        CString *pEnd = m_aElements.Data() + nHigh + 1;
        if ( bAscending )
            std::stable_sort( pFirst, pEnd, []( const CString &a, const CString &b ) { return a.Compare( b ) < 0; } );
        else
            std::stable_sort( pFirst, pEnd, []( const CString &a, const CString &b ) { return a.Compare( b ) > 0; } );
    }

    void CStringArray::DeleteAll( void )
    {
        m_aElements.Clear();
        m_aElements.Compact();
    }

    void CStringArray::EmptyAll( void )
    {
        DeleteAll();
    }

    CIterator *CStringArray::Enumerate( void )
    {
        return new CStringArrayIterator( this );
    }
} // namespace IASLib
