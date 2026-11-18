/**
 * @file PooledThread.h
 * @brief CPooledThread, one thread of a CThreadPool.
 *
 * The original header comment follows.
 */
/*
 *  Pooled Thread Class
 *
 *      This class encapsulates a thread that belongs to a thread pool and can
 * run multiple tasks. It maintains information and state about the thread.
 *
 *  Author: Jeffrey R. Naujok
 *  Created: 5/02/2019
 *  $LOG$
 *
 */

#ifndef IASLIB_POOLEDTHREAD_H__
#define IASLIB_POOLEDTHREAD_H__

#ifdef IASLIB_MULTI_THREADED__

    #include "Thread.h"
    #include "ThreadTask.h"
    #include <atomic>


    namespace IASLib
    {
        // Forward declaration of Thread Pool
        class CThreadPool;
        class CRunThread;

        /// One thread of a CThreadPool, which runs the pool's tasks one at a time.
        ///
        /// The pool creates and owns these; user code only reads them. The
        /// thread itself is a private CRunThread, joinable and owned by this
        /// object (not by the thread monitor).
        class CPooledThread : public CObject
        {
            protected:
                CRunThread     *m_ptThread;         ///< The thread the tasks run on (owned).
                CString         m_strThreadName;    ///< The thread's name (prefix and number).
                std::atomic<CThreadTask *> m_assignedTask;  ///< The task being run, or NULL when idle.
                CObject        *m_pResult;          ///< The last task's result, until the pool takes it.
                std::atomic<bool> m_bThreadInShutdown;  ///< Set once ShutdownThread() has been called.
                int             m_nTimeoutSeconds;  ///< The time limit SetTimeout() stored.
                CThreadPool    *m_pParent;          ///< The pool this thread belongs to.

            public:
                    /// Creates the thread, idle, and starts it waiting for tasks.
                    /// @param parent  The pool it belongs to.
                    /// @param strThreadName  The name prefix; the thread is named
                    ///                       this followed by @p nNumber.
                    /// @param nNumber  The thread's number in the pool.
                                        CPooledThread( CThreadPool *parent, const char *strThreadName = "Pooled_Thread_", int nNumber = 0 );
                    /// Stops the thread (after its task, if it has one) and
                    /// waits for it.
                virtual                ~CPooledThread( void );

                                        DEFINE_OBJECT(CPooledThread);

                    /// The last task's result (the pool takes it when the task ends).
                CObject *               GetResult( void ) { return m_pResult; }
                    /// Whether a task is assigned to the thread.
                bool                    IsRunning( void ) { return m_assignedTask != NULL; }
                    /// Whether the thread has no task.
                bool                    IsIdle( void ) { return m_assignedTask == NULL; }
                    /// The thread's name.
                const char             *GetName( void ) { return (const char *)m_strThreadName; }
                    /// Stores a time limit for the thread's tasks.
                    /// @param nSeconds  The limit, in seconds.
                    /// @note The value is only stored; nothing enforces it yet.
                void                    SetTimeout( int nSeconds );
                    /// The CThread the tasks run on (still owned by this object).
                CThread                *GetThread( void );

            private:
                friend class CThreadPool;
                friend class CRunThread;
                    /// Hands the thread a task; false if it already has one.
                bool                    SetTask( CThreadTask *task );
                    /// Asks the thread to stop once its current task (if any) ends.
                void                    ShutdownThread( void );
                    /// Waits for the thread to end.
                void                    Join( void );
                    /// Stores a finished task's result and tells the pool (on the run thread).
                void                    SetResult( CObject *pResult );
                    /// The task being run, or NULL.
                CThreadTask *           GetTask( void ) { return m_assignedTask; }
                    /// Clears the task and result, making the thread idle.
                void                    ResetThread( void ) { m_assignedTask = NULL; m_pResult = NULL; }

                                        CPooledThread( const CPooledThread & );        ///< Not copyable.
                CPooledThread          &operator =( const CPooledThread & );          ///< Not copyable.
        };
    } // Namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_POOLEDTHREAD_H__
