/**
 * @file ThreadMonitor.h
 * @brief CThreadMonitor, which tracks every CThread and deletes finished detached ones.
 *
 * The original header comment follows.
 */
/*
 * Thread Monitor Class
 *
 *  This class provides an internal tool for monitoring the state
 * of threads within the system. In a multi-threaded system, threads
 * can start and stop and you run into the issue that a detached
 * thread may stop running, and no one is responsible for the memory
 * associated with the thread object. In that case, memory is leaked,
 * often along with crucial system resources associated with the
 * thread.
 *  The thread monitor keeps track of all the threads on the system
 * and properly disposes of threads that are done running and set to
 * a detatched state. The monitor can also kill threads (when the
 * capability exists) that have reached some time-out value. Any thread
 * can use this as a "heartbeat" function to assure that a thread 
 * that hangs cannot exhaust system resources.
 *  In order to perform all this functionality, the thread monitor is,
 * itself, a thread and thus also monitors itself. Apart from the obvious 
 * issues of getting a thread monitor to correctly monitor itself, this
 * introduces some odd code in the constructor of the Thread Monitor class.
 *  Obviously there should only be one thread monitor instantiated per 
 * application and the thread monitor constructor will throw an exception 
 * if a thread monitor already exists. 
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
 *	Created: 3/22/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_THREADMONITOR_H__
#define IASLIB_THREADMONITOR_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/IASLib_String.h"
#include "../Collections/Array.h"
#include "Thread.h"
#include "Mutex.h"

#ifdef IASLIB_MULTI_THREADED__

namespace IASLib
{
    /// The one thread that keeps track of every CThread in the process.
    ///
    /// Each CThread registers itself here when it is created and removes
    /// itself when destroyed. Once a second the monitor deletes detached
    /// threads that have finished (unless they are marked NoDelete), and
    /// kills threads that have run past their time limit
    /// (CThread::SetTimeout()). Joinable threads belong to whoever created
    /// them; the monitor never deletes those.
    ///
    /// There is one monitor per process, created on first use by
    /// GetThreadMonitor(); do not create one yourself. The monitor is
    /// itself a detached thread and monitors itself. All methods lock the
    /// monitor's list and may be called from any thread.
    class CThreadMonitor : public CThread
    {
        protected:
            CArray                  m_aThreads;         ///< One record per registered thread (owned).
            int                     m_nTotalThreads;    ///< How many threads have ever been registered.
            int                     m_nCurrentThreads;  ///< How many are registered now.
            int                     m_nPeakThreads;     ///< The most registered at once.
            CMutex                  m_mutexArray;       ///< Guards m_aThreads and the counts.

            static bool             m_bInitialized;     ///< Whether the monitor has been created.
            static CThreadMonitor  *m_pThreadMonitor;   ///< The monitor.

        public:
                /// Creates and starts the monitor. Use GetThreadMonitor() instead.
                                    CThreadMonitor( void );
                /// Deletes the registered threads this process created, then the monitor.
            virtual                ~CThreadMonitor( void );

                                    DEFINE_OBJECT(CThreadMonitor);

                /// The monitor, created by the first call (from whichever thread).
            static CThreadMonitor  *GetThreadMonitor( void );

                /// The monitoring loop: once a second, deletes finished detached
                /// threads and kills timed-out ones, until shut down.
                /// @return NULL.
            virtual void           *Run( void );

                /// Registers a thread (CThread's constructor does this).
                /// @param pThread  The thread; the monitor does not take ownership.
                /// @param strIdentifier  The name to list it under.
                /// @return false if it was already registered.
            bool                    AddThread( CThread *pThread, const char *strIdentifier );

                /// Unregisters a thread (CThread's destructor does this).
                /// @param pThread  The thread.
                /// @return false if it was not registered.
            bool                    RemoveThread( CThread *pThread );

                /// Whether a thread is registered.
                /// @param pThread  The thread.
            bool                    IsThreadActive( CThread *pThread );

                /// How many threads are registered now.
            int                     GetThreadCount( void );
                /// The most threads registered at once.
            int                     GetPeakThreads( void );
                /// How many threads have ever been registered.
            int                     GetTotalThreads( void );

                /// The name a thread is listed under, by its index in the list.
                /// @param nThread  The index, from 0 to GetThreadCount() - 1.
                /// @return The name, or "" if the index is out of range. The
                ///         string belongs to the monitor.
            const char             *GetThreadIdentifier( int nThread );
                /// Same, for a thread given by pointer ("" if it is not registered).
                /// @param pThread  The thread.
            const char             *GetThreadIdentifier( CThread *pThread );

                /// One formatted line about a thread, by its index: its number,
                /// name, start time and run time (and time limit, if it has one).
                ///
                /// The line starts with "*" if the thread has finished, "[" if not.
                /// @param nThread  The index, from 0 to GetThreadCount() - 1.
                /// @return The line, or "" if the index is out of range.
            CString                 GetThreadData( int nThread );
                /// Same, for a thread given by pointer ("" if it is not registered).
                /// @param pThread  The thread.
            CString                 GetThreadData( CThread *pThread );

                /// Sets a thread's time limit; the monitor kills it once it has
                /// run longer (once).
                /// @param pThread  The thread.
                /// @param nSeconds  The limit in seconds; 0 for none.
            void                    SetThreadTimeout( CThread *pThread, int nSeconds );
                /// A thread's time limit.
                /// @param pThread  The thread.
                /// @return The limit in milliseconds (0 for none), or -1 if the
                ///         thread is not registered.
            int                     GetThreadTimeout( CThread *pThread );
                /// Restarts a thread's time limit.
                /// @param pThread  The thread.
                /// @note Declared but not implemented: calling it fails to link.
            void                    ResetTimeout( CThread *pThread );

                /// The registered CThread for a platform thread handle.
                /// @param ptThreadID  The handle.
                /// @return The thread, or NULL if none is registered for it.
            CThread                *FindThread( THREAD_T ptThreadID );

                /// A registered thread, by its index in the list.
                /// @param nThread  The index, from 0 to GetThreadCount() - 1.
                /// @return The thread, or NULL if the index is out of range.
            CThread                *GetThread( int nThread );
                /// The monitor claims no capabilities.
                /// @return 0.
            virtual unsigned long   GetCapabilities( void ) { return 0; }
    };
} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREADMONITOR_H__
