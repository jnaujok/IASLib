/***********************************************************************
**  TestStreams
**
**  Tests for the streams: CStringStream (binary-safe, lines, the end of
**  the stream), the text operators reading back what they write, the
**  binary big-endian reads and writes, CFileStream, CBufferedStream over
**  another stream, CSocketStream over a real connection, and CNullStream.
***********************************************************************/

#include "IASLib.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

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

#define CHECK_THROWS( expression ) \
    do { \
        g_nChecks++; \
        bool _bThrew = false; \
        try { expression; } catch ( CStreamException & ) { _bThrew = true; } \
        if ( ! _bThrew ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s did not throw\n", __FILE__, __LINE__, #expression ); \
        } \
    } while ( 0 )

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
}

static std::string readAll( CStream &oStream )
{
    std::string strAll;
    char achBuffer[ 1000 ];
    int nRead;
    while ( ( nRead = oStream.GetBuffer( achBuffer, (int)sizeof( achBuffer ) ) ) > 0 )
        strAll.append( achBuffer, (size_t)nRead );
    return strAll;
}

static void testStringStream( void )
{
    section( "CStringStream" );

    CStringStream oIn( CString( "first\r\nsecond\nthird" ) );
    CHECK( oIn.bytesRemaining() == 19 );
    CHECK( oIn.PeekChar() == 'f' && oIn.PeekChar() == 'f' );
    CHECK( oIn.GetLine() == "first" );                // "\r" is left off
    CHECK( oIn.GetLine() == "second" );
    CHECK( ! oIn.IsEOS() );
    CHECK( oIn.GetLine() == "third" );
    CHECK( oIn.IsEOS() );
    CHECK( oIn.GetLine() == "" );
    CHECK_THROWS( oIn.GetChar() );
    CHECK_THROWS( oIn.PeekChar() );
    char ach[4];
    CHECK( oIn.GetBuffer( ach, 4 ) == 0 );

        // Bytes, NULs included (they used to be dropped).
    std::string strBinary( "a\0b\0\xff", 5 );
    CStringStream oBinary( strBinary );
    CHECK( readAll( oBinary ) == strBinary );
    CStringStream oOut;
    oOut.PutBuffer( strBinary.data(), (int)strBinary.size() );
    oOut.PutChar( '\0' );
    CHECK( oOut.GetOutput() == strBinary + std::string( 1, '\0' ) );
    CHECK( oOut.GetString() == "a" );                 // as text: up to the NUL

        // PutLine() ends the line (it did not, here).
    CStringStream oLines;
    oLines.PutLine( "one" );
    oLines << "two" << '\n';
    CHECK( oLines.GetOutput() == "one\ntwo\n" );
}

static void testText( void )
{
    section( "Text: << and >>" );

    CStringStream oOut;
    oOut << (short)-12 << ' ' << (unsigned short)65535 << ' ' << 42 << ' ' << -7 << ' ' << 4000000000u
         << ' ' << -123456789L << ' ' << 4294967295UL << ' ' << -9223372036854775807LL << ' '
         << 18446744073709551615ULL << ' ' << 2.5 << ' ' << 1e-300 << ' ' << CString( "word" ) << " text\n";
    CHECK( oOut.GetOutput() == "-12 65535 42 -7 4000000000 -123456789 4294967295 -9223372036854775807 18446744073709551615 2.5 1e-300 word text\n" );

        // Reading back what was written (>> used to read binary).
    CStringStream oIn( oOut.GetOutput() );
    short s; unsigned short us; int i, i2; unsigned int ui; long l; unsigned long ul; long long ll; unsigned long long ull;
    double d1, d2; CString w1, w2;
    oIn >> s >> us >> i >> i2 >> ui >> l >> ul >> ll >> ull >> d1 >> d2 >> w1 >> w2;
    CHECK( s == -12 && us == 65535 && i == 42 && i2 == -7 && ui == 4000000000u && l == -123456789L );
    CHECK( ul == 4294967295UL && ll == -9223372036854775807LL && ull == 18446744073709551615ULL );
    CHECK( d1 == 2.5 && d2 == 1e-300 );
    CHECK( w1 == "word" && w2 == "text" );

        // Errors.
    CStringStream oBad( CString( "abc 70000 -5 99999999999999999999" ) );
    int nBad = 0;
    CHECK_THROWS( oBad >> nBad );                      // not a number
    CString strSkip;
    oBad >> strSkip;
    short sBig = 0;
    CHECK_THROWS( oBad >> sBig );                      // out of range
    unsigned int uNeg = 0;
    CHECK_THROWS( oBad >> uNeg );                      // negative
    long long llHuge = 0;
    CHECK_THROWS( oBad >> llHuge );
    CStringStream oEmpty( CString( "   " ) );
    CHECK_THROWS( oEmpty >> nBad );
    CString strNone = "x";
    oEmpty >> strNone;
    CHECK( strNone == "" );

        // A double in the C locale whatever the program's.
    CStringStream oFloat( CString( "-1.25e3" ) );
    double dFloat = 0;
    oFloat >> dFloat;
    CHECK( dFloat == -1250.0 );

    CStringStream oNull;
    oNull << (const char *)NULL;                      // used to crash
    CHECK( oNull.GetOutput().empty() );
}

static void testBinary( void )
{
    section( "Binary" );

    CStringStream oOut;
    oOut.PutUInt8( 0xAB );
    oOut.PutUInt16( 0x1234 );
    oOut.PutUInt32( 0xDEADBEEF );
    oOut.PutUInt64( 0x0102030405060708ULL );
    CHECK( oOut.GetOutput() == std::string( "\xAB\x12\x34\xDE\xAD\xBE\xEF\x01\x02\x03\x04\x05\x06\x07\x08", 15 ) );

    CStringStream oIn( oOut.GetOutput() );
    CHECK( oIn.GetUInt8() == 0xAB );
    CHECK( oIn.GetUInt16() == 0x1234 );
    CHECK( oIn.GetUInt32() == 0xDEADBEEF );
    CHECK( oIn.GetUInt64() == 0x0102030405060708ULL );
    CHECK_THROWS( oIn.GetUInt16() );

    CStringStream oShort( std::string( "\x01", 1 ) );
    CHECK_THROWS( oShort.GetUInt32() );
}

static void testFileStream( void )
{
    section( "CFileStream" );

    const char *pszName = "TestStreams.tmp";
    {
        CFileStream oOut( new CFile( pszName, CFile::WRITE | CFile::BINARY ) );
        oOut.PutLine( "alpha" );
        oOut << "beta\r\n" << 123 << '\n';
        oOut.PutUInt32( 0x01020304 );
    }
    {
        CFileStream oIn( new CFile( pszName, CFile::READ | CFile::BINARY ) );
        CHECK( oIn.bytesRemaining() == 6 + 6 + 4 + 4 );
        CHECK( oIn.PeekChar() == 'a' );
        CHECK( oIn.PeekChar() == 'a' );               // (it used to seek in the wrong mode)
        CHECK( oIn.GetLine() == "alpha" );
        CHECK( oIn.GetLine() == "beta" );
        int n = 0;
        oIn >> n;
        CHECK( n == 123 );
        CHECK( oIn.GetChar() == '\n' );
        CHECK( oIn.GetUInt32() == 0x01020304 );
        CHECK( oIn.IsEOS() );
        CHECK_THROWS( oIn.GetChar() );
        char ach[8];
        CHECK( oIn.GetBuffer( ach, 8 ) == 0 );
    }
    remove( pszName );

    CFileStream oNone;
    CHECK( oNone.IsEOS() && oNone.GetBuffer( (char *)"", 0 ) == 0 && oNone.PutBuffer( "x", 1 ) == 0 );
}

static void testBufferedStream( void )
{
    section( "CBufferedStream" );

        // Reading across many blocks, in pieces of every size.
    std::string strData;
    for ( int n = 0; n < 20000; n++ )
        strData += (char)( 'a' + n % 26 );
    {
        CStringStream oUnder( strData );
        CBufferedStream oBuffered( oUnder );
        std::string strRead;
        int nPiece = 1;
        char achBuffer[ 9000 ];
        while ( ! oBuffered.IsEOS() )
        {
            int nRead = oBuffered.GetBuffer( achBuffer, nPiece );
            strRead.append( achBuffer, (size_t)nRead );
            nPiece = ( nPiece * 7 + 3 ) % 8999 + 1;
        }
        CHECK( strRead == strData );
    }

        // The last bytes after the stream underneath has ended (they used
        // to be lost).
    {
        CStringStream oUnder( CString( "0123456789" ) );
        CBufferedStream oBuffered( oUnder );
        CHECK( oBuffered.GetChar() == '0' );
        char achBuffer[ 32 ];
        CHECK( oBuffered.GetBuffer( achBuffer, 32 ) == 9 && memcmp( achBuffer, "123456789", 9 ) == 0 );
        CHECK( oBuffered.IsEOS() );
        CHECK_THROWS( oBuffered.GetChar() );
    }

        // Writes, by character and by buffer, are all there once flushed
        // (characters used to be thrown away on close).
    {
        CStringStream oUnder;
        {
            CBufferedStream oBuffered( oUnder );
            for ( int n = 0; n < 10000; n++ )
                oBuffered.PutChar( (char)( 'A' + n % 26 ) );
            oBuffered << "|tail";
            CHECK( oUnder.GetOutput().size() < 10000 + 5 );          // some still held
            oBuffered.PutBuffer( strData.data(), (int)strData.size() );   // large: flushes, then goes straight through
            oBuffered << "!";
            CHECK( oUnder.GetOutput().size() == 10000 + 5 + strData.size() );
        }                                                                     // the "!" is flushed here
        std::string strExpected;
        for ( int n = 0; n < 10000; n++ )
            strExpected += (char)( 'A' + n % 26 );
        strExpected += "|tail" + strData + "!";
        CHECK( oUnder.GetOutput() == strExpected );
        CHECK( oUnder.IsOpen() );                     // destroying did not close it
    }
    {
        CStringStream oUnder;
        CBufferedStream oBuffered( oUnder );
        oBuffered << "x";
        oBuffered.Flush();
        CHECK( oUnder.GetOutput() == "x" );
        oBuffered.Close();
        CHECK( ! oUnder.IsOpen() );
    }
}

#ifdef IASLIB_NETWORKING__
static void testSocketStream( void )
{
    section( "CSocketStream" );

    CServerSocket *pServer = NULL;
    int nPort = 0;
    for ( int nTry = 0; nTry < 20 && pServer == NULL; nTry++ )
    {
        nPort = 26000 + (int)( std::chrono::steady_clock::now().time_since_epoch().count() % 3000 ) + nTry;
        try
        {
            pServer = new CServerSocket( nPort, 4 );
            if ( ! pServer->IsConnected() )
            {
                delete pServer;
                pServer = NULL;
            }
        }
        catch ( ... )
        {
            pServer = NULL;
        }
    }
    CHECK( pServer != NULL );
    if ( pServer == NULL )
        return;

    std::thread oPeer( [nPort]() {
        try
        {
            CClientSocket oClient( "127.0.0.1", nPort );
            CSocketStream oOut( &oClient );
            oOut.SetNoDelete();
            oOut.PutLine( "HELLO there\r" );
            oOut.PutUInt16( 513 );
            oOut << 77 << ' ';
            CThread::Millisleep( 100 );
        }
        catch ( ... )
        {
        }
    } );

    CSocket *pAccepted = pServer->Accept( 5000 );
    CHECK( pAccepted != NULL );
    if ( pAccepted != NULL )
    {
        CSocketStream oIn( pAccepted );
        CHECK( ! oIn.IsEOS() );                       // (it used to say "ended" when data was waiting)
        CHECK( oIn.PeekChar() == 'H' && oIn.PeekChar() == 'H' );   // (a second peek used to take it)
        CHECK( oIn.GetLine() == "HELLO there" );
        CHECK( oIn.GetUInt16() == 513 );
        int n = 0;
        oIn >> n;
        CHECK( n == 77 );
        CHECK( oIn.GetChar() == ' ' );
            // The peer closes: the stream ends (reads used to spin for ever).
        CHECK( oIn.IsEOS() );
        char ach[4];
        CHECK( oIn.GetBuffer( ach, 4 ) == 0 );
        CHECK_THROWS( oIn.GetChar() );
    }
    oPeer.join();
    delete pServer;
}
#endif

static void testNullStream( void )
{
    section( "CNullStream" );

    CNullStream oNull;
    oNull << "anything" << 42;
    CHECK( oNull.PutBuffer( "abc", 3 ) == 3 );
    CHECK( oNull.IsEOS() && oNull.bytesRemaining() == 0 );
    char ach[4];
    CHECK( oNull.GetBuffer( ach, 4 ) == 0 );
    CHECK( oNull.GetLine() == "" );
    CHECK_THROWS( oNull.GetChar() );
}

int main( void )
{
    testStringStream();
    testText();
    testBinary();
    testFileStream();
    testBufferedStream();
#ifdef IASLIB_NETWORKING__
    testSocketStream();
#endif
    testNullStream();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
