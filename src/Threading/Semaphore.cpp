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
        m_threadSemaphore = CreateSemaphore( NULL, nValue, 65535, NULL );
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
        if ( WaitForSingleObject( m_threadSemaphore, 0 ) != WAIT_TIMEOUT )
            return true;
    #endif

        return false;
    }
} // namespace IASLib

#endif