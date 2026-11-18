/***********************************************************************
**  Semaphore Class
**
**  Description:
**      This class wraps a semaphore for protecting a limited resource,
** or for doing producer/consumer work.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include "Semaphore.h"
#include <exception>
#include <stdexcept>
#ifdef IASLIB_PTHREAD__
#include <errno.h>
#include <time.h>
#endif

#ifdef IASLIB_MULTI_THREADED__

namespace IASLib
{
    CSemaphore::CSemaphore( unsigned int nValue )
    {
    #ifdef IASLIB_SUN__
        sema_init( &m_threadSemaphore, nValue, USYNC_THREAD, NULL );
    #endif

    #ifdef IASLIB_PTHREAD__
        pthread_mutex_init( &m_threadSemaphore.mutex, NULL );
        pthread_cond_init( &m_threadSemaphore.condition, NULL );
        m_threadSemaphore.nCount = nValue;
    #endif

    #ifdef IASLIB_WIN32__
        m_threadSemaphore = CreateSemaphore( NULL, (LONG)nValue, 0x7FFFFFFF, NULL );
    #endif
    }

    CSemaphore::~CSemaphore( void )
    {
    #ifdef IASLIB_SUN__
        sema_destroy( &m_threadSemaphore );
    #endif
    #ifdef IASLIB_PTHREAD__
        pthread_cond_destroy( &m_threadSemaphore.condition );
        pthread_mutex_destroy( &m_threadSemaphore.mutex );
    #endif
    #ifdef IASLIB_WIN32__
        CloseHandle( m_threadSemaphore );
    #endif
    }

    void CSemaphore::Wait( void )
    {
    #ifdef IASLIB_SUN__
        sema_wait( &m_threadSemaphore );
    #endif

    #ifdef IASLIB_PTHREAD__
        pthread_mutex_lock( &m_threadSemaphore.mutex );
        while ( m_threadSemaphore.nCount == 0 )
            pthread_cond_wait( &m_threadSemaphore.condition, &m_threadSemaphore.mutex );
        m_threadSemaphore.nCount--;
        pthread_mutex_unlock( &m_threadSemaphore.mutex );
    #endif

    #ifdef IASLIB_WIN32__
        WaitForSingleObject( m_threadSemaphore, INFINITE );
    #endif
    }

    void CSemaphore::Post( void )
    {
    #ifdef IASLIB_SUN__
        sema_post( &m_threadSemaphore );
    #endif

    #ifdef IASLIB_PTHREAD__
        pthread_mutex_lock( &m_threadSemaphore.mutex );
        m_threadSemaphore.nCount++;
        pthread_cond_signal( &m_threadSemaphore.condition );
        pthread_mutex_unlock( &m_threadSemaphore.mutex );
    #endif

    #ifdef IASLIB_WIN32__
        ReleaseSemaphore( m_threadSemaphore, 1, NULL );
    #endif
    }

    bool CSemaphore::TimedWait( int nTimeoutMs )
    {
        if ( nTimeoutMs < 0 )
            nTimeoutMs = 0;
    #ifdef IASLIB_PTHREAD__
        bool bTaken = false;
        struct timespec tsDeadline;
        clock_gettime( CLOCK_REALTIME, &tsDeadline );
        tsDeadline.tv_sec += nTimeoutMs / 1000;
        tsDeadline.tv_nsec += (long)( nTimeoutMs % 1000 ) * 1000000L;
        if ( tsDeadline.tv_nsec >= 1000000000L )
        {
            tsDeadline.tv_sec++;
            tsDeadline.tv_nsec -= 1000000000L;
        }
        pthread_mutex_lock( &m_threadSemaphore.mutex );
        while ( m_threadSemaphore.nCount == 0 )
        {
            if ( pthread_cond_timedwait( &m_threadSemaphore.condition, &m_threadSemaphore.mutex, &tsDeadline ) == ETIMEDOUT )
                break;
        }
        if ( m_threadSemaphore.nCount > 0 )
        {
            m_threadSemaphore.nCount--;
            bTaken = true;
        }
        pthread_mutex_unlock( &m_threadSemaphore.mutex );
        return bTaken;
    #endif
    #ifdef IASLIB_WIN32__
        return WaitForSingleObject( m_threadSemaphore, (DWORD)nTimeoutMs ) == WAIT_OBJECT_0;
    #endif
    }

    bool CSemaphore::TryWait( void )
    {
    #ifdef IASLIB_SUN__
        if ( sema_trywait( &m_threadSemaphore ) == 0 )
        {
            return true;
        }
    #endif

    #ifdef IASLIB_PTHREAD__
        bool bTaken = false;
        pthread_mutex_lock( &m_threadSemaphore.mutex );
        if ( m_threadSemaphore.nCount > 0 )
        {
            m_threadSemaphore.nCount--;
            bTaken = true;
        }
        pthread_mutex_unlock( &m_threadSemaphore.mutex );
        if ( bTaken )
            return true;
    #endif

    #ifdef IASLIB_WIN32__
        if ( WaitForSingleObject( m_threadSemaphore, 0 ) == WAIT_OBJECT_0 )
            return true;
    #endif

        return false;
    }
} // namespace IASLib

#endif