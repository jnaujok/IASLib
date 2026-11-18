/*
 * Thread Pool Class
 *
 *  A pool of threads that run tasks (CThreadTask) and then wait for the
 * next one. Tasks added while every thread is busy wait in a queue, in
 * order, until a thread is free. The pool owns the tasks it is given:
 * each is deleted when it has run, unless retainTasks (then the caller
 * deletes it), and the ones still queued when the pool is destroyed are
 * deleted without running. A task's result is deleted too, unless
 * storeResults: then GetResults( identifier ) hands it over.
 *
 *  Destroying the pool lets running tasks finish, then stops and joins
 * its threads.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_THREADPOOL_H__
#define IASLIB_THREADPOOL_H__

#include "../Collections/Queue.h"
#include "../Collections/Hash.h"
#include "../Templates/TArray.h"
#include "../Templates/THash.h"
#include "Thread.h"
#include "Mutex.h"
#include "ThreadTask.h"
#include "PooledThread.h"

#include <atomic>

#ifdef IASLIB_MULTI_THREADED__

namespace IASLib
{
    class CQueueingThread;

    class CThreadPool : public CObject
    {
        protected:
            TArray<CPooledThread *> m_aAvailableThreads;
            TArray<CPooledThread *> m_aBusyThreads;
            size_t                  m_nTotalThreads;
            std::atomic<size_t>     m_nCurrentThreads;
            std::atomic<size_t>     m_nPeakThreads;
            mutable CMutex          m_mutexArray;
            CQueue                  m_qTaskQueue;
            CHash                   m_hashResults;
            THash<CString, int>     m_hashStatus;       // TASK_STATUS by identifier
            CQueueingThread        *m_pQueueingThread;
            bool                    m_bStoreResults;
            bool                    m_bRetainTasks;
            std::atomic<bool>       m_bInShutdown;

        public:
                                    CThreadPool( size_t maximumThreads = 16, bool storeResults = false, bool retainTasks = false );
            virtual                ~CThreadPool( void );

                                    DEFINE_OBJECT(CThreadPool);

                // Queues a task (the pool owns it, unless retainTasks). True
                // if a thread was free for it; false if it waits (or, when
                // the pool is shutting down, was not taken).
            bool                    AddTask( CThreadTask *task );

                // The latest status of the task with this identifier (UNKNOWN
                // if there was none).
            CThreadTask::TASK_STATUS TaskStatus( const char *strIdentifier );
                // With storeResults: the result of the task with this
                // identifier, which the caller now owns (NULL if none yet).
            CObject                *GetResults( const char *strIdentifier );

            size_t                  GetActiveThreads( void ) const { return m_nCurrentThreads; }
            size_t                  GetPeakThreads( void ) const { return m_nPeakThreads; }
            size_t                  GetIdleThreads( void ) const { return m_nTotalThreads - m_nCurrentThreads; }
            size_t                  GetTotalThreads( void ) const { return m_nTotalThreads; }
            size_t                  GetQueueSize( void ) const { return m_qTaskQueue.Count(); }

                // Waits until no task is queued or running, or nTimeoutMs
                // passes (negative: no limit); true if the pool is idle.
            bool                    WaitUntilIdle( int nTimeoutMs = -1 );

        private:
            friend class CPooledThread;
            friend class CQueueingThread;
            void                    taskComplete( CPooledThread *thread );
                // Hands queued tasks to free threads.
            void                    dispatch( void );
            void                    setStatus( CThreadTask *pTask, CThreadTask::TASK_STATUS eStatus );

                                    CThreadPool( const CThreadPool & );
            CThreadPool            &operator =( const CThreadPool & );
    };
} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREADPOOL_H__
