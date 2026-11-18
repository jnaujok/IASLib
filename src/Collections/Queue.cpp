/*
 * CQueue Class
 *
 *  First in, first out.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Queue.h"

namespace IASLib
{
#ifdef IASLIB_MULTI_THREADED__
    #define IASLIB_QUEUE_LOCK__     CMutexLock oLock__( m_mutex )
#else
    #define IASLIB_QUEUE_LOCK__     do { } while ( 0 )
#endif

    namespace
    {
        class CQueueIterator : public CIterator
        {
            protected:
                const CQueue       *m_pQueue;
                size_t              m_nCurrentPos;
            public:
                                    CQueueIterator( const CQueue *pQueue ) : m_pQueue( pQueue ), m_nCurrentPos( 0 ) {}
                                    DECLARE_OBJECT( CQueueIterator, CIterator )
                virtual            ~CQueueIterator( void ) {}
                virtual CObject    *Next( void ) { return HasMore() ? m_pQueue->Get( m_nCurrentPos++ ) : NULL; }
                virtual CObject    *Prev( void ) { return ( m_nCurrentPos > 0 ) ? m_pQueue->Get( --m_nCurrentPos ) : NULL; }
                virtual void        Reset( void ) { m_nCurrentPos = 0; }
                virtual bool        HasMore( void ) const { return m_nCurrentPos < m_pQueue->GetLength(); }
        };
    }

    IMPLEMENT_OBJECT( CQueue, CCollection );

    CQueue::CQueue( size_t nScale )
        : m_qElements( ( nScale > 0 ) ? nScale : 4 )
    {
    }

    CQueue::~CQueue( void )
    {
        DeleteAll();
    }

    size_t CQueue::GetLength( void ) const
    {
        IASLIB_QUEUE_LOCK__;
        return m_qElements.Length();
    }

    size_t CQueue::Push( CObject *pNew )
    {
        IASLIB_QUEUE_LOCK__;
        return m_qElements.Push( pNew );
    }

    CObject *CQueue::Pop( void )
    {
        IASLIB_QUEUE_LOCK__;
        return m_qElements.IsEmpty() ? NULL : m_qElements.Pop();
    }

    CObject *CQueue::Peek( void ) const
    {
        IASLIB_QUEUE_LOCK__;
        return m_qElements.IsEmpty() ? NULL : m_qElements.Peek();
    }

    CObject *CQueue::Get( size_t nPosition ) const
    {
        IASLIB_QUEUE_LOCK__;
        return m_qElements.Get( nPosition, NULL );
    }

    void CQueue::DeleteAll( void )
    {
        TQueue<CObject *> qTaken;
        {
            IASLIB_QUEUE_LOCK__;
            qTaken.Swap( m_qElements );
            m_qElements.SetScale( 4 );
        }
        qTaken.DeleteAll();
    }

    void CQueue::EmptyAll( void )
    {
        IASLIB_QUEUE_LOCK__;
        m_qElements.Clear();
        m_qElements.Compact();
    }

    void CQueue::SetScale( size_t nScale )
    {
        IASLIB_QUEUE_LOCK__;
        m_qElements.SetScale( nScale );
    }

    CIterator *CQueue::Enumerate( void )
    {
        return new CQueueIterator( this );
    }
} // End of Namespace IASLib
