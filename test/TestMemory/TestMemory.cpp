/***********************************************************************
**  TestMemory
**
**  Tests for the memory manager: CDefaultMemoryManager and statistics,
**  CMemoryFixedHeap (blocks, chunks, fallback, compaction, threads),
**  CMemoryHeap (size classes), CMemoryDebugHeap (fenceposts, double and
**  foreign frees, spoiling, leak tracking), and - when the library routes
**  CObject through the manager - objects going back to the allocator
**  they came from.
***********************************************************************/

#include "IASLib.h"

#include <cstdio>
#include <cstring>
#include <string>
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
        } \
    } while ( 0 )

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
}

    // Catches what the allocators report.
static std::vector<std::string> g_aErrors;
static void captureError( const char *strMessage )
{
    g_aErrors.push_back( strMessage );
}

static bool lastErrorHas( const char *strText )
{
    return ! g_aErrors.empty() && g_aErrors.back().find( strText ) != std::string::npos;
}

static bool aligned( const void *p )
{
    return ( (size_t)p % 16 ) == 0;
}

static void testDefault( void )
{
    section( "CDefaultMemoryManager" );

    CDefaultMemoryManager manager;
    CHECK( strcmp( manager.GetName(), "malloc" ) == 0 );
    void *p = manager.Allocate( 100 );
    void *q = manager.Allocate( 50 );
    CHECK( p != NULL && q != NULL );
    CMemoryManager::CStatistics stats = manager.GetStatistics();
    CHECK( stats.nAllocations == 2 );
    CHECK( stats.nBytesInUse == 150 );
    manager.Release( p, 100 );
    stats = manager.GetStatistics();
    CHECK( stats.nReleases == 1 && stats.nBytesInUse == 50 && stats.nPeakBytesInUse == 150 );
    manager.Release( q, 50 );
    manager.Release( NULL, 0 );
    CHECK( manager.GetStatistics().nBytesInUse == 0 );
}

static void testFixedHeap( void )
{
    section( "CMemoryFixedHeap" );

    CMemoryFixedHeap heap( 24, 4 );
    CHECK( heap.GetBlockSize() == 32 );         // rounded up to 16s
    CHECK( heap.GetChunkCount() == 0 );

    void *ap[ 9 ];
    for ( int n = 0; n < 9; n++ )
    {
        ap[n] = heap.Allocate( 20 );
        CHECK( ap[n] != NULL && aligned( ap[n] ) );
        memset( ap[n], n, 20 );
    }
    CHECK( heap.GetChunkCount() == 3 );         // 4 blocks a chunk
    CHECK( heap.GetBlocksInUse() == 9 );
    CHECK( heap.GetFreeBlocks() == 3 );
    CHECK( heap.Owns( ap[0] ) && heap.Owns( ap[8] ) );
    for ( int n = 0; n < 9; n++ )
        CHECK( ((unsigned char *)ap[n])[19] == (unsigned char)n );   // no overlap

        // A freed block is the next one given out.
    heap.Release( ap[4], 20 );
    void *pAgain = heap.Allocate( 20 );
    CHECK( pAgain == ap[4] );
    ap[4] = pAgain;

        // Larger requests go to the fallback.
    void *pBig = heap.Allocate( 1000 );
    CHECK( pBig != NULL && ! heap.Owns( pBig ) );
    heap.Release( pBig, 1000 );

        // Compaction gives back only empty chunks.
    for ( int n = 0; n < 4; n++ )
        heap.Release( ap[n], 20 );
    CHECK( heap.Compact() == 1 );
    CHECK( heap.GetChunkCount() == 2 );
    CHECK( heap.Compact() == 0 );
    for ( int n = 4; n < 9; n++ )
        heap.Release( ap[n], 20 );
    CHECK( heap.GetBlocksInUse() == 0 );
    CHECK( heap.Compact() == 2 );
    CHECK( heap.GetChunkCount() == 0 );
    CHECK( heap.GetStatistics().nBytesInUse == 0 );

        // A pointer that is not one of its blocks is reported, not used.
    CMemoryManager::ErrorHandler pfnOld = CMemoryManager::SetErrorHandler( captureError );
    int nLocal = 0;
    size_t nErrors = g_aErrors.size();
    heap.Release( &nLocal, 8 );
    CHECK( g_aErrors.size() == nErrors + 1 && lastErrorHas( "not a block" ) );
    void *pOne = heap.Allocate( 8 );
    heap.Release( (char *)pOne + 8, 8 );        // inside a block, not its start
    CHECK( lastErrorHas( "not a block" ) );
    heap.Release( pOne, 8 );
    CMemoryManager::SetErrorHandler( pfnOld );

        // Threads.
    CMemoryFixedHeap shared( 64, 32 );
    std::vector<std::thread> threads;
    bool abOk[ 4 ] = { true, true, true, true };
    for ( int t = 0; t < 4; t++ )
    {
        threads.push_back( std::thread( [&shared, &abOk, t]() {
            std::vector<unsigned char *> blocks;
            for ( int round = 0; round < 50; round++ )
            {
                for ( int i = 0; i < 40; i++ )
                {
                    unsigned char *p = (unsigned char *)shared.Allocate( 64 );
                    if ( p == NULL ) { abOk[t] = false; continue; }
                    memset( p, t + 1, 64 );
                    blocks.push_back( p );
                }
                for ( size_t i = 0; i < blocks.size(); i++ )
                {
                    if ( blocks[i][0] != t + 1 || blocks[i][63] != t + 1 )
                        abOk[t] = false;
                    shared.Release( blocks[i], 64 );
                }
                blocks.clear();
            }
        } ) );
    }
    for ( size_t t = 0; t < threads.size(); t++ )
        threads[t].join();
    CHECK( abOk[0] && abOk[1] && abOk[2] && abOk[3] );
    CHECK( shared.GetBlocksInUse() == 0 );
    CHECK( shared.GetStatistics().nAllocations == 4 * 50 * 40 );
}

static void testPooledHeap( void )
{
    section( "CMemoryHeap" );

    CMemoryHeap heap( 8 );
    CHECK( heap.PoolFor( 1 )->GetBlockSize() == 16 );
    CHECK( heap.PoolFor( 17 )->GetBlockSize() == 32 );
    CHECK( heap.PoolFor( 512 )->GetBlockSize() == 512 );
    CHECK( heap.PoolFor( 513 ) == NULL );

    std::vector<std::pair<void *, size_t> > blocks;
    size_t asSizes[] = { 1, 16, 30, 100, 200, 500, 600, 5000 };
    for ( size_t n = 0; n < sizeof( asSizes ) / sizeof( asSizes[0] ); n++ )
    {
        void *p = heap.Allocate( asSizes[n] );
        CHECK( p != NULL && aligned( p ) );
        memset( p, 0xAB, asSizes[n] );
        blocks.push_back( std::make_pair( p, asSizes[n] ) );
    }
    CHECK( heap.GetStatistics().nAllocations == 8 );
    for ( size_t n = 0; n < blocks.size(); n++ )
        heap.Release( blocks[n].first, blocks[n].second );
    CHECK( heap.GetStatistics().nBytesInUse == 0 );
    CHECK( heap.Compact() == 5 );               // a chunk in each pool used (not 64)
}

static void testDebugHeap( void )
{
    section( "CMemoryDebugHeap" );

    CMemoryManager::ErrorHandler pfnOld = CMemoryManager::SetErrorHandler( captureError );
    g_aErrors.clear();

    CMemoryDebugHeap heap;
    unsigned char *p = (unsigned char *)heap.Allocate( 20, "Thing" );
    CHECK( p != NULL && aligned( p ) );
    bool bZeroed = true;
    for ( int n = 0; n < 20; n++ )
        if ( p[n] != 0 ) bZeroed = false;
    CHECK( bZeroed );
    memset( p, 1, 20 );
    heap.Release( p, 20 );
    CHECK( g_aErrors.empty() );                 // a clean one is quiet

        // Overrun: the padding after the data counts too.
    p = (unsigned char *)heap.Allocate( 20, "Overrun" );
    p[20] = 0x55;
    p[24] = 0x66;
    heap.Release( p, 20 );
    CHECK( g_aErrors.size() == 1 && lastErrorHas( "[Overrun] Back fencepost damaged (2 bytes, first at 0: 55)" ) );

        // Underrun.
    p = (unsigned char *)heap.Allocate( 32, "Underrun" );
    p[-1] = 0x77;
    heap.Release( p, 32 );
    CHECK( g_aErrors.size() == 2 && lastErrorHas( "[Underrun] Front fencepost damaged" ) );

        // Freed twice, and not ours. The memory stays with the heap's
        // underlying allocator (so freeing twice is only safe to test
        // through a pool that keeps the chunk).
    CMemoryFixedHeap pool( 256, 4 );
    CMemoryDebugHeap pooled( &pool );
    p = (unsigned char *)pooled.Allocate( 16, "Twice" );
    pooled.Release( p, 16 );
    CHECK( p[0] == 'X' && p[15] == 'X' );       // spoiled
    pooled.Release( p, 16 );
    CHECK( lastErrorHas( "freed twice" ) );
    unsigned char abyLocal[ 256 ];
    memset( abyLocal, 0, sizeof( abyLocal ) );
    pooled.Release( abyLocal + 128, 16 );
    CHECK( lastErrorHas( "MAGIC NUMBER MISMATCH" ) );

        // Wrong size.
    p = (unsigned char *)heap.Allocate( 40, "Sized" );
    heap.Release( p, 41 );
    CHECK( lastErrorHas( "[Sized] Freed with size 41, allocated with 40" ) );

        // Leak tracking: only what is allocated while it is on.
    void *pUntracked = heap.Allocate( 10, "Untracked" );
    heap.SetLeakTracking( true );
    CHECK( heap.IsLeakTracking() );
    void *pA = heap.Allocate( 10, "A", "a.cpp", 11 );
    void *pB = heap.Allocate( 30, "B", "b.cpp", 22 );
    CHECK( heap.GetLeakCount() == 2 );
    CHECK( heap.GetLeakBytes() == 40 );
    heap.Release( pA, 10 );
    CHECK( heap.GetLeakCount() == 1 );
    FILE *pReport = tmpfile();
    CHECK( heap.ReportLeaks( pReport ) == 1 );
    if ( pReport )
    {
        rewind( pReport );
        char achLine[ 256 ] = "";
        CHECK( fgets( achLine, sizeof( achLine ), pReport ) != NULL );
        CHECK( strstr( achLine, "30 bytes as B at b.cpp:22" ) != NULL );
        fclose( pReport );
    }
        // CheckAll() finds damage before the free.
    ((unsigned char *)pB)[30] = 1;
    size_t nErrors = g_aErrors.size();
    CHECK( heap.CheckAll() == 1 );
    CHECK( g_aErrors.size() == nErrors + 1 && lastErrorHas( "checked while in use" ) );
    ((unsigned char *)pB)[30] = 0;
    heap.Release( pB, 30 );
    CHECK( heap.GetLeakCount() == 0 );
    heap.SetLeakTracking( false );
    heap.Release( pUntracked, 10 );
    CHECK( heap.GetStatistics().nBytesInUse == 0 );

    CMemoryManager::SetErrorHandler( pfnOld );
}

    // A CObject class, to see where objects come from.
class CTracked : public CObject
{
    public:
        int     m_nValue;
        char    m_achPad[ 40 ];
                CTracked( int nValue ) : m_nValue( nValue ) { memset( m_achPad, 0, sizeof( m_achPad ) ); }
        virtual ~CTracked( void ) {}
                DECLARE_OBJECT( CTracked, CObject );
};

class CDerivedTracked : public CTracked
{
    public:
        CDerivedTracked( void ) : CTracked( 2 ) {}
            // No DECLARE_OBJECT: it uses CTracked's new and delete.
};

class CThrowing : public CObject
{
    public:
                CThrowing( void ) { throw 7; }
        virtual ~CThrowing( void ) {}
                DECLARE_OBJECT( CThrowing, CObject );
};

static void testObjectRouting( void )
{
    section( "CObject through the memory manager" );
#if defined( IASLIB_MEMORY_MANAGER__ ) || defined( IASLIB_MEMORY_DEBUGGING__ )
    CMemoryManager::ErrorHandler pfnOld = CMemoryManager::SetErrorHandler( captureError );
    g_aErrors.clear();

#ifdef IASLIB_MEMORY_DEBUGGING__
    CHECK( strcmp( CMemoryManager::Get()->GetName(), "debug" ) == 0 );
#else
    CHECK( strcmp( CMemoryManager::Get()->GetName(), "malloc" ) == 0 );
#endif

        // A pool for objects.
    CMemoryHeap pooled;
    CMemoryManager *pPrevious = CMemoryManager::Set( &pooled );
    CHECK( CMemoryManager::Get() == &pooled );
    CTracked *pInPool = new CTracked( 1 );
    CHECK( CMemoryManager::AllocatedType( pInPool ) != NULL && strcmp( CMemoryManager::AllocatedType( pInPool ), "CTracked" ) == 0 );
    CHECK( pooled.GetStatistics().nAllocations == 1 );

        // Deleted twice: reported, not freed again. (Only where freed memory
        // stays the allocator's, as in a pool; after free() reading it at all
        // is undefined.)
    CTracked *pTwice = new CTracked( 4 );
    delete pTwice;
    size_t nErrors = g_aErrors.size();
    CMemoryManager::ReleaseObject( pTwice, "CTracked" );
    CHECK( g_aErrors.size() == nErrors + 1 && lastErrorHas( "deleted twice" ) );
    g_aErrors.clear();

        // Changing the allocator: each object goes back to its own.
    CMemoryDebugHeap debug( NULL, true );
    CMemoryManager::Set( &debug );
    CTracked *pDebug = IASLIB_NEW CTracked( 3 );
    CDerivedTracked *pDerived = new CDerivedTracked();
    CHECK( debug.GetLeakCount() == 2 );

    FILE *pReport = tmpfile();
    debug.ReportLeaks( pReport );
    if ( pReport )
    {
        rewind( pReport );
        std::string strAll;
        char achLine[ 256 ];
        while ( fgets( achLine, sizeof( achLine ), pReport ) )
            strAll += achLine;
        fclose( pReport );
        CHECK( strAll.find( "as CTracked at " ) != std::string::npos );
        CHECK( strAll.find( "TestMemory.cpp:" ) != std::string::npos );     // IASLIB_NEW recorded it
    }

    delete pInPool;                     // back to the pool, though debug is current
    CHECK( pooled.GetStatistics().nBytesInUse == 0 );
    delete pDebug;
    CTracked *pAsBase = pDerived;
    delete pAsBase;
    CHECK( debug.GetLeakCount() == 0 );
    CHECK( g_aErrors.empty() );

        // A constructor that throws frees its memory.
    size_t nBefore = debug.GetStatistics().nBytesInUse;
    bool bCaught = false;
    try
    {
        CThrowing *pNever = IASLIB_NEW CThrowing();
        (void)pNever;
    }
    catch ( int )
    {
        bCaught = true;
    }
    CHECK( bCaught );
    CHECK( debug.GetStatistics().nBytesInUse == nBefore );
    CHECK( debug.GetLeakCount() == 0 );

        // Placement new still works.
    alignas( 16 ) unsigned char abyStorage[ sizeof( CTracked ) ];
    CTracked *pPlaced = new ( abyStorage ) CTracked( 5 );
    CHECK( pPlaced->m_nValue == 5 );
    pPlaced->~CTracked();

    CMemoryManager::Set( pPrevious );
    CHECK( CMemoryManager::Get() == pPrevious );
    CMemoryManager::SetErrorHandler( pfnOld );

        // A library object.
    CString *pString = new CString( "hello" );
    CHECK( *pString == "hello" );
    delete pString;
#else
    printf( "  (the memory manager is not built in; skipped)\n" );
    CTracked *p = IASLIB_NEW CTracked( 1 );     // plain new
    CHECK( p->m_nValue == 1 );
    delete p;
#endif
}

int main( void )
{
    testDefault();
    testFixedHeap();
    testPooledHeap();
    testDebugHeap();
    testObjectRouting();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
