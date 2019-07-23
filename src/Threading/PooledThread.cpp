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

    #include "PooledThread.h"
    #include "ThreadPool.h"
    #include "../../inc/BaseTypes/Date.h"

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

            CRunThread( CPooledThread *pParent, const char *name, int nNumber ) : CThread( (const char *)CString::FormatString("Pooled_Thread_%s_%d", (const char *)name, nNumber ), true, false, true, false )
            {
                m_pActiveTask = NULL;
                // We always lock the mutex to start, to ensure we'll block waiting for a task.
                m_RunMutex.Lock();
                m_nTimeout = 0;
                m_dttStartTime.SetToCurrent();
                m_bShutdown = false;
                m_pParent = pParent;

                printf( "Starting Thread: %s:%d\n", name, nNumber );
                // Start the thread, which will immediately go into a suspend because of the
                // locked mutex until a task is set.
                this->Resume();
            }

            void *Run( void )
            {
                printf( "In thread CPooledThread.Run(): %s - Run\n", (const char *)m_strThreadName );
                while ( ! m_bShutdown )
                {
                    printf( "In thread CPooledThread.Run(): %s - Lock\n", (const char *)m_strThreadName );
                    m_RunMutex.Lock();
                    printf( "In thread CPooledThread.Run(): %s - Work loop\n", (const char *)m_strThreadName );
                    if ( m_pActiveTask )
                    {
                        m_pActiveTask->setRunning();
                        CObject *result = NULL;
                        try
                        {
                            printf( "In thread CPooledThread.Run(): %s - Run Task\n", (const char *)m_strThreadName );
                            result = m_pActiveTask->Run();
                            m_pActiveTask->setComplete();
                        }
                        catch(const std::exception& e)
                        {
                            m_pActiveTask->setException();
                            std::cerr << e.what() << '\n';
                        }
                        m_pParent->SetResult( result );
                        m_pActiveTask = NULL;
                        printf( "In thread CPooledThread.Run(): %s - Run Task Complete/Looping\n", (const char *)m_strThreadName );
                    }
                    else
                    {
                        printf( "In thread CPooledThread.Run(): %s - Thread unlocked without active task.\n", (const char *)m_strThreadName );
                    }
                }
                return NULL;
            }

            bool setTask( CThreadTask *pActiveTask )
            {
                printf( "In thread CPooledThread.setTask(): %s - Setting Active Task\n", (const char *)m_strThreadName );

                if ( m_pActiveTask == NULL )
                {
                    m_pActiveTask = pActiveTask;
                    m_RunMutex.Unlock();
                    return true;
                }
                return false;
            }

            virtual unsigned long   GetCapabilities( void )
            {
                return CapabilityFlags::STATE | CapabilityFlags::SLEEP | CapabilityFlags::SUSPEND;
            }

            void shutdown( void )
            {
                m_bShutdown = true;
                if ( ! m_pActiveTask )
                    m_RunMutex.Unlock();
            }
    };

    IMPLEMENT_OBJECT( CPooledThread, CObject );

    CPooledThread::CPooledThread( CThreadPool *parent, const char *strThreadName, int nThreadNumber  )
    {
        m_assignedTask = NULL;
        m_pResult = NULL;
        m_pParent = parent;
        m_nTimeoutSeconds = 0;
        m_bThreadInShutdown = false;
        m_ptThread = new CRunThread( this, strThreadName, nThreadNumber );
        m_strThreadName = m_ptThread->GetName();
    }

    CPooledThread::~CPooledThread( void )
    {
        delete m_ptThread;
    }

    void CPooledThread::SetTimeout( int nSeconds )
    {
        m_nTimeoutSeconds = nSeconds;
    }

     void CPooledThread::ShutdownThread()
    {
        m_bThreadInShutdown = true;
        m_ptThread->shutdown();
    }

    void CPooledThread::ExitThread( int errorCode )
    {
        if ( errorCode != 0 )
        {
            SetResult( new CString( CString::FormatString( "Error Code: %d", errorCode ) ) );
        }
    }

    CThread *CPooledThread::GetThread( void )
    {
        return m_ptThread;
    }

    bool CPooledThread::SetTask( CThreadTask *task )
    {
        m_assignedTask = task;
        return m_ptThread->setTask( m_assignedTask );
    }

    void CPooledThread::Join( void )
    {
        m_ptThread->Join();
    }

    void CPooledThread::SetResult( CObject *pResult )
    {
        m_pResult = pResult;
        m_pParent->taskComplete( this );
    }

} // Namespace IASLib

#endif // IASLIB_MULTI_THREADED__

