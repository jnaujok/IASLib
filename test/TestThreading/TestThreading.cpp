/***********************************************************************
**  TestThreading
**
**  Tests for the threading classes: CThread (starting suspended and
**  resuming from another thread, Join() and return codes, ExitThread(),
**  exceptions in Run(), Self(), Kill(), threads destroyed before they
**  start, detached threads the monitor deletes), CMutex with CSemaphore
**  across threads, CThreadPool (many tasks, results, statuses, failing
**  tasks, shutting down with work queued) and CServerThread.
***********************************************************************/

#include "IASLib.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

using namespace IASLib;

static int g_nChecks = 0;
static int g_nFailures = 0;

#define CHECK( condition ) \
    do { \
        g_nChecks++; \
        if ( ! ( condition ) ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition ); \
            fflush( stdout ); \
        } \
    } while ( 0 )

    // What the test is doing now, for the watchdog to report.
static std::atomic<const char *> g_pszStep( "starting" );
static std::atomic<long long> g_llStepStarted( 0 );

static long long nowMs( void )
{
    return (long long)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch() ).count();
}

static void step( const char *pszStep )
{
    g_pszStep = pszStep;
    g_llStepStarted = nowMs();
}

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
    fflush( stdout );
    step( pszName );
}

    // A step that takes more than a minute is stuck: say which one, and
    // stop, rather than be killed silently by ctest's timeout. (The Windows
    // CI run hung once in the CSemaphore section; this says where, next
    // time.)
static void startWatchdog( void )
{
    std::thread( []() {
        for ( ;; )
        {
            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
            if ( nowMs() - g_llStepStarted.load() > 60000 )
            {
                printf( "\nWATCHDOG: stuck for 60 s in: %s\n", g_pszStep.load() );
                fflush( stdout );
                _Exit( 2 );
            }
        }
    } ).detach();
}

static long long elapsedMs( std::chrono::steady_clock::time_point tpStart )
{
    return (long long)std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - tpStart ).count();
}

    // Waits (up to nMs) for a condition.
template <class F> static bool waitFor( F fnDone, int nMs )
{
    std::chrono::steady_clock::time_point tpStart = std::chrono::steady_clock::now();
    while ( ! fnDone() )
    {
        if ( elapsedMs( tpStart ) > nMs )
            return false;
        CThread::Millisleep( 2 );
    }
    return true;
}

/***********************************************************************
**  CThread
***********************************************************************/

class CWorker : public CThread
{
    public:
        std::atomic<int>    m_nRuns;
        std::atomic<bool>   m_bSawSelf;
        int                 m_nValue;       // set by the derived constructor
        int                 m_nSeen;
        int                 m_nMode;

        explicit CWorker( int nMode = 0, bool bDetached = false )
            : CThread( "Worker", true, bDetached ), m_nRuns( 0 ), m_bSawSelf( false ), m_nValue( 0 ), m_nSeen( -1 ), m_nMode( nMode )
        {
            m_nValue = 42;
        }

        virtual void *Run( void )
        {
            m_nRuns++;
            m_nSeen = m_nValue;
            m_bSawSelf = ( CThread::Self() == this );
            switch ( m_nMode )
            {
                case 1:     // a return code
                    SetReturnCode( 7 );
                    break;
                case 2:     // ExitThread()
                    ExitThread( 9 );
                    SetReturnCode( 99 );     // never reached
                    break;
                case 3:     // an exception
                    throw CException( "out of Run()" );
                case 4:     // until asked to stop
                    while ( ! IsShutdown() )
                        Millisleep( 5 );
                    break;
                case 5:     // for ever
                    for ( ;; )
                        Millisleep( 5 );
                default:
                    break;
            }
            return NULL;
        }
};

static std::atomic<int> g_nDetachedDeleted( 0 );

class CDetachedWorker : public CThread
{
    public:
        CDetachedWorker( void ) : CThread( "Detached", true, true ) {}
        virtual ~CDetachedWorker( void ) { g_nDetachedDeleted++; }
        virtual void *Run( void ) { Millisleep( 10 ); return NULL; }
};

static void testThread( void )
{
    section( "CThread" );

    {
            // Started from another thread (it used to need the thread that
            // made it: the start was a mutex locked by one and unlocked by
            // the other).
        CWorker oWorker( 1 );
        CHECK( ! oWorker.IsStarted() );
        CThread::Millisleep( 30 );
        CHECK( oWorker.m_nRuns == 0 );            // suspended
        std::thread( [&oWorker]() { oWorker.Resume(); } ).join();
        CHECK( oWorker.Join() == 7 );
        CHECK( oWorker.m_nRuns == 1 );
        CHECK( oWorker.m_nSeen == 42 );           // after the derived constructor
        CHECK( oWorker.m_bSawSelf );
        CHECK( ! oWorker.IsRunning() );
        CHECK( oWorker.Join() == 7 );             // again, after it finished
    }

    {
        CWorker oWorker( 0 );
        oWorker.Start();
        CThread::Millisleep( 50 );                // finishes before the join
        CHECK( oWorker.Join() == 0 );             // (it used to be left unjoined)
    }

    {
        CWorker oWorker( 2 );
        oWorker.Resume();
        CHECK( oWorker.Join() == 9 );             // ExitThread() (it used to abort the program on Linux)
        CHECK( ! oWorker.IsRunning() );
    }

    {
        CWorker oWorker( 3 );
        oWorker.Resume();
        oWorker.Join();                           // the exception ended the thread, not the program
        CHECK( oWorker.m_nRuns == 1 && ! oWorker.IsRunning() );
    }

    {
        CWorker oWorker( 4 );
        oWorker.Resume();
        CHECK( waitFor( [&oWorker]() { return oWorker.m_nRuns == 1; }, 2000 ) );
        CHECK( oWorker.IsRunning() );
        oWorker.RequestShutdown();
        std::chrono::steady_clock::time_point tpStart = std::chrono::steady_clock::now();
        oWorker.Join();
        CHECK( elapsedMs( tpStart ) < 1000 );
    }

    {
        CWorker *pForever = new CWorker( 5 );
        pForever->Resume();
        CHECK( waitFor( [pForever]() { return pForever->m_nRuns == 1; }, 2000 ) );
        CHECK( pForever->Kill() == 0 );
        pForever->Join();
        CHECK( ! pForever->IsRunning() );
        delete pForever;
    }

    {
            // Destroyed without ever being started: it never runs, and the
            // destructor does not hang.
        std::chrono::steady_clock::time_point tpStart = std::chrono::steady_clock::now();
        CWorker *pNever = new CWorker( 1 );
        std::atomic<int> *pRuns = &pNever->m_nRuns;
        (void)pRuns;
        delete pNever;
        CHECK( elapsedMs( tpStart ) < 1000 );
    }

    {
            // Detached threads belong to the monitor, which deletes them
            // when they finish.
        g_nDetachedDeleted = 0;
        for ( int n = 0; n < 5; n++ )
            ( new CDetachedWorker() )->Resume();
        CHECK( waitFor( []() { return g_nDetachedDeleted == 5; }, 5000 ) );
    }

        // Self() and Equals() (on Windows they compared one pseudo handle).
    CHECK( CThread::Self() == NULL );
    CWorker oA( 0 ), oB( 0 );
    CHECK( CThread::Equals( oA.GetThreadID(), oA.GetThreadID() ) );
    CHECK( ! CThread::Equals( oA.GetThreadID(), oB.GetThreadID() ) );
    CHECK( ( oA.GetCapabilities() & CThread::JOIN ) != 0 );
    oA.Resume();
    oB.Resume();

    CThreadMonitor *pMonitor = CThreadMonitor::GetThreadMonitor();
    CHECK( pMonitor != NULL && pMonitor == CThreadMonitor::GetThreadMonitor() );
    CHECK( pMonitor->GetTotalThreads() >= 10 );
}

/***********************************************************************
**  CMutex and CSemaphore together
***********************************************************************/

static void testSemaphore( void )
{
    section( "CSemaphore" );

    step( "CSemaphore: counting and timed waits" );
    CSemaphore oSem( 2 );
    CHECK( oSem.TryWait() && oSem.TryWait() );
    CHECK( ! oSem.TryWait() );
    std::chrono::steady_clock::time_point tpStart = std::chrono::steady_clock::now();
    CHECK( ! oSem.TimedWait( 100 ) );
    long long nWaited = elapsedMs( tpStart );
    CHECK( nWaited >= 90 && nWaited < 1000 );
    oSem.Post();
    CHECK( oSem.TimedWait( 100 ) );

        // Woken from another thread.
    step( "CSemaphore: woken from another thread" );
    std::thread oPoster( [&oSem]() { CThread::Millisleep( 50 ); oSem.Post(); } );
    tpStart = std::chrono::steady_clock::now();
    CHECK( oSem.TimedWait( 5000 ) );
    CHECK( elapsedMs( tpStart ) < 2000 );
    oPoster.join();

        // A producer and consumers: every item taken exactly once.
    step( "CSemaphore: starting the consumers" );
    CSemaphore oItems( 0 );
    CMutex oLock;
    std::vector<int> aQueue;
    std::atomic<int> nTaken( 0 );
    long nSum = 0;
    std::vector<std::thread> aConsumers;
    for ( int t = 0; t < 4; t++ )
    {
        aConsumers.push_back( std::thread( [&]() {
            for ( ;; )
            {
                oItems.Wait();
                CMutexLock oGuard( oLock );
                int nItem = aQueue.back();
                aQueue.pop_back();
                if ( nItem < 0 )
                    break;
                nSum += nItem;
                nTaken++;
            }
        } ) );
    }
    step( "CSemaphore: producing" );
    for ( int n = 1; n <= 1000; n++ )
    {
        {
            CMutexLock oGuard( oLock );
            aQueue.insert( aQueue.begin(), n );
        }
        oItems.Post();
    }
    step( "CSemaphore: waiting for the consumers" );
    CHECK( waitFor( [&nTaken]() { return nTaken == 1000; }, 5000 ) );
    for ( int t = 0; t < 4; t++ )
    {
        {
            CMutexLock oGuard( oLock );
            aQueue.insert( aQueue.begin(), -1 );
        }
        oItems.Post();
    }
    step( "CSemaphore: joining the consumers" );
    for ( size_t t = 0; t < aConsumers.size(); t++ )
        aConsumers[t].join();
    CHECK( nSum == 500500 );
}

/***********************************************************************
**  CThreadPool
***********************************************************************/

static std::atomic<int> g_nTasksAlive( 0 );
static std::atomic<int> g_nTasksRun( 0 );

class CSumTask : public CThreadTask
{
    public:
        int m_nTo;
        bool m_bFail;
        int m_nSleepMs;
        CSumTask( const char *pszId, int nTo, bool bFail = false, int nSleepMs = 0 )
            : CThreadTask( pszId ), m_nTo( nTo ), m_bFail( bFail ), m_nSleepMs( nSleepMs ) { g_nTasksAlive++; }
        virtual ~CSumTask( void ) { g_nTasksAlive--; }
        virtual CObject *Run( void )
        {
            g_nTasksRun++;
            if ( m_nSleepMs > 0 )
                CThread::Millisleep( m_nSleepMs );
            if ( m_bFail )
                throw CException( "task failed" );     // not a std::exception
            long nSum = 0;
            for ( int n = 1; n <= m_nTo; n++ )
                nSum += n;
            CString *pResult = new CString();
            *pResult = nSum;
            return pResult;
        }
};

static void testPool( void )
{
    section( "CThreadPool" );

    g_nTasksAlive = 0;
    g_nTasksRun = 0;
    {
        CThreadPool oPool( 4, true );
        CHECK( oPool.GetTotalThreads() == 4 && oPool.GetIdleThreads() == 4 );
        for ( int n = 0; n < 200; n++ )
            oPool.AddTask( new CSumTask( CString::FormatString( "sum%d", n ), n ) );
        CHECK( oPool.WaitUntilIdle( 20000 ) );
        CHECK( g_nTasksRun == 200 );
        CHECK( g_nTasksAlive == 0 );              // the pool deleted them
        CHECK( oPool.GetPeakThreads() >= 1 && oPool.GetPeakThreads() <= 4 );
        CHECK( oPool.GetActiveThreads() == 0 && oPool.GetQueueSize() == 0 );

        CHECK( oPool.TaskStatus( "sum10" ) == CThreadTask::COMPLETE );   // used to be UNKNOWN always
        CHECK( oPool.TaskStatus( "nothing" ) == CThreadTask::UNKNOWN );
        CObject *pResult = oPool.GetResults( "sum100" );
        CHECK( pResult != NULL && *(CString *)pResult == "5050" );
        delete pResult;
        CHECK( oPool.GetResults( "sum100" ) == NULL );   // handed over

            // A task that throws a library exception (it used to kill the
            // worker for good, and the pool lost a thread each time).
        for ( int n = 0; n < 8; n++ )
            oPool.AddTask( new CSumTask( CString::FormatString( "fail%d", n ), 1, true ) );
        CHECK( oPool.WaitUntilIdle( 10000 ) );
        CHECK( oPool.TaskStatus( "fail3" ) == CThreadTask::EXCEPTION );
        oPool.AddTask( new CSumTask( "after", 3 ) );
        CHECK( oPool.WaitUntilIdle( 10000 ) );
        CObject *pAfter = oPool.GetResults( "after" );
        CHECK( pAfter != NULL && *(CString *)pAfter == "6" );
        delete pAfter;
        CHECK( oPool.GetIdleThreads() == 4 );
    }
    CHECK( g_nTasksAlive == 0 );

        // Destroyed with work queued: running tasks finish, queued ones are
        // deleted without running, and nothing is left behind.
    g_nTasksRun = 0;
    {
        CThreadPool oPool( 2 );
        for ( int n = 0; n < 20; n++ )
            oPool.AddTask( new CSumTask( "slow", 10, false, 50 ) );
        CHECK( waitFor( []() { return g_nTasksRun >= 2; }, 5000 ) );
    }
    CHECK( g_nTasksAlive == 0 );
    CHECK( g_nTasksRun < 20 );

        // Retained tasks are the caller's.
    {
        CThreadPool oPool( 2, false, true );
        CSumTask *pTask = new CSumTask( "kept", 4 );
        oPool.AddTask( pTask );
        CHECK( oPool.WaitUntilIdle( 5000 ) );
        CHECK( pTask->GetStatus() == CThreadTask::COMPLETE );
        delete pTask;
    }
    CHECK( g_nTasksAlive == 0 );
}

/***********************************************************************
**  CServerThread
***********************************************************************/

#ifdef IASLIB_NETWORKING__
class CEchoServer : public CServerThread
{
    public:
        std::atomic<int> m_nConnections;
        CEchoServer( int nPort ) : CServerThread( "Echo", nPort ), m_nConnections( 0 ) {}
        virtual bool HandleConnection( CSocket *pSocket )
        {
            char achBuffer[64];
            int nRead = pSocket->ReadWithTimeout( achBuffer, sizeof( achBuffer ), 2000 );
            if ( nRead > 0 )
                pSocket->Send( achBuffer, nRead );
            m_nConnections++;
            delete pSocket;
            return true;
        }
};

static void testServerThread( void )
{
    section( "CServerThread" );

    int nPort = 0;
    CEchoServer *pServer = NULL;
    for ( int nTry = 0; nTry < 20 && pServer == NULL; nTry++ )
    {
        nPort = 23000 + ( (int)( std::chrono::steady_clock::now().time_since_epoch().count() % 2000 ) ) + nTry * 7;
        pServer = new CEchoServer( nPort );
        pServer->Resume();
        CThread::Millisleep( 100 );
        if ( ! pServer->IsRunning() )
        {
            pServer->Join();
            delete pServer;
            pServer = NULL;
        }
    }
    CHECK( pServer != NULL );
    if ( pServer == NULL )
        return;

    bool bEchoed = false;
    try
    {
        CClientSocket oClient( "127.0.0.1", nPort );
        oClient.Send( "ping", 4 );
        char achBuffer[8] = { 0 };
        int nRead = oClient.ReadWithTimeout( achBuffer, 4, 3000 );
        bEchoed = ( nRead == 4 && memcmp( achBuffer, "ping", 4 ) == 0 );
    }
    catch ( ... )
    {
    }
    CHECK( bEchoed );
    CHECK( waitFor( [pServer]() { return pServer->m_nConnections == 1; }, 3000 ) );

        // It stops when asked (it used to wait in Accept() for ever).
    pServer->RequestShutdown();
    std::chrono::steady_clock::time_point tpStart = std::chrono::steady_clock::now();
    CHECK( pServer->Join() == 0 );
    CHECK( elapsedMs( tpStart ) < 2000 );
    delete pServer;
}
#endif

int main( void )
{
    step( "starting" );
    startWatchdog();
    testThread();
    testSemaphore();
    testPool();
#ifdef IASLIB_NETWORKING__
    testServerThread();
#endif

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
