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
    void CMutex::Init( void )
    {
        m_nLocked = 0;
    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 0;
    #else
    #ifdef IASLIB_PTHREAD__
        pthread_mutexattr_t attributes;
        pthread_mutexattr_init( &attributes );
        pthread_mutexattr_settype( &attributes, PTHREAD_MUTEX_RECURSIVE );
        pthread_mutex_init( &m_Mutex, &attributes );
        pthread_mutexattr_destroy( &attributes );
    #endif

    #ifdef IASLIB_WIN32__
        InitializeCriticalSection( &m_Mutex );
    #endif
    #endif
    }

    CMutex::CMutex( void )
    {
        Init();
    }

    CMutex::CMutex( bool bLocked )
    {
        Init();
        if ( bLocked )
            Lock();
    }

    CMutex::~CMutex( void )
    {
    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 0;
    #else
    #ifdef IASLIB_PTHREAD__
        pthread_mutex_destroy( &m_Mutex );
    #endif

    #ifdef IASLIB_WIN32__
        DeleteCriticalSection( &m_Mutex );
    #endif
    #endif
    }

    void CMutex::Lock( void )
    {
    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 1;
    #else
    #ifdef IASLIB_PTHREAD__
        pthread_mutex_lock( &m_Mutex );
    #endif

    #ifdef IASLIB_WIN32__
        EnterCriticalSection( &m_Mutex );
    #endif
    #endif
        m_nLocked++;
    }

    bool CMutex::TryLock( void )
    {
    #ifndef IASLIB_MULTI_THREADED__
        m_Mutex = 1;
    #else
    #ifdef IASLIB_PTHREAD__
        if ( pthread_mutex_trylock( &m_Mutex ) != 0 )
            return false;
    #endif

    #ifdef IASLIB_WIN32__
        if ( ! TryEnterCriticalSection( &m_Mutex ) )
            return false;
    #endif
    #endif
        m_nLocked++;
        return true;
    }

    void CMutex::Unlock( void )
    {
        ASSERT( 0 != m_nLocked );
            // Update the count while we still hold the mutex; another thread
            // can acquire it as soon as it is released.
        m_nLocked--;

    #ifndef IASLIB_MULTI_THREADED__
        if ( m_nLocked == 0 )
            m_Mutex = 0;
    #else
    #ifdef IASLIB_PTHREAD__
        pthread_mutex_unlock( &m_Mutex );
    #endif

    #ifdef IASLIB_WIN32__
        LeaveCriticalSection( &m_Mutex );
    #endif
    #endif
    }

    bool CMutex::IsLocked( void ) const
    {
        return m_nLocked > 0;
    }

} // namespace IASLib
