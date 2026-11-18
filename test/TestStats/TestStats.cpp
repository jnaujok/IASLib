/***********************************************************************
**  TestStats
**
**  Tests for CHistogram (counts, CSV and XML output, gaps, escaping),
**  CCoverageMap (allocating, merging, releasing, the counters and the
**  dump) and CCPUUsage (normalised times and differences).
***********************************************************************/

#include "IASLib.h"

#include <cstdio>
#include <cstring>
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
        } \
    } while ( 0 )

#define CHECK_TEXT( actual, expected ) \
    do { \
        g_nChecks++; \
        std::string _a = ( actual ), _e = ( expected ); \
        if ( _a != _e ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d:\n    got:      [%s]\n    expected: [%s]\n", __FILE__, __LINE__, _a.c_str(), _e.c_str() ); \
        } \
    } while ( 0 )

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
}

static std::string contents( CStringStream &oStream )
{
    return oStream.GetOutput();
}

static std::string dumpHistogram( CHistogram &oHistogram, CHistogram::OutputFormatSpecifier nFormat, bool bFill = false )
{
    CStringStream oStream;
    oHistogram.Dump( oStream, nFormat, bFill );
    return contents( oStream );
}

static void testHistogram( void )
{
    section( "CHistogram" );

        // Named categories: the header lines up with the data (it was one
        // column to the left).
    CHistogram named;
    named.AddToBucket( 0, "A", 5 );
    named.AddToBucket( 0, "B", 3 );
    named.AddToBucket( 2, "B" );
    named.AddToBucket( 2, "B" );
    CHECK( named.GetCount( 0, 0 ) == 5 );
    CHECK( named.GetCount( 2, 1 ) == 2 );
    CHECK( named.GetCount( 1, 0 ) == 0 );
    CHECK( named.GetCategoryCount() == 2 );
    CHECK_TEXT( dumpHistogram( named, CHistogram::CSV ), ", A, B\n0, 5, 3\n2, 0, 2\n" );
    CHECK_TEXT( dumpHistogram( named, CHistogram::CSV, true ), ", A, B\n0, 5, 3\n1, 0, 0\n2, 0, 2\n" );

        // Numbered categories only: every row is as wide as the header.
    CHistogram numbered;
    numbered.AddToBucket( 1, (size_t)0, 1 );
    numbered.AddToBucket( 2, (size_t)2, 4 );
    CHECK_TEXT( dumpHistogram( numbered, CHistogram::CSV ), ", 0, 1, 2\n1, 1, 0, 0\n2, 0, 0, 4\n" );

        // Bucket names, escaping, large numbers.
    CHistogram labelled;
    labelled.AddCategory( "R&D, \"core\"" );
    labelled.SetBucketName( 7, "<seven>" );
    labelled.AddToBucket( 7, (size_t)0, 3000000000LL );
    labelled.AddToBucket( (size_t)3000000000UL, (size_t)0, 1 );
    CHECK( labelled.HasBucketName( 7 ) );
    CHECK( strcmp( labelled.GetBucketName( 7 ), "<seven>" ) == 0 );
    CHECK( labelled.GetBucketName( 8 ) == NULL );
    CHECK_TEXT( dumpHistogram( labelled, CHistogram::CSV ),
                ", \"R&D, \"\"core\"\"\"\n<seven>, 3000000000\n3000000000, 1\n" );
    std::string strXml = dumpHistogram( labelled, CHistogram::XML );
    CHECK( strXml.find( "<td>R&amp;D, &quot;core&quot;</td>" ) != std::string::npos );
    CHECK( strXml.find( "<td>&lt;seven&gt;</td>" ) != std::string::npos );
    CHECK( strXml.find( "<td>3000000000</td>" ) != std::string::npos );
    CHECK( strXml.find( "encoding=\"UTF-8\"" ) != std::string::npos );
    CHECK( strXml.find( "</table>" ) != std::string::npos );

        // The XML parses.
    CXMLDocument oDoc;
    CHECK( oDoc.Parse( strXml.c_str(), strXml.size() ) );

    CHistogram empty;
    CHECK_TEXT( dumpHistogram( empty, CHistogram::CSV ), "\n" );
    CHECK( ! empty.AddToBucket( 1, IASLib::NOT_FOUND ) );
}

static std::string dumpCoverage( CCoverageMap &oMap, CCoverageMap::OutputFormatSpecifier nFormat, bool bFill = false )
{
    CStringStream oStream;
    oMap.Dump( oStream, nFormat, bFill );
    return contents( oStream );
}

static void testCoverageMap( void )
{
    section( "CCoverageMap" );

    CCoverageMap map;
    CHECK( map.GetSize() == 0 );           // (was 1 when empty)
    CHECK( map.GetUsed() == 0 && map.GetFree() == 0 );
    CHECK( map.GetFragmentation() == 0.0 );
    CHECK( ! map.Allocate( 5, 0 ) );

    CHECK( map.Allocate( 100, 50 ) );       // [100,150)
    CHECK( map.Allocate( 200, 50 ) );       // [200,250)
    CHECK( map.GetRunCount() == 2 );
    CHECK( map.GetSize() == 150 );
    CHECK( map.GetUsed() == 100 );
    CHECK( map.GetFree() == 50 );
    CHECK( map.IsUsed( 120, 10 ) );
    CHECK( ! map.IsUsed( 140, 20 ) );

        // Overlapping: reported, and merged.
    CHECK( ! map.Allocate( 140, 20 ) );     // [100,160)
    CHECK( map.GetUsed() == 110 );
    CHECK( map.GetRunCount() == 2 );
        // Touching runs merge too.
    CHECK( map.Allocate( 160, 40 ) );       // [100,250)
    CHECK( map.GetRunCount() == 1 );
    CHECK( map.GetUsed() == 150 && map.GetFree() == 0 );

        // Releasing the middle splits the run.
    CHECK( map.Release( 150, 20 ) );
    CHECK( map.GetRunCount() == 2 );
    CHECK( map.GetUsed() == 130 );
    CHECK( ! map.IsUsed( 160 ) );
    CHECK( map.IsUsed( 170 ) );
    CHECK( ! map.Release( 155, 5 ) );       // already free

        // Releasing with no size: to the end of the run.
    CHECK( map.Release( 200 ) );            // [170,250) -> [170,200)
    CHECK( map.GetUsed() == 80 );
    CHECK( ! map.Release( 300 ) );

        // Across several runs.
    map.Allocate( 300, 10 );
    map.Allocate( 320, 10 );
    CHECK( map.Release( 0, 1000 ) );
    CHECK( map.GetRunCount() == 0 && map.GetUsed() == 0 );

        // Fragmentation: 0 with one gap, more with many.
    CCoverageMap frag;
    frag.Allocate( 0, 10 );
    frag.Allocate( 20, 10 );
    CHECK( frag.GetFragmentation() == 0.0 );
    frag.Allocate( 40, 10 );
    CHECK( frag.GetFragmentation() == 0.5 );

        // Huge ranges do not wrap.
    CCoverageMap huge;
    CHECK( huge.Allocate( (size_t)-1 - 10, 100 ) );
    CHECK( huge.GetUsed() == 10 );

        // The dump: a row per block that has something in use.
    CCoverageMap blocks;
    blocks.SetBlockSize( 100 );
    blocks.Allocate( 50, 100 );             // blocks 0 and 100
    blocks.Allocate( 420, 10 );             // block 400
    CHECK_TEXT( dumpCoverage( blocks, CCoverageMap::CSV ),
                "Start, Used, Free\n0, 50, 50\n100, 50, 50\n400, 10, 90\n" );
    CHECK_TEXT( dumpCoverage( blocks, CCoverageMap::CSV, true ),
                "Start, Used, Free\n0, 50, 50\n100, 50, 50\n200, 0, 100\n300, 0, 100\n400, 10, 90\n" );
    std::string strXml = dumpCoverage( blocks, CCoverageMap::XML );
    CXMLDocument oDoc;
    CHECK( oDoc.Parse( strXml.c_str(), strXml.size() ) );
    CHECK( strXml.find( "<td>400</td>" ) != std::string::npos );
}

static void testCPUUsage( void )
{
    section( "CCPUUsage" );

    CCPUUsage before;
        // Burn a little CPU.
    volatile unsigned long long ullSum = 0;
    for ( unsigned long long i = 0; i < 30000000ULL; i++ )
        ullSum += i * i;
    const CCPUUsage after;

    int nSec, nMicro;
    after.GetTotalTime( nSec, nMicro );
    CHECK( nSec >= 0 && nMicro >= 0 && nMicro < 1000000 );
    after.GetUserTime( nSec, nMicro );
    CHECK( nMicro >= 0 && nMicro < 1000000 );
    after.GetSystemTime( nSec, nMicro );
    CHECK( nMicro >= 0 && nMicro < 1000000 );
    CHECK( after.GetTotalMicroseconds() == after.GetUserMicroseconds() + after.GetSystemMicroseconds() );
    CHECK( after.GetTotalMicroseconds() >= before.GetTotalMicroseconds() );

    int nSec2, nMicro2;
    after.TotalElapsed( before, nSec, nMicro );
    before.TotalElapsed( after, nSec2, nMicro2 );
    CHECK( nSec == nSec2 && nMicro == nMicro2 );
    CHECK( nMicro >= 0 && nMicro < 1000000 );
    CHECK( (long long)nSec * 1000000 + nMicro == after.GetTotalMicroseconds() - before.GetTotalMicroseconds() );
    after.UserElapsed( before, nSec, nMicro );
    CHECK( nMicro >= 0 && nMicro < 1000000 );
    after.SysElapsed( before, nSec, nMicro );
    CHECK( nMicro >= 0 && nMicro < 1000000 );

        // A temporary works as the other snapshot.
    after.TotalElapsed( CCPUUsage(), nSec, nMicro );
    CHECK( nMicro >= 0 && nMicro < 1000000 );

    CCPUUsage copy( after );
    CHECK( copy.GetTotalMicroseconds() == after.GetTotalMicroseconds() );
    CCPUUsage assigned;
    assigned = after;
    CHECK( assigned.GetUserMicroseconds() == after.GetUserMicroseconds() );
    (void)ullSum;
}

int main( void )
{
    testHistogram();
    testCoverageMap();
    testCPUUsage();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
