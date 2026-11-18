/*
 * CList Class
 *
 *  A doubly linked list of CObject pointers.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "List.h"

namespace IASLib
{
    namespace
    {
        class CListIterator : public CIterator
        {
            protected:
                const TList<CObject *>     *m_pList;
                TList<CObject *>::const_iterator m_itCurrent;
            public:
                                    CListIterator( const TList<CObject *> *pList ) : m_pList( pList ), m_itCurrent( pList->begin() ) {}
                                    DECLARE_OBJECT( CListIterator, CIterator )
                virtual            ~CListIterator( void ) {}
                virtual CObject    *Next( void )
                {
                    if ( m_itCurrent == m_pList->end() )
                        return NULL;
                    CObject *pValue = *m_itCurrent;
                    ++m_itCurrent;
                    return pValue;
                }
                virtual CObject    *Prev( void )
                {
                    if ( m_itCurrent == m_pList->begin() )
                        return NULL;
                    --m_itCurrent;
                    return *m_itCurrent;
                }
                virtual void        Reset( void ) { m_itCurrent = m_pList->begin(); }
                virtual bool        HasMore( void ) const { return m_itCurrent != m_pList->end(); }
        };
    }

    IMPLEMENT_OBJECT( CList, CCollection );

    CList::CList( void )
    {
    }

    CList::~CList( void )
    {
        DeleteAll();
    }

    size_t CList::AddFirst( CObject *pNew )
    {
        return m_lstElements.AddFirst( pNew );
    }

    size_t CList::AddLast( CObject *pNew )
    {
        return m_lstElements.AddLast( pNew );
    }

    size_t CList::AddAt( size_t nIndex, CObject *pNew )
    {
        return m_lstElements.AddAt( nIndex, pNew );
    }

    size_t CList::AddBefore( CObject *pValue, CObject *pNew )
    {
        size_t nIndex = m_lstElements.Find( pValue );
        if ( nIndex == NOT_FOUND )
            return NOT_FOUND;
        return m_lstElements.AddAt( nIndex, pNew );
    }

    size_t CList::AddAfter( CObject *pValue, CObject *pNew )
    {
        size_t nIndex = m_lstElements.Find( pValue );
        if ( nIndex == NOT_FOUND )
            return NOT_FOUND;
        return m_lstElements.AddAt( nIndex + 1, pNew );
    }

    CObject *CList::RemoveFirst( void )
    {
        return m_lstElements.IsEmpty() ? NULL : m_lstElements.RemoveFirst();
    }

    CObject *CList::RemoveLast( void )
    {
        return m_lstElements.IsEmpty() ? NULL : m_lstElements.RemoveLast();
    }

    CObject *CList::RemoveAt( size_t nIndex )
    {
        return ( nIndex < m_lstElements.Length() ) ? m_lstElements.RemoveAt( nIndex ) : NULL;
    }

    CObject *CList::GetAt( size_t nIndex ) const
    {
        return m_lstElements.Get( nIndex, NULL );
    }

    CObject *CList::GetFirst( void ) const
    {
        return m_lstElements.IsEmpty() ? NULL : m_lstElements.GetFirst();
    }

    CObject *CList::GetLast( void ) const
    {
        return m_lstElements.IsEmpty() ? NULL : m_lstElements.GetLast();
    }

    int CList::Find( CObject *pValue ) const
    {
        size_t nIndex = m_lstElements.Find( pValue );
        return ( nIndex == NOT_FOUND ) ? -1 : (int)nIndex;
    }

    void CList::DeleteAll( void )
    {
        m_lstElements.DeleteAll();
    }

    void CList::EmptyAll( void )
    {
        m_lstElements.Clear();
    }

    CIterator *CList::Enumerate( void )
    {
        return new CListIterator( &m_lstElements );
    }
} // End of Namespace IASLib
