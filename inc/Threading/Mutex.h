/*
 * Mutex Class
 *
 *  This class wraps a mutex and hides the platform-specific details
 * of how that mutex operates. This let's us do cross-platform threaded
 * programming without worrying about the little nagging details of
 * how threads and mutexes and the like work on each platform.
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

#ifndef IASLIB_MULTI_THREADED__
    #pragma message( "Mutex Type for Non-threaded" )
    #define MUTEX_T int
#else
    #ifdef IASLIB_SUN__
        #include <thread.h>
        #define MUTEX_T mutex_t
    #endif

    #ifdef IASLIB_DEC__
        #include <pthread.h>
        #define MUTEX_T pthread_mutex_t
    #endif

    #ifdef IASLIB_PTHREAD__
        #include <pthread.h>
        #define MUTEX_T pthread_mutex_t
    #endif

    #ifdef IASLIB_WIN32__
        #include <windows.h>
        #define MUTEX_T HANDLE
        #pragma message( "Mutex Type: Windows" )
    #endif
#endif

namespace IASLib
{
    class CMutex
    {
        protected:
            MUTEX_T             m_Mutex;
            int                 m_nLocked;

        public:
                                CMutex( void );
                                CMutex( bool bLocked );
            virtual            ~CMutex( void );

            void                Lock( void );
            void                Unlock( void );
            bool                IsLocked( void );
    };
} // End of Namespace IASLib

#endif // IASLIB_MUTEX_H__
