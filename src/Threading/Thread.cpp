/***********************************************************************
**  Thread Class
**
**  Description:
**      This class encapsulates a thread. Creating an object of this 
** class creates a new thread. The new object then begins execution at 
** the virtual Run method of the class. Note that this class isn't
** really meant to be created directly, since the Run method of this
** class simply exits the thread. 
**      The real power comes from inheriting from this object and over-
** riding the Run method. This lets you create unique objects that run
** as threads.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include "Thread.h"
#include "ThreadMonitor.h"
#include "Mutex.h"
#include "Object.h"

#ifndef IASLIB_WIN32__
    #include <unistd.h>
    #include <time.h>
#endif

#ifdef IASLIB_MULTI_THREADED__
IASLib::CMutex IASLib::CThread::m_mutexCriticalSection;

extern "C"
{
#ifdef IASLIB_WIN32__
    static DWORD WINAPI ThreadStartRoutine( LPVOID pThreadObject )
    {
            // This is a simple lock-out mechanism to make sure the constructor finishes
            // before we begin the process of running the thread.
        IASLib::CThread::EnterCriticalSection();
        IASLib::CThread::LeaveCriticalSection();

        IASLib::CThread *pThread = (IASLib::CThread *)pThreadObject;
        DWORD   *pRetVal = NULL;

        pThread->SetRunning( true );
        pRetVal = (DWORD *)pThread->Run();
        pThread->SetRunning( false );
        if ( pRetVal )
            return *pRetVal;
        return 0;
    }
#else
    static void *ThreadStartRoutine( void *pThreadObject )
    {
            // This is a simple lock-out mechanism to make sure the constructor finishes
            // before we begin the process of running the thread.
        IASLib::CThread::EnterCriticalSection();
        IASLib::CThread::LeaveCriticalSection();

        IASLib::CThread *pThread = (IASLib::CThread *)pThreadObject;
        void    *pRetVal = NULL;

        pThread->SetRunning( true );

#ifdef IASLIB_DEC__
			// Here we play a trick on DECs to start suspended
		pThread->m_mutexStartSuspended.Lock();
        sleep(0);
		pThread->m_mutexStartSuspended.Unlock();
#endif

        try
        {
            pRetVal = pThread->Run();
        }
        catch ( ... )
        {
            //CObject::ErrorLog( "%x: Exception caught in Thread->Run", (long)pThreadObject );
        }

        pThread->SetRunning( false );
        return pRetVal;
    }
#endif
};

#else
#ifdef IASLIB_WIN32__
#pragma warning(disable:4100)
#endif
#endif // IASLIB_MULTI_THREADED__

namespace IASLib
{

CThread::CThread( const char *strThreadName, bool bStartSuspended, bool bDetached, bool bNoDelete, bool bDaemon )
{
#ifdef IASLIB_MULTI_THREADED__
    CThreadMonitor *pMonitor;
    int nRetCode;

#ifdef IASLIB_DEC__
	if ( bStartSuspended )
	{
		m_mutexStartSuspended.Lock();
	}
#endif

    m_strThreadName = strThreadName;
    m_bShutdown = false;
    m_bNoDelete = bNoDelete;
    m_bDaemon = bDaemon;

    m_bIsRunning = false;
    m_nReturnCode = 0;
    m_bDetached = bDetached;
        // This is a simple lock-out mechanism to make sure the constructor finishes
        // before we begin the process of running the thread.
    if ( ! bStartSuspended )
    {
        CThread::EnterCriticalSection();
    }

#ifdef IASLIB_SUN__
    long lFlags = 0; // THR_BOUND;

    if ( bStartSuspended )
    {
        lFlags |= THR_SUSPENDED;
    }

    if ( m_bDetached )
    {
        lFlags |= THR_DETACHED;
    }

    if ( m_bDaemon )
    {
        lFlags |=  THR_DAEMON;
    }

        // We should use our stack frames here, but they seem to need to be ungodly huge.
    if ( nRetCode = thr_create( NULL, NULL, ThreadStartRoutine, this, lFlags, &m_ptThreadID ) )
    {
        ErrorLog( "Could not instantiate thread! (Error Code: %d)\n", nRetCode );
        if ( ! bStartSuspended )
        {
            CThread::LeaveCriticalSection();
        }
        return;
    }

#endif

#ifdef IASLIB_DEC__
    if ( pthread_attr_init( &m_stAttributes ) )
    {
        ErrorLog( "Could not initialize thread attributes!\n" );
        if ( ! bStartSuspended )
        {
            CThread::LeaveCriticalSection();
        }
        return;
    }

    if ( pthread_attr_setscope( &m_stAttributes, PTHREAD_SCOPE_SYSTEM ) )
    {
        ErrorLog( "Could not set thread contention scope!\n" );
        if ( ! bStartSuspended )
        {
            CThread::LeaveCriticalSection();
        }
        return;
    }

    if ( nRetCode = pthread_create( &m_ptThreadID, &m_stAttributes, ThreadStartRoutine, this ) )
    {
        ErrorLog( "Could not instantiate thread! (Error Code: %d)\n", nRetCode );
        if ( ! bStartSuspended )
        {
            CThread::LeaveCriticalSection();
        }
        return;
    }

    if ( m_bDetached )
    {
        pthread_detach( m_ptThreadID );
    }
#endif

#ifdef IASLIB_WIN32__
    DWORD dwCreateFlags = 0;
    DWORD dwThreadIdentifier;
    if ( bStartSuspended )
        dwCreateFlags = CREATE_SUSPENDED;

    m_ptThreadID = CreateThread( NULL, 0, ThreadStartRoutine, this, dwCreateFlags, &dwThreadIdentifier );

    if ( m_ptThreadID == NULL )
    {
        if ( ! bStartSuspended )
        {
            CThread::LeaveCriticalSection();
        }
        return;
    }

    nRetCode = 0;
#endif

    m_bContained = true;
    m_bIsRunning = true;

        // This prevents recursion in the Thread Monitor. It adds itself to the thread monitor array.
    if ( m_strThreadName != "Thread Monitor" )
    {
        pMonitor = CThreadMonitor::GetThreadMonitor();
        pMonitor->AddThread( this, m_strThreadName );
    }

    if ( ! bStartSuspended )
    {
        CThread::LeaveCriticalSection();
    }
#endif // IASLIB_MULTI_THREADED__
}

CThread::CThread( THREAD_T ptThreadID, const char *strThreadName )
{
#ifdef IASLIB_MULTI_THREADED__
    CThreadMonitor *pMonitor;

    if ( ptThreadID == (THREAD_T)NULL )
    {
#ifdef IASLIB_SUN__
        ptThreadID = thr_self();
#endif

#ifdef IASLIB_DEC__
        ptThreadID = pthread_self();
#endif

#ifdef IASLIB_WIN32__
        ptThreadID = GetCurrentThread();
#endif
    }

    m_ptThreadID = ptThreadID;
    m_strThreadName = strThreadName;
    m_bShutdown = false;

    m_bIsRunning = true;
    m_nReturnCode = 0;
    m_bDetached = false;
    m_bContained = false;

    pMonitor = CThreadMonitor::GetThreadMonitor();
    pMonitor->AddThread( this, m_strThreadName );
#endif // IASLIB_MULTI_THREADED
}

CThread::~CThread( void )
{
#ifdef IASLIB_MULTI_THREADED__
    if ( m_bContained )
    {
        if ( m_bIsRunning )
        {
            Kill();
        }

        if ( ! m_bDetached )
        {
            Join();
        }

#ifdef IASLIB_DEC__
        pthread_attr_destroy( &m_stAttributes );
#endif

#ifdef IASLIB_WIN32__
        CloseHandle( m_ptThreadID );
#endif
    }

    CThreadMonitor *pMonitor = CThreadMonitor::GetThreadMonitor();
    pMonitor->RemoveThread( this );
#endif // IASLIB_MULTI_THREADED
}

void *CThread::Run( void )
{
//    ErrorLog( "Thread Run method was not overridden on object %s.\n", m_strObjectType );
    return NULL;
}

int CThread::Kill( void )
{
#ifdef IASLIB_MULTI_THREADED__
    if ( m_bIsRunning )
    {
#ifdef IASLIB_SUN__
        return -1;
#endif

#ifdef IASLIB_DEC__
        return pthread_cancel( m_ptThreadID );
#endif

#ifdef IASLIB_WIN32__
        if ( TerminateThread( m_ptThreadID, (DWORD)-1 ) )
            return 0;
        return -1;
#endif
    }
#endif // IASLIB_MULTI_THREADED__
    return 0;
}

int CThread::Join( void )
{
#ifdef IASLIB_MULTI_THREADED__
    if ( m_bIsRunning )
    {
        void *pRetVal;
#ifdef IASLIB_SUN__
        if ( thr_join( m_ptThreadID, NULL, &pRetVal ) == 0 )
#endif

#ifdef IASLIB_DEC__
        if ( pthread_join( m_ptThreadID, &pRetVal ) == 0 )
#endif

#ifdef IASLIB_WIN32__
        WaitForSingleObject( m_ptThreadID, INFINITE );

        DWORD dwResult;
        pRetVal = &dwResult;
        if ( GetExitCodeThread( m_ptThreadID, &dwResult ) )
#endif
        {
            if ( pRetVal )
            {
                return *((int *)pRetVal);
            }
            else
            {
                return 0;
            }
        }
    }
#endif // IASLIB_MULTI_THREADED__
    return 0;
}

void CThread::ExitThread( int nExitCode )
{
#ifdef IASLIB_MULTI_THREADED__
#ifdef IASLIB_SUN__
    if ( m_ptThreadID == thr_self() )
    {
        m_nReturnCode = nExitCode;
        if ( m_bIsRunning )
        {
            thr_exit( &m_nReturnCode );
        }
    }
#endif

#ifdef IASLIB_DEC__
    if ( pthread_equal( pthread_self(), m_ptThreadID ) )
    {
        m_nReturnCode = nExitCode;
        if ( m_bIsRunning )
        {
            pthread_exit( &m_nReturnCode );
        }
    }
#endif

#ifdef IASLIB_WIN32__
    if ( m_ptThreadID == GetCurrentThread() )
    {
        m_nReturnCode = nExitCode;
        if ( m_bIsRunning )
        {
            ExitThread( (DWORD)m_nReturnCode );
        }
    }
#endif
#endif
}

void CThread::Sleep( int nSeconds )
{
#ifdef IASLIB_WIN32__
    ::Sleep( (DWORD)( nSeconds * 1000 ) );
#else
    sleep( nSeconds );
#endif
}

void CThread::Millisleep( int milliseconds )
{
#ifdef IASLIB_WIN32__
    ::Sleep( (DWORD)milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}

void CThread::Suspend( void )
{
#ifdef IASLIB_MULTI_THREADED__
#ifdef IASLIB_SUN__
    thr_suspend( m_ptThreadID );
#endif

#ifdef IASLIB_WIN32__
    SuspendThread( m_ptThreadID );
#endif
#endif // IASLIB_MULTI_THREADED__
}

void CThread::Resume( void )
{
#ifdef IASLIB_MULTI_THREADED__

#ifdef IASLIB_SUN__
    thr_continue( m_ptThreadID );
#endif

#ifdef IASLIB_DEC__
		// On DEC we can only start a thread suspended with a mutex trick, so this 
		// whole thing is a big kluge to prevent race conditions.
	m_mutexStartSuspended.Unlock();
#endif

#ifdef IASLIB_WIN32__
    ResumeThread( m_ptThreadID );
#endif

#endif // IASLIB_MULTI_THREADED__
}

bool CThread::Detach( void )
{
#ifdef IASLIB_MULTI_THREADED__
#ifdef IASLIB_DEC__
    if ( ! m_bDetached )
    {
        pthread_detach( m_ptThreadID );
        m_bDetached = true;
        return true;
    }
#endif
#endif // IASLIB_MULTI_THREADED__
    return false;
}

void CThread::SetTimeout( int nSeconds )
{
#ifdef IASLIB_MULTI_THREADED__
    CThreadMonitor *pMonitor = CThreadMonitor::GetThreadMonitor();
    pMonitor->SetThreadTimeout( this, nSeconds );
#endif // IASLIB_MULTI_THREADED__
}

CThread *CThread::Self( void )
{
#ifdef IASLIB_MULTI_THREADED__
    THREAD_T        ptThreadID;
    CThreadMonitor *pMonitor;

#ifdef IASLIB_SUN__
    ptThreadID = thr_self();
#endif

#ifdef IASLIB_DEC__
    ptThreadID = pthread_self();
#endif

#ifdef IASLIB_PTHREAD__
    ptThreadID = pthread_self();
#endif

#ifdef IASLIB_WIN32__
    ptThreadID = GetCurrentThread();
#endif

    pMonitor = CThreadMonitor::GetThreadMonitor();

    return pMonitor->FindThread( ptThreadID );
#else
    return NULL;
#endif // IASLIB_MULTI_THREADED__
}

bool CThread::Equals( THREAD_T ptThread1, THREAD_T ptThread2 )
{
#ifdef IASLIB_MULTI_THREADED__
#ifdef IASLIB_SUN__
    if ( ptThread1 == ptThread2 )
    {
        return true;
    }
#endif

#ifdef IASLIB_DEC__
    if ( pthread_equal( ptThread1, ptThread2 ) )
    {
        return true;
    }
#endif

#ifdef IASLIB_PTHREAD__
    if ( pthread_equal( ptThread1, ptThread2 ) )
    {
        return true;
    }
#endif

#ifdef IASLIB_WIN32__
    if ( ptThread1 == ptThread2 )
    {
        return true;
    }
#endif
#endif
    return false;
}

} // namespace IASLib
