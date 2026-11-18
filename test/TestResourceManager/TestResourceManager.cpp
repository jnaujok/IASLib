/***********************************************************************
**  TestResourceManager
**
**  Tests for CResourceManager: paths, sources in priority order
**  (an archive and a directory of loose files), typed loaders, shared
**  handles, the memory budget, unloading and reloading, and background
**  loading with callbacks run by ProcessCompleted().
***********************************************************************/

#include "IASLib.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#ifdef IASLIB_WIN32__
    #include <direct.h>
    #define makeDirectory( p ) _mkdir( p )
    #define removeDirectory( p ) _rmdir( p )
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #define makeDirectory( p ) mkdir( p, 0755 )
    #define removeDirectory( p ) rmdir( p )
#endif

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

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
    fflush( stdout );
}

static void writeFile( const std::string &strPath, const std::string &strData )
{
    std::ofstream out( strPath.c_str(), std::ios::binary | std::ios::trunc );
    out << strData;
}

static std::string textOf( const std::shared_ptr<CLoadedResource> &pResource )
{
    std::shared_ptr<CRawResource> pRaw = std::dynamic_pointer_cast<CRawResource>( pResource );
    if ( ! pRaw )
        return std::string( "<not raw>" );
    return std::string( (const char *)pRaw->GetBytes(), pRaw->GetLength() );
}

    // A typed resource: a number written as text.
class CNumberResource : public CLoadedResource
{
    public:
        int     m_nValue;
        explicit CNumberResource( int nValue ) : m_nValue( nValue ) {}
        virtual size_t GetMemorySize( void ) const { return sizeof( *this ); }
};

static const char *DIR = "rm_assets";

static void makeAssets( void )
{
    makeDirectory( DIR );
    makeDirectory( "rm_assets/textures" );
    makeDirectory( "rm_assets/levels" );
    writeFile( "rm_assets/textures/hero.txt", "hero from disk" );
    writeFile( "rm_assets/textures/only_on_disk.txt", "disk only" );
    writeFile( "rm_assets/levels/one.num", "42" );
    writeFile( "rm_assets/levels/bad.num", "not a number" );
    for ( int n = 0; n < 10; n++ )
        writeFile( std::string( "rm_assets/levels/big" ) + std::to_string( n ) + ".bin", std::string( 1000, (char)( 'a' + n ) ) );

    CResourceFile archive( "rm_assets.iasres", CResourceFile::CREATE );
    CResource hero( 1, 1 );
    hero.SetName( "textures/hero.txt" );
    hero.SetData( "hero from the archive", 21 );
    archive.Put( hero );
    CResource map( 1, 2 );
    map.SetName( "levels/map.txt" );
    map.SetData( "archive map", 11 );
    archive.Put( map );
}

static void removeAssets( void )
{
    const char *apFiles[] = { "rm_assets/textures/hero.txt", "rm_assets/textures/only_on_disk.txt",
                              "rm_assets/levels/one.num", "rm_assets/levels/bad.num", "rm_assets/levels/new.txt",
                              "rm_assets/levels/reload.txt" };
    for ( size_t n = 0; n < sizeof( apFiles ) / sizeof( apFiles[0] ); n++ )
        std::remove( apFiles[n] );
    for ( int n = 0; n < 10; n++ )
        std::remove( ( std::string( "rm_assets/levels/big" ) + std::to_string( n ) + ".bin" ).c_str() );
    removeDirectory( "rm_assets/textures" );
    removeDirectory( "rm_assets/levels" );
    removeDirectory( DIR );
    std::remove( "rm_assets.iasres" );
}

static void testPaths( void )
{
    section( "Paths" );
    CHECK( CResourceManager::NormalizePath( "textures/hero.png" ) == "textures/hero.png" );
    CHECK( CResourceManager::NormalizePath( "/textures//hero.png" ) == "textures/hero.png" );
    CHECK( CResourceManager::NormalizePath( "./textures/./hero.png" ) == "textures/hero.png" );
    CHECK( CResourceManager::NormalizePath( "textures\\hero.png" ) == "textures/hero.png" );
    CHECK( CResourceManager::NormalizePath( "../secret.txt" ) == "" );
    CHECK( CResourceManager::NormalizePath( "textures/../../x" ) == "" );
    CHECK( CResourceManager::NormalizePath( "" ) == "" );
    CHECK( CResourceManager::NormalizePath( NULL ) == "" );
}

static void testSources( void )
{
    section( "Sources and loaders" );
    std::shared_ptr<CResourceFile> pArchive = std::make_shared<CResourceFile>( "rm_assets.iasres", CResourceFile::READ_ONLY );
    CHECK( pArchive->IsOpen() );

    CResourceManager manager( 1024 * 1024, 0 );
    manager.AddSource( std::make_shared<CDirectorySource>( DIR ), 0 );
    manager.AddSource( std::make_shared<CArchiveSource>( pArchive ), 10 );

        // The archive (higher priority) wins; the directory fills in.
    std::shared_ptr<CLoadedResource> pHero = manager.Get( "textures/hero.txt" );
    CHECK( textOf( pHero ) == "hero from the archive" );
    CHECK( pHero && pHero->GetPath() == "textures/hero.txt" );
    CHECK( textOf( manager.Get( "textures/only_on_disk.txt" ) ) == "disk only" );
    CHECK( textOf( manager.Get( "levels/map.txt" ) ) == "archive map" );
    CHECK( manager.Exists( "textures/only_on_disk.txt" ) && ! manager.Exists( "nothing/here.txt" ) );

        // The same object every time.
    CHECK( manager.Get( "/textures//hero.txt" ) == pHero );
    CResourceManager::CStatistics stats = manager.GetStatistics();
    CHECK( stats.nLoaded == 3 && stats.nMisses == 3 && stats.nHits == 1 );

        // Missing, and not allowed.
    CHECK( ! manager.Get( "nothing/here.txt" ) );
    CHECK( manager.GetLastError().IndexOf( "not found" ) != NOT_FOUND );
    CHECK( ! manager.Get( "../rm_assets.iasres" ) );
    CHECK( manager.GetLastError().IndexOf( "not a valid" ) != NOT_FOUND );
    CHECK( manager.GetStatistics().nFailed == 2 );

        // A typed loader, by extension (any case).
    manager.RegisterLoader( "NUM", []( const CString &, const CDataBlock &oData, CString &strError ) -> std::shared_ptr<CLoadedResource> {
        std::string strText( (const char *)oData, oData.GetSize() );
        char *pEnd = NULL;
        long nValue = strtol( strText.c_str(), &pEnd, 10 );
        if ( strText.empty() || *pEnd != '\0' )
        {
            strError = "not a number";
            return std::shared_ptr<CLoadedResource>();
        }
        return std::make_shared<CNumberResource>( (int)nValue );
    } );
    std::shared_ptr<CNumberResource> pNumber = manager.Get<CNumberResource>( "levels/one.num" );
    CHECK( pNumber && pNumber->m_nValue == 42 );
    CHECK( ! manager.Get<CRawResource>( "levels/one.num" ) );   // it is a number, not raw
    CHECK( ! manager.Get( "levels/bad.num" ) );
    CHECK( manager.GetLastError().IndexOf( "not a number" ) != NOT_FOUND );

        // A loader that throws is a failure, not a crash.
    manager.RegisterLoader( ".boom", []( const CString &, const CDataBlock &, CString & ) -> std::shared_ptr<CLoadedResource> {
        throw std::runtime_error( "bang" );
    } );
    writeFile( "rm_assets/levels/new.txt", "x" );
    std::rename( "rm_assets/levels/new.txt", "rm_assets/levels/new.boom" );
    CHECK( ! manager.Get( "levels/new.boom" ) );
    CHECK( manager.GetLastError().IndexOf( "bang" ) != NOT_FOUND );
    std::remove( "rm_assets/levels/new.boom" );

        // The archive's own cache is off: the manager caches.
    CHECK( pArchive->GetCachedCount() == 0 );
}

static void testBudget( void )
{
    section( "The memory budget" );
    CResourceManager manager( 3000, 0 );
    manager.AddSource( std::make_shared<CDirectorySource>( DIR ) );

        // Nothing held: the oldest go to stay within 3000 bytes.
    for ( int n = 0; n < 10; n++ )
        CHECK( manager.Get( ( std::string( "levels/big" ) + std::to_string( n ) + ".bin" ).c_str() ) );
    CResourceManager::CStatistics stats = manager.GetStatistics();
    CHECK( stats.nBytes <= 3000 );
    CHECK( stats.nLoaded == 3 );
    CHECK( manager.IsLoaded( "levels/big9.bin" ) && ! manager.IsLoaded( "levels/big0.bin" ) );

        // Held resources stay, over the budget if need be.
    std::vector<std::shared_ptr<CLoadedResource> > aHeld;
    for ( int n = 0; n < 5; n++ )
        aHeld.push_back( manager.Get( ( std::string( "levels/big" ) + std::to_string( n ) + ".bin" ).c_str() ) );
    stats = manager.GetStatistics();
    CHECK( stats.nBytes == 5000 && stats.nLoaded == 5 );
    for ( int n = 0; n < 5; n++ )
        CHECK( manager.IsLoaded( ( std::string( "levels/big" ) + std::to_string( n ) + ".bin" ).c_str() ) );

        // Let go of them, and a lower budget trims.
    aHeld.clear();
    manager.SetBudget( 2000 );
    CHECK( manager.GetStatistics().nBytes <= 2000 );

        // Recently used survive.
    manager.SetBudget( 10000 );
    manager.Get( "levels/big0.bin" );
    manager.Get( "levels/big1.bin" );
    manager.Get( "levels/big0.bin" );           // now the most recent
    manager.Get( "levels/big2.bin" );
    manager.SetBudget( 2000 );
    CHECK( manager.IsLoaded( "levels/big0.bin" ) && manager.IsLoaded( "levels/big2.bin" ) );
    CHECK( ! manager.IsLoaded( "levels/big1.bin" ) );

        // Unloading.
    std::shared_ptr<CLoadedResource> pKeep = manager.Get( "levels/big3.bin" );
    manager.SetBudget( 100000 );
    manager.Get( "levels/big4.bin" );
    CHECK( manager.UnloadUnused() >= 1 );
    CHECK( manager.IsLoaded( "levels/big3.bin" ) && ! manager.IsLoaded( "levels/big4.bin" ) );
    CHECK( manager.Unload( "levels/big3.bin" ) );
    CHECK( ! manager.Unload( "levels/big3.bin" ) );
    CHECK( textOf( pKeep ).size() == 1000 );    // the holder keeps it
    CHECK( manager.Get( "levels/big3.bin" ) != pKeep );     // a fresh load
}

static void testReload( void )
{
    section( "Reloading" );
    CResourceManager manager( 1024 * 1024, 0 );
    manager.AddSource( std::make_shared<CDirectorySource>( DIR ) );
    writeFile( "rm_assets/levels/reload.txt", "version one" );
    std::shared_ptr<CLoadedResource> pOld = manager.Get( "levels/reload.txt" );
    CHECK( textOf( pOld ) == "version one" );
    writeFile( "rm_assets/levels/reload.txt", "version two" );
    CHECK( textOf( manager.Get( "levels/reload.txt" ) ) == "version one" );    // still the loaded one
    std::shared_ptr<CLoadedResource> pNew = manager.Reload( "levels/reload.txt" );
    CHECK( textOf( pNew ) == "version two" );
    CHECK( textOf( manager.Get( "levels/reload.txt" ) ) == "version two" );
    CHECK( textOf( pOld ) == "version one" );   // the old holder is unaffected
}

static void testAsync( void )
{
    section( "Background loading" );

    std::atomic<int> nLoads( 0 );
    CResourceManager manager( 1024 * 1024, 3 );
    manager.AddSource( std::make_shared<CDirectorySource>( DIR ) );
    manager.RegisterLoader( ".bin", [&nLoads]( const CString &, const CDataBlock &oData, CString & ) -> std::shared_ptr<CLoadedResource> {
        nLoads++;
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
        return std::make_shared<CRawResource>( oData );
    } );

    std::thread::id idMain = std::this_thread::get_id();
    std::atomic<int> nCallbacks( 0 );
    bool bOnMainThread = true;
    std::vector<std::string> aDone;
    for ( int n = 0; n < 10; n++ )
    {
        std::string strPath = std::string( "levels/big" ) + std::to_string( n ) + ".bin";
        manager.LoadAsync( strPath.c_str(), [&]( const CString &strDone, const std::shared_ptr<CLoadedResource> &p, const CString & ) {
            nCallbacks++;
            if ( std::this_thread::get_id() != idMain )
                bOnMainThread = false;
            if ( p && p->GetMemorySize() == 1000 )
                aDone.push_back( (const char *)strDone );
        } );
    }
        // Asked twice: one load, two callbacks.
    manager.LoadAsync( "levels/big0.bin", [&nCallbacks]( const CString &, const std::shared_ptr<CLoadedResource> &, const CString & ) { nCallbacks++; } );
        // A missing one reports its error.
    CString strMissingError;
    manager.LoadAsync( "levels/missing.bin", [&strMissingError]( const CString &, const std::shared_ptr<CLoadedResource> &p, const CString &strError ) {
        if ( ! p )
            strMissingError = strError;
    } );

    CHECK( nCallbacks == 0 );                   // nothing until ProcessCompleted()
    CHECK( manager.WaitForPending( 10000 ) );
    CHECK( nCallbacks == 0 );
    size_t nRun = manager.ProcessCompleted();
    CHECK( nRun == 12 );
    CHECK( nCallbacks == 11 );
    CHECK( bOnMainThread );
    CHECK( aDone.size() == 10 );
    CHECK( nLoads == 10 );
    CHECK( strMissingError.IndexOf( "not found" ) != NOT_FOUND );
    CHECK( manager.ProcessCompleted() == 0 );

        // Already loaded: the callback still comes through ProcessCompleted().
    bool bLoadedCallback = false;
    manager.LoadAsync( "levels/big5.bin", [&bLoadedCallback]( const CString &, const std::shared_ptr<CLoadedResource> &p, const CString & ) {
        bLoadedCallback = ( p != NULL );
    } );
    CHECK( manager.ProcessCompleted() == 1 && bLoadedCallback );
    CHECK( nLoads == 10 );

        // A Get() for a path loading in the background waits for that load.
    manager.Unload( "levels/big6.bin" );
    int nBefore = nLoads;
    manager.LoadAsync( "levels/big6.bin" );
    std::shared_ptr<CLoadedResource> pWaited = manager.Get( "levels/big6.bin" );
    CHECK( pWaited && pWaited->GetMemorySize() == 1000 );
    CHECK( nLoads == nBefore + 1 );

        // Many threads asking at once: one load each.
    manager.UnloadUnused();
    nBefore = nLoads;
    std::vector<std::thread> aThreads;
    std::atomic<int> nGot( 0 );
    for ( int t = 0; t < 6; t++ )
    {
        aThreads.push_back( std::thread( [&manager, &nGot]() {
            for ( int n = 0; n < 10; n++ )
            {
                if ( manager.Get( ( std::string( "levels/big" ) + std::to_string( n ) + ".bin" ).c_str() ) )
                    nGot++;
            }
        } ) );
    }
    for ( size_t t = 0; t < aThreads.size(); t++ )
        aThreads[t].join();
    CHECK( nGot == 60 );
        // One load per path (big6 is still held by pWaited, so it stayed).
    CHECK( nLoads == nBefore + 9 );

        // Destroyed with loads under way: no hang.
    {
        CResourceManager busy( 1024 * 1024, 2 );
        busy.AddSource( std::make_shared<CDirectorySource>( DIR ) );
        for ( int n = 0; n < 10; n++ )
            busy.LoadAsync( ( std::string( "levels/big" ) + std::to_string( n ) + ".bin" ).c_str() );
    }

        // Without threads, LoadAsync() loads at once; callbacks still wait.
    CResourceManager inline0( 1024 * 1024, 0 );
    inline0.AddSource( std::make_shared<CDirectorySource>( DIR ) );
    bool bCalled = false;
    inline0.LoadAsync( "levels/big7.bin", [&bCalled]( const CString &, const std::shared_ptr<CLoadedResource> &, const CString & ) { bCalled = true; } );
    CHECK( inline0.IsLoaded( "levels/big7.bin" ) && ! bCalled );
    CHECK( inline0.ProcessCompleted() == 1 && bCalled );
}

int main( void )
{
    makeAssets();
    testPaths();
    testSources();
    testBudget();
    testReload();
    testAsync();
    removeAssets();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
