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

#ifndef IASLIB_SEMAPHORE_H__
#define IASLIB_SEMAPHORE_H__

#include "ThreadPlatform.h"

#ifdef IASLIB_PTHREAD__
#include <pthread.h>
    // A counting semaphore from a mutex and condition variable: unnamed
    // POSIX semaphores (sem_init) are not supported on macOS.
struct IASLibPthreadSemaphore
{
    pthread_mutex_t mutex;
    pthread_cond_t  condition;
    unsigned int    nCount;
};
#define SEMAPHORE_T IASLibPthreadSemaphore
#endif

#ifdef IASLIB_WIN32__
#include "../BaseTypes/Platform.h"
#define SEMAPHORE_T HANDLE
#endif

#ifdef IASLIB_MULTI_THREADED__

namespace IASLib
{
    class CSemaphore
    {
        protected:
            SEMAPHORE_T         m_threadSemaphore;
        public:
                                CSemaphore( unsigned int nValue );
            virtual            ~CSemaphore( void );

                // Waits until the count is above zero, then takes one.
            void                Wait( void );
                // The same, giving up (false) after nTimeoutMs.
            bool                TimedWait( int nTimeoutMs );
                // Adds one (waking a waiter).
            void                Post( void );
                // Takes one if there is one, without waiting.
            bool                TryWait( void );

        private:
                                CSemaphore( const CSemaphore & );
            CSemaphore         &operator =( const CSemaphore & );
    };
} // Namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_SEMAPHORE_H__
