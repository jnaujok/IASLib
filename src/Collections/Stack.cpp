/*
 * CStack Class
 *
 *  Last in, first out.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Stack.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CStack, CCollection );

    CStack::CStack( int nScale )
        : m_aElements( ( nScale > 0 ) ? (size_t)nScale : 4 )
    {
    }

    CStack::~CStack( void )
    {
        m_aElements.DeleteAll();
    }

    size_t CStack::Push( CObject *pNew )
    {
        return m_aElements.Push( pNew );
    }

    CObject *CStack::Pop( void )
    {
        size_t nLength = m_aElements.GetLength();
        return ( nLength > 0 ) ? m_aElements.Remove( nLength - 1 ) : NULL;
    }

    CObject *CStack::Peek( void ) const
    {
        size_t nLength = m_aElements.GetLength();
        return ( nLength > 0 ) ? m_aElements.Get( nLength - 1 ) : NULL;
    }

    void CStack::DeleteAll( void )
    {
        m_aElements.DeleteAll();
    }

    void CStack::EmptyAll( void )
    {
        m_aElements.EmptyAll();
    }

    void CStack::SetScale( int nScale )
    {
        if ( nScale > 0 )
            m_aElements.SetScale( (size_t)nScale );
    }

    CIterator *CStack::Enumerate( void )
    {
        return m_aElements.Enumerate();
    }
} // End of Namespace IASLib
