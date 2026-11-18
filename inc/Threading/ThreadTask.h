/**
 * @file ThreadTask.h
 * @brief CThreadTask, the base class for work run by a CThreadPool.
 *
 * The original header comment follows.
 */
/*
 * Thread Task Class
 *
 *  This class provides a base, abstract class to handle the allocation of
 * threadable tasks within the IAS library. It is recommended to subclass
 * this class with a concrete class for implementing tasks that are meant to
 * run threaded.
 *
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/30/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_THREADTASK_H__
#define IASLIB_THREADTASK_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/IASLib_String.h"
#include "Thread.h"
#include "Mutex.h"
#include <atomic>

#ifdef IASLIB_MULTI_THREADED__

namespace IASLib
{
    /// A piece of work for a CThreadPool to run on one of its threads.
    ///
    /// Derive from it and implement Run(). Give each task an identifier:
    /// the pool reports status and results by identifier. Once added to a
    /// pool, the pool owns the task (see CThreadPool). The status is set by
    /// the pool's threads and may be read from any thread.
    ///
    /// @code
    /// class CSum : public CThreadTask
    /// {
    ///     public:
    ///         CSum( void ) : CThreadTask( "sum" ) {}
    ///         virtual CObject *Run( void ) { ...; return NULL; }
    /// };
    /// @endcode
    class CThreadTask : public CObject
    {
        public:
            /// Where a task is in its life.
            enum TASK_STATUS
            {
                PREQUEUE,       ///< Created, not yet given to a pool.
                QUEUED,         ///< Waiting in the pool's queue.
                RUNNING,        ///< Running on a pool thread.
                SLEEPING,       ///< Sleeping (not set by the pool itself).
                BLOCKED,        ///< Blocked (not set by the pool itself).
                COMPLETE,       ///< Run() returned.
                EXCEPTION,      ///< Run() threw an exception.
                UNKNOWN         ///< No such task (from CThreadPool::TaskStatus()).
            };
        protected:
            CString                 m_strIdentifier;    ///< The task's identifier.
            std::atomic<int>        m_currentStatus;    ///< A TASK_STATUS, set by the pool's threads.

        public:
                /// Creates a task in the PREQUEUE state.
                /// @param strIdentifier  The task's identifier; NULL means "".
                                    CThreadTask( const char *strIdentifier )
                                    {
                                        if ( strIdentifier )
                                        {
                                            m_strIdentifier = strIdentifier;
                                        }
                                        else
                                        {
                                            m_strIdentifier = "";
                                        }

                                        m_currentStatus = PREQUEUE;
                                    };
                /// Destroys the task.
            virtual                ~CThreadTask( void ) {}

                                    DECLARE_OBJECT(CThreadTask, CObject );

                /// The work; the result (which may be NULL) goes to the pool.
                ///
                /// An exception marks the task EXCEPTION; the thread carries on.
                /// @return A new object for the pool to own, or NULL.
            virtual CObject        *Run( void ) = 0;
                /// The task's current status.
            virtual TASK_STATUS     GetStatus( void ) { return (TASK_STATUS)m_currentStatus.load(); }

                /// The task's identifier.
            virtual CString         GetIdentifier( void ) { return m_strIdentifier; }

        private:
            friend class CThreadPool;
            friend class CRunThread;

            void                    addToQueue(void) { m_currentStatus = QUEUED; }        ///< Marks the task QUEUED.
            void                    setRunning( void ) { m_currentStatus = RUNNING; }     ///< Marks the task RUNNING.
            void                    setSleeping( void ) { m_currentStatus = SLEEPING; }   ///< Marks the task SLEEPING.
            void                    setBlocked( void ) { m_currentStatus = BLOCKED; }     ///< Marks the task BLOCKED.
            void                    setComplete( void ) { m_currentStatus = COMPLETE; }   ///< Marks the task COMPLETE.
            void                    setException(void ) { m_currentStatus = EXCEPTION; }  ///< Marks the task EXCEPTION.
    };
} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREADTASK_H__
