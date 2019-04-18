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

namespace IASLib
{
    CSemaphore::CSemaphore( unsigned int nValue )
    {
    #ifdef IASLIB_SUB__
        sema_init( &m_threadSemaphore, nValue, USYNC_THREAD, NULL );
    #endif

    #ifdef IASLIB_DEC__
        sem_init( &m_threadSemaphore, NULL, nValue );
    #endif

    #ifdef IASLIB_WIN32__
        m_threadSemaphore = CreateSemaphore( NULL, nValue, 65535, NULL );
    #endif
    }

    CSemaphore::~CSemaphore( void )
    {
    #ifdef IASLIB_SUB__
        sema_destroy( &m_threadSemaphore );
    #endif
    #ifdef IASLIB_DEC__
        sem_destroy( &m_threadSemaphore );
    #endif
    #ifdef IASLIB_WIN32__
        CloseHandle( m_threadSemaphore );
    #endif
    }

    void CSemaphore::Wait( void )
    {
    #ifdef IASLIB_SUB__
        sema_wait( &m_threadSemaphore );
    #endif

    #ifdef IASLIB_DEC__
        sem_wait( &m_threadSemaphore );
    #endif

    #ifdef IASLIB_WIN32__
        WaitForSingleObject( m_threadSemaphore, INFINITE );
    #endif
    }

    void CSemaphore::Post( void )
    {
    #ifdef IASLIB_SUB__
        sema_post( &m_threadSemaphore );
    #endif

    #ifdef IASLIB_DEC__
        sem_post( &m_threadSemaphore );
    #endif

    #ifdef IASLIB_WIN32__
        ReleaseSemaphore( m_threadSemaphore, 1, NULL );
    #endif
    }

    bool CSemaphore::TryWait( void )
    {
    #ifdef IASLIB_SUB__
        if ( sema_trywait( &m_threadSemaphore ) == 0 )
        {
            return true;
        }
    #endif

    #ifdef IASLIB_DEC__
        if ( sem_trywait( &m_threadSemaphore ) == 0 )
        {
            return true;
        }
    #endif

    #ifdef IASLIB_WIN32__
        if ( WaitForSingleObject( m_threadSemaphore, 0 ) != WAIT_TIMEOUT )
            return true;
    #endif
        return false;
    }
} // namespace IASLib
