/***********************************************************************
**  Thread Monitor Class
**
**  Description:
**      This class provides a thread monitor that can manage and delete
** thread objects that are no longer running. It can also keep track of
** the threads that are running, and thus provide a more user-friendly
** debugging tool for watching the threads.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include "ThreadMonitor.h"
#include "Date.h"
#include "Exception.h"

#ifdef IASLIB_MULTI_THREADED__
namespace IASLib
{
/***********************************************************************
**  Thread Storage class
**
**      This simple class is used for storing information about threads
** within the thread monitor.
**
**      It keeps track of the thread, a meaningful (Text) name for that
** thread, a monitor assigned ID number, the start time of the thread,
** and a timeout value (which is set to zero for a thread with no time-
** out setting) for terminating run-away threads.
**
***********************************************************************/

class CThreadStorage : public CObject
{
    public:
        CThread    *m_pThread;
        CString     m_strIdentifier;
        int         m_nThreadID;
        CDate       m_dttStartTime;
        int         m_nElapsed;
        CDate       m_dttEndTime;
        int         m_nTimeout;

                    DECLARE_OBJECT( CThreadStorage, CObject );

                    CThreadStorage( CThread *pThread, const char *strIdentifier, int nThreadID )
                    {
                        m_pThread = pThread;
                        m_strIdentifier = strIdentifier;
                        m_nThreadID = nThreadID;
                        m_dttStartTime.SetToCurrent();
                        m_nTimeout = 0;
                        m_nElapsed = -1;
                    }

        virtual    ~CThreadStorage( void )
                    {
                        if ( m_nElapsed == -1 )
                        {
                            CDate   dttNow;
                            m_nElapsed = dttNow.Elapsed( m_dttStartTime );
                        }

                            // For some reason, Thread monitor recurses in the destructor.
                        if ( m_strIdentifier == "Thread Monitor" )
                            return;

                        delete m_pThread;
                    }

        void        SetEndTime( void )
                    {
                        m_dttEndTime.SetToCurrent();
                        m_nElapsed = m_dttEndTime.Elapsed( m_dttStartTime );
                    }
};

/***********************************************************************
**  Static members of Thread Monitor
**
**      These member values are stored statically for several reasons.
** One, to provide a global pointer to the thread monitor. Two, to
** prevent a call to the thread creation process for the monitor to 
** recursively try to create a new monitor. And three, to provide access
** to shutting down the monitor across all instances.
**
***********************************************************************/
bool            CThreadMonitor::m_bInitialized      = false;
CThreadMonitor *CThreadMonitor::m_pThreadMonitor    = NULL;

IMPLEMENT_OBJECT( CThreadMonitor, CThread );

/***********************************************************************
**  Thread Monitor constructor
**
**      Creates the thread monitor.
**
***********************************************************************/
CThreadMonitor::CThreadMonitor( void ) : 
    CThread( "Thread Monitor", true ),  
    m_aThreads(16), 
    m_nTotalThreads(0), 
    m_nCurrentThreads(0),  
    m_nPeakThreads(0),
    m_mutexArray()

{
    m_bInitialized = true;
    m_pThreadMonitor = this;
    AddThread( this, "Thread Monitor" );
    Detach();
    Resume();
}

/***********************************************************************
**  Thread Monitor destructor
**
**      Shuts down and cleans up the thread monitor.
**
***********************************************************************/
CThreadMonitor::~CThreadMonitor( void )
{
    size_t          nCount;
    CArray          aDeleteList;
    CThreadStorage *pStorage;
    CThread        *pThread;

    m_mutexArray.Lock();
    for ( nCount = 1; nCount < m_aThreads.Length(); nCount++ )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nCount );
        if ( ( pStorage->m_pThread != this ) && ( pStorage->m_pThread ) && ( pStorage->m_pThread->IsContained() ) )
        {
            aDeleteList.Push( pStorage->m_pThread );
        }
    }
    m_mutexArray.Unlock();

    for ( nCount = 0; nCount < aDeleteList.Length(); nCount++ )
    {
        pThread = (CThread *)aDeleteList.Get( nCount );
        try
        {
            // Thread monitor is already being deleted... deleting it again is bad.
            if ( strcmp( pThread->GetName(), "Thread Monitor" ) ) 
            {
                delete pThread;
            }
        }
        catch ( CException *pException )
        {
            pException = pException;
//            ErrorLog( "Thread Monitor: Exception thrown on delete. [%s]\n", (const char *)(*pException) );
        }
    }
}

/***********************************************************************
**  GetThreadMonitor method
**
**      This static member function returns a pointer to the global
** thread monitor object. If there is no Thread Monitor object, it 
** creates one. Note the code to prevent re-entry.
**
***********************************************************************/
CThreadMonitor *CThreadMonitor::GetThreadMonitor( void )
{
    if ( ! m_bInitialized )
    {
        m_bInitialized = true;
            // The assignment isn't necessary, but it looks really weird without it.
        m_pThreadMonitor = new CThreadMonitor();
    }

    return m_pThreadMonitor;
}

/***********************************************************************
**  Thread Monitor Main loop
**
**      Does the job of monitoring the threads and makes sure that the
** thread objects are deleted when their main loop completes.
**
***********************************************************************/
void *CThreadMonitor::Run( void )
{
    size_t          nCount;
    CThreadStorage *pStorage;
    CThread        *pThread;
    CDate           dttNow;
    int             nElapsed;
    CArray          aDeleteList;

    while ( ! IsShutdown() )
    {
        nCount = 0;

        m_mutexArray.Lock();

        while ( nCount < m_aThreads.Length() )
        {
            pStorage = (CThreadStorage *)m_aThreads.Get( nCount );

            pThread = pStorage->m_pThread;

                // Threads can mark themselves to not be deleted by the thread
                // monitor. Here we check for this situation
            if ( ! pThread->IsNoDelete() )
            {
                    // We check if the thread has finished (and if it is a thread we created,
                    // as we can't delete a thread we didn't create.
                if ( ( ! pThread->IsRunning() ) && ( pThread->IsContained() ) )
                {
                    if ( pThread->IsDetached() )
                    {
                        aDeleteList.Push( pThread );
                    }
                    else
                    {
                        if ( pStorage->m_nElapsed == -1 )
                        {
                            pStorage->SetEndTime();
                        }
                        dttNow.SetToCurrent();

                            // Threads that are dead must be joined within 30 seconds, or
                            // we're gonna kill 'em anyway.
                        if ( dttNow.Elapsed( pStorage->m_dttEndTime ) > 30000 )
                        {
                            aDeleteList.Push( pThread );
                        }
                    }
                }
                else
                {
                    if ( pStorage->m_nTimeout > 0 )
                    {
                        dttNow.SetToCurrent();

                        nElapsed = dttNow.Elapsed( pStorage->m_dttStartTime );
                        if ( nElapsed > pStorage->m_nTimeout )
                        {
                            printf( "Thread Monitor: Timeout on thread [%s] Elapsed time: %d.%03d", (const char *)pStorage->m_strIdentifier, (nElapsed / 1000), (nElapsed % 1000) );
                            pThread->Kill();
                            aDeleteList.Push( pThread );
                        }
                    }
                }
            } // Thread NoDelete
            nCount++;
        }
        m_mutexArray.Unlock();

        nCount = 0;
        while ( nCount < aDeleteList.Length() )
        {
            pThread = (CThread *)aDeleteList.Get( nCount );
            try
            {
                delete pThread;
            }
            catch ( CException *pException )
            {
                pException=pException;
//                ErrorLog( "Thread Monitor: Exception thrown on delete. [%s]\n", (const char *)(*pException) );
            }
            
            nCount++;
        }

        aDeleteList.EmptyAll();

        Sleep( 1 );
    }

    return NULL;
}

/***********************************************************************
**  AddThread method
**
**      Adds a new thread to the thread monitor. The thread is assigned
** an ID number and the number of total threads is incremented.
**
***********************************************************************/
bool CThreadMonitor::AddThread( CThread *pThread, const char *strIdentifier )
{
    CThreadStorage *pStorage;

    // Check that we don't already have this thread in the active thread array.
    if ( ! IsThreadActive( pThread ) )
    {
        m_mutexArray.Lock();

        pStorage = new CThreadStorage( pThread, strIdentifier, m_nTotalThreads + 1 );

        m_nTotalThreads++;
        m_nCurrentThreads++;

        if ( m_nCurrentThreads > m_nPeakThreads )
        {
            m_nPeakThreads = m_nCurrentThreads;
        }

        m_aThreads.Push( pStorage );

        m_mutexArray.Unlock();

        return true;
    }
    return false;
}

/***********************************************************************
**  RemoveThread method
**
**      Called from a thread destructor, this method removes a thread 
** from the monitor.
**
***********************************************************************/
bool CThreadMonitor::RemoveThread( CThread *pThread )
{
    bool            bRetVal = false;

    if ( IsThreadActive( pThread ) )
    {
        CThreadStorage *pStorage;
        size_t          nCount = 0;

        m_mutexArray.Lock();

        for ( nCount = 0; nCount < m_aThreads.Length(); nCount++ )
        {
            pStorage = (CThreadStorage *)m_aThreads.Get( nCount );
            if ( pStorage->m_pThread == pThread )
            {
                    // We need to null out the thread, because we're already
                    // getting called from the thread's destructor. It would
                    // be bad to delete the thread object twice.
                pStorage->m_pThread = NULL;
                m_aThreads.Delete( nCount );
                m_nCurrentThreads--;
                bRetVal = true;
                break;
            }
        }

        m_mutexArray.Unlock();
    }

    return bRetVal;
}

/***********************************************************************
**  IsThreadActive method
**
**      Checks to see if a thread is still in the active thread array.
**
***********************************************************************/
bool CThreadMonitor::IsThreadActive( CThread *pThread )
{
    CThreadStorage *pStorage;
    size_t          nCount = 0;
    bool            bRetVal = false;

    m_mutexArray.Lock();

    for ( nCount = 0; nCount < m_aThreads.Length(); nCount++ )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nCount );
        if ( pStorage->m_pThread == pThread )
        {
            bRetVal = true;
            break;
        }
    }

    m_mutexArray.Unlock();

    return bRetVal;
}

/***********************************************************************
**  GetThreadIdentifier method
**
**      Given an array index, this function returns the human readable
** (text string) identifier attached to that thread. This is useful for
** enumerating the threads on the system.
**
***********************************************************************/
const char *CThreadMonitor::GetThreadIdentifier( int nThread )
{
    CThreadStorage *pStorage;
    const char     *pRetVal = "";

    m_mutexArray.Lock();
    if ( ( nThread >= 0 ) && ( nThread < (int)m_aThreads.Length() ) )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nThread );

        pRetVal = (const char *)pStorage->m_strIdentifier;
    }
    m_mutexArray.Unlock();

    return pRetVal;
}

/***********************************************************************
**  GetThreadIdentifier method
**
**      Given a thread pointer, this function returns the human readable
** (text string) identifier attached to that thread.
**
***********************************************************************/
const char *CThreadMonitor::GetThreadIdentifier( CThread *pThread )
{
    CThreadStorage *pStorage;
    size_t          nCount = 0;
    const char     *pRetVal = "";

    m_mutexArray.Lock();

    for ( nCount = 0; nCount < m_aThreads.Length(); nCount++ )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nCount );
        if ( pStorage->m_pThread == pThread )
        {
            pRetVal = (const char *)pStorage->m_strIdentifier;
            break;
        }
    }

    m_mutexArray.Unlock();

    return pRetVal;
}

/***********************************************************************
**  GetThreadData method
**
**      Given an array index, this function returns a complete set of
** formatted data for the thread, including ID number, Name, and start
** time for the thread. This is useful for displaying a list of currently
** running threads on the system.
**
***********************************************************************/
CString CThreadMonitor::GetThreadData( int nThread )
{
    CThreadStorage *pStorage;
    CString         strRetVal;
    CDate           dttNow;
    int             nElapsed;
    CString         strRunFlag;

    m_mutexArray.Lock();
    if ( ( nThread >= 0 ) && ( nThread < (int)m_aThreads.Length() ) )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nThread );

        dttNow.SetToCurrent();

        if ( pStorage->m_nElapsed != -1 )
        {
            nElapsed = pStorage->m_nElapsed;
            strRunFlag = "*";
        }
        else
        {
            nElapsed = dttNow.Elapsed( pStorage->m_dttStartTime );
            strRunFlag = "[";
        }

        if ( pStorage->m_nTimeout > 0 )
        {
            strRetVal.Format( "%s%4d] %-40s %-22s %5d.%03d (%5d.%03d)", (const char *)strRunFlag, pStorage->m_nThreadID, (const char *)pStorage->m_strIdentifier, (const char *)(pStorage->m_dttStartTime.PrettyDate() ), (nElapsed / 1000), (nElapsed % 1000), (pStorage->m_nTimeout / 1000), (pStorage->m_nTimeout % 1000) );
        }
        else
        {
            strRetVal.Format( "%s%4d] %-40s %-22s %5d.%03d", (const char *)strRunFlag, pStorage->m_nThreadID, (const char *)pStorage->m_strIdentifier, (const char *)(pStorage->m_dttStartTime.PrettyDate() ), (nElapsed / 1000), (nElapsed % 1000) );
        }
    }
    m_mutexArray.Unlock();

    return strRetVal;
}

/***********************************************************************
**  GetThreadData method
**
**      Given a thread pointer, this function returns a complete set of
** formatted data for the thread, including ID number, Name, and start
** time for the thread. 
**
***********************************************************************/
CString CThreadMonitor::GetThreadData( CThread *pThread )
{
    CThreadStorage *pStorage;
    size_t          nCount = 0;
    CString         strRetVal;
    CDate           dttNow;
    int             nElapsed;
    CString         strRunFlag;

    m_mutexArray.Lock();

    for ( nCount = 0; nCount < m_aThreads.Length(); nCount++ )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nCount );
        if ( pStorage->m_pThread == pThread )
        {
            dttNow.SetToCurrent();
            if ( pStorage->m_nElapsed != -1 )
            {
                nElapsed = pStorage->m_nElapsed;
                strRunFlag = "*";
            }
            else
            {
                nElapsed = dttNow.Elapsed( pStorage->m_dttStartTime );
                strRunFlag = "[";
            }

            if ( pStorage->m_nTimeout > 0 )
            {
                strRetVal.Format( "%s%4d] %-40s %-22s %5d.%03d (%5d.%03d)", (const char *)strRunFlag, pStorage->m_nThreadID, (const char *)pStorage->m_strIdentifier, (const char *)(pStorage->m_dttStartTime.PrettyDate() ), (nElapsed / 1000), (nElapsed % 1000), (pStorage->m_nTimeout / 1000), (pStorage->m_nTimeout % 1000) );
            }
            else
            {
                strRetVal.Format( "%s%4d] %-40s %-22s %5d.%03d", (const char *)strRunFlag, pStorage->m_nThreadID, (const char *)pStorage->m_strIdentifier, (const char *)(pStorage->m_dttStartTime.PrettyDate() ), (nElapsed / 1000), (nElapsed % 1000) );
            }
            break;
        }
    }

    m_mutexArray.Unlock();

    return strRetVal;
}

/***********************************************************************
**  SetThreadTimeout method
**
**      Given a thread pointer, this method sets the number of seconds
** that can elapse before the thread will enter a "timed out" state and
** be terminated.
**
***********************************************************************/
void CThreadMonitor::SetThreadTimeout( CThread *pThread, int nSeconds )
{
    CThreadStorage *pStorage;
    size_t          nCount = 0;

    m_mutexArray.Lock();

    for ( nCount = 0; nCount < m_aThreads.Length(); nCount++ )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nCount );
        if ( pStorage->m_pThread == pThread )
        {
            pStorage->m_nTimeout = nSeconds * 1000;
            break;
        }
    }

    m_mutexArray.Unlock();
}

/***********************************************************************
**  GetThreadTimeout method
**
**      Given a thread pointer, this method returns the number of seconds
** that can elapse before the thread will enter a "timed out" state and
** be terminated.
**
***********************************************************************/
int  CThreadMonitor::GetThreadTimeout( CThread *pThread )
{
    CThreadStorage *pStorage;
    size_t          nCount = 0;
    int             nRetVal = -1;

    m_mutexArray.Lock();

    for ( nCount = 0; nCount < m_aThreads.Length(); nCount++ )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nCount );
        if ( pStorage->m_pThread == pThread )
        {
            nRetVal = pStorage->m_nTimeout;
            break;
        }
    }

    m_mutexArray.Unlock();

    return nRetVal;
}

CThread *CThreadMonitor::FindThread( THREAD_T ptThreadID )
{
    CThreadStorage *pStorage;
    size_t          nCount = 0;
    CThread        *pRetVal = NULL;

    m_mutexArray.Lock();

    for ( nCount = 0; nCount < m_aThreads.Length(); nCount++ )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nCount );
        if ( CThread::Equals( pStorage->m_pThread->GetThreadID(), ptThreadID ) )
        {
            pRetVal = pStorage->m_pThread;
            break;
        }
    }

    m_mutexArray.Unlock();

    return pRetVal;
}

/***********************************************************************
**  GetThread method
**
**      Given an array index, this function returns the thread.
**
***********************************************************************/
CThread *CThreadMonitor::GetThread( int nThread )
{
    CThreadStorage *pStorage;
    CThread        *pRetVal = NULL;

    m_mutexArray.Lock();
    if ( ( nThread >= 0 ) && ( nThread < (int)m_aThreads.Length() ) )
    {
        pStorage = (CThreadStorage *)m_aThreads.Get( nThread );

        pRetVal = pStorage->m_pThread;
    }
    m_mutexArray.Unlock();

    return pRetVal;
}

int CThreadMonitor::GetThreadCount( void ) { return m_nCurrentThreads; }
int CThreadMonitor::GetPeakThreads( void ) { return m_nPeakThreads; }
int CThreadMonitor::GetTotalThreads( void ) { return m_nTotalThreads; }

} // namespace IASLib
#endif //IASLIB_MULTI_THREADED
