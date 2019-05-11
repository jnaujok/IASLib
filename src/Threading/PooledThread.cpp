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
            bool            m_bShutdown;
            CPooledThread  *m_pParent;

            CRunThread::CRunThread( CPooledThread *pParent, const char *name, int nNumber ) : CThread( CString::FormatString("Pooled_Thread_%s_%d", name, nNumber ), true, false, true, false )
            {
                m_pActiveTask = NULL;
                // We always lock the mutex to start, to ensure we'll block waiting for a task.
                m_RunMutex.Lock();
                m_nTimeout = 0;
                m_dttStartTime.SetToCurrent();
                m_bShutdown = false;
                m_pParent = pParent;
                // Start the thread, which will immediately go into a suspend because of the
                // locked mutex until a task is set.
                this->Resume();
            }

            void *CRunThread::Run( void )
            {
                while ( ! m_bShutdown )
                {
                    m_RunMutex.Lock();
                    CObject *result = m_pActiveTask->Run();
                    m_pParent->SetResult( result );
                    m_pActiveTask = NULL;
                }
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

    CPooledThread::CPooledThread( const char *strThreadName, int nThreadNumber  )
    {
        m_ptThread = new CRunThread( this, strThreadName, nThreadNumber );
        m_strThreadName = m_ptThread->GetName();
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

    void CPooledThread::SetResult( CObject *pResult )
    {
        m_pResult = pResult;
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

