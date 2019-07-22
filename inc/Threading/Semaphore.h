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

#ifdef IASLIB_SUN__
#include <thread.h>
#define SEMAPHORE_T sema_t
#endif

#ifdef IASLIB_PTHREAD__
#include <pthread.h>
#include <semaphore.h>
#define SEMAPHORE_T sem_t
#endif

#ifdef IASLIB_WIN32__
#include <windows.h>
#define SEMAPHORE_T HANDLE
#endif

namespace IASLib
{
    class CSemaphore
    {
        protected:
            SEMAPHORE_T         m_threadSemaphore;

        public:
                                CSemaphore( unsigned int nValue );
            virtual            ~CSemaphore( void );

            void                Wait( void );
            void                Post( void );
            bool                TryWait( void );
    };
} // Namespace IASLib
#endif // IASLIB_SEMAPHORE_H__
