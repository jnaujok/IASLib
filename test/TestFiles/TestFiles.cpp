/***********************************************************************
**  TestFiles
**
**  Tests for CFile's 64-bit ("Big") methods: seeking, sizes and
**  positions past 4 GB (in sparse files, so nothing large is written),
**  the long methods reporting -1 when a value does not fit, ReadBig() and
**  WriteBig(), and a resource archive with data past 4 GB.
***********************************************************************/

#include "IASLib.h"

#include <climits>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifdef IASLIB_WIN32__
    #include <winioctl.h>
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

static const int64_t FIVE_GB = (int64_t)5 * 1024 * 1024 * 1024;

    // An empty file that will not take real disk space when written far
    // past its end. (Linux and macOS do that anyway; NTFS needs asking.)
static bool makeSparse( const char *strPath )
{
#ifdef IASLIB_WIN32__
    HANDLE hFile = CreateFileA( strPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( hFile == INVALID_HANDLE_VALUE )
        return false;
    DWORD dwReturned = 0;
    BOOL bOk = DeviceIoControl( hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwReturned, NULL );
    CloseHandle( hFile );
    return bOk != 0;
#else
    FILE *pFile = fopen( strPath, "wb" );
    if ( pFile == NULL )
        return false;
    fclose( pFile );
    return true;
#endif
}

static void testBigFile( void )
{
    section( "CFile past 4 GB" );
    const char *strPath = "files_big.bin";
    if ( ! makeSparse( strPath ) )
    {
        printf( "  (cannot make a sparse file here; skipped)\n" );
        return;
    }

    {
        CFile oFile( strPath, CFile::READ | CFile::WRITE | CFile::BINARY );
        CHECK( oFile.IsOpen() );
        CHECK( oFile.WriteBig( "start", 5 ) == 5 );
        CHECK( oFile.SeekBig( FIVE_GB, CFile::BEGIN ) == FIVE_GB );
        CHECK( oFile.GetPosBig() == FIVE_GB );
        CHECK( oFile.WriteBig( "end", 3 ) == 3 );
        CHECK( oFile.GetPosBig() == FIVE_GB + 3 );
        CHECK( oFile.GetSizeBig() == FIVE_GB + 3 );

            // The long methods cannot say 5 GB where long is 32 bits.
        if ( sizeof( long ) == 4 )
        {
            CHECK( oFile.GetSize() == -1 );
            CHECK( oFile.GetPos() == -1 );
        }
        else
        {
            CHECK( oFile.GetSize() == (long)( FIVE_GB + 3 ) );
        }
        CHECK( oFile.Close() );
    }

    {
        CFile oFile( strPath, CFile::READ | CFile::BINARY );
        CHECK( oFile.GetSizeBig() == FIVE_GB + 3 );
        CHECK( oFile.SeekBig( 0, CFile::END ) == FIVE_GB + 3 );
        CHECK( oFile.SeekBig( -3, CFile::END ) == FIVE_GB );
        char achEnd[ 4 ] = { 0 };
        CHECK( oFile.ReadBig( achEnd, 3 ) == 3 && memcmp( achEnd, "end", 3 ) == 0 );
            // At the end: nothing more.
        CHECK( oFile.ReadBig( achEnd, 3 ) == 0 );

            // The gap reads as zeros.
        CHECK( oFile.SeekBig( (int64_t)4 * 1024 * 1024 * 1024, CFile::BEGIN ) == (int64_t)4 * 1024 * 1024 * 1024 );
        char achGap[ 8 ];
        memset( achGap, 1, sizeof( achGap ) );
        CHECK( oFile.ReadBig( achGap, sizeof( achGap ) ) == (int64_t)sizeof( achGap ) );
        bool bZeros = true;
        for ( size_t n = 0; n < sizeof( achGap ); n++ )
            if ( achGap[n] != 0 ) bZeros = false;
        CHECK( bZeros );

            // Relative seeks across the 2 and 4 GB marks.
        CHECK( oFile.SeekBig( 0, CFile::BEGIN ) == 0 );
        CHECK( oFile.SeekBig( (int64_t)3 * 1024 * 1024 * 1024, CFile::CURRENT ) == (int64_t)3 * 1024 * 1024 * 1024 );
        CHECK( oFile.SeekBig( (int64_t)2 * 1024 * 1024 * 1024, CFile::CURRENT ) == FIVE_GB );
        CHECK( oFile.SeekBig( -FIVE_GB, CFile::CURRENT ) == 0 );
        char achStart[ 6 ] = { 0 };
        CHECK( oFile.ReadBig( achStart, 5 ) == 5 && memcmp( achStart, "start", 5 ) == 0 );

            // A seek that cannot be done fails, and says so.
        CHECK( oFile.SeekBig( -1, CFile::BEGIN ) == -1 );
        CHECK( oFile.SeekBig( 0, 99 ) == -1 );
            // Small positions still work through the long methods.
        CHECK( oFile.Seek( 1, CFile::BEGIN ) == 1 );
        CHECK( oFile.GetPos() == 1 );
            // Not open for writing.
        CHECK( oFile.WriteBig( "x", 1 ) == -1 );
    }
    std::remove( strPath );
}

static void testReadWriteBig( void )
{
    section( "ReadBig() and WriteBig()" );
    const char *strPath = "files_rw.bin";
    std::vector<char> aData( 10 * 1024 * 1024 );
    for ( size_t n = 0; n < aData.size(); n++ )
        aData[n] = (char)( n * 31 + 7 );
    {
        CFile oFile( strPath, CFile::WRITE | CFile::BINARY );
        CHECK( oFile.WriteBig( &aData[0], aData.size() ) == (int64_t)aData.size() );
        CHECK( oFile.GetSizeBig() == (int64_t)aData.size() );
        CHECK( oFile.ReadBig( &aData[0], 1 ) == -1 );      // not open for reading
    }
    {
        CFile oFile( strPath, CFile::READ | CFile::BINARY );
        std::vector<char> aBack( aData.size() + 100 );
            // More than there is: what there is.
        CHECK( oFile.ReadBig( &aBack[0], aBack.size() ) == (int64_t)aData.size() );
        CHECK( memcmp( &aBack[0], &aData[0], aData.size() ) == 0 );
        CHECK( oFile.GetPosBig() == (int64_t)aData.size() );
    }
    CFile closed;
    char ch;
    CHECK( closed.ReadBig( &ch, 1 ) == -1 && closed.SeekBig( 0, CFile::BEGIN ) == -1 && closed.GetSizeBig() == -1 );
    std::remove( strPath );
}

static void testBigArchive( void )
{
    section( "A resource archive past 4 GB" );
    const char *strPath = "files_big.iasres";
    if ( ! makeSparse( strPath ) )
    {
        printf( "  (cannot make a sparse file here; skipped)\n" );
        return;
    }
    {
            // An archive in the (sparse) file...
        CResourceFile archive;
        CHECK( archive.Open( strPath, CResourceFile::CREATE ) );
        CResource small( 1, 1 );
        small.SetName( "small" );
        small.SetData( "near the start", 14 );
        CHECK( archive.Put( small ) );
    }
    {
            // ...pushed past 5 GB: the bytes after the index are unused.
        CFile oFile( strPath, CFile::READ | CFile::WRITE | CFile::BINARY );
        CHECK( oFile.SeekBig( FIVE_GB, CFile::BEGIN ) == FIVE_GB );
        CHECK( oFile.WriteBig( "", 1 ) == 1 );
    }
    {
            // New data goes at the end, past 5 GB.
        CResourceFile archive( strPath, CResourceFile::READ_WRITE );
        CHECK( archive.IsOpen() );
        CHECK( archive.GetWastedBytes() > (uint64_t)FIVE_GB - 1024 );
        CResource far( 1, 2 );
        far.SetName( "far" );
        far.SetData( "past five gigabytes", 19 );
        CHECK( archive.Put( far ) );
        CResourceIndexEntry entry;
        CHECK( archive.GetEntry( 1, 2, entry ) && entry.m_nOffset > (uint64_t)FIVE_GB );
    }
    {
        CResourceFile archive( strPath, CResourceFile::READ_ONLY );
        CHECK( archive.IsOpen() );
        CResource out;
        CHECK( archive.Read( 1, 2, out ) );
        CHECK( std::string( (const char *)out.GetData(), out.GetLength() ) == "past five gigabytes" );
        CHECK( archive.Read( 1, 1, out ) );
        CHECK( std::string( (const char *)out.GetData(), out.GetLength() ) == "near the start" );
    }
    std::remove( strPath );
}

int main( void )
{
    testBigFile();
    testReadWriteBig();
    testBigArchive();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
