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
 * Copyright (C) 2006, Irene Adler Software, all rights reserved.
 * [A division of BlackStar Programming]
 */

#include "Mutex.h"
#include "Exception.h"

namespace IASLib
{
    CMutex::CMutex( void )
    {
        m_nLocked = 0;

    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 0;
    #else
    #ifdef IASLIB_SUN__
        mutex_init( &m_Mutex, USYNC_THREAD, NULL );
    #endif

    #ifdef IASLIB_DEC__
        pthread_mutex_init( &m_Mutex, NULL );
    #endif

    #ifdef IASLIB_PTHREAD__
        pthread_mutex_init( &m_Mutex, NULL );
    #endif

    #ifdef IASLIB_WIN32__
        m_Mutex = CreateMutex( NULL, FALSE, NULL );
    #endif
    #endif
    }

    CMutex::CMutex( bool bLocked )
    {
        m_nLocked = 0;

    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 0;
    #else
    #ifdef IASLIB_SUN__
        mutex_init( &m_Mutex, USYNC_THREAD, NULL );
    #endif

    #ifdef IASLIB_DEC__
        pthread_mutex_init( &m_Mutex, NULL );
    #endif

    #ifdef IASLIB_PTHREAD__
        pthread_mutex_init( &m_Mutex, NULL );
    #endif

    #ifdef IASLIB_WIN32__
        m_Mutex = CreateMutex( NULL, FALSE, NULL );
    #endif
        if ( bLocked )
            Lock();
    #endif
    }

    CMutex::~CMutex( void )
    {
    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 0;
    #else
    #ifdef IASLIB_SUN__
        mutex_destroy( &m_Mutex );
    #endif

    #ifdef IASLIB_DEC__
        pthread_mutex_destroy( &m_Mutex );
    #endif

    #ifdef IASLIB_PTHREAD__
        pthread_mutex_destroy( &m_Mutex );
    #endif

    #ifdef IASLIB_WIN32__
        CloseHandle( m_Mutex );
    #endif
    #endif
    }

    void CMutex::Lock( void )
    {
    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 1;
    #else
    #ifdef IASLIB_SUN__
        mutex_lock( &m_Mutex );
    #endif

    #ifdef IASLIB_DEC__
        pthread_mutex_lock( &m_Mutex );
    #endif

    #ifdef IASLIB_WIN32__
        WaitForSingleObject( m_Mutex, INFINITE );
    #endif

    #ifdef IASLIB_PTHREAD__
        pthread_mutex_lock( &m_Mutex );
    #endif

    #endif
        ASSERT( 0 == m_nLocked );
        m_nLocked++;
    }

    void CMutex::Unlock( void )
    {
        ASSERT( 0 != m_nLocked );

    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 0;
    #else
    #ifdef IASLIB_SUN__
        mutex_unlock( &m_Mutex );
    #endif

    #ifdef IASLIB_DEC__
        pthread_mutex_unlock( &m_Mutex );
    #endif

    #ifdef IASLIB_PTHREAD__
        pthread_mutex_unlock( &m_Mutex );
    #endif

    #ifdef IASLIB_WIN32__
        //LeaveCriticalSection( &m_Mutex );
        ReleaseMutex( m_Mutex );
    #endif
    #endif
        m_nLocked--;
    }

    bool CMutex::IsLocked( void )
    {
        return m_nLocked > 0;
    }

} // namespace IASLib
