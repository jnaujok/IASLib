/***********************************************************************
**  TestBaseTypes
**
**  Tests for the base types: CObject (IDs, RTTI, the assert handler and
**  the memory-debugging allocator), CString (sharing, fixed buffers,
**  numbers, comparison, searching, editing, escaping, wildcards, threads),
**  CStringTokenizer, CDataBlock, CUUID and CVariant.
***********************************************************************/

#include "IASLib.h"

#include <algorithm>
#include <clocale>
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

#define CHECK_STR( got, expected ) \
    do { \
        g_nChecks++; \
        std::string _got = (const char *)( got ); \
        std::string _exp = ( expected ); \
        if ( _got != _exp ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s gave [%s], expected [%s]\n", __FILE__, __LINE__, #got, _got.c_str(), _exp.c_str() ); \
        } \
    } while ( 0 )

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
}

/***********************************************************************
**  CObject
***********************************************************************/

static int g_nAsserts = 0;
static void countAssert( const char *, int, const char * )
{
    g_nAsserts++;
}

static void testObject( void )
{
    section( "CObject" );

    CString a( "x" );
    CString b( a );
    CString c;
    CHECK( a.GetID() != 0 );
    CHECK( b.GetID() != a.GetID() );          // a copy is another object
    unsigned long nID = c.GetID();
    c = a;
    CHECK( c.GetID() == nID );                // assignment keeps the identity
    CHECK( c.GetID() > b.GetID() || c.GetID() < b.GetID() );

#ifdef IASLIB_RTTI__
    CHECK( strcmp( a.GetType(), "CString" ) == 0 );
    CHECK( a.IsType( "CString" ) );
    CHECK( a.IsType( "CObject" ) );
    CHECK( ! a.IsType( "CDate" ) );
    CHECK( ! a.IsType( NULL ) );
    const CObject &obj = a;
    CHECK( strcmp( obj.GetType(), "CString" ) == 0 );
#endif

    CObject::AssertHandler pfnOld = CObject::SetAssertHandler( countAssert );
    CObject::CallAssertHandler( __FILE__, __LINE__, "test" );
    CHECK( g_nAsserts == 1 );
    CHECK( CObject::SetAssertHandler( pfnOld ) == countAssert );

        // The allocator: aligned, zeroed, and quiet when nothing is damaged.
    void *p = CObject::allocate( 40, "Test" );
    CHECK( ( (size_t)p % 16 ) == 0 );
    bool bZero = true;
    for ( int n = 0; n < 40; n++ )
        if ( ((unsigned char *)p)[n] != 0 )
            bZero = false;
    CHECK( bZero );
    memset( p, 0xAB, 40 );
    CObject::deallocate( p, "Test" );
    CObject::deallocate( NULL, "Test" );      // like delete of NULL
    CHECK( true );

        // Thread-safe IDs: no two the same.
    std::vector<unsigned long> aIDs[4];
    std::vector<std::thread> aThreads;
    for ( int t = 0; t < 4; t++ )
    {
        aThreads.push_back( std::thread( [t, &aIDs]() {
            for ( int n = 0; n < 2000; n++ )
            {
                CString s;
                aIDs[t].push_back( s.GetID() );
            }
        } ) );
    }
    for ( size_t t = 0; t < aThreads.size(); t++ )
        aThreads[t].join();
    std::vector<unsigned long> aAll;
    for ( int t = 0; t < 4; t++ )
        aAll.insert( aAll.end(), aIDs[t].begin(), aIDs[t].end() );
    std::sort( aAll.begin(), aAll.end() );
    CHECK( std::adjacent_find( aAll.begin(), aAll.end() ) == aAll.end() );
}

/***********************************************************************
**  CString
***********************************************************************/

static void testStringBasics( void )
{
    section( "CString: construction, sharing and assignment" );

    CString strEmpty;
    CHECK( strEmpty.GetLength() == 0 );
    CHECK( strEmpty.IsEmpty() );
    CHECK_STR( strEmpty, "" );
    CHECK( (const char *)strEmpty != NULL );

    CHECK_STR( CString( "hello" ), "hello" );
    CHECK_STR( CString( "hello", 3 ), "hel" );
    CHECK_STR( CString( "hello", (size_t)3 ), "hel" );
    CHECK_STR( CString( "hello", 99 ), "hello" );
    CHECK_STR( CString( (const char *)NULL ), "" );
    CHECK_STR( CString( "hello", 0 ), "" );

        // A length shorter than the text: nothing past it is read.
    char achUnterminated[3] = { 'a', 'b', 'c' };
    CHECK_STR( CString( achUnterminated, (size_t)3 ), "abc" );

    CString a = "shared";
    CString b = a;
    CHECK( a.c_str() == b.c_str() );          // copies share the text...
    b += "!";
    CHECK_STR( a, "shared" );                 // ...until one changes
    CHECK_STR( b, "shared!" );

    CString c = a;
    c[0] = 'S';
    CHECK_STR( a, "shared" );
    CHECK_STR( c, "Shared" );
    const CString &cr = c;
    CHECK( cr[0] == 'S' );
    CHECK( cr[99] == '\0' );
    CHECK( cr[-1] == '\0' );
    c[99] = 'x';                              // past the end: harmless
    CHECK_STR( c, "Shared" );

    CString m = std::move( c );
    CHECK_STR( m, "Shared" );
    CHECK( c.IsEmpty() );
    CString n;
    n = std::move( m );
    CHECK_STR( n, "Shared" );

    n = n;
    CHECK_STR( n, "Shared" );
    n = n.c_str() + 1;                        // assigning part of itself
    CHECK_STR( n, "hared" );
    n += n.c_str() + 2;                       // appending part of itself
    CHECK_STR( n, "haredred" );
    n += n;
    CHECK_STR( n, "haredredharedred" );

    n = 'x';
    CHECK_STR( n, "x" );
    n = '\0';
    CHECK_STR( n, "" );
    n = (const char *)NULL;
    CHECK_STR( n, "" );
    n = true;
    CHECK_STR( n, "true" );

    n = "abc";
    n.Clear();
    CHECK( n.IsEmpty() );
    n.Copy( "abcdef", 4 );
    CHECK_STR( n, "abcd" );
    CString fresh;
    fresh.Copy( "xyz" );                      // used to do nothing on an empty string
    CHECK_STR( fresh, "xyz" );

        // Building a long string a character at a time.
    CString strLong;
    for ( int i = 0; i < 10000; i++ )
        strLong += (char)( 'a' + i % 26 );
    CHECK( strLong.GetLength() == 10000 );
    CHECK( strLong[9999] == (char)( 'a' + 9999 % 26 ) );
    strLong.Compact();
    CHECK( strLong.GetLength() == 10000 );

    CString strReserved;
    strReserved.Reserve( 100 );
    CHECK( strReserved.IsEmpty() );
    strReserved += "abc";
    CHECK_STR( strReserved, "abc" );
}

static void testStringFixed( void )
{
    section( "CString: fixed buffers" );

    char achBuffer[8];
    strcpy( achBuffer, "abc" );
    CString strFixed( achBuffer, -1, (int)sizeof( achBuffer ) );
    CHECK_STR( strFixed, "abc" );
    CHECK( strFixed.c_str() == achBuffer );

    strFixed += "defghijk";                   // cut to the 7 characters that fit
    CHECK_STR( strFixed, "abcdefg" );
    CHECK( strcmp( achBuffer, "abcdefg" ) == 0 );

    CString strCopy = strFixed;               // shares the buffer
    strCopy = "xy";
    CHECK_STR( strFixed, "xy" );
    CHECK( strcmp( achBuffer, "xy" ) == 0 );

    strFixed.Clear();
    CHECK( achBuffer[0] == 0 );
    strFixed.Format( "%d-%d", 12, 34 );
    CHECK_STR( achBuffer, "12-34" );
    strFixed.ToUpperCase();
    strFixed = strFixed.c_str() + 1;          // assigning part of its own buffer
    CHECK_STR( achBuffer, "2-34" );

    CString strOwn( (char *)NULL, 0, 5 );     // the string's own fixed buffer
    strOwn = "123456789";
    CHECK_STR( strOwn, "12345" );

    char achSmall[4] = "xyz";
    CString strSized( achSmall, (size_t)2, (size_t)4 );
    CHECK_STR( strSized, "xy" );
}

static void testStringNumbers( void )
{
    section( "CString: numbers" );

    CString s;
    s = 42;                         CHECK_STR( s, "42" );
    s = -7L;                        CHECK_STR( s, "-7" );
    s = (short)-3;                  CHECK_STR( s, "-3" );
    s = 4000000000u;                CHECK_STR( s, "4000000000" );
    s = 9223372036854775807LL;      CHECK_STR( s, "9223372036854775807" );
    s = 18446744073709551615ULL;    CHECK_STR( s, "18446744073709551615" );
    s = (size_t)12345;              CHECK_STR( s, "12345" );
    s = 0.1;                        CHECK_STR( s, "0.1" );
    s = 2.0;                        CHECK_STR( s, "2" );
    s = 1e100;                      CHECK_STR( s, "1e+100" );
    s = 0.1f;                       CHECK_STR( s, "0.1" );
    s = -1.5;                       CHECK_STR( s, "-1.5" );

    CString strSum = CString( "n=" ) + 5;
    CHECK_STR( strSum, "n=5" );
    CHECK_STR( 5 + CString( "x" ), "5x" );
    CHECK_STR( CString( "x" ) + (size_t)7, "x7" );
    CHECK_STR( CString( "x" ) + 2.5, "x2.5" );
    CHECK_STR( CString( "x" ) + 'y', "xy" );
    CHECK_STR( 'y' + CString( "x" ), "yx" );
    s = "v";
    s += 1;
    s += 2u;
    s += 3L;
    s += 4ULL;
    s += 0.5;
    CHECK_STR( s, "v12340.5" );

        // The decimal point stays '.' whatever the locale.
    const char *pszOld = setlocale( LC_NUMERIC, NULL );
    std::string strOld = ( pszOld != NULL ) ? pszOld : "C";
    if ( setlocale( LC_NUMERIC, "de_DE.UTF-8" ) != NULL || setlocale( LC_NUMERIC, "de_DE" ) != NULL )
    {
        s = 1.25;
        CHECK_STR( s, "1.25" );
    }
    setlocale( LC_NUMERIC, strOld.c_str() );

    CString strFormatted;
    CHECK_STR( strFormatted.Format( "%s-%05d", "id", 42 ), "id-00042" );
    std::string strBig( 100000, 'z' );        // longer than the old 64K buffer
    strFormatted.Format( "%s!", strBig.c_str() );
    CHECK( strFormatted.GetLength() == 100001 );
    CHECK_STR( CString::FormatString( "%d+%d", 2, 3 ), "2+3" );
}

static void testStringCompare( void )
{
    section( "CString: comparison and hashing" );

    const CString a( "apple" );
    const CString b( "banana" );
    const CString e;
    CHECK( a == "apple" );
    CHECK( "apple" == a );                    // used to compare pointers
    CHECK( a != b );
    CHECK( a < b );
    CHECK( b > a );
    CHECK( "apple" < b );
    CHECK( a <= "apple" );
    CHECK( a >= "apple" );
    CHECK( e < a );                           // "" < "apple" (used to be false)
    CHECK( e == "" );
    CHECK( e == (const char *)NULL );
    CHECK( a == CString( "apple" ) );
    CHECK( a.Compare( "APPLE", false ) == 0 );
    CHECK( a.Compare( "APPLE" ) > 0 );
    CHECK( a.Compare( b ) == -1 );
    CHECK( b.Compare( a ) == 1 );
    CHECK( CString( "ab" ).Compare( "abc" ) < 0 );
    CHECK( CString( "\xe9" ).Compare( "z" ) > 0 );    // bytes compare unsigned

    CHECK( CString( "hello" ).hashCode() == ( CString( "hel" ) + "lo" ).hashCode() );
    CHECK( CString( "hello" ).hashCode() != CString( "world" ).hashCode() );
    CHECK( e.hashCode() == 131 );             // used to crash on an empty string
    CHECK( CString( "a" ).hashcode() == 37 * 131 + 'a' );
    const CObject &obj = a;
    CHECK( obj.hashCode() == a.hashCode() );
}

static void testStringSearch( void )
{
    section( "CString: searching" );

    const CString s( "abcabcABC" );
    CHECK( s.IndexOf( "bc" ) == 1 );
    CHECK( s.IndexOf( "bc", 2 ) == 4 );
    CHECK( s.IndexOf( "bc", 5 ) == NOT_FOUND );
    CHECK( s.IndexOf( "BC", 0, true ) == 1 );
    CHECK( s.IndexOf( "" ) == 0 );
    CHECK( s.IndexOf( "", 3 ) == 3 );
    CHECK( s.IndexOf( (const char *)NULL ) == NOT_FOUND );
    CHECK( s.IndexOf( 'c' ) == 2 );
    CHECK( s.IndexOf( 'C', 0, true ) == 2 );
    CHECK( s.IndexOf( 'c', 99 ) == NOT_FOUND );
    CHECK( s.IndexOf( 'c', 99, true ) == NOT_FOUND );
    CHECK( s.IndexOf( 'z' ) == NOT_FOUND );
    CHECK( s.IndexOf( CString( "cA" ) ) == 5 );

    CHECK( s.LastIndexOf( "abc" ) == 3 );
    CHECK( s.LastIndexOf( "abc", 0, true ) == 0 );
    CHECK( s.LastIndexOf( "abc", NOT_FOUND, true ) == 6 );
    CHECK( s.LastIndexOf( "a" ) == 3 );
    CHECK( CString( "abc" ).LastIndexOf( "a" ) == 0 );   // used to be NOT_FOUND
    CHECK( s.LastIndexOf( "abc", 2 ) == 0 );
    CHECK( s.LastIndexOf( "zz" ) == NOT_FOUND );
    CHECK( s.LastIndexOf( 'b' ) == 4 );
    CHECK( s.LastIndexOf( 'b', 3 ) == 1 );
    CHECK( s.LastIndexOf( 'B', NOT_FOUND, true ) == 7 );
    CHECK( CString().LastIndexOf( 'a' ) == NOT_FOUND );

    CHECK( s.Contains( "cab" ) );
    CHECK( s.Contains( 'A' ) );
    CHECK( ! s.Contains( "xyz" ) );
    CHECK( s.StartsWith( "abc" ) );
    CHECK( s.StartsWith( "ABC", false ) );
    CHECK( ! s.StartsWith( "abcabcABCx" ) );
    CHECK( s.EndsWith( "ABC" ) );
    CHECK( s.EndsWith( "" ) );

    CHECK( CString( "12345" ).IsDigits() );
    CHECK( ! CString( "12a45" ).IsDigits() );
    CHECK( ! CString().IsDigits() );

    CHECK( CString( "report.txt" ).WildcardCompare( "*.txt" ) );
    CHECK( CString( "report.txt" ).WildcardCompare( "r?port.*" ) );
    CHECK( CString( "report.txt" ).WildcardCompare( "REPORT.TXT", false ) );
    CHECK( ! CString( "report.txt" ).WildcardCompare( "*.doc" ) );
    CHECK( CString( "abc" ).WildcardCompare( "abc*" ) );           // used to be false
    CHECK( CString( "abc" ).WildcardCompare( "a*b*c" ) );
    CHECK( CString( "aXbXc" ).WildcardCompare( "a*b*c" ) );
    CHECK( ! CString( "abc" ).WildcardCompare( "abcd" ) );
    CHECK( ! CString( "abc" ).WildcardCompare( "ab" ) );
    CHECK( CString().WildcardCompare( "*" ) );
    CHECK( CString( "x" ).WildcardCompare( "" ) );
    CHECK( ! CString( "x" ).WildcardCompare( NULL ) );
}

static void testStringEditing( void )
{
    section( "CString: editing" );

    CString s = "Hello, World";
    CHECK_STR( s.Substring( 7 ), "World" );
    CHECK_STR( s.Substring( 0, 5 ), "Hello" );
    CHECK_STR( s.Substring( 7, 100 ), "World" );
    CHECK_STR( s.Substring( 100 ), "" );
    CHECK_STR( s.Substring( 3, 0 ), "" );

    CString t = s;
    t.ToUpperCase();
    CHECK_STR( t, "HELLO, WORLD" );
    CHECK_STR( s, "Hello, World" );
    t.ToLowerCase();
    CHECK_STR( t, "hello, world" );

    CString w = "  \t padded \r\n";
    CString w2 = w;
    CHECK_STR( w.LeftTrim(), "padded \r\n" );
    CHECK_STR( w.RightTrim(), "padded" );
    CHECK_STR( w2.Trim(), "padded" );
    CString allSpace = "   ";
    CHECK_STR( allSpace.Trim(), "" );
    CString xs = "xxabcxx";
    CHECK_STR( xs.Trim( "x" ), "abc" );

    CString r = "a-b-c";
    r.Replace( '-', '+' );
    CHECK_STR( r, "a+b+c" );
    r.Replace( '+', '\0' );                   // replacing with NUL removes
    CHECK_STR( r, "abc" );
    r = "one two two";
    r.Replace( "two", "three" );
    CHECK_STR( r, "one three three" );
    r = "aaa";
    r.Replace( "a", "aa" );                   // used to loop forever
    CHECK_STR( r, "aaaaaa" );
    r.Replace( "", "x" );                     // used to loop forever
    CHECK_STR( r, "aaaaaa" );
    r = "a.b.c";
    r.Replace( ".", NULL );
    CHECK_STR( r, "abc" );

    CString d = "banana";
    d.Remove( 'a' );
    CHECK_STR( d, "bnn" );                    // used to keep the old length
    CHECK( d.GetLength() == 3 );
    d = "banana";
    d.Remove( "an" );
    CHECK_STR( d, "ba" );

    CString o = "it's";
    o.OracleEscape();
    CHECK_STR( o, "it''s" );
    o.OracleUnescape();
    CHECK_STR( o, "it's" );

    CHECK_STR( CString( "ab" ).Pad( 5 ), "ab   " );
    CHECK_STR( CString( "ab" ).Pad( 5, '*', false ), "***ab" );
    CHECK_STR( CString( "abcdef" ).Pad( 3 ), "abcdef" );
    CHECK_STR( CString().Pad( 2, '-' ), "--" );

    CString h = "Hi\xff";
    h.ToHex();
    CHECK_STR( h, "4869FF" );                 // used to write past its buffer
    h.FromHex();
    CHECK_STR( h, "Hi\xff" );
    CString hz = "0041";
    hz.FromHex();
    CHECK_STR( hz, "?A" );
}

static void testStringEscaping( void )
{
    section( "CString: escaping" );

    CString e = "a b+c&<\xe9>";
    e.Escape();
    CHECK_STR( e, "a+b%2Bc&%3C%E9%3E" );      // '+' is escaped now, so it reads back
    e.Unescape();
    CHECK_STR( e, "a b+c&<\xe9>" );

    CString u = "100%%+%41%zz%4";
    u.Unescape();
    CHECK_STR( u, "100% A%zz%4" );            // bad escapes stay as they are
    CString z = "x%00y";
    z.Unescape();
    CHECK_STR( z, "x%00y" );                  // a NUL would end the string

    CString tpl = "Hello ${name}, from ${place}. ${name}!";
    CHECK( tpl.hasDelimitedTokens( "${", "}" ) );
    CHECK( ! CString( "no ${tokens" ).hasDelimitedTokens( "${", "}" ) );
    tpl.replaceDelimitedToken( "${", "}", "name", "Irene" );
    CHECK_STR( tpl, "Hello Irene, from ${place}. Irene!" );

    const char *astrTokens[] = { "alpha", "beta*", "**" };
    int anTokens[] = { 1, 2, 99 };
    CHECK( CString::ConvertToToken( "alpha", astrTokens, anTokens ) == 1 );
    CHECK( CString::ConvertToToken( "ALPHA", astrTokens, anTokens, -1, false ) == 1 );
    CHECK( CString::ConvertToToken( "betamax", astrTokens, anTokens, -1, true, true ) == 2 );
    CHECK( CString::ConvertToToken( "gamma", astrTokens, anTokens ) == 99 );
    CHECK( CString::ConvertToToken( "gamma", astrTokens, anTokens, 2 ) == -9999 );

    const unsigned char *pBytes = CString( "AB" ).getBytes();
    CHECK( pBytes != NULL );
}

static void testStringThreads( void )
{
    section( "CString: copies on many threads" );

        // Each thread copies and changes strings that share one stub; the
        // atomic count keeps them apart (run under ThreadSanitizer too).
    CString strShared = "shared text that every thread copies";
    std::vector<std::thread> aThreads;
    std::vector<int> aBad( 4, 0 );
    for ( int t = 0; t < 4; t++ )
    {
        aThreads.push_back( std::thread( [t, &strShared, &aBad]() {
            for ( int n = 0; n < 5000; n++ )
            {
                CString strCopy = strShared;
                strCopy += (char)( '0' + t );
                if ( strCopy.GetLength() != strShared.GetLength() + 1 || strCopy[ strCopy.GetLength() - 1 ] != (char)( '0' + t ) )
                    aBad[t]++;
            }
        } ) );
    }
    for ( size_t t = 0; t < aThreads.size(); t++ )
        aThreads[t].join();
    CHECK( aBad[0] + aBad[1] + aBad[2] + aBad[3] == 0 );
    CHECK_STR( strShared, "shared text that every thread copies" );
}

/***********************************************************************
**  CStringTokenizer
***********************************************************************/

static std::string tokens( CStringTokenizer &tok )
{
    std::string strOut;
    while ( tok.HasMoreTokens() )
    {
        strOut += "[";
        strOut += (const char *)tok.NextToken();
        strOut += "]";
    }
    return strOut;
}

static void testTokenizer( void )
{
    section( "CStringTokenizer" );

    CStringTokenizer t1( "  the quick\tbrown  fox " );
    CHECK( t1.CountTokens() == 4 );
    CHECK_STR( tokens( t1 ).c_str(), "[the][quick][brown][fox]" );
    CHECK_STR( t1.NextToken(), "" );

    CStringTokenizer t2( "a,,b,", "," );
    CHECK_STR( tokens( t2 ).c_str(), "[a][b]" );

    CStringTokenizer t3( "a,,b", ",", true );
    CHECK_STR( tokens( t3 ).c_str(), "[a][,][,][b]" );

    CStringTokenizer t4( "a,,b,", ",", false, true );
    CHECK( t4.CountTokens() == 4 );
    CHECK_STR( tokens( t4 ).c_str(), "[a][][b][]" );
    CStringTokenizer t5( ",a", ",", false, true );
    CHECK_STR( tokens( t5 ).c_str(), "[][a]" );
    CStringTokenizer t6( "a", ",", false, true );     // used to give nothing
    CHECK_STR( tokens( t6 ).c_str(), "[a]" );
    CStringTokenizer t7( "", ",", false, true );
    CHECK( ! t7.HasMoreTokens() );
    CHECK( t7.CountTokens() == 0 );

    CStringTokenizer t8( "x=\"a b\" y=c", " ", false, false, true );
    CHECK_STR( tokens( t8 ).c_str(), "[x=\"a b\"][y=c]" );

    CStringTokenizer t9( "key=value;rest" , "=" );
    CHECK_STR( t9.NextToken(), "key" );
    CHECK_STR( t9.NextToken( ";" ), "=value" );
    CHECK_STR( t9.NextToken(), "rest" );
    CHECK( ! t9.HasMoreTokens() );
}


/***********************************************************************
**  CDataBlock
***********************************************************************/

static void testDataBlock( void )
{
    section( "CDataBlock" );

    CDataBlock block( 4 );
    CHECK( block.GetSize() == 4 );
    const unsigned char *pData = block;
    CHECK( pData[0] == 0 && pData[3] == 0 );          // new bytes are zero

    block.Copy( "abcdef", 6 );                        // grows
    CHECK( block.GetSize() == 6 );
    CHECK( memcmp( (const char *)block, "abcdef", 6 ) == 0 );
    block.Copy( "XY", 2, 8 );                         // past the end: the gap is zero
    CHECK( block.GetSize() == 10 );
    CHECK( ((const char *)block)[6] == 0 && ((const char *)block)[8] == 'X' );
    block.Set( '-', 3, 1 );
    CHECK( memcmp( (const char *)block, "a---ef", 6 ) == 0 );
    block.Set( (unsigned char)'!', 2, 12 );           // grows to offset + length
    CHECK( block.GetSize() == 14 );

    block[20] = 'Z';                                  // grows to include it
    CHECK( block.GetSize() == 21 );
    CHECK( ((const char *)block)[20] == 'Z' );        // the data, not the guardpost
    const CDataBlock &cblock = block;
    CHECK( cblock[20] == 'Z' );
    CHECK( cblock[99] == 0 );

    CDataBlock copy( block );
    CHECK( copy.GetSize() == block.GetSize() && memcmp( (const char *)copy, (const char *)block, block.GetSize() ) == 0 );
    CDataBlock assigned;
    assigned = block;
    CHECK( assigned.GetSize() == 21 );
    block.ChangeSize( 3 );
    CHECK( block.GetSize() == 3 && memcmp( (const char *)block, "a--", 3 ) == 0 );
    block.ChangeSize( 5, false );
    CHECK( block.GetSize() == 5 && ((const char *)block)[0] == 0 );

        // Writing past the end (through the pointer) is caught.
    CDataBlock guarded( 8, 16 );
    char *pGuarded = guarded;
    pGuarded[8] = 'x';
    bool bCaught = false;
    try
    {
        guarded.CheckGuardposts();
    }
    catch ( CException & )
    {
        bCaught = true;
    }
    CHECK( bCaught );
    pGuarded[8] = (char)CDataBlock::GUARDPOST_PATTERN;    // repaired
    bCaught = false;
    try
    {
        guarded.CheckGuardposts();
    }
    catch ( CException & )
    {
        bCaught = true;
    }
    CHECK( ! bCaught );

    CDataBlock unguarded( (const char *)"hi", 2, 0 );
    CHECK( unguarded.GetSize() == 2 && unguarded.GetGuardpostSize() == 0 );
    CDataBlock empty( 0 );
    CHECK( (const void *)empty != NULL );
}

/***********************************************************************
**  CUUID
***********************************************************************/

static void testUuid( void )
{
    section( "CUUID" );

    CUUID a;
    CString strA = a.toString();
    CHECK( a.isValid() );
    CHECK( strA.GetLength() == 36 );
    CHECK( strA[8] == '-' && strA[13] == '-' && strA[18] == '-' && strA[23] == '-' );
    CHECK( strA[14] == '4' );                                         // version 4
    CHECK( strchr( "89ab", strA[19] ) != NULL );                     // RFC 9562 variant
    CHECK( a.getVersion() == 4 );

    CUUID b( strA );
    CHECK( b.isValid() );
    CHECK( b == a );
    CHECK_STR( b.toString(), strA.c_str() );

    CUUID known( "123E4567-E89B-12D3-A456-426614174000" );
    CHECK( known.isValid() );
    CHECK_STR( known.toString(), "123e4567-e89b-12d3-a456-426614174000" );
    CHECK( known.getBytes()[0] == 0x12 && known.getBytes()[15] == 0x00 );
    CHECK( known.getVersion() == 1 );
    CHECK( CUUID( "{123e4567-e89b-12d3-a456-426614174000}" ) == known );
    CHECK( CUUID( "123e4567e89b12d3a456426614174000" ) == known );

    CHECK( ! CUUID( "123e4567-e89b-12d3-a456-42661417400" ).isValid() );   // short
    CHECK( ! CUUID( "123e4567-e89b-12d3-a456-4266141740000" ).isValid() ); // long
    CHECK( ! CUUID( "123e4567+e89b-12d3-a456-426614174000" ).isValid() );  // separator
    CHECK( ! CUUID( "123e4567-e89b-12d3-a456-42661417400g" ).isValid() );  // digit
    CHECK( ! CUUID( (const char *)NULL ).isValid() );
    CHECK( CUUID( "nonsense" ).toString().IsEmpty() );
    CHECK( CUUID( "nonsense" ).getBytes() == NULL );

    CHECK( CUUID::Nil().isNil() );
    CHECK_STR( CUUID::Nil().toString(), "00000000-0000-0000-0000-000000000000" );
    CHECK( ! a.isNil() );

        // Many UUIDs, all different.
    std::vector<std::string> aSeen;
    for ( int n = 0; n < 2000; n++ )
        aSeen.push_back( (const char *)CUUID().toString() );
    std::sort( aSeen.begin(), aSeen.end() );
    CHECK( std::adjacent_find( aSeen.begin(), aSeen.end() ) == aSeen.end() );

    CUUID c = known;
    CHECK( c == known && c.hashCode() == known.hashCode() );
    CHECK( ( known < a ) != ( a < known ) || known == a );
    CHECK( CUUID( "bad" ) < known );
}

/***********************************************************************
**  TRandom (ISAAC), against independent reference implementations
***********************************************************************/

    // Bob Jenkins' readable ISAAC (readable.c), written out separately,
    // and the same for ISAAC-64 (isaac64.c), to check TRandom against.
template <class T> struct CReferenceIsaac
{
    T mm[256], randrsl[256], aa, bb, cc;
    static const bool B64 = sizeof( T ) == 8;

    static void mix( T &a, T &b, T &c, T &d, T &e, T &f, T &g, T &h )
    {
        if ( ! B64 )
        {
            a^=b<<11; d+=a; b+=c;  b^=c>>2;  e+=b; c+=d;  c^=d<<8;  f+=c; d+=e;  d^=e>>16; g+=d; e+=f;
            e^=f<<10; h+=e; f+=g;  f^=g>>4;  a+=f; g+=h;  g^=h<<8;  b+=g; h+=a;  h^=a>>9;  c+=h; a+=b;
        }
        else
        {
            a-=e; f^=h>>9;  h+=a;  b-=f; g^=a<<9;  a+=b;  c-=g; h^=b>>23; b+=c;  d-=h; a^=c<<15; c+=d;
            e-=a; b^=d>>14; d+=e;  f-=b; c^=e<<20; e+=f;  g-=c; d^=f>>17; f+=g;  h-=d; e^=g<<14; g+=h;
        }
    }

    void isaac()
    {
        cc = cc + 1;
        bb = bb + cc;
        for ( int i = 0; i < 256; ++i )
        {
            T x = mm[i];
            switch ( i % 4 )
            {
                case 0: aa = B64 ? ~( aa ^ ( aa << 21 ) ) : aa ^ ( aa << 13 ); break;
                case 1: aa = B64 ? aa ^ ( aa >> 5 ) : aa ^ ( aa >> 6 ); break;
                case 2: aa = B64 ? aa ^ ( aa << 12 ) : aa ^ ( aa << 2 ); break;
                case 3: aa = B64 ? aa ^ ( aa >> ( B64 ? 33 : 1 ) ) : aa ^ ( aa >> 16 ); break;
            }
            aa = mm[ ( i + 128 ) % 256 ] + aa;
            T y = mm[ ( x >> ( B64 ? 3 : 2 ) ) % 256 ] + aa + bb;
            mm[i] = y;
            bb = mm[ ( y >> ( B64 ? 11 : 10 ) ) % 256 ] + x;
            randrsl[i] = bb;
        }
    }

    void randinit( const T *pSeed )
    {
        T a, b, c, d, e, f, g, h;
        aa = bb = cc = 0;
        a = b = c = d = e = f = g = h = B64 ? (T)0x9e3779b97f4a7c13ULL : (T)0x9e3779b9UL;
        for ( int i = 0; i < 256; i++ )
            randrsl[i] = pSeed ? pSeed[i] : 0;
        for ( int i = 0; i < 4; ++i )
            mix( a, b, c, d, e, f, g, h );
        for ( int i = 0; i < 256; i += 8 )
        {
            a+=randrsl[i]; b+=randrsl[i+1]; c+=randrsl[i+2]; d+=randrsl[i+3];
            e+=randrsl[i+4]; f+=randrsl[i+5]; g+=randrsl[i+6]; h+=randrsl[i+7];
            mix( a, b, c, d, e, f, g, h );
            mm[i]=a; mm[i+1]=b; mm[i+2]=c; mm[i+3]=d; mm[i+4]=e; mm[i+5]=f; mm[i+6]=g; mm[i+7]=h;
        }
        for ( int i = 0; i < 256; i += 8 )
        {
            a+=mm[i]; b+=mm[i+1]; c+=mm[i+2]; d+=mm[i+3]; e+=mm[i+4]; f+=mm[i+5]; g+=mm[i+6]; h+=mm[i+7];
            mix( a, b, c, d, e, f, g, h );
            mm[i]=a; mm[i+1]=b; mm[i+2]=c; mm[i+3]=d; mm[i+4]=e; mm[i+5]=f; mm[i+6]=g; mm[i+7]=h;
        }
        isaac();
    }
};

    // TRandom's first 4 blocks are the reference's results, each block
    // handed out from the end (as rand.c's rand() does).
template <class T> static bool matchesReference( const T *pSeed )
{
    CReferenceIsaac<T> ref;
    ref.randinit( pSeed );
    TRandom<8, T> gen;
    gen.srand( 0, 0, 0, pSeed );
    for ( int nBlock = 0; nBlock < 4; nBlock++ )
    {
        for ( int i = 255; i >= 0; i-- )
            if ( gen.rand() != ref.randrsl[i] )
                return false;
        ref.isaac();
    }
    return true;
}

static void testRandom( void )
{
    section( "TRandom (ISAAC)" );

    CHECK( matchesReference<uint32_t>( NULL ) );
    CHECK( matchesReference<uint64_t>( NULL ) );
    uint32_t aSeed32[256];
    uint64_t aSeed64[256];
    for ( int i = 0; i < 256; i++ )
    {
        aSeed32[i] = 0x12345678u * (uint32_t)( i + 1 );
        aSeed64[i] = 0x123456789abcdefULL * (uint64_t)( i + 1 );
    }
    CHECK( matchesReference<uint32_t>( aSeed32 ) );
    CHECK( matchesReference<uint64_t>( aSeed64 ) );

        // Jenkins' published test vector (randvect.txt): after randinit
        // with a zero seed, the next isaac() gives f650e4c8 e448e96d ...
    {
        CReferenceIsaac<uint32_t> ref;
        ref.randinit( NULL );
        ref.isaac();
        CHECK( ref.randrsl[0] == 0xf650e4c8u );
        CHECK( ref.randrsl[1] == 0xe448e96du );
        CHECK( ref.randrsl[2] == 0x98db2fb4u );
        CHECK( ref.randrsl[3] == 0xf5fad54fu );
    }

    {
        TRandom<8, uint32_t> a( 1, 2, 3 ), b( 1, 2, 3 );
        bool bSame = true;
        for ( int i = 0; i < 1000; i++ )
            if ( a.rand() != b.rand() )
                bSame = false;
        CHECK( bSame );                       // the same seed, the same sequence
        TRandom<8, uint32_t> c( a );          // a copy continues independently
        CHECK( c.rand() == a.rand() );
        CHECK( c.rand() == a.rand() );

        TRandom<8, uint32_t> s1, s2;
        s1.SeedFromSystem();
        s2.SeedFromSystem();
        CHECK( s1.rand() != s2.rand() || s1.rand() != s2.rand() );
        CHECK( TRandom<>::max() == 0xffffffffu );
    }
}

/***********************************************************************
**  CVariant
***********************************************************************/

static void testVariant( void )
{
    section( "CVariant" );

    CVariant v;
    CHECK( v.getType() == CVariant::UNKNOWN );
    CHECK( v.isEmpty() );
    CHECK_STR( v.getString(), "" );
    CHECK( v.getInt() == 0 && v.getArray() == NULL );

    v = "text";                               // used to become the boolean true
    CHECK( v.getType() == CVariant::STRING );
    CHECK_STR( v.getString(), "text" );

    v = 42;
    CHECK( v.getType() == CVariant::INTEGER );
    CHECK( v.getInt() == 42 && v.getLong() == 42 && v.getDouble() == 42.0 && v.getBoolean() );
    CHECK_STR( v.getString(), "42" );

    v = 7L;
    CHECK( v.getInt() == 7 );                 // used to be 0
    v = 2.75;
    CHECK( v.getInt() == 2 && v.getFloat() == 2.75f );
    CHECK_STR( v.getString(), "2.75" );
    v = 1e30;
    CHECK( v.getInt() == 2147483647 );
    v = false;
    CHECK( ! v.getBoolean() && v.getInt() == 0 );
    CHECK_STR( v.getString(), "false" );

    v = CString( " 123 " );
    CHECK( v.getInt() == 123 && v.getDouble() == 123.0 );
    v = "2.5";
    CHECK( v.getDouble() == 2.5 && v.getLong() == 2 );
    v = "yes";
    CHECK( v.getBoolean() );
    v = "no";
    CHECK( ! v.getBoolean() );
    v = "9007199254740993";
    CHECK( v.getLong() == 9007199254740993L || sizeof( long ) < 8 );

    v = (CArray *)NULL;
    CHECK_STR( v.getString(), "[null]" );     // used to crash
    v = (CObject *)NULL;
    CHECK_STR( v.getString(), "Object[null]" );
    CString strObject( "x" );
    v = (CObject *)&strObject;
    CHECK( v.getObject() == &strObject );
    CHECK( v.getString().StartsWith( "Object[" ) );

    CVariant copy( v );
    CHECK( copy.getType() == CVariant::OBJECT && copy.getObject() == &strObject );
    const CVariant cv( 3.5f );
    CHECK( cv.getFloat() == 3.5f && cv.isNumber() );
}

int main( void )
{
    testObject();
    testStringBasics();
    testStringFixed();
    testStringNumbers();
    testStringCompare();
    testStringSearch();
    testStringEditing();
    testStringEscaping();
    testStringThreads();
    testTokenizer();
    testDataBlock();
    testUuid();
    testRandom();
    testVariant();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
