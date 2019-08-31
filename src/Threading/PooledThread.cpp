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
#include "Date.h"
#include "ThreadException.h"

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
            CPooledThread  *m_pParent;

            CRunThread( CPooledThread *pParent, const char *name, int nNumber ) : CThread( (const char *)CString::FormatString("%s_%d", (const char *)name, nNumber ), true, true, false, false )
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

            void *Run( void )
            {
                while ( ! m_bShutdown )
                {
                    m_RunMutex.Lock();
                    if ( m_pActiveTask )
                    {
                        m_pActiveTask->setRunning();
                        CObject *result = NULL;
                        try
                        {
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
                    }
                    else
                    {
                        if ( ! m_bShutdown )
                            printf( "In thread CPooledThread.Run(): %s - Thread unlocked without active task.\n", (const char *)m_strThreadName );
                        else
                        {
                            printf( "Worker Thread %s shutting down.\n", (const char *)m_strThreadName );
                        }
                    }
                }
                //printf( "Thread exiting.\n" );
                m_pParent->m_bThreadInShutdown = true;
                Millisleep(50);
                return NULL;
            }

            bool setTask( CThreadTask *pActiveTask )
            {
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

            virtual void RequestShutdown( void )
            {
                CThread::RequestShutdown();
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
        m_ptThread->RequestShutdown();
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
        if ( ! m_bThreadInShutdown )
            m_pParent->taskComplete( this );
    }

} // Namespace IASLib

#endif // IASLIB_MULTI_THREADED__

