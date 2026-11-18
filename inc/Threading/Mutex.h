/*
 * Mutex Class
 *
 *  This class wraps a mutex and hides the platform-specific details
 * of how that mutex operates. This let's us do cross-platform threaded
 * programming without worrying about the little nagging details of
 * how threads and mutexes and the like work on each platform.
 *
 *  The mutex is recursive on every platform: the thread that holds it
 * can lock it again, and it is released when that thread has unlocked
 * it as many times as it locked it. (A POSIX recursive mutex, or a
 * critical section on Windows.)
 *
 *=======================================================================
 *  Please Note: Because of the basic level of this class, and it's use
 * internally in the CObject class, it was decided not to derive a mutex
 * (and semaphores) from the CObject class. This precludes their use in
 * a situation where a CObject can be passed.
 *=======================================================================
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/17/1995
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MUTEX_H__
#define IASLIB_MUTEX_H__

#include "ThreadPlatform.h"

#include <atomic>

#ifndef IASLIB_MULTI_THREADED__
    #define MUTEX_T int
#else
    #ifdef IASLIB_PTHREAD__
        #include <pthread.h>
        #define MUTEX_T pthread_mutex_t
    #endif

    #ifdef IASLIB_WIN32__
        #include "../BaseTypes/Platform.h"
        #define MUTEX_T CRITICAL_SECTION
    #endif
#endif

namespace IASLib
{
    class CMutex
    {
        protected:
            MUTEX_T             m_Mutex;
            std::atomic<int>    m_nLocked;      // how deeply the owner holds it

        public:
                                CMutex( void );
                                CMutex( bool bLocked );
            virtual            ~CMutex( void );

            void                Lock( void );
                // Locks only if no other thread holds the mutex, returning
                // whether it did.
            bool                TryLock( void );
            void                Unlock( void );
                // Whether some thread holds the mutex (only a hint: another
                // thread may lock or unlock it straight after).
            bool                IsLocked( void ) const;

        private:
            void                Init( void );
                                CMutex( const CMutex & );
            CMutex             &operator =( const CMutex & );
    };
        // Locks a mutex for as long as it exists:
        //      { CMutexLock oLock( m_mutex ); ... }
    class CMutexLock
    {
            CMutex             &m_mutex;
        public:
            explicit            CMutexLock( CMutex &mutex ) : m_mutex( mutex ) { m_mutex.Lock(); }
                               ~CMutexLock( void ) { m_mutex.Unlock(); }
        private:
                                CMutexLock( const CMutexLock & );
            CMutexLock         &operator =( const CMutexLock & );
    };
} // End of Namespace IASLib

#endif // IASLIB_MUTEX_H__
