/***********************************************************************
**  TestUtilities
**
**  Tests for the smaller folders: Configuration (CSettings and CProfile),
**  DynamicLibraries (CDynamicLibrary), Encryption (CSHA256) and
**  Exceptions (rethrowing keeps the class, errno text, call stacks).
***********************************************************************/

#include "IASLib.h"
#include "Configuration/Profile.h"
#include "Configuration/Settings.h"
#include "Exceptions/FileException.h"
#include "Exceptions/SocketException.h"
#include "Exceptions/XMLException.h"

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

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

static void writeFile( const char *strPath, const char *strText )
{
    std::ofstream oOut( strPath, std::ios::out | std::ios::binary | std::ios::trunc );
    oOut << strText;
}

static std::string readFile( const char *strPath )
{
    std::ifstream oIn( strPath, std::ios::in | std::ios::binary );
    std::stringstream oText;
    oText << oIn.rdbuf();
    return oText.str();
}

static void testSettings( void )
{
    section( "CSettings" );
    const char *strPath = "test_utilities_settings.txt";
    writeFile( strPath,
        "# a comment\n"
        "   # an indented comment = not a setting\n"
        "; another comment\n"
        "\n"
        "name = value one\n"
        "url=http://host/path?a=1&b=2\n"
        "  count = 42  \r\n"
        "name = value two\n"
        "= no name\n"
        "no equals sign\n" );

    CSettings oSettings;
    oSettings.SetFileName( strPath );
    CHECK( oSettings.ReadSettings() );
    CHECK( oSettings.GetSetting( "name" ) == "value two" );             // the later line
    CHECK( oSettings.GetSetting( "url" ) == "http://host/path?a=1&b=2" ); // after the first '='
    CHECK( oSettings.GetIntSetting( "count" ) == 42 );
    CHECK( ! oSettings.HasSetting( "# an indented comment" ) );
    CHECK( ! oSettings.HasSetting( "" ) );
    CHECK( oSettings.GetSetting( "missing", "dflt" ) == "dflt" );
    CHECK( oSettings.GetIntSetting( "missing", 7 ) == 7 );
    std::remove( strPath );

        // A missing file: false, no exception.
    CSettings oMissing;
    oMissing.SetFileName( "no_such_settings_file.txt" );
    bool bThrew = false;
    bool bRead = true;
    try
    {
        bRead = oMissing.ReadSettings();
    }
    catch ( ... )
    {
        bThrew = true;
    }
    CHECK( ! bThrew );
    CHECK( ! bRead );
}

static void testProfile( void )
{
    section( "CProfile" );
    const char *strPath = "test_utilities_profile.ini";
    std::remove( strPath );

        // A missing file reads as empty and does not throw.
    {
        bool bThrew = false;
        try
        {
            CProfile oProfile( strPath );
            CHECK( ! oProfile.DoesExist() );
            CHECK( oProfile.GetInt( "Section", "Value", -3 ) == -3 );
            CHECK( strcmp( oProfile.GetString( "Section", "Name", "dflt" ), "dflt" ) == 0 );
        }
        catch ( ... )
        {
            bThrew = true;
        }
        CHECK( ! bThrew );
    }

    writeFile( strPath,
        "; Header comment\n"
        "rootkey = rootvalue\n"
        "\n"
        "[Window]\n"
        "  Width = 640\n"
        "Height=480\n"
        "; trailing comment\n"
        "\n"
        "[Paths]\n"
        "Data = /var/data\n" );

    {
        CProfile oProfile( strPath );
        CHECK( oProfile.DoesExist() );
        CHECK( oProfile.GetInt( "Window", "Width", 0 ) == 640 );
        CHECK( oProfile.GetInt( "window", "HEIGHT", 0 ) == 480 );       // any case
        CHECK( oProfile.GetInt( "Window", "Missing", 9 ) == 9 );
        CHECK( strcmp( oProfile.GetString( NULL, "rootkey", "" ), "rootvalue" ) == 0 );
        CString strData;
        CHECK( oProfile.GetString( "Paths", "Data", "", strData ) == 9 );
        CHECK( strData == "/var/data" );

            // The buffer form always ends the text.
        char achSmall[ 5 ];
        memset( achSmall, 'x', sizeof( achSmall ) );
        CHECK( oProfile.GetString( "Paths", "Data", "", achSmall, (int)sizeof( achSmall ) ) == 4 );
        CHECK( strcmp( achSmall, "/var" ) == 0 );

            // The pointer stays valid across other reads and writes.
        const char *pszWidth = oProfile.GetString( "Window", "Width", "" );
        CHECK( oProfile.WriteString( "Window", "Title", "Main" ) );
        CHECK( oProfile.GetInt( "Paths", "Nothing", 0 ) == 0 );
        CHECK( strcmp( pszWidth, "640" ) == 0 );

        CHECK( oProfile.WriteInt( "Window", "Height", 768 ) );
        CHECK( oProfile.WriteString( "New Section", "Key", "a = b" ) );
        CHECK( ! oProfile.WriteString( "Window", "Bad\nName", "x" ) );
        CHECK( ! oProfile.WriteString( "Window", "Name", "two\nlines" ) );

        std::vector<CString> aSections = oProfile.GetSectionNames();
        CHECK( aSections.size() == 3 );
        std::vector<CString> aEntries = oProfile.GetEntryNames( "Window" );
        CHECK( aEntries.size() == 3 );
    }

        // Comments, order and indentation survive; new entries land in
        // their section.
    std::string strText = readFile( strPath );
    CHECK( strText.find( "; Header comment\n" ) == 0 );
    CHECK( strText.find( "; trailing comment" ) != std::string::npos );
    CHECK( strText.find( "  Width = 640" ) != std::string::npos );
    CHECK( strText.find( "Height = 768" ) != std::string::npos );
    CHECK( strText.find( "Title = Main" ) < strText.find( "; trailing comment" ) );
    CHECK( strText.find( "[New Section]\nKey = a = b" ) != std::string::npos );

    {
        CProfile oProfile( strPath );
        CHECK( strcmp( oProfile.GetString( "New Section", "Key", "" ), "a = b" ) == 0 );
        CHECK( oProfile.GetInt( "Window", "Height", 0 ) == 768 );

            // Changes another program makes are seen (no cache).
        CProfile oOther( strPath );
        CHECK( oOther.WriteInt( "Window", "Width", 1024 ) );
        CHECK( oProfile.GetInt( "Window", "Width", 0 ) == 1024 );

        CHECK( oProfile.DeleteEntry( "Window", "Title" ) );
        CHECK( ! oProfile.HasEntry( "Window", "Title" ) );
        CHECK( oProfile.DeleteSection( "Paths" ) );
        CHECK( oProfile.GetSectionNames().size() == 2 );
        CHECK( ! oProfile.DeleteSection( "Paths" ) );
    }

        // A cached profile reads the file once.
    {
        CProfile oCached( strPath, true );
        CHECK( oCached.IsCached() );
        CHECK( oCached.GetInt( "Window", "Width", 0 ) == 1024 );
        CProfile oOther( strPath );
        CHECK( oOther.WriteInt( "Window", "Width", 1 ) );
        CHECK( oCached.GetInt( "Window", "Width", 0 ) == 1024 );
        oCached.Reload();
        CHECK( oCached.GetInt( "Window", "Width", 0 ) == 1 );
    }

        // A new file is created by the first write.
    std::remove( strPath );
    {
        CProfile oProfile( strPath );
        CHECK( oProfile.WriteString( "S", "K", "V" ) );
        CHECK( oProfile.DoesExist() );
    }
    CHECK( readFile( strPath ) == "[S]\nK = V\n" );
    std::remove( strPath );
}

static void testDynamicLibrary( void )
{
    section( "CDynamicLibrary" );
#if defined( IASLIB_WIN32__ )
    const char *strName = "kernel32";
    const char *strFunction = "GetTickCount";
#elif defined( IASLIB_MAC__ ) || defined( __APPLE__ )
    const char *strName = "/usr/lib/libSystem.B.dylib";
    const char *strFunction = "strlen";
#else
    const char *strName = "libc.so.6";
    const char *strFunction = "strlen";
#endif
    {
        CDynamicLibrary oLib( strName );
        CHECK( oLib.IsLoaded() );
        CHECK( strlen( oLib.GetPath() ) > 0 );
        CHECK( oLib.FindFunction( strFunction ) != NULL );
        CHECK( oLib.FindFunction( "no_such_function_anywhere_42" ) == NULL );
        CHECK( strlen( oLib.GetError() ) > 0 );
        bool bThrew = false;
        try
        {
            oLib.GetFunction( "no_such_function_anywhere_42" );
        }
        catch ( CException & )
        {
            bThrew = true;
        }
        CHECK( bThrew );
    }
#if ! defined( IASLIB_WIN32__ ) && ! defined( IASLIB_MAC__ ) && ! defined( __APPLE__ )
    {
            // A bare name in the platform's form: "m" is libm.so, but that
            // is often a linker script, so only check nothing breaks.
        CDynamicLibrary oLib( "m" );
        (void)oLib.IsLoaded();
    }
#endif
    {
            // A library that is not there: not loaded, and safe to use and
            // destroy. (dlclose(NULL) crashed; dlsym(NULL) searched the
            // program itself.)
        CDynamicLibrary oLib( "no_such_library_at_all_42" );
        CHECK( ! oLib.IsLoaded() );
        CHECK( strlen( oLib.GetError() ) > 0 );
        CHECK( oLib.FindFunction( "strlen" ) == NULL );
        bool bThrew = false;
        try
        {
            oLib.GetFunction( "strlen" );
        }
        catch ( CException & )
        {
            bThrew = true;
        }
        CHECK( bThrew );
    }
    CHECK( CDynamicLibrary::PlatformName( "foo" ).GetLength() > 3 );
}

static void testSha256( void )
{
    section( "CSHA256" );
    const char *strABC = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    const char *strEmpty = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

        // Usable without init().
    CSHA256 oHash;
    CSha256Digest digest;
    oHash.update( (const unsigned char *)"abc", 3 );
    oHash.final( digest.data() );
    CHECK( CSHA256::ToHex( digest ) == strABC );

        // final() starts over.
    oHash.final( digest.data() );
    CHECK( CSHA256::ToHex( digest ) == strEmpty );
    oHash.update( (const unsigned char *)"ab", 2 );
    oHash.update( (const unsigned char *)"c", 1 );
    oHash.final( digest.data() );
    CHECK( CSHA256::ToHex( digest ) == strABC );

    CHECK( CSHA256::ToHex( CSHA256::Digest( std::string( "abc" ) ) ) == strABC );
    CHECK( CSHA256::StringDigest( CString( "abc" ) ) == strABC );
    oHash.update( NULL, 0 );
    oHash.final( digest.data() );
    CHECK( CSHA256::ToHex( digest ) == strEmpty );

        // A million 'a's (FIPS 180-2 test vector), in uneven pieces.
    std::string strA( 1000000, 'a' );
    CSHA256 oLong;
    size_t nAt = 0, nStep = 1;
    while ( nAt < strA.size() )
    {
        size_t nPart = ( nStep < strA.size() - nAt ) ? nStep : strA.size() - nAt;
        oLong.updateData( strA.data() + nAt, nPart );
        nAt += nPart;
        nStep = nStep * 3 + 1;
    }
    oLong.final( digest.data() );
    CHECK( CSHA256::ToHex( digest ) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0" );
}

static void inner( void )
{
    IASLIB_THROW_FILE_EXCEPTION( ENOENT );
}

static void middle( void )
{
    try
    {
        inner();
    }
    catch ( CFileException &e )
    {
        IASLIB_RETHROW( e, "while opening the settings" );
    }
}

static void testExceptions( void )
{
    section( "Exceptions" );

        // Rethrowing keeps the class and adds to the stack.
    bool bCaughtFile = false;
    try
    {
        middle();
    }
    catch ( CFileException &e )
    {
        bCaughtFile = true;
        CHECK( e.GetErrorNumber() == ENOENT );
        CString strStack = e.DumpStack();
        CHECK( strstr( (const char *)strStack, "while opening the settings" ) != NULL );
        CHECK( strstr( (const char *)strStack, "does not name an existing file" ) != NULL );
    }
    catch ( CException & )
    {
    }
    CHECK( bCaughtFile );

        // errno text comes from the system.
    CHECK( strlen( CException::ErrnoToText( ENOENT ) ) > 0 );
    CHECK( strlen( CException::ErrnoToText( 123456 ) ) > 0 );
    CHECK( strlen( CException::ErrnoToText( -5 ) ) > 0 );

        // A number with no message of its own gets the system's.
    CFileException oOther( EDOM );
    CHECK( strcmp( (const char *)oOther.GetMessage(), "Unknown File Exception" ) != 0 );

        // The int constructors put their text on the call stack too.
    CXMLException oXML( 1 );
    CHECK( strstr( (const char *)oXML.DumpStack(), "No ending brace found." ) != NULL );

        // Copies (as thrown by value) keep the stack.
    CException oOriginal( "original" );
    oOriginal.AddStack( "file.cpp", 10, "level two" );
    CException oCopy( oOriginal );
    CHECK( strstr( (const char *)oCopy.DumpStack(), "level two" ) != NULL );
}

int main( void )
{
    testSettings();
    testProfile();
    testDynamicLibrary();
    testSha256();
    testExceptions();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
