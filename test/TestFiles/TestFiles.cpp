/***********************************************************************
**  TestFiles
**
**  Tests for the Files folder: CFile (modes, lines, end of file,
**  Rename() replacing, Exists()), its 64-bit ("Big") methods past 4 GB (in
**  sparse files, so nothing large is written), CFileStat, CDirectory and
**  CDirectoryEntry, CMemMappedFile, CRWFile, and a resource archive with
**  data past 4 GB.
***********************************************************************/

#include "IASLib.h"

#include <climits>
#include <fstream>
#include <sstream>
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


static std::string slurp( const char *strPath )
{
    std::ifstream in( strPath, std::ios::binary );
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void spit( const char *strPath, const std::string &strData )
{
    std::ofstream out( strPath, std::ios::binary | std::ios::trunc );
    out << strData;
}

static bool throwsFileException( void (*fn)( void ) )
{
    try
    {
        fn();
    }
    catch ( CFileException & )
    {
        return true;
    }
    return false;
}

static void testCFile( void )
{
    section( "CFile" );
    const char *strPath = "files_basic.txt";
    std::remove( strPath );

        // CREATE: a new file, both ways.
    {
        CFile oFile( strPath, CFile::READ | CFile::WRITE | CFile::CREATE | CFile::BINARY );
        CHECK( oFile.IsOpen() );
        CHECK( oFile.WriteString( "line one\nline two\r\n" ) == 19 );
        CHECK( oFile.SeekBig( 0, CFile::BEGIN ) == 0 );
        char achBuffer[ 8 ] = { 0 };
        CHECK( oFile.Read( achBuffer, 4 ) == 4 && memcmp( achBuffer, "line", 4 ) == 0 );
        CHECK( oFile.GetFileName() == strPath );
        CHECK( oFile.Close() );
        CHECK( ! oFile.Close() );
    }
    CHECK( CFile::Exists( strPath ) );
    CHECK( ! CFile::Exists( "files_no_such_file" ) );
    CHECK( ! CFile::Exists( NULL ) );

        // Lines, with their endings; binary mode works too; any length.
    {
        CFile oFile( strPath, CFile::READ | CFile::BINARY );
        CHECK( ! oFile.IsEOF() );
        CHECK( oFile.GetLine() == "line one\n" );
        CHECK( oFile.GetLine() == "line two\r\n" );
        CHECK( oFile.IsEOF() );
        CHECK( oFile.GetLine() == "" );
    }
    std::string strLong( 5000, 'q' );
    spit( strPath, strLong + "\nshort" );
    {
        CFile oFile( strPath, CFile::READ );
        CString strLine = oFile.GetLine();
        CHECK( strLine.GetLength() == 5001 );
        CHECK( oFile.GetLine() == "short" );          // no newline at the end
        CHECK( oFile.IsEOF() );
    }

        // Reopening in another mode resets the flags (they stuck).
    {
        CFile oFile( strPath, CFile::WRITE );
        CHECK( oFile.Write( "x", 1 ) == 1 );
        CHECK( oFile.Open( strPath, CFile::READ ) );
        char ch = 0;
        CHECK( oFile.Read( &ch, 1 ) == 1 && ch == 'x' );
        CHECK( oFile.Write( "y", 1 ) == -1 );          // read only now
    }

        // APPEND.
    {
        CFile oFile( strPath, CFile::APPEND );
        oFile.WriteString( "z" );
    }
    CHECK( slurp( strPath ) == "xz" );

        // Rename() replaces the target, Windows included.
    spit( "files_other.txt", "old" );
    CHECK( CFile::Rename( strPath, "files_other.txt" ) );
    CHECK( slurp( "files_other.txt" ) == "xz" );
    CHECK( ! CFile::Exists( strPath ) );
    CHECK( CFile::Delete( "files_other.txt" ) );
    CHECK( throwsFileException( []() { CFile::Delete( "files_no_such_file" ); } ) );
    CHECK( throwsFileException( []() { CFile::Rename( "files_no_such_file", "files_x" ); } ) );
    CHECK( throwsFileException( []() { CFile oFile( "files_no_such_file", CFile::READ ); } ) );
}

static void testFileStat( void )
{
    section( "CFileStat" );
    spit( "files_stat.bin", std::string( 1234, 'a' ) );
    CFileStat oStat( "files_stat.bin" );
    CHECK( oStat.IsFile() && ! oStat.IsDir() );
    CHECK( oStat.GetSizeBig() == 1234 && oStat.GetSize() == 1234 );
    CDate dttNow;
    CHECK( oStat.GetLastModified().IsValid() );
    CHECK( oStat.GetLastModified().GetEpochDay() >= dttNow.GetEpochDay() - 1 );
    std::remove( "files_stat.bin" );

    CDirectory::Create( "files_stat_dir" );
    CFileStat oDir( "files_stat_dir" );
    CHECK( oDir.IsDir() && ! oDir.IsFile() && oDir.GetSizeBig() == 0 );
    CDirectory::Remove( "files_stat_dir" );

        // A name with a trailing space is a real name (it was trimmed).
    spit( "files_space ", "s" );
    bool bFound = false;
    try
    {
        CFileStat oSpace( "files_space " );
        bFound = oSpace.IsFile() && oSpace.GetSizeBig() == 1;
    }
    catch ( CFileException & )
    {
    }
#ifndef IASLIB_WIN32__
    CHECK( bFound );        // (Windows drops trailing spaces from names itself)
#else
    (void)bFound;
#endif
    std::remove( "files_space " );

    CHECK( throwsFileException( []() { CFileStat oMissing( "files_no_such_file" ); } ) );
}

static void testDirectory( void )
{
    section( "CDirectory" );
    CHECK( CDirectory::Create( "files_dir/a/b/c", true ) );
    CHECK( CDirectory::Exists( "files_dir/a/b/c" ) );
    CHECK( CDirectory::Create( "files_dir/a/b/c", true ) );     // already there
    CHECK( ! CDirectory::Create( "files_dir/x/y" ) );           // no parents asked for
    spit( "files_dir/zeta.log", "zz" );
    spit( "files_dir/alpha.log", "a" );
    spit( "files_dir/mid.txt", "mmm" );
    spit( "files_dir/.hidden", "h" );

    CDirectory oAll( "files_dir" );
    CHECK( oAll.IsValid() );
        // Sorted, with the dot entries.
    std::vector<std::string> aNames;
    for ( size_t n = 0; n < oAll.GetEntryCount(); n++ )
        aNames.push_back( oAll.GetFileName( n ) );
    std::vector<std::string> aExpected = { ".", "..", ".hidden", "a", "alpha.log", "mid.txt", "zeta.log" };
    CHECK( aNames == aExpected );
    CDirectoryEntry *pA = NULL;
    CDirectoryEntry *pMid = NULL;
    for ( size_t n = 0; n < oAll.GetEntryCount(); n++ )
    {
        if ( std::string( oAll.GetFileName( n ) ) == "a" ) pA = oAll.GetEntry( n );
        if ( std::string( oAll.GetFileName( n ) ) == "mid.txt" ) pMid = oAll.GetEntry( n );
        if ( oAll.GetEntry( n )->IsDotEntry() ) CHECK( oAll.GetEntry( n )->m_strFileName.GetLength() <= 2 );
    }
    CHECK( pA && pA->IsDirectory() && pA->m_strExtension == "" );
    CHECK( pMid && ! pMid->IsDirectory() && pMid->m_lSize == 3 && pMid->m_strExtension == "txt" );
    CHECK( pMid && pMid->m_strEntryName == "files_dir/mid.txt" );
    CHECK( oAll.GetEntry( 1000 ) == NULL && oAll.GetFileName( 1000 ) == NULL );

        // A filter.
    CDirectory oLogs( "files_dir", "*.log" );
    CHECK( oLogs.GetEntryCount() == 2 );
    CHECK( std::string( oLogs.GetFileName( 0 ) ) == "alpha.log" && std::string( oLogs.GetFileName( 1 ) ) == "zeta.log" );

        // Lightweight: names only.
    CDirectory oLight( "files_dir", "mid.txt", true );
    CHECK( oLight.GetEntryCount() == 1 && oLight.GetEntry( 0 )->m_lSize == 0 );

        // Refresh() sees changes.
    spit( "files_dir/new.log", "n" );
    oLogs.Refresh();
    CHECK( oLogs.GetEntryCount() == 3 );

        // A directory that is not there (it crashed).
    CDirectory oMissing( "files_no_such_dir" );
    CHECK( ! oMissing.IsValid() && oMissing.GetEntryCount() == 0 );

    const char *apFiles[] = { "files_dir/zeta.log", "files_dir/alpha.log", "files_dir/mid.txt", "files_dir/.hidden", "files_dir/new.log" };
    for ( size_t n = 0; n < 5; n++ )
        std::remove( apFiles[n] );
    CHECK( ! CDirectory::Remove( "files_dir/a" ) );     // not empty
    CHECK( CDirectory::Remove( "files_dir/a/b/c" ) && CDirectory::Remove( "files_dir/a/b" ) &&
           CDirectory::Remove( "files_dir/a" ) && CDirectory::Remove( "files_dir" ) );
    CHECK( ! CDirectory::Exists( "files_dir" ) );
}

static void testMemMappedFile( void )
{
    section( "CMemMappedFile" );
    const char *strPath = "files_mapped.bin";
    std::remove( strPath );

        // A new file: writes grow it, Close() trims it to what was written.
    {
        CMemMappedFile oFile( strPath, CFile::CREATE );
        CHECK( oFile.IsOpen() && oFile.IsWritable() );
        CHECK( oFile.GetSizeBig() == 0 && oFile.GetData() == NULL );
        CHECK( oFile.WriteString( "hello, " ) == 7 );
        CHECK( oFile.Write( "mapped world\n", 13 ) == 13 );
        CHECK( oFile.GetSizeBig() == 20 );
        CHECK( oFile.GetData() != NULL && memcmp( oFile.GetData(), "hello, mapped world\n", 20 ) == 0 );
        CHECK( oFile.Flush() );
        CHECK( oFile.Close() );
    }
    CHECK( slurp( strPath ) == "hello, mapped world\n" );

        // Reading, seeking, lines.
    {
        CMemMappedFile oFile( strPath, CFile::READ );
        CHECK( oFile.IsOpen() && ! oFile.IsWritable() );
        CHECK( oFile.GetSize() == 20 );
        char achBuffer[ 32 ] = { 0 };
        CHECK( oFile.Read( achBuffer, 5 ) == 5 && memcmp( achBuffer, "hello", 5 ) == 0 );
        CHECK( oFile.GetPos() == 5 );
        CHECK( oFile.Seek( -6, CFile::END ) == 14 );
        CHECK( oFile.Read( achBuffer, 100 ) == 6 && memcmp( achBuffer, "world\n", 6 ) == 0 );
        CHECK( oFile.IsEOF() );
        CHECK( oFile.Read( achBuffer, 1 ) == 0 );
        CHECK( oFile.Seek( 0, CFile::BEGIN ) == 0 );
        CHECK( oFile.GetLine() == "hello, mapped world\n" );
        CHECK( oFile.GetLine() == "" );
        CHECK( oFile.Write( "x", 1 ) == -1 );           // read only
        CHECK( oFile.Seek( -1, CFile::BEGIN ) == -1 );
    }

        // Separate read and write pointers.
    {
        CMemMappedFile oFile( strPath, CFile::READ | CFile::WRITE );
        oFile.SetSyncPointers( false );
        CHECK( ! oFile.GetSyncPointers() );
        CHECK( oFile.SeekWrite( 0, CFile::END ) == 20 );
        CHECK( oFile.WriteString( "second line\n" ) == 12 );
        CHECK( oFile.GetReadPos() == 0 && oFile.GetWritePos() == 32 );
        CHECK( oFile.GetLine() == "hello, mapped world\n" );
        CHECK( oFile.GetLine() == "second line\n" );
        CHECK( oFile.SeekRead( 7, CFile::BEGIN ) == 7 && oFile.GetWritePos() == 32 );
            // Seek() moves both.
        CHECK( oFile.Seek( 0, CFile::BEGIN ) == 0 && oFile.GetReadPos() == 0 && oFile.GetWritePos() == 0 );
        CHECK( oFile.Write( "HELLO", 5 ) == 5 && oFile.GetReadPos() == 0 );
        oFile.SetSyncPointers( true );
            // Direct writes through the mapping.
        char *pData = oFile.GetWritableData();
        CHECK( pData != NULL );
        if ( pData )
            pData[5] = ';';
    }
    CHECK( slurp( strPath ) == "HELLO; mapped world\nsecond line\n" );

        // Writing past the end leaves zeros; Resize() both ways; APPEND.
    {
        CMemMappedFile oFile( strPath, CFile::READ | CFile::WRITE );
        CHECK( oFile.Seek( 100, CFile::BEGIN ) == 100 );
        CHECK( oFile.Write( "!", 1 ) == 1 );
        CHECK( oFile.GetSizeBig() == 101 );
        CHECK( oFile.GetData()[ 50 ] == 0 );
        CHECK( oFile.Resize( 10 ) && oFile.GetSizeBig() == 10 );
        CHECK( oFile.Resize( 12 ) && oFile.GetData()[ 11 ] == 0 );
    }
    CHECK( slurp( strPath ) == std::string( "HELLO; map\0\0", 12 ) );
    {
        CMemMappedFile oFile( strPath, CFile::APPEND );
        CHECK( oFile.Seek( 0, CFile::BEGIN ) == 0 );
        CHECK( oFile.WriteString( "END" ) == 3 );       // at the end, wherever the pointer was
    }
    CHECK( slurp( strPath ) == std::string( "HELLO; map\0\0END", 15 ) );

        // A large write: many growth steps, all trimmed at the end.
    {
        CMemMappedFile oFile( strPath, CFile::CREATE );
        std::string strChunk( 1000, 'k' );
        for ( int n = 0; n < 3000; n++ )
            CHECK( oFile.WriteBig( strChunk.data(), strChunk.size() ) == 1000 );
        CHECK( oFile.GetSizeBig() == 3000000 );
    }
    CFileStat oStat( strPath );
    CHECK( oStat.GetSizeBig() == 3000000 );

        // Through the CFile interface.
    {
        CMemMappedFile oMapped( strPath, CFile::READ );
        CFile *pFile = &oMapped;
        CHECK( pFile->GetSizeBig() == 3000000 );
        CHECK( pFile->SeekBig( 2999999, CFile::BEGIN ) == 2999999 );
        char ch = 0;
        CHECK( pFile->ReadBig( &ch, 1 ) == 1 && ch == 'k' );
        CHECK( pFile->IsEOF() );
    }

        // Errors.
    CHECK( throwsFileException( []() { CMemMappedFile oMissing( "files_no_such_file", CFile::READ ); } ) );
    CMemMappedFile oClosed;
    char ch;
    CHECK( ! oClosed.IsOpen() && oClosed.Read( &ch, 1 ) == -1 && oClosed.GetSizeBig() == -1 && ! oClosed.Close() );
    std::remove( strPath );

#ifndef IASLIB_WIN32__
        // Past 4 GB, in a sparse file (64-bit address spaces only).
    if ( sizeof( void * ) == 8 )
    {
        const char *strBig = "files_mapped_big.bin";
        {
            CFile oFile( strBig, CFile::WRITE | CFile::BINARY );
            oFile.SeekBig( FIVE_GB, CFile::BEGIN );
            oFile.WriteBig( "far", 3 );
        }
        {
            CMemMappedFile oFile( strBig, CFile::READ | CFile::WRITE );
            CHECK( oFile.GetSizeBig() == FIVE_GB + 3 );
            CHECK( memcmp( oFile.GetData() + FIVE_GB, "far", 3 ) == 0 );
            CHECK( oFile.SeekBig( FIVE_GB, CFile::BEGIN ) == FIVE_GB );
            CHECK( oFile.WriteBig( "FAR", 3 ) == 3 );
        }
        {
            CFile oFile( strBig, CFile::READ | CFile::BINARY );
            char achEnd[ 3 ];
            CHECK( oFile.SeekBig( FIVE_GB, CFile::BEGIN ) == FIVE_GB && oFile.ReadBig( achEnd, 3 ) == 3 && memcmp( achEnd, "FAR", 3 ) == 0 );
        }
        std::remove( strBig );
    }
#endif
}

static void testRWFile( void )
{
    section( "CRWFile" );
    const char *strPath = "files_rw.log";
    std::remove( strPath );
    std::remove( "files_rw.log.idx" );

        // A reader needs a writer to have started.
    CRWFile oEarly( strPath, CRWFile::READ );
    CHECK( ! oEarly.IsOpen() );

    {
        CRWFile oWriter( strPath, CRWFile::WRITE );
        CHECK( oWriter.IsOpen() );
        CHECK( oWriter.WriteLine( "one" ) );
        CHECK( oWriter.WriteLine( "two\nthree" ) );     // two lines
        CHECK( oWriter.GetPos() == 3 );

        CRWFile oReader( strPath, CRWFile::READ );
        CHECK( oReader.IsOpen() );
        CHECK( oReader.GetLine() == "one\n" );
        CHECK( oReader.GetLine() == "two\n" );
        CHECK( ! oReader.IsWriterDone() );

            // A line being written is not taken until it is finished.
        CHECK( oWriter.Write( "fo", 2 ) );
        CHECK( oReader.GetLine() == "three\n" );
        CHECK( oReader.GetLine() == "" );
        CHECK( oReader.GetPos() == 3 );
        CHECK( oWriter.Write( "ur\n", 3 ) );
        CHECK( oReader.GetLine() == "four\n" );
        CHECK( oReader.IsEOF() );
        oReader.Close();

            // A new reader picks up where the last one stopped.
        oWriter.WriteLine( "five" );
        CRWFile oReader2( strPath, CRWFile::READ );
        CHECK( oReader2.GetPos() == 4 );
        CHECK( oReader2.GetLine() == "five\n" );
        oWriter.Close();            // finished
        CHECK( oReader2.IsWriterDone() );
    }

        // The index is two 16-byte records on every system.
    std::string strIndex = slurp( "files_rw.log.idx" );
    CHECK( strIndex.size() == 32 );
    CHECK( atol( strIndex.c_str() ) == -1 && atol( strIndex.c_str() + 16 ) == 5 );

        // Reopening a finished file adds to the end (it wrote over line 0).
    {
        CRWFile oWriter( strPath, CRWFile::WRITE );
        CHECK( oWriter.GetPos() == 5 );
        oWriter.WriteLine( "six" );
    }
    CHECK( slurp( strPath ) == "one\ntwo\nthree\nfour\nfive\nsix\n" );
    std::remove( strPath );
    std::remove( "files_rw.log.idx" );

    CRWFile oNowhere( "files_no_such_dir/x.log", CRWFile::WRITE );
    CHECK( ! oNowhere.IsOpen() && ! oNowhere.WriteLine( "x" ) );
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
        CResource oFar( 1, 2 );
        oFar.SetName( "far" );
        oFar.SetData( "past five gigabytes", 19 );
        CHECK( archive.Put( oFar ) );   // (not "far": Windows #defines it)
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
    testCFile();
    testFileStat();
    testDirectory();
    testMemMappedFile();
    testRWFile();
    testBigFile();
    testReadWriteBig();
    testBigArchive();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
