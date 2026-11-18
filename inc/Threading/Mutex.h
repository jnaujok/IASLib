/**
 * @file Mutex.h
 * @brief CMutex, a recursive mutex, and CMutexLock, which holds one for a scope.
 *
 * The original header comment follows.
 */
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
        /// The platform's mutex type (an int when the library is single-threaded).
    #define MUTEX_T int
#else
    #ifdef IASLIB_PTHREAD__
        #include <pthread.h>
            /// The platform's mutex type (pthread_mutex_t with POSIX threads).
        #define MUTEX_T pthread_mutex_t
    #endif

    #ifdef IASLIB_WIN32__
        #include "../BaseTypes/Platform.h"
            /// The platform's mutex type (a CRITICAL_SECTION on Windows).
        #define MUTEX_T CRITICAL_SECTION
    #endif
#endif

namespace IASLib
{
    /// A recursive mutex that hides the platform's own.
    ///
    /// The mutex is recursive on every platform: the thread that holds it
    /// can lock it again, and it is released when that thread has unlocked
    /// it as many times as it locked it (a POSIX recursive mutex, or a
    /// critical section on Windows). In a single-threaded build it only
    /// counts.
    ///
    /// Because CObject uses it, CMutex is not derived from CObject, so it
    /// cannot be passed where a CObject is expected. It is not copyable.
    /// Prefer CMutexLock to pairing Lock() and Unlock() by hand.
    ///
    /// @code
    /// CMutex mutexData;
    /// mutexData.Lock();
    /// // ... use the shared data ...
    /// mutexData.Unlock();
    /// @endcode
    class CMutex
    {
        protected:
            MUTEX_T             m_Mutex;        ///< The platform's mutex.
            std::atomic<int>    m_nLocked;      ///< How deeply the owner holds it.

        public:
                /// Creates an unlocked mutex.
                                CMutex( void );
                /// Creates a mutex, locked by the calling thread if @p bLocked is set.
                /// @param bLocked  Lock it at once.
                                CMutex( bool bLocked );
                /// Destroys the mutex, which must not be held.
            virtual            ~CMutex( void );

                /// Locks the mutex, waiting while another thread holds it.
            void                Lock( void );
                /// Locks only if no other thread holds the mutex, returning
                /// whether it did.
                /// @return true if the calling thread now holds the mutex.
            bool                TryLock( void );
                /// Unlocks the mutex once; the calling thread must hold it.
            void                Unlock( void );
                /// Whether some thread holds the mutex (only a hint: another
                /// thread may lock or unlock it straight after).
            bool                IsLocked( void ) const;

        private:
                /// Sets up the platform mutex (both constructors call it).
            void                Init( void );
                                CMutex( const CMutex & );      ///< Not copyable.
            CMutex             &operator =( const CMutex & );  ///< Not copyable.
    };
        /// Locks a mutex for as long as it exists.
        ///
        /// @code
        /// { CMutexLock oLock( m_mutex ); ... }
        /// @endcode
    class CMutexLock
    {
            CMutex             &m_mutex;    ///< The mutex held.
        public:
                /// Locks @p mutex.
                /// @param mutex  The mutex to hold; it must outlive this object.
            explicit            CMutexLock( CMutex &mutex ) : m_mutex( mutex ) { m_mutex.Lock(); }
                /// Unlocks the mutex.
                               ~CMutexLock( void ) { m_mutex.Unlock(); }
        private:
                                CMutexLock( const CMutexLock & );      ///< Not copyable.
            CMutexLock         &operator =( const CMutexLock & );      ///< Not copyable.
    };
} // End of Namespace IASLib

#endif // IASLIB_MUTEX_H__
