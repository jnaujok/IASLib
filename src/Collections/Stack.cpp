/*
 * Stack Class
 *
 *  This class provides a dynamically re-sizable stack for use in
 * storing CObject derived pointers. Any object that has been
 * derived from the CObject type can be stored in this stack.
 *  The stack itself will dynamically re-size itself as used. The
 * rate at which it is re-sized can be adjusted by changing the
 * "Scale" value passed to the constructor, or by using the SetScale
 * function to change it dynamically.
 *  The stack structure only allows insertion and deletion from the
 * top of the stack. The Push function adds an element, the Pop function
 * removes an element. The Peek function allows the viewing of the
 * top element without its removal.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/24/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include <memory>
#include "Stack.h"

namespace IASLib
{
    CStack::CStack( int nScale )
        : m_aElements( nScale )
    {
    }

    CStack::~CStack( void )
    {
        m_aElements.DeleteAll();
    }

    IMPLEMENT_OBJECT( CStack, CObject );

    size_t CStack::Push( CObject *pNew )
    {
        return m_aElements.Push( pNew );
    }

    CObject *CStack::Pop( void )
    {
        CObject *pRetVal = NULL;

        if ( m_aElements.GetCount() != 0 )
        {
            pRetVal = m_aElements.Remove( m_aElements.GetCount() - 1 );
        }
        return pRetVal;
    }

    CObject *CStack::Peek( void )
    {
        CObject *pRetVal = NULL;

        if ( m_aElements.GetCount() != 0 )
        {
            pRetVal = m_aElements.Get( m_aElements.GetCount() - 1 );
        }
        return pRetVal;
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
        m_aElements.SetScale( nScale );
    }

    CIterator *CStack::Enumerate( void )
    {
        return new CArrayIterator( &m_aElements );
    }
} // End of Namespace IASLib

