/***********************************************************************
**  TestResources
**
**  Tests for the resource archive: CResource, CResourceFile (create,
**  put, get, replace, remove, reopen, folders and names, the cache,
**  wasted space and Compact(), damaged and foreign files, read-only use,
**  threads) and the directory view.
***********************************************************************/

#include "IASLib.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
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

static std::string readFile( const std::string &strPath )
{
    std::ifstream in( strPath.c_str(), std::ios::binary );
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void writeFile( const std::string &strPath, const std::string &strData )
{
    std::ofstream out( strPath.c_str(), std::ios::binary | std::ios::trunc );
    out << strData;
}

static long fileLength( const std::string &strPath )
{
    return (long)readFile( strPath ).size();
}

static CResource makeResource( short nFolder, short nResource, const std::string &strName, const std::string &strData, int nType = 0 )
{
    CResource oResource( nFolder, nResource );
    oResource.SetName( strName.c_str() );
    oResource.SetModifiedBy( "tester" );
    oResource.SetResourceType( nType );
    oResource.SetData( strData.data(), strData.size() );
    return oResource;
}

static std::string dataOf( const CResource &oResource )
{
    return std::string( (const char *)oResource.GetData(), oResource.GetLength() );
}

static void testCRC( void )
{
    section( "CRC-32" );
        // The standard check value.
    CHECK( CResourceFile::CRC32( "123456789", 9 ) == 0xCBF43926U );
    CHECK( CResourceFile::CRC32( NULL, 0 ) == 0 );
        // In pieces.
    uint32_t nPart = CResourceFile::CRC32( "1234", 4 );
    CHECK( CResourceFile::CRC32( "56789", 5, nPart ) == 0xCBF43926U );
}

static void testResource( void )
{
    section( "CResource" );

    CResource empty;
    CHECK( empty.GetLength() == 0 && empty.GetData() == NULL );
    CHECK( empty.GetFolder() == 0 && empty.GetResource() == 0 );

    CResource oResource( 3, 7 );
    CHECK( oResource.GetID() == ( ( 3u << 16 ) | 7u ) );
    CResource negative( -1, -2 );
    CHECK( negative.GetID() == 0xFFFFFFFEU );

    oResource.SetData( "abc", 3 );
    CHECK( dataOf( oResource ) == "abc" );
    CHECK( oResource.GetLastModified().IsValid() );

    writeFile( "res_source.bin", std::string( "\0\1\2binary\0", 10 ) );
    CResource fromFile( 1, 1, "bin", "me", 5, "res_source.bin" );
    CHECK( fromFile.GetLength() == 10 );
    CHECK( fromFile.GetResourceType() == 5 && fromFile.GetName() == "bin" && fromFile.GetModifiedBy() == "me" );
    CHECK( fromFile.SaveData( "res_copy.bin" ) );
    CHECK( readFile( "res_copy.bin" ) == std::string( "\0\1\2binary\0", 10 ) );
    CHECK( ! fromFile.LoadData( "no_such_file.bin" ) );
    CHECK( fromFile.GetLength() == 10 );        // unchanged
    std::remove( "res_source.bin" );
    std::remove( "res_copy.bin" );

    CResource copy( fromFile );
    CHECK( dataOf( copy ) == dataOf( fromFile ) );
    CHECK( copy.GetType() != NULL );            // the CObject name, untouched
}

static void testArchive( void )
{
    section( "CResourceFile" );
    const char *strPath = "res_test.iasres";
    std::remove( strPath );

    {
        CResourceFile archive( strPath, CResourceFile::CREATE );
        CHECK( archive.IsOpen() );
        CHECK( archive.GetCount() == 0 );
        CHECK( fileLength( strPath ) == (long)CResourceFile::HEADER_SIZE + 4 );

        CHECK( archive.Put( makeResource( 1, 1, "greeting", "hello, world", 10 ) ) );
        CHECK( archive.Put( makeResource( 1, 2, "empty", "" ) ) );
        std::string strBig( 200000, 'z' );
        strBig[ 1234 ] = '!';
        CHECK( archive.Put( makeResource( 2, 1, "big", strBig ) ) );
        CHECK( archive.Put( makeResource( -5, 9, "negative folder", "neg" ) ) );
        CHECK( archive.GetCount() == 4 );
        CHECK( archive.Exists( 1, 1 ) && ! archive.Exists( 1, 3 ) );

        std::shared_ptr<const CResource> p = archive.Get( 1, 1 );
        CHECK( p && dataOf( *p ) == "hello, world" && p->GetResourceType() == 10 && p->GetName() == "greeting" );
        CResource out;
        CHECK( archive.Read( 2, 1, out ) && dataOf( out ) == strBig );
        CHECK( archive.Read( 1, 2, out ) && out.GetLength() == 0 );
        CHECK( ! archive.Read( 9, 9, out ) );
        CHECK( ! archive.Get( 9, 9 ) );

        short nFolder = 0, nResource = 0;
        CHECK( archive.FindByName( "big", nFolder, nResource ) && nFolder == 2 && nResource == 1 );
        CHECK( ! archive.FindByName( "nothing", nFolder, nResource ) );
    }

        // Reopened: everything is there, with its details.
    {
        CResourceFile archive( strPath, CResourceFile::READ_WRITE );
        CHECK( archive.IsOpen() );
        CHECK( archive.GetCount() == 4 );
        CResource out;
        CHECK( archive.Read( 1, 1, out ) );
        CHECK( dataOf( out ) == "hello, world" );
        CHECK( out.GetName() == "greeting" && out.GetModifiedBy() == "tester" && out.GetResourceType() == 10 );
        CResourceIndexEntry entry;
        CHECK( archive.GetEntry( 1, 1, entry ) && entry.m_nLength == 12 && entry.m_nCRC == CResourceFile::CRC32( "hello, world", 12 ) );
        CHECK( entry.m_dttLastModified.GetEpochDay() == out.GetLastModified().GetEpochDay() );
        CHECK( archive.Read( -5, 9, out ) && dataOf( out ) == "neg" );

            // Folders, in ID order (as unsigned tags), and a folder's view.
        std::vector<short> aFolders = archive.GetFolders();
        CHECK( aFolders.size() == 3 && aFolders[0] == 1 && aFolders[1] == 2 && aFolders[2] == -5 );
        CResourceDirectoryNode folder1 = archive.GetDirectory( 1 );
        CHECK( folder1.IsDirectory() && folder1.GetFolder() == 1 && folder1.GetCount() == 2 );
        CHECK( folder1.GetDirectorySize() == 12 );
        CHECK( folder1.Get( 0 ).GetName() == "greeting" && folder1.Get( 1 ).GetName() == "empty" );
        CHECK( ! folder1.Get( 0 ).IsDirectory() && ! folder1.Get( 0 ).IsCompressed() );
        CHECK( archive.GetDirectory( 77 ).GetCount() == 0 );
        CHECK( archive.GetEntries().size() == 4 );

            // Each flush leaves the index it replaced behind as waste.
        CHECK( archive.GetWastedBytes() > 0 && archive.GetWastedBytes() < 1000 );
            // Replacing and removing waste space; Compact() takes it back.
        CHECK( archive.Put( makeResource( 1, 1, "greeting", "hi" ) ) );
        CHECK( archive.Read( 1, 1, out ) && dataOf( out ) == "hi" );
        CHECK( archive.Remove( 2, 1 ) );
        CHECK( ! archive.Remove( 2, 1 ) );
        CHECK( archive.GetCount() == 3 );
        CHECK( archive.GetWastedBytes() >= 200000 + 12 );
        long nBefore = fileLength( strPath );
        CHECK( archive.Compact() );
        CHECK( archive.GetWastedBytes() == 0 );
        CHECK( fileLength( strPath ) < nBefore - 200000 );
        CHECK( archive.Read( 1, 1, out ) && dataOf( out ) == "hi" );
        CHECK( archive.Read( -5, 9, out ) && dataOf( out ) == "neg" );
            // And it still works after compacting.
        CHECK( archive.Put( makeResource( 3, 3, "after", "compact" ) ) );
    }
    {
        CResourceFile archive( strPath, CResourceFile::READ_ONLY );
        CHECK( archive.IsOpen() && archive.IsReadOnly() );
        CHECK( archive.GetCount() == 4 );
            // Only the index the last Put() replaced.
        CHECK( archive.GetWastedBytes() > 0 && archive.GetWastedBytes() < 200 );
        CResource out;
        CHECK( archive.Read( 3, 3, out ) && dataOf( out ) == "compact" );
        CHECK( ! archive.Put( makeResource( 4, 4, "x", "y" ) ) );
        CHECK( ! archive.Remove( 3, 3 ) );
        CHECK( ! archive.Compact() );
        CHECK( archive.GetLastError() == "read-only" );
    }

        // Without auto-flush, nothing reaches the index until Flush().
    {
        CResourceFile archive( strPath );
        archive.SetAutoFlush( false );
        CHECK( archive.Put( makeResource( 5, 5, "batched", "later" ) ) );
        {
            CResourceFile other( strPath, CResourceFile::READ_ONLY );
            CHECK( ! other.Exists( 5, 5 ) );
        }
        CHECK( archive.Flush() );
        {
            CResourceFile other( strPath, CResourceFile::READ_ONLY );
            CHECK( other.Exists( 5, 5 ) );
        }
        CHECK( archive.Put( makeResource( 5, 6, "on close", "flushed" ) ) );
    }
    {
        CResourceFile archive( strPath, CResourceFile::READ_ONLY );
        CHECK( archive.Exists( 5, 6 ) );        // Close() flushed it
    }

        // An unflushed write leaves the archive as it was (a crash).
    {
        CResourceFile archive( strPath );
        archive.SetAutoFlush( false );
        archive.Put( makeResource( 6, 6, "lost", "never indexed" ) );
        std::string strImage = readFile( strPath );     // as if it stopped here
        writeFile( "res_crash.iasres", strImage );
    }
    {
        CResourceFile crashed( "res_crash.iasres", CResourceFile::READ_ONLY );
        CHECK( crashed.IsOpen() );
        CHECK( crashed.GetCount() == 6 && ! crashed.Exists( 6, 6 ) && crashed.Exists( 5, 6 ) );
    }
    std::remove( "res_crash.iasres" );
    std::remove( strPath );
}

static void testDamage( void )
{
    section( "Damaged and foreign files" );
    const char *strPath = "res_damage.iasres";

    {
        CResourceFile archive( strPath, CResourceFile::CREATE );
        archive.Put( makeResource( 1, 1, "a", "payload-one" ) );
        archive.Put( makeResource( 1, 2, "b", "payload-two" ) );
    }
    std::string strGood = readFile( strPath );

        // A flipped byte in a resource's data: that resource is refused.
    std::string strBad = strGood;
    size_t nAt = strBad.find( "payload-one" );
    CHECK( nAt != std::string::npos );
    strBad[ nAt ] = 'P';
    writeFile( strPath, strBad );
    {
        CResourceFile archive( strPath, CResourceFile::READ_ONLY );
        CHECK( archive.IsOpen() );
        CResource out;
        CHECK( ! archive.Read( 1, 1, out ) );
        CHECK( archive.GetLastError().IndexOf( "damaged" ) != NOT_FOUND );
        CHECK( archive.Read( 1, 2, out ) && dataOf( out ) == "payload-two" );
    }

        // A flipped byte in the index: the archive is refused.
    strBad = strGood;
    strBad[ strBad.size() - 10 ] ^= 0x40;
    writeFile( strPath, strBad );
    {
        CResourceFile archive( strPath, CResourceFile::READ_ONLY );
        CHECK( ! archive.IsOpen() );
        CHECK( archive.GetLastError().IndexOf( "index is damaged" ) != NOT_FOUND );
    }

        // Cut short, not an archive, empty.
    writeFile( strPath, strGood.substr( 0, strGood.size() - 20 ) );
    {
        CResourceFile archive( strPath, CResourceFile::READ_ONLY );
        CHECK( ! archive.IsOpen() );
    }
    writeFile( strPath, "this is a text file, not an archive at all" );
    {
        CResourceFile archive( strPath, CResourceFile::READ_WRITE );
        CHECK( ! archive.IsOpen() );
        CHECK( archive.GetLastError().IndexOf( "not a resource archive" ) != NOT_FOUND );
            // READ_WRITE does not replace a file that is not an archive.
    }
    CHECK( readFile( strPath ) == "this is a text file, not an archive at all" );
    writeFile( strPath, "" );
    {
        CResourceFile archive( strPath, CResourceFile::READ_ONLY );
        CHECK( ! archive.IsOpen() );
    }
    {
        CResourceFile archive( "no_such_directory_xyz/archive.iasres", CResourceFile::READ_WRITE );
        CHECK( ! archive.IsOpen() );
        CResource out;
        CHECK( ! archive.Read( 1, 1, out ) );
        CHECK( ! archive.Put( makeResource( 1, 1, "x", "y" ) ) );
    }
    std::remove( strPath );
}

static void testCache( void )
{
    section( "The cache" );
    const char *strPath = "res_cache.iasres";
    {
        CResourceFile archive( strPath, CResourceFile::CREATE );
        for ( short n = 0; n < 10; n++ )
            archive.Put( makeResource( 1, n, "r", std::string( 1000, (char)( 'a' + n ) ) ) );
    }
    CResourceFile archive( strPath, CResourceFile::READ_ONLY );
    archive.SetCacheLimit( 3000 );
    CHECK( archive.GetCachedCount() == 0 );
    for ( short n = 0; n < 10; n++ )
        CHECK( archive.Get( 1, n ) );
    CHECK( archive.GetCachedCount() == 3 );
    CHECK( archive.GetCacheBytes() == 3000 );

        // A held resource outlives its eviction.
    std::shared_ptr<const CResource> pHeld = archive.Get( 1, 0 );
    for ( short n = 1; n < 10; n++ )
        archive.Get( 1, n );
    CHECK( pHeld && pHeld->GetLength() == 1000 && pHeld->GetData()[0] == 'a' );

        // Use counts.
    CResourceIndexEntry entry;
    CHECK( archive.GetEntry( 1, 0, entry ) && entry.m_nUsage == 2 );

    archive.SetCacheLimit( 0 );
    CHECK( archive.GetCachedCount() == 0 && archive.GetCacheBytes() == 0 );
    CHECK( archive.Get( 1, 5 ) );               // still readable, just not kept
    CHECK( archive.GetCachedCount() == 0 );
    archive.Close();
    std::remove( strPath );
}

static void testThreads( void )
{
    section( "Threads" );
    const char *strPath = "res_threads.iasres";
    CResourceFile archive( strPath, CResourceFile::CREATE );
    archive.SetAutoFlush( false );
    archive.SetCacheLimit( 4096 );
    for ( short n = 0; n < 20; n++ )
        archive.Put( makeResource( 1, n, "r", std::string( 300, (char)( 'a' + n ) ) ) );

    bool abOk[ 4 ] = { true, true, true, true };
    std::vector<std::thread> threads;
    for ( int t = 0; t < 3; t++ )
    {
        threads.push_back( std::thread( [&archive, &abOk, t]() {
            for ( int i = 0; i < 300; i++ )
            {
                short n = (short)( ( i * 7 + t ) % 20 );
                std::shared_ptr<const CResource> p = archive.Get( 1, n );
                if ( ! p || p->GetLength() != 300 || p->GetData()[ 299 ] != (unsigned char)( 'a' + n ) )
                    abOk[t] = false;
            }
        } ) );
    }
    threads.push_back( std::thread( [&archive, &abOk]() {
        for ( short n = 0; n < 100; n++ )
        {
            if ( ! archive.Put( makeResource( 2, n, "w", std::string( 50, 'w' ) ) ) )
                abOk[3] = false;
        }
    } ) );
    for ( size_t t = 0; t < threads.size(); t++ )
        threads[t].join();
    CHECK( abOk[0] && abOk[1] && abOk[2] && abOk[3] );
    CHECK( archive.GetCount() == 120 );
    CHECK( archive.Flush() );
    archive.Close();
    {
        CResourceFile reopened( strPath, CResourceFile::READ_ONLY );
        CHECK( reopened.GetCount() == 120 );
    }
    std::remove( strPath );
}

int main( void )
{
    testCRC();
    testResource();
    testArchive();
    testDamage();
    testCache();
    testThreads();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
