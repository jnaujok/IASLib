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
#include "Semaphore.h"

namespace IASLib
{
        // The thread a pooled thread runs its tasks on. It waits on a
        // semaphore for each task (it used to wait on a mutex that another
        // thread unlocked, which mutexes do not allow).
    class CRunThread : public CThread
    {
        public:
            DECLARE_OBJECT( CRunThread, CThread );

            std::atomic<CThreadTask *>  m_pActiveTask;
            CSemaphore                  m_semWork;
            CPooledThread              *m_pParent;

                // Joinable, and owned by the pooled thread (not the monitor).
            CRunThread( CPooledThread *pParent, const char *name, int nNumber )
                : CThread( (const char *)CString::FormatString( "%s%d", ( name != NULL ) ? name : "", nNumber ), true, false, true, false ),
                  m_pActiveTask( NULL ), m_semWork( 0 ), m_pParent( pParent )
            {
                Resume();
            }

            void *Run( void )
            {
                for ( ;; )
                {
                    m_semWork.Wait();
                    CThreadTask *pTask = m_pActiveTask;
                    if ( pTask == NULL )
                    {
                        if ( m_bShutdown )
                            break;
                        continue;
                    }

                    pTask->setRunning();
                    CObject *pResult = NULL;
                    try
                    {
                        pResult = pTask->Run();
                        pTask->setComplete();
                    }
                    catch ( ... )
                    {
                            // Any exception (library exceptions are not
                            // std::exceptions): the task failed, the thread
                            // carries on.
                        pTask->setException();
                    }
                    m_pActiveTask = NULL;
                    m_pParent->SetResult( pResult );
                    if ( m_bShutdown )
                        break;
                }
                return NULL;
            }

            bool setTask( CThreadTask *pTask )
            {
                CThreadTask *pNone = NULL;
                if ( ! m_pActiveTask.compare_exchange_strong( pNone, pTask ) )
                    return false;
                m_semWork.Post();
                return true;
            }

            virtual unsigned long GetCapabilities( void )
            {
                return STATE | SLEEP;
            }

            virtual void RequestShutdown( void )
            {
                CThread::RequestShutdown();
                m_semWork.Post();
            }
    };

    IMPLEMENT_OBJECT( CPooledThread, CObject );

    CPooledThread::CPooledThread( CThreadPool *parent, const char *strThreadName, int nThreadNumber )
        : m_assignedTask( NULL ), m_bThreadInShutdown( false )
    {
        m_pResult = NULL;
        m_pParent = parent;
        m_nTimeoutSeconds = 0;
        m_ptThread = new CRunThread( this, strThreadName, nThreadNumber );
        m_strThreadName = m_ptThread->GetName();
    }

    CPooledThread::~CPooledThread( void )
    {
        ShutdownThread();
        m_ptThread->Join();
        delete m_ptThread;
    }

    void CPooledThread::SetTimeout( int nSeconds )
    {
        m_nTimeoutSeconds = nSeconds;
    }

    void CPooledThread::ShutdownThread( void )
    {
        m_bThreadInShutdown = true;
        m_ptThread->RequestShutdown();
    }

    CThread *CPooledThread::GetThread( void )
    {
        return m_ptThread;
    }

    bool CPooledThread::SetTask( CThreadTask *task )
    {
        m_assignedTask = task;
        if ( ! m_ptThread->setTask( task ) )
        {
            m_assignedTask = NULL;
            return false;
        }
        return true;
    }

    void CPooledThread::Join( void )
    {
        m_ptThread->Join();
    }

        // On the run thread, when a task has finished.
    void CPooledThread::SetResult( CObject *pResult )
    {
        m_pResult = pResult;
        m_pParent->taskComplete( this );
    }
} // Namespace IASLib

#endif // IASLIB_MULTI_THREADED__
