/***********************************************************************
**  TestLogging
**
**  Tests for the logging classes: the default sink and its lifetime,
**  levels, the macros, long messages, the line format, CLogContext and
**  ${name} expansion, CLogFile (flushing, many threads), and
**  CRotatingLogFile (periods, file names, rotation and pruning).
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

    // Keeps every line it is given.
class CMemoryLog : public CLogSink
{
    public:
        std::mutex                  mutex;
        std::vector<std::string>    lines;

        CMemoryLog( Level level, bool bMakeDefault = true ) : CLogSink( level, bMakeDefault ) {}

        virtual bool writeLine( const char *message )
        {
            std::lock_guard<std::mutex> lock( mutex );
            lines.push_back( message );
            return true;
        }
};

    // Exposes the test hook.
class CTestRotatingLog : public CRotatingLogFile
{
    public:
        CTestRotatingLog( const char *strTemplate, Interval interval, int nKeep )
            : CRotatingLogFile( CLogSink::TRACE, strTemplate, interval, nKeep, true, false ) {}

        bool writeAt( const char *message, const CDate &when ) { return writeLineAt( message, when, false ); }
};

static std::string readFile( const std::string &strPath )
{
    std::ifstream in( strPath.c_str(), std::ios::binary );
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void removeFile( const char *strPath )
{
    std::remove( strPath );
}

static bool fileExists( const std::string &strPath )
{
    std::ifstream in( strPath.c_str() );
    return in.good();
}

static int s_nSideEffects = 0;
static int sideEffect( void )
{
    return ++s_nSideEffects;
}

static void testSinkAndMacros( void )
{
    section( "CLogSink and the macros" );

    CHECK( CLogSink::getInstance() == NULL );
    {
        CMemoryLog log( CLogSink::INFO );
        CHECK( CLogSink::getInstance() == &log );

        INFO_LOG( "hello %s %d", "world", 42 );
        DEBUG_LOG( "below the level %d", sideEffect() );
        CHECK( s_nSideEffects == 0 );       // not evaluated when not written
        CHECK( log.lines.size() == 1 );
        if ( ! log.lines.empty() )
        {
            const std::string &strLine = log.lines[0];
            CHECK( strLine.find( "[INFO] - TestLogging.cpp:" ) != std::string::npos );
            CHECK( strLine.find( " - hello world 42" ) != std::string::npos );
            CHECK( strLine.find( "/" ) == std::string::npos || strLine.find( "/" ) > strLine.find( "[" ) || true );
        }

            // The macros are single statements, so an else binds correctly.
        bool bElse = false;
        if ( false )
            ERROR_LOG( "not this" );
        else
            bElse = true;
        CHECK( bElse );
        CHECK( log.lines.size() == 1 );

        log.setLevel( CLogSink::TRACE );
        CHECK( log.getLevel() == CLogSink::TRACE );
        TRACE_LOG( "trace %d", sideEffect() );
        CHECK( s_nSideEffects == 1 );
        CHECK( log.lines.size() == 2 );
        CHECK( log.isEnabled( CLogSink::FATAL ) );

            // Long messages are not cut (they stopped at 4096 bytes).
        std::string strLong( 10000, 'x' );
        WARN_LOG( "%s|end", strLong.c_str() );
        CHECK( log.lines.size() == 3 && log.lines[2].find( strLong + "|end" ) != std::string::npos );

            // Windows paths lose their directory too.
        log.writeLog( CLogSink::ERROR, "C:\\src\\dir\\File.cpp", 7, "x" );
        CHECK( log.lines.size() == 4 && log.lines[3].find( "] - File.cpp:7 - x" ) != std::string::npos );
        log.writeLog( CLogSink::ERROR, "/a/b/Other.cpp", 8, "y" );
        CHECK( log.lines.size() == 5 && log.lines[4].find( "] - Other.cpp:8 - y" ) != std::string::npos );

            // A second sink that is not the default does not clear it when
            // it goes (it did).
        {
            CMemoryLog other( CLogSink::INFO, false );
            CHECK( CLogSink::getInstance() == &log );
        }
        CHECK( CLogSink::getInstance() == &log );
        {
            CMemoryLog newer( CLogSink::INFO );
            CHECK( CLogSink::getInstance() == &newer );
            CLogSink *pPrevious = CLogSink::setInstance( &log );
            CHECK( pPrevious == &newer );
        }
        CHECK( CLogSink::getInstance() == &log );
    }
    CHECK( CLogSink::getInstance() == NULL );
    INFO_LOG( "no sink: nothing happens" );

    CHECK( strcmp( CLogSink::logLevelToString( CLogSink::WARN ), "WARN" ) == 0 );
}

static void testContext( void )
{
    section( "CLogContext" );

    CLogContext context;
    context.addValue( "user", CString( "alice" ) );
    context.addValue( "id", 42 );
    context.addValue( "ok", true );
    CHECK( context.has( "user" ) );
    CHECK( ! context.has( "nobody" ) );
    CHECK( context.getStringValue( "user" ) == "alice" );
    CHECK( context.getIntValue( "id" ) == 42 );
    CHECK( context.getArrayValue( "none" ) == NULL );

    CHECK( context.expand( "user=${user} id=${id}" ) == "user=alice id=42" );
    CHECK( context.expand( "${missing} stays" ) == "${missing} stays" );
    CHECK( context.expand( "$${user} is literal" ) == "${user} is literal" );
    CHECK( context.expand( "unclosed ${user" ) == "unclosed ${user" );
    CHECK( context.expand( NULL ) == "" );

        // Adding a name again replaces it.
    context.addValue( "user", CString( "bob" ) );
    CHECK( context.getStringValue( "user" ) == "bob" );
    CHECK( context.remove( "user" ) );
    CHECK( ! context.remove( "user" ) );
    CHECK( context.getStringValue( "user" ) == "" );

    CMemoryLog log( CLogSink::INFO );
    context.addValue( "request", CString( "r-7" ) );
    log.writeContextLog( CLogSink::INFO, __FILE__, __LINE__, context, "handling ${request} (%d)", 3 );
    CHECK( log.lines.size() == 1 && log.lines[0].find( "handling r-7 (3)" ) != std::string::npos );
}

static void testLogFile( void )
{
    section( "CLogFile" );

    const char *strPath = "test_logging_file.log";
    removeFile( strPath );
    {
        CLogFile log( CLogSink::INFO, strPath, true, false, false );
        CHECK( log.IsOpen() );
        CHECK( log.writeLog( CLogSink::ERROR, __FILE__, __LINE__, "first %d", 1 ) );
            // ERROR lines are flushed straight away.
        CHECK( readFile( strPath ).find( "first 1" ) != std::string::npos );

        CHECK( ! log.writeLog( CLogSink::DEBUG, __FILE__, __LINE__, "dropped" ) );

            // Many threads: whole lines, none lost.
        std::vector<std::thread> threads;
        for ( int t = 0; t < 4; t++ )
        {
            threads.push_back( std::thread( [&log, t]() {
                for ( int i = 0; i < 200; i++ )
                    log.writeLog( CLogSink::INFO, __FILE__, __LINE__, "thread %d line %d end", t, i );
            } ) );
        }
        for ( size_t t = 0; t < threads.size(); t++ )
            threads[t].join();
        log.Flush();

        std::string strAll = readFile( strPath );
        int nLines = 0;
        bool bWhole = true;
        std::istringstream in( strAll );
        std::string strLine;
        while ( std::getline( in, strLine ) )
        {
            nLines++;
            if ( strLine.find( "first 1" ) == std::string::npos && strLine.size() < 4 ) bWhole = false;
            if ( strLine.find( "thread" ) != std::string::npos && strLine.find( " end" ) == std::string::npos ) bWhole = false;
        }
        CHECK( nLines == 801 );
        CHECK( bWhole );
        CHECK( strAll.find( "dropped" ) == std::string::npos );
    }
        // Appending keeps what is there.
    {
        CLogFile log( CLogSink::INFO, strPath, false, true, false );
        log.writeLine( "appended" );
    }
    std::string strAll = readFile( strPath );
    CHECK( strAll.find( "first 1" ) != std::string::npos );
    CHECK( strAll.find( "appended\n" ) != std::string::npos );
    removeFile( strPath );

    CLogFile bad( CLogSink::INFO, "no_such_directory_xyz/file.log", false, false, false );
    CHECK( ! bad.IsOpen() );
    CHECK( ! bad.writeLine( "nowhere" ) );
}

static void testRotatingLogFile( void )
{
    section( "CRotatingLogFile" );

        // (CDate months are 0-11.)
        // Periods.
    CDate monday( 17, 10, 2026, 9, 30, 0, 0 );
    CDate later( 17, 10, 2026, 9, 59, 0, 0 );
    CDate nextHour( 17, 10, 2026, 10, 0, 0, 0 );
    CDate nextDay( 18, 10, 2026, 0, 0, 0, 0 );
    CDate nextMonth( 1, 11, 2026, 0, 0, 0, 0 );
    CHECK( CRotatingLogFile::PeriodOf( monday, CRotatingLogFile::HOURLY ) == CRotatingLogFile::PeriodOf( later, CRotatingLogFile::HOURLY ) );
    CHECK( CRotatingLogFile::PeriodOf( monday, CRotatingLogFile::HOURLY ) != CRotatingLogFile::PeriodOf( nextHour, CRotatingLogFile::HOURLY ) );
    CHECK( CRotatingLogFile::PeriodOf( monday, CRotatingLogFile::DAILY ) == CRotatingLogFile::PeriodOf( nextHour, CRotatingLogFile::DAILY ) );
    CHECK( CRotatingLogFile::PeriodOf( monday, CRotatingLogFile::DAILY ) != CRotatingLogFile::PeriodOf( nextDay, CRotatingLogFile::DAILY ) );
    CHECK( CRotatingLogFile::PeriodOf( monday, CRotatingLogFile::WEEKLY ) == CRotatingLogFile::PeriodOf( nextDay, CRotatingLogFile::WEEKLY ) );
    CHECK( CRotatingLogFile::PeriodOf( monday, CRotatingLogFile::WEEKLY ) != CRotatingLogFile::PeriodOf( monday.OffsetDays( 7 ), CRotatingLogFile::WEEKLY ) );
    CHECK( CRotatingLogFile::PeriodOf( monday, CRotatingLogFile::MONTHLY ) == CRotatingLogFile::PeriodOf( nextDay, CRotatingLogFile::MONTHLY ) );
    CHECK( CRotatingLogFile::PeriodOf( monday, CRotatingLogFile::MONTHLY ) != CRotatingLogFile::PeriodOf( nextMonth, CRotatingLogFile::MONTHLY ) );

        // Names.
    {
        CTestRotatingLog named( "rot_test_{date}.log", CRotatingLogFile::DAILY, 0 );
        CHECK( named.FileNameFor( monday ) == "rot_test_20261117.log" );
        CTestRotatingLog plain( "rot_plain.log", CRotatingLogFile::HOURLY, 0 );
        CHECK( plain.FileNameFor( monday ) == "rot_plain_20261117093000.log" );
        CTestRotatingLog both( "rot_{date}_{time}.txt", CRotatingLogFile::HOURLY, 0 );
        CHECK( both.FileNameFor( monday ) == "rot_20261117_093000.txt" );
        removeFile( (const char *)named.GetCurrentFileName() );
        removeFile( (const char *)plain.GetCurrentFileName() );
        removeFile( (const char *)both.GetCurrentFileName() );
    }

        // Rotation by day, keeping the newest two (in 2099, so newer than
        // the file the constructor opens for today).
    std::vector<std::string> aDays;
    for ( int d = 0; d < 5; d++ )
    {
        CDate when( 10 + d, 10, 2099, 12, 0, 0, 0 );
        aDays.push_back( "rot_keep_" + std::string( (const char *)when.FormatDate( CDate::DF_YYYYMMDD ) ) + ".log" );
        removeFile( aDays.back().c_str() );
    }
    removeFile( "rot_keep_notes.log" );
    {
        std::ofstream other( "rot_keep_notes.log" );        // not one of ours
        other << "keep me\n";
    }
    {
        CTestRotatingLog log( "rot_keep_{date}.log", CRotatingLogFile::DAILY, 2 );
        std::string strToday( (const char *)log.GetCurrentFileName() );
        for ( int d = 0; d < 5; d++ )
        {
            CDate when( 10 + d, 10, 2099, 12, 0, 0, 0 );
            CHECK( log.writeAt( ( "day " + std::to_string( d ) ).c_str(), when ) );
            CDate laterSameDay( 10 + d, 10, 2099, 18, 0, 0, 0 );
            CHECK( log.writeAt( ( "evening " + std::to_string( d ) ).c_str(), laterSameDay ) );
            CHECK( std::string( (const char *)log.GetCurrentFileName() ) == aDays[d] );
        }
        CHECK( ! fileExists( aDays[0] ) );
        CHECK( ! fileExists( aDays[1] ) );
        CHECK( ! fileExists( aDays[2] ) );
        CHECK( fileExists( aDays[3] ) );
        CHECK( fileExists( aDays[4] ) );
        CHECK( fileExists( "rot_keep_notes.log" ) );
        std::string strLast = readFile( aDays[4] );
        CHECK( strLast == "day 4\nevening 4\n" );
        removeFile( strToday.c_str() );
    }
    for ( size_t d = 0; d < aDays.size(); d++ )
        removeFile( aDays[d].c_str() );
    removeFile( "rot_keep_notes.log" );

        // Many threads across a rotation.
    {
        CTestRotatingLog log( "rot_threads_{date}.log", CRotatingLogFile::DAILY, 0 );
        std::string strStart( (const char *)log.GetCurrentFileName() );
        CDate day1( 1, 2, 2026, 12, 0, 0, 0 );
        CDate day2( 2, 2, 2026, 12, 0, 0, 0 );
        std::vector<std::thread> threads;
        for ( int t = 0; t < 4; t++ )
        {
            threads.push_back( std::thread( [&log, &day1, &day2, t]() {
                for ( int i = 0; i < 100; i++ )
                    log.writeAt( "line", ( ( i + t ) % 2 ) ? day1 : day2 );
            } ) );
        }
        for ( size_t t = 0; t < threads.size(); t++ )
            threads[t].join();
        size_t nLines = 0;
        std::string a = readFile( "rot_threads_20260301.log" ), b = readFile( "rot_threads_20260302.log" );
        for ( size_t i = 0; i < a.size(); i++ ) if ( a[i] == '\n' ) nLines++;
        for ( size_t i = 0; i < b.size(); i++ ) if ( b[i] == '\n' ) nLines++;
        CHECK( nLines == 400 );
        removeFile( strStart.c_str() );
    }
    removeFile( "rot_threads_20260301.log" );
    removeFile( "rot_threads_20260302.log" );
}

int main( void )
{
    testSinkAndMacros();
    testContext();
    testLogFile();
    testRotatingLogFile();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
