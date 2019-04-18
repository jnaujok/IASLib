/*
 * Queue Class
 *
 *  This class provides a dynamically re-sizable queue for use in
 * storing CObject derived pointers. Any object that has been
 * derived from the CObject type can be stored in this queue.
 *  The queue itself will dynamically re-size itself as used. The
 * rate at which it is re-sized can be adjusted by changing the
 * "Scale" value passed to the constructor, or by using the SetScale
 * function to change it dynamically.
 *  The queue structure only allows insertion at the back of the queue
 * and removal/deletion from the front of the queue. The Push function 
 * adds an element, the Pop function removes an element. The Peek 
 * function allows the viewing of the front element without its removal.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/28/2005
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Queue.h"
#include <memory>

namespace IASLib
{
    CQueue::CQueue( size_t nScale )
        : m_aElements( nScale )
    {
    }

    CQueue::~CQueue( void )
    {
        m_aElements.DeleteAll();
    }

    IMPLEMENT_OBJECT( CQueue, CCollection );

    size_t CQueue::Push( CObject *pNew )
    {
        return m_aElements.Prepend( pNew );
    }

    CObject *CQueue::Pop( void )
    {
        CObject *pRetVal = NULL;

        if ( m_aElements.GetCount() != 0 )
        {
            pRetVal = m_aElements.Remove( m_aElements.GetCount() - 1 );
        }
        return pRetVal;
    }

    CObject *CQueue::Peek( void )
    {
        CObject *pRetVal = NULL;

        if ( m_aElements.GetCount() != 0 )
        {
            pRetVal = m_aElements.Get( m_aElements.GetCount() - 1 );
        }
        return pRetVal;
    }

    void CQueue::DeleteAll( void )
    {
        m_aElements.DeleteAll();
    }

    void CQueue::EmptyAll( void )
    {
        m_aElements.EmptyAll();
    }

    void CQueue::SetScale( size_t nScale )
    {
        m_aElements.SetScale( nScale );
    }

    CIterator *CQueue::Enumerate( void )
    {
        return new CArrayIterator( &m_aElements );
    }
} // End of Namespace IASLib

