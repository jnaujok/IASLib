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

#ifdef IASLIB_MULTI_THREADED__

    #include "Threading/PooledThread.h"
    #include "Threading/ThreadPool.h"
    #include "BaseTypes/Date.h"
    
namespace IASLib
{
    class CRunThread : public CThread
    {
        public:
            DECLARE_OBJECT( CRunThread, CThread );

            CThreadTask    *m_pActiveTask;
            CMutex          m_RunMutex;
            unsigned int    m_nTimeout;
            CDate           m_dttStartTime;

            virtual void           *Run( void )
            {
                
            }

            void setTask( CThreadTask *pActiveTask )
            {
                m_pActiveTask = pActiveTask;
            }

            virtual unsigned long   GetCapabilities( void )
            {
                return CapabilityFlags::STATE | CapabilityFlags::SLEEP | CapabilityFlags::SUSPEND;
            }
    };

    IMPLEMENT_OBJECT( CPooledThread, CObject );

    CPooledThread::CPooledThread( const char *strThreadName )
    {
        m_ptThread = new CRunThread();
        m_strThreadName = strThreadName;
        m_assignedTask = NULL;
        m_pResult = NULL;
    }

    CPooledThread::~CPooledThread( void )
    {
        delete m_ptThread;
    }

    void CPooledThread::SetTimeout( int nSeconds )
    {
        m_nTimeoutSeconds = nSeconds;
    }

    void CPooledThread::ExitThread( int nExitCode )
    {
        m_bThreadInShutdown = true;
    }

    void CPooledThread::SetTask( CThreadTask *task )
    {
        m_assignedTask = task;
        m_ptThread->startTaskRun();
    }

} // Namespace IASLib

#endif // IASLIB_MULTI_THREADED__

