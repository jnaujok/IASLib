/*
 * Thread Pool Class
 *
 *  This class provides a pool of threads that can be allocated, run a
 * task, and then be returned to the pool to wait in standby until needed
 * again.
 *  This allows us to limit the overhead involved in the creation and the
 * subsequent deletion of threads and their contexts. If a task is assigned
 * to a thread pool, when threads are not available to process it, then the
 * task will be queued until such time as a thread becomes available to
 * process that task. If the queue is exhausted, then an exception will
 * be thrown.
 * Tasks can be added with a maximum timeout value. If a timeout is set,
 * a task which takes longer than that amount of time will be interrupted
 * and the thread returned to the pool. Note that this may require a large
 * amount of overhead and could cause performace impacts. Timeouts should
 * not be used as a standard means of terminating tasks.
 *
 * Using a task identifier allows retrieval of task status and task
 * results from the pool, but care must be taken as every task with an
 * identifier will require storage for the result after completion and
 * will not be cleared until retrieved. In other words, if you use an
 * identifier you -MUST- retrieve the results.
 *
 * Note: If the task identifier is passed as NULL, the results of the task
 * will not be stored, but status and results cannot be retrieved.
 *
 * ======================================================================
 * Since this module is based on threads, it is extremely dependent on
 * the underlying threads provided by the system. Although all possible
 * care has been taken to assure that operations are equivalent on all
 * different systems, this dependence may cause differences on different
 * systems.
 * ======================================================================
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/30/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */


#ifdef IASLIB_MULTI_THREADED__

#include "ThreadPool.h"
#include "PooledThread.h"
#include "Semaphore.h"
#include "ThreadException.h"

namespace IASLib
{
    class CQueueingThread : public CThread
    {
        private:
            CSemaphore  m_semQueueAvailable;
            CQueue     *m_pQueue;
            bool        m_bShutdown;
            CThreadPool * m_pParent;

        public:
            CQueueingThread( CQueue *pQueue, CThreadPool *parent ) : CThread( "ThreadPoolQueueingThread", true, false, true ),
                m_semQueueAvailable( 0 )
            {
                m_pQueue = pQueue;
                m_bShutdown = false;
                m_pParent = parent;
            }

            virtual void *Run( void )
            {
                while ( ! m_bShutdown )
                {
                    m_semQueueAvailable.Wait();
                    if ( m_pParent->GetIdleThreads() )
                    {
                        CThreadTask *task = (CThreadTask *)m_pQueue->Pop();
                        if ( ! m_pParent->startTask( task ) )
                        {
                            // Failed to allocate a thread -- push the task back
                            // onto the queue and re-signal the semaphore.
                            m_pQueue->Push(task);
                            m_semQueueAvailable.Post();
                        }
                    }
                    else
                    {
                        // There's no idle threads available. Wait 50 milliseconds and try again.
                        Millisleep(50);
                        m_semQueueAvailable.Post();
                    }
                }

                return NULL;
            }

            virtual unsigned long   GetCapabilities( void )
            {
                return CThread::CapabilityFlags::SLEEP;
            }

            void signalQueue( void )
            {
                m_semQueueAvailable.Post();
            }
    };

    CThreadPool::CThreadPool( size_t maximumThreads, bool storeResults, bool retainTasks ) : m_aAvailableThreads(), m_aBusyThreads(), m_qTaskQueue(), m_hashResults( CHash::NORMAL ), m_hashTaskIds(CHash::NORMAL)
    {
        m_bRetainTasks = retainTasks;
        m_bStoreResults = storeResults;
        m_mutexArray.Lock();
        m_pQueueingThread = new CQueueingThread( &m_qTaskQueue, this );

        for ( size_t nX = 0; nX < maximumThreads; nX++ )
        {
            m_aAvailableThreads.Append( new CPooledThread( this, "Pooled_Thread_", nX ) );
        }
        m_nTotalThreads = maximumThreads;
        m_nCurrentThreads = 0;
        m_nPeakThreads = 0;

        m_mutexArray.Unlock();

        // Release the queueing thread to start running.
        m_pQueueingThread->Resume();
    }

    CThreadPool::~CThreadPool(void)
    {
        for ( size_t nX = 0; nX < m_aBusyThreads.Length(); nX++ )
        {
            CPooledThread *work = (CPooledThread *)m_aBusyThreads.Get( nX );
            work->ShutdownThread();
            work->Join();
            delete work;
        }
    }

    IMPLEMENT_OBJECT(CThreadPool, CObject);

    bool CThreadPool::AddTask( CThreadTask *task )
    {
        bool retVal = false;

        m_mutexArray.Lock();
        m_qTaskQueue.Push( task );
        if ( m_aAvailableThreads.GetLength() )
            retVal = true;
        m_mutexArray.Unlock();
        m_pQueueingThread->signalQueue();
        return retVal;
    }

    CThreadTask::TASK_STATUS CThreadPool::TaskStatus(const char *strIdentifier)
    {
        if ( m_hashTaskIds.HasKey(strIdentifier) )
        {
            CThreadTask *task = (CThreadTask *)m_hashTaskIds.Get( strIdentifier );
            return task->GetStatus();
        }

        return CThreadTask::TASK_STATUS::UNKNOWN;
    }

    CObject *CThreadPool::GetResults(const char *strIdentifier)
    {
        if ( m_hashResults.HasKey( strIdentifier ) )
        {
            return m_hashResults.Remove( strIdentifier );
        }

        return NULL;
    }

    bool CThreadPool::startTask( CThreadTask *task )
    {
        bool retVal = false;
        m_mutexArray.Lock();

        if ( m_aAvailableThreads.Length() )
        {
            CPooledThread *workThread = (CPooledThread *)m_aAvailableThreads.Remove(1);
            m_aBusyThreads.Push( workThread );
            m_nCurrentThreads++;
            if ( m_nCurrentThreads > this->m_nPeakThreads )
            {
                m_nPeakThreads = m_nCurrentThreads;
            }

            workThread->SetTask( task );
            retVal = true;
        }
        m_mutexArray.Unlock();
        return retVal;
    }

    void CThreadPool::taskComplete( CPooledThread *thread )
    {
        m_mutexArray.Lock();

        CThreadTask *pTask = thread->GetTask();
        CObject *result = thread->GetResult();

        if ( m_bStoreResults )
        {
            this->m_hashResults.Push( pTask->GetIdentifier(), result );
        }
        else
        {
            delete result;
        }
        
        if ( ! m_bRetainTasks )
        {
            delete pTask;
        }
        
        thread->ResetThread();

        if ( m_nCurrentThreads > 0 )
        {
            for ( size_t nX = 0; nX < m_aBusyThreads.Length(); nX++ )
            {
                if ( m_aBusyThreads[nX] == thread )
                {
                    m_aBusyThreads.Remove( nX );
                    m_nCurrentThreads--;
                    break;
                }
            }

            m_aAvailableThreads.Push( thread );
        }
        else
        {
            m_mutexArray.Unlock();
            IASLIB_THROW_THREAD_EXCEPTION( "An attempt was made to release an unknown thread to the thread pool.");
        }

        m_mutexArray.Unlock();
    }

} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
