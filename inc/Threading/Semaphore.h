/**
 * @file Semaphore.h
 * @brief CSemaphore, a counting semaphore.
 *
 * The original header comment follows.
 */
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
    /// A counting semaphore from a mutex and condition variable: unnamed
    /// POSIX semaphores (sem_init) are not supported on macOS.
struct IASLibPthreadSemaphore
{
    pthread_mutex_t mutex;          ///< Guards nCount.
    pthread_cond_t  condition;      ///< Signalled when nCount goes up.
    unsigned int    nCount;         ///< The semaphore's count.
};
    /// The platform's semaphore type (IASLibPthreadSemaphore with POSIX threads).
#define SEMAPHORE_T IASLibPthreadSemaphore
#endif

#ifdef IASLIB_WIN32__
#include "../BaseTypes/Platform.h"
    /// The platform's semaphore type (a semaphore HANDLE on Windows).
#define SEMAPHORE_T HANDLE
#endif

#ifdef IASLIB_MULTI_THREADED__

namespace IASLib
{
    /// A counting semaphore, for guarding a limited resource or for
    /// producer/consumer work.
    ///
    /// Any thread may Post() or Wait(). Not copyable.
    ///
    /// @code
    /// CSemaphore semItems( 0 );
    /// // producer:                // consumer:
    /// semItems.Post();            semItems.Wait();
    /// @endcode
    class CSemaphore
    {
        protected:
            SEMAPHORE_T         m_threadSemaphore;  ///< The platform's semaphore.
        public:
                /// Creates the semaphore with a starting count.
                /// @param nValue  The starting count.
                                CSemaphore( unsigned int nValue );
                /// Destroys the semaphore; no thread may be waiting on it.
            virtual            ~CSemaphore( void );

                /// Waits until the count is above zero, then takes one.
            void                Wait( void );
                /// The same as Wait(), giving up after nTimeoutMs.
                /// @param nTimeoutMs  How long to wait, in milliseconds (less than 0 counts as 0).
                /// @return true if one was taken, false if the time ran out.
            bool                TimedWait( int nTimeoutMs );
                /// Adds one (waking a waiter).
            void                Post( void );
                /// Takes one if there is one, without waiting.
                /// @return true if one was taken.
            bool                TryWait( void );

        private:
                                CSemaphore( const CSemaphore & );      ///< Not copyable.
            CSemaphore         &operator =( const CSemaphore & );      ///< Not copyable.
    };
} // Namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_SEMAPHORE_H__
