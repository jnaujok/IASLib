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
    class CThreadMonitor : public CThread
    {
        protected:
            CArray                  m_aThreads;
            int                     m_nTotalThreads;
            int                     m_nCurrentThreads;
            int                     m_nPeakThreads;
            CMutex                  m_mutexArray;

            static bool             m_bInitialized;
            static CThreadMonitor  *m_pThreadMonitor;

        public:
                                    CThreadMonitor( void );
            virtual                ~CThreadMonitor( void );

                                    DEFINE_OBJECT(CThreadMonitor);

            static CThreadMonitor  *GetThreadMonitor( void );

            virtual void           *Run( void );

            bool                    AddThread( CThread *pThread, const char *strIdentifier );

            bool                    RemoveThread( CThread *pThread );

            bool                    IsThreadActive( CThread *pThread );

            int                     GetThreadCount( void ) { return m_nCurrentThreads; }
            int                     GetPeakThreads( void ) { return m_nPeakThreads; }
            int                     GetTotalThreads( void ) { return m_nTotalThreads; }

            const char             *GetThreadIdentifier( int nThread );
            const char             *GetThreadIdentifier( CThread *pThread );

            CString                 GetThreadData( int nThread );
            CString                 GetThreadData( CThread *pThread );

            void                    SetThreadTimeout( CThread *pThread, int nSeconds );
            int                     GetThreadTimeout( CThread *pThread );
            void                    ResetTimeout( CThread *pThread );

            CThread                *FindThread( THREAD_T ptThreadID );

            CThread                *GetThread( int nThread );
            virtual unsigned long   GetCapabilities( void ) { return 0; }
    };
} // end of namespace IASLib

#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREADMONITOR_H__
