/*
 * Thread Pool Class
 *
 *  A pool of threads that run tasks. See ThreadPool.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_MULTI_THREADED__

#include "ThreadPool.h"
#include "PooledThread.h"
#include "Semaphore.h"
#include "ThreadException.h"

#include <chrono>

namespace IASLib
{
        // Hands queued tasks to free threads whenever something changes (a
        // task is added, or one finishes).
    class CQueueingThread : public CThread
    {
        private:
            CSemaphore      m_semChanged;
            CThreadPool    *m_pParent;

        public:
            CQueueingThread( CThreadPool *parent )
                : CThread( "ThreadPoolQueueingThread", true, false, true ), m_semChanged( 0 ), m_pParent( parent )
            {
            }

            virtual void *Run( void )
            {
                for ( ;; )
                {
                    m_semChanged.Wait();
                    if ( m_bShutdown )
                        break;
                    m_pParent->dispatch();
                }
                return NULL;
            }

            virtual void RequestShutdown( void )
            {
                CThread::RequestShutdown();
                m_semChanged.Post();
            }

            virtual unsigned long GetCapabilities( void )
            {
                return CThread::SLEEP;
            }

            void signalQueue( void )
            {
                m_semChanged.Post();
            }
    };

    IMPLEMENT_OBJECT( CThreadPool, CObject );

    CThreadPool::CThreadPool( size_t maximumThreads, bool storeResults, bool retainTasks )
        : m_nTotalThreads( ( maximumThreads > 0 ) ? maximumThreads : 1 ), m_nCurrentThreads( 0 ), m_nPeakThreads( 0 ),
          m_hashResults( CHash::NORMAL ), m_pQueueingThread( NULL ), m_bStoreResults( storeResults ),
          m_bRetainTasks( retainTasks ), m_bInShutdown( false )
    {
        for ( size_t nX = 0; nX < m_nTotalThreads; nX++ )
        {
            m_aAvailableThreads.Push( new CPooledThread( this, "Pooled_Thread_", (int)nX ) );
        }
        m_pQueueingThread = new CQueueingThread( this );
        m_pQueueingThread->Resume();
    }

    CThreadPool::~CThreadPool( void )
    {
        m_bInShutdown = true;

            // No more dispatching.
        m_pQueueingThread->RequestShutdown();
        m_pQueueingThread->Join();
        delete m_pQueueingThread;
        m_pQueueingThread = NULL;

            // Let running tasks finish, then stop every thread. (Outside the
            // lock: a finishing task takes it.)
        TArray<CPooledThread *> aThreads;
        {
            CMutexLock oLock( m_mutexArray );
            for ( size_t nX = 0; nX < m_aBusyThreads.Length(); nX++ )
                aThreads.Push( m_aBusyThreads[nX] );
            for ( size_t nX = 0; nX < m_aAvailableThreads.Length(); nX++ )
                aThreads.Push( m_aAvailableThreads[nX] );
            m_aBusyThreads.Clear();
            m_aAvailableThreads.Clear();
        }
        for ( size_t nX = 0; nX < aThreads.Length(); nX++ )
            aThreads[nX]->ShutdownThread();
        for ( size_t nX = 0; nX < aThreads.Length(); nX++ )
            delete aThreads[nX];        // joins it

            // Tasks that never ran.
        m_qTaskQueue.DeleteAll();
    }

    void CThreadPool::setStatus( CThreadTask *pTask, CThreadTask::TASK_STATUS eStatus )
    {
            // (The lock is held.)
        m_hashStatus.Put( pTask->GetIdentifier(), (int)eStatus );
    }

    bool CThreadPool::AddTask( CThreadTask *task )
    {
        if ( task == NULL || m_bInShutdown )
            return false;

        bool bFree;
        {
            CMutexLock oLock( m_mutexArray );
            task->addToQueue();
            setStatus( task, CThreadTask::QUEUED );
            m_qTaskQueue.Push( task );
            bFree = ! m_aAvailableThreads.IsEmpty();
        }
        m_pQueueingThread->signalQueue();
        return bFree;
    }

    void CThreadPool::dispatch( void )
    {
        CMutexLock oLock( m_mutexArray );
        while ( ! m_bInShutdown && ! m_aAvailableThreads.IsEmpty() && m_qTaskQueue.GetLength() > 0 )
        {
            CThreadTask *pTask = (CThreadTask *)m_qTaskQueue.Pop();
            if ( pTask == NULL )
                break;
            CPooledThread *pThread = m_aAvailableThreads.Pop();
            m_aBusyThreads.Push( pThread );
            size_t nBusy = ++m_nCurrentThreads;
            if ( nBusy > m_nPeakThreads )
                m_nPeakThreads = nBusy;
            setStatus( pTask, CThreadTask::RUNNING );
            pThread->SetTask( pTask );
        }
    }

        // On a pooled thread, when its task has finished.
    void CThreadPool::taskComplete( CPooledThread *thread )
    {
        {
            CMutexLock oLock( m_mutexArray );
            CThreadTask *pTask = thread->GetTask();
            CObject *pResult = thread->GetResult();
            if ( pTask != NULL )
            {
                setStatus( pTask, pTask->GetStatus() );
                if ( m_bStoreResults && pResult != NULL )
                    m_hashResults.Push( pTask->GetIdentifier(), pResult, true );
                else
                    delete pResult;
                if ( ! m_bRetainTasks )
                    delete pTask;
            }
            else
            {
                delete pResult;
            }
            thread->ResetThread();

            if ( ! m_bInShutdown )
            {
                size_t nAt = m_aBusyThreads.Find( thread );
                if ( nAt == NOT_FOUND )
                    IASLIB_THROW_THREAD_EXCEPTION( "An attempt was made to release an unknown thread to the thread pool." );
                m_aBusyThreads.Remove( nAt );
                m_aAvailableThreads.Push( thread );
                m_nCurrentThreads--;
            }
        }
        if ( ! m_bInShutdown && m_pQueueingThread != NULL )
            m_pQueueingThread->signalQueue();
    }

    CThreadTask::TASK_STATUS CThreadPool::TaskStatus( const char *strIdentifier )
    {
        CMutexLock oLock( m_mutexArray );
        const int *pStatus = m_hashStatus.Find( CString( strIdentifier ) );
        return ( pStatus != NULL ) ? (CThreadTask::TASK_STATUS)*pStatus : CThreadTask::UNKNOWN;
    }

    CObject *CThreadPool::GetResults( const char *strIdentifier )
    {
        CMutexLock oLock( m_mutexArray );
        return m_hashResults.Remove( strIdentifier );
    }

    bool CThreadPool::WaitUntilIdle( int nTimeoutMs )
    {
        std::chrono::steady_clock::time_point tpEnd = std::chrono::steady_clock::now() + std::chrono::milliseconds( ( nTimeoutMs > 0 ) ? nTimeoutMs : 0 );
        for ( ;; )
        {
            {
                CMutexLock oLock( m_mutexArray );
                if ( m_aBusyThreads.IsEmpty() && m_qTaskQueue.GetLength() == 0 )
                    return true;
            }
            if ( nTimeoutMs >= 0 && std::chrono::steady_clock::now() >= tpEnd )
                return false;
            CThread::Millisleep( 2 );
        }
    }
} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
