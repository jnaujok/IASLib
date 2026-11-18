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

#include <chrono>
#include <thread>

#if defined( __GLIBC__ )
    #include <cxxabi.h>
#endif

#ifdef IASLIB_MULTI_THREADED__

IASLib::CMutex IASLib::CThread::m_mutexCriticalSection;

extern "C"
{
#ifdef IASLIB_WIN32__
        // Kill()'s APC: runs on the thread itself, at an alertable wait.
    static void CALLBACK KillThreadApc( ULONG_PTR )
    {
        ExitThread( (DWORD)-1 );
    }

    static DWORD WINAPI ThreadStartRoutine( LPVOID pThreadObject )
    {
        IASLib::CThread *pThread = (IASLib::CThread *)pThreadObject;
        pThread->ThreadMain();
            // pThread may be gone now (a detached thread is deleted by the
            // monitor once it has finished).
        return 0;
    }
#else
    static void *ThreadStartRoutine( void *pThreadObject )
    {
        IASLib::CThread *pThread = (IASLib::CThread *)pThreadObject;
        pThread->ThreadMain();
        return NULL;
    }
#endif
};

namespace IASLib
{
    namespace
    {
            // Marks the thread finished however Run() ends, including
            // pthread_exit() and cancellation, which unwind the stack.
        struct CRunningGuard
        {
            std::atomic<bool> &m_bRunning;
            explicit CRunningGuard( std::atomic<bool> &bRunning ) : m_bRunning( bRunning ) {}
            ~CRunningGuard( void ) { m_bRunning = false; }
        };
    }

    CThread::CThread( const char *strThreadName, bool bStartSuspended, bool bDetached, bool bNoDelete, bool bDaemon )
        : m_ptThreadID(), m_bIsRunning( false ), m_nReturnCode( 0 ), m_bDetached( bDetached ), m_bShutdown( false ),
          m_bContained( false ), m_bNoDelete( bNoDelete ), m_bDaemon( bDaemon ), m_strThreadName( strThreadName ),
          m_bReleased( false ), m_bAbandoned( false ), m_bLeftGate( false ), m_bReaped( false )
    {
    #ifdef IASLIB_WIN32__
        m_dwThreadID = 0;
    #endif

            // Running from now: a short thread can finish (and clear the
            // flag) before the call that creates it even returns.
        m_bIsRunning = true;

    #ifdef IASLIB_PTHREAD__
        memset( &m_stAttributes, 0, sizeof( m_stAttributes ) );
        if ( pthread_attr_init( &m_stAttributes ) != 0 ||
             pthread_create( &m_ptThreadID, &m_stAttributes, ThreadStartRoutine, this ) != 0 )
        {
            m_bIsRunning = false;
            m_bReaped = true;
            return;
        }
        if ( m_bDetached )
        {
            pthread_detach( m_ptThreadID );
        }
    #endif

    #ifdef IASLIB_WIN32__
        m_ptThreadID = CreateThread( NULL, 0, ThreadStartRoutine, this, 0, &m_dwThreadID );
        if ( m_ptThreadID == NULL )
        {
            m_bIsRunning = false;
            m_bReaped = true;
            return;
        }
    #endif

        m_bContained = true;

            // The monitor adds itself (it is created by this call).
        if ( m_strThreadName != "Thread Monitor" )
        {
            CThreadMonitor::GetThreadMonitor()->AddThread( this, m_strThreadName );
        }

        if ( ! bStartSuspended )
        {
            Release();
        }
    }

    CThread::CThread( THREAD_T ptThreadID, const char *strThreadName )
        : m_ptThreadID( ptThreadID ), m_bIsRunning( true ), m_nReturnCode( 0 ), m_bDetached( false ), m_bShutdown( false ),
          m_bContained( false ), m_bNoDelete( true ), m_bDaemon( false ), m_strThreadName( strThreadName ),
          m_bReleased( true ), m_bAbandoned( false ), m_bLeftGate( true ), m_bReaped( true )
    {
    #ifdef IASLIB_PTHREAD__
        memset( &m_stAttributes, 0, sizeof( m_stAttributes ) );
        if ( ptThreadID == (THREAD_T)0 )
        {
            m_ptThreadID = pthread_self();
        }
    #endif

    #ifdef IASLIB_WIN32__
        if ( ptThreadID == (THREAD_T)NULL )
        {
            m_ptThreadID = GetCurrentThread();
            m_dwThreadID = GetCurrentThreadId();
        }
        else
        {
            m_dwThreadID = GetThreadId( ptThreadID );
        }
    #endif

        CThreadMonitor::GetThreadMonitor()->AddThread( this, m_strThreadName );
    }

    CThread::~CThread( void )
    {
        if ( m_bContained )
        {
            if ( ! IsStarted() )
            {
                    // Never started: let it go without Run(), and wait until it
                    // no longer needs this object.
                std::unique_lock<std::mutex> oLock( m_mutexGate );
                m_bAbandoned = true;
                m_bShutdown = true;
                m_bReleased = true;
                m_cvGate.notify_all();
                m_cvGate.wait( oLock, [this]() { return m_bLeftGate; } );
            }
            else if ( m_bIsRunning )
            {
                Kill();
            }

            if ( ! m_bDetached )
            {
                Join();
            }

    #ifdef IASLIB_PTHREAD__
            pthread_attr_destroy( &m_stAttributes );
    #endif

    #ifdef IASLIB_WIN32__
            CloseHandle( m_ptThreadID );
    #endif
        }

        CThreadMonitor::GetThreadMonitor()->RemoveThread( this );
    }

    void CThread::Release( void )
    {
            // Notified with the lock held: once the thread is free it may
            // finish and (if detached) be deleted at once, so nothing here
            // may touch the object after the lock is let go.
        std::lock_guard<std::mutex> oLock( m_mutexGate );
        m_bReleased = true;
        m_cvGate.notify_all();
    }

    bool CThread::IsStarted( void )
    {
        std::lock_guard<std::mutex> oLock( m_mutexGate );
        return m_bReleased;
    }

    void CThread::ThreadMain( void )
    {
        bool bAbandoned;
        {
            std::unique_lock<std::mutex> oLock( m_mutexGate );
            m_cvGate.wait( oLock, [this]() { return m_bReleased; } );
            bAbandoned = m_bAbandoned;
            if ( bAbandoned )
            {
                    // Destroyed before it was started: the destructor waits
                    // for this, and the object may be gone straight after.
                m_bLeftGate = true;
                m_cvGate.notify_all();
                return;
            }
        }

        CRunningGuard oGuard( m_bIsRunning );
        try
        {
            Run();
        }
        catch ( ... )
        {
    #if defined( __GLIBC__ )
                // pthread_exit() and cancellation unwind with a "foreign"
                // exception (no C++ type), which must go on unwinding.
            if ( abi::__cxa_current_exception_type() == NULL )
                throw;
    #endif
                // Any other exception that escapes Run() ends the thread,
                // not the program.
        }
    }

    void *CThread::Run( void )
    {
        return NULL;
    }

    int CThread::Kill( void )
    {
        if ( ! m_bContained || ! m_bIsRunning )
            return 0;
    #ifdef IASLIB_PTHREAD__
        return pthread_cancel( m_ptThreadID );
    #endif
    #ifdef IASLIB_WIN32__
        if ( GetCurrentThreadId() == m_dwThreadID )
        {
            m_bIsRunning = false;
            ::ExitThread( (DWORD)-1 );
        }
            // Delivered when the thread next waits alertably, which the
            // library's sleeps and semaphore waits do.
        if ( ! QueueUserAPC( KillThreadApc, m_ptThreadID, 0 ) )
            return -1;
        if ( WaitForSingleObject( m_ptThreadID, 2000 ) == WAIT_OBJECT_0 )
        {
            m_bIsRunning = false;
            return 0;
        }
        return -1;
    #endif
    }

    int CThread::Join( void )
    {
        if ( ! m_bContained || m_bDetached )
            return 0;

        std::lock_guard<std::mutex> oLock( m_mutexJoin );
        if ( ! m_bReaped )
        {
                // A suspended thread would never end: start it.
            Release();
    #ifdef IASLIB_PTHREAD__
            pthread_join( m_ptThreadID, NULL );
    #endif
    #ifdef IASLIB_WIN32__
            WaitForSingleObject( m_ptThreadID, INFINITE );
    #endif
            m_bReaped = true;
            m_bIsRunning = false;
        }
        return m_nReturnCode;
    }

    void CThread::ExitThread( int nExitCode )
    {
    #ifdef IASLIB_PTHREAD__
        if ( pthread_equal( pthread_self(), m_ptThreadID ) )
        {
            m_nReturnCode = nExitCode;
        #if ! defined( __GLIBC__ )
                // Elsewhere pthread_exit() does not unwind the stack, so the
                // guard in ThreadMain() never runs.
            m_bIsRunning = false;
        #endif
            pthread_exit( NULL );
        }
    #endif
    #ifdef IASLIB_WIN32__
        if ( GetCurrentThreadId() == m_dwThreadID )
        {
            m_nReturnCode = nExitCode;
            m_bIsRunning = false;
                // The Windows function (this member's name hid it, so the old
                // code called itself forever).
            ::ExitThread( (DWORD)nExitCode );
        }
    #endif
    }

    void CThread::Sleep( int nSeconds )
    {
        if ( nSeconds > 0 )
            std::this_thread::sleep_for( std::chrono::seconds( nSeconds ) );
    }

    void CThread::Millisleep( int milliseconds )
    {
        if ( milliseconds <= 0 )
            return;
    #ifdef IASLIB_WIN32__
            // Alertable, so Kill()'s APC can end the thread here. Other APCs
            // end the sleep early: sleep the rest.
        std::chrono::steady_clock::time_point tpEnd = std::chrono::steady_clock::now() + std::chrono::milliseconds( milliseconds );
        for ( ;; )
        {
            long long llLeft = std::chrono::duration_cast<std::chrono::milliseconds>( tpEnd - std::chrono::steady_clock::now() ).count();
            if ( llLeft <= 0 )
                break;
            SleepEx( (DWORD)llLeft, TRUE );
        }
    #else
        std::this_thread::sleep_for( std::chrono::milliseconds( milliseconds ) );
    #endif
    }

    void CThread::Suspend( void )
    {
    #ifdef IASLIB_WIN32__
        if ( m_bContained && IsStarted() )
            SuspendThread( m_ptThreadID );
    #endif
    }

    void CThread::Resume( void )
    {
        if ( ! m_bContained )
            return;
        if ( ! IsStarted() )
        {
            Release();
            return;
        }
    #ifdef IASLIB_WIN32__
        ResumeThread( m_ptThreadID );
    #endif
    }

    bool CThread::Detach( void )
    {
        if ( ! m_bContained || m_bDetached )
            return false;
        std::lock_guard<std::mutex> oLock( m_mutexJoin );
        if ( m_bReaped )
            return false;
    #ifdef IASLIB_PTHREAD__
        pthread_detach( m_ptThreadID );
    #endif
        m_bDetached = true;
        return true;
    }

    unsigned long CThread::GetCapabilities( void )
    {
        unsigned long nFlags = THREADS | RESUME | KILL | JOIN | DETACH | SLEEP | YIELD | NAMED | STATE;
    #ifdef IASLIB_WIN32__
        nFlags |= SUSPEND;
    #endif
        return nFlags;
    }

    void CThread::SetTimeout( int nSeconds )
    {
        CThreadMonitor::GetThreadMonitor()->SetThreadTimeout( this, nSeconds );
    }

    CThread *CThread::Self( void )
    {
    #ifdef IASLIB_PTHREAD__
        return CThreadMonitor::GetThreadMonitor()->FindThread( pthread_self() );
    #endif
    #ifdef IASLIB_WIN32__
        return CThreadMonitor::GetThreadMonitor()->FindThread( GetCurrentThread() );
    #endif
    }

    bool CThread::Equals( THREAD_T ptThread1, THREAD_T ptThread2 )
    {
    #ifdef IASLIB_PTHREAD__
        return pthread_equal( ptThread1, ptThread2 ) != 0;
    #endif
    #ifdef IASLIB_WIN32__
            // Handles differ (GetCurrentThread() is the same for every thread);
            // thread IDs do not.
        if ( ptThread1 == NULL || ptThread2 == NULL )
            return ptThread1 == ptThread2;
        return GetThreadId( ptThread1 ) == GetThreadId( ptThread2 );
    #endif
    }
} // namespace IASLib

#endif // IASLIB_MULTI_THREADED__
