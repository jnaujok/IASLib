/*
 *  Pooled Thread Class
 *
 *      This class encapsulates a thread that belongs to a thread pool and can
 * run multiple tasks. It maintains information and state about the thread.
 *
 *  Author: Jeffrey R. Naujok
 *  Created: 5/02/2019
 *  $LOG$
 *
 */

#ifndef IASLIB_POOLEDTHREAD_H__
#define IASLIB_POOLEDTHREAD_H__

#ifdef IASLIB_MULTI_THREADED__

    #include "Thread.h"
    #include "ThreadTask.h"

    namespace IASLib
    {
        class CPooledThread : public CObject
        {
            protected:
                CThread        *m_ptThread;
                CString         m_strThreadName;
                CThreadTask    *m_assignedTask;
                CObject        *m_pResult;

            public:
                                        CPooledThread( const char *strThreadName = "Pooled_Thread_" );
                virtual                ~CPooledThread( void );

                                        DEFINE_OBJECT(CPooledThread);

                CObject *               GetResult( void ) { return m_pResult; }
                bool                    IsRunning( void ) { return m_assignedTask != NULL; }
                bool                    IsIdle( void ) { return m_assignedTask == NULL; }

                const char             *GetName( void ) { return (const char *)m_strThreadName; }

                void                    SetTimeout( int nSeconds );

                CThread                *GetThread( void ) { return m_ptThread; };

            protected:
                void                    ExitThread( int nExitCode );

            private:
                friend class CThreadPool;
                void                    SetTask( CThreadTask *task );
        };

    } // Namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_POOLEDTHREAD_H__
