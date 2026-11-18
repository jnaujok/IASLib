/**
 * @file ThreadPool.h
 * @brief CThreadPool, a fixed set of threads that run CThreadTask objects.
 *
 * The original header comment follows.
 */
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

    /// A pool of threads that run tasks (CThreadTask) and then wait for the next one.
    ///
    /// Tasks added while every thread is busy wait in a queue, in order,
    /// until a thread is free. A separate queueing thread hands queued
    /// tasks to free threads.
    ///
    /// Ownership: the pool owns the tasks it is given. Each is deleted when
    /// it has run, unless @p retainTasks (then the caller deletes it), and
    /// the ones still queued when the pool is destroyed are deleted without
    /// running. A task's result is deleted too, unless @p storeResults:
    /// then GetResults() hands it over.
    ///
    /// Destroying the pool lets running tasks finish, then stops and joins
    /// its threads. All public methods may be called from any thread.
    ///
    /// @code
    /// CThreadPool oPool( 4, true );
    /// oPool.AddTask( new CSum() );        // identifier "sum"
    /// oPool.WaitUntilIdle();
    /// CObject *pResult = oPool.GetResults( "sum" );  // the caller owns it
    /// delete pResult;
    /// @endcode
    class CThreadPool : public CObject
    {
        protected:
            TArray<CPooledThread *> m_aAvailableThreads;    ///< Idle threads (owned).
            TArray<CPooledThread *> m_aBusyThreads;         ///< Threads running a task (owned).
            size_t                  m_nTotalThreads;        ///< How many threads the pool has.
            std::atomic<size_t>     m_nCurrentThreads;      ///< How many threads are busy.
            std::atomic<size_t>     m_nPeakThreads;         ///< The most threads ever busy at once.
            mutable CMutex          m_mutexArray;           ///< Guards the arrays, queue and hashes.
            CQueue                  m_qTaskQueue;           ///< Tasks waiting for a thread (owned).
            CHash                   m_hashResults;          ///< Stored results by task identifier (with storeResults).
            THash<CString, int>     m_hashStatus;           ///< TASK_STATUS by identifier.
            CQueueingThread        *m_pQueueingThread;      ///< The thread that hands out queued tasks (owned).
            bool                    m_bStoreResults;        ///< Keep task results for GetResults().
            bool                    m_bRetainTasks;         ///< Leave finished tasks to the caller to delete.
            std::atomic<bool>       m_bInShutdown;          ///< Set when the destructor starts.

        public:
                /// Creates the pool and starts its threads.
                /// @param maximumThreads  How many threads to run (0 counts as 1).
                /// @param storeResults  Keep each task's result for GetResults();
                ///                      otherwise results are deleted.
                /// @param retainTasks  Do not delete tasks once they have run.
                                    CThreadPool( size_t maximumThreads = 16, bool storeResults = false, bool retainTasks = false );
                /// Lets running tasks finish, stops and joins the threads, and
                /// deletes tasks that never ran.
            virtual                ~CThreadPool( void );

                                    DEFINE_OBJECT(CThreadPool);

                /// Queues a task (the pool owns it, unless retainTasks).
                /// @param task  The task to run.
                /// @return true if a thread was free for it; false if it waits
                ///         (or, when @p task is NULL or the pool is shutting
                ///         down, was not taken).
            bool                    AddTask( CThreadTask *task );

                /// The latest status of the task with this identifier.
                /// @param strIdentifier  The task's identifier.
                /// @return Its status, or CThreadTask::UNKNOWN if there was none.
            CThreadTask::TASK_STATUS TaskStatus( const char *strIdentifier );
                /// With storeResults: takes the result of the task with this
                /// identifier, which the caller now owns.
                /// @param strIdentifier  The task's identifier.
                /// @return The result, or NULL if there is none (yet).
            CObject                *GetResults( const char *strIdentifier );

                /// How many threads are running a task.
            size_t                  GetActiveThreads( void ) const { return m_nCurrentThreads; }
                /// The most threads that have been busy at once.
            size_t                  GetPeakThreads( void ) const { return m_nPeakThreads; }
                /// How many threads are idle.
            size_t                  GetIdleThreads( void ) const { return m_nTotalThreads - m_nCurrentThreads; }
                /// How many threads the pool has.
            size_t                  GetTotalThreads( void ) const { return m_nTotalThreads; }
                /// How many tasks are waiting for a thread.
            size_t                  GetQueueSize( void ) const { return m_qTaskQueue.Count(); }

                /// Waits until no task is queued or running, or nTimeoutMs
                /// passes.
                /// @param nTimeoutMs  The limit in milliseconds; negative for no limit.
                /// @return true if the pool is idle.
            bool                    WaitUntilIdle( int nTimeoutMs = -1 );

        private:
            friend class CPooledThread;
            friend class CQueueingThread;
                /// Takes a finished task's result and returns the thread to the
                /// idle list (on that pooled thread).
                /// @throws CThreadException if the thread is not one of the busy ones.
            void                    taskComplete( CPooledThread *thread );
                /// Hands queued tasks to free threads.
            void                    dispatch( void );
                /// Records a task's status under its identifier (the lock is held).
            void                    setStatus( CThreadTask *pTask, CThreadTask::TASK_STATUS eStatus );

                                    CThreadPool( const CThreadPool & );    ///< Not copyable.
            CThreadPool            &operator =( const CThreadPool & );    ///< Not copyable.
    };
} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREADPOOL_H__
