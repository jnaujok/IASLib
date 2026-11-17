/**
 * CDate tests
 *
 * Checks CDate against independent references: Julian Day Number arithmetic
 * for the calendar (historical Julian/Gregorian and proleptic Gregorian),
 * gmtime() and time() for Unix times and timestamps, and fixed expectations
 * for parsing, time zones and ISO 8601.
 */

#include "IASLib.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <random>
#include <memory>
#include <string>
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

    // Reports the input and both values when a string comparison fails.
#define CHECK_EQ_STR( input, got, expected ) \
    do { \
        g_nChecks++; \
        std::string _got = ( got ); \
        std::string _exp = ( expected ); \
        if ( _got != _exp ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: \"%s\" gave \"%s\", expected \"%s\"\n", __FILE__, __LINE__, ( input ), _got.c_str(), _exp.c_str() ); \
        } \
    } while ( 0 )

/***********************************************************************
 * References
 ***********************************************************************/

    // Julian Day Numbers; years are astronomical (1 BC = 0).
static long jdnGregorian( long y, long m, long d )
{
    long a = ( 14 - m ) / 12, y2 = y + 4800 - a, m2 = m + 12 * a - 3;
    return d + ( 153 * m2 + 2 ) / 5 + 365 * y2 + y2 / 4 - y2 / 100 + y2 / 400 - 32045;
}

static long jdnJulian( long y, long m, long d )
{
    long a = ( 14 - m ) / 12, y2 = y + 4800 - a, m2 = m + 12 * a - 3;
    return d + ( 153 * m2 + 2 ) / 5 + 365 * y2 + y2 / 4 - 32083;
}

static bool gregorianLeap( long y ) { return ( y % 4 == 0 && y % 100 != 0 ) || y % 400 == 0; }
static bool julianLeap( long y ) { return ( ( y % 4 ) + 4 ) % 4 == 0; }

static int monthLength( long y, long m, bool julian )
{
    static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool leap = julian ? julianLeap( y ) : gregorianLeap( y );
    return ( m == 2 && leap ) ? 29 : days[ m - 1 ];
}

    // Gregorian date of a Julian Day Number (astronomical year).
static void gregorianFromJdn( long jdn, long &y, long &m, long &d )
{
    long a = jdn + 32044;
    long b = ( 4 * a + 3 ) / 146097;
    long c = a - 146097 * b / 4;
    long dd = ( 4 * c + 3 ) / 1461;
    long e = c - 1461 * dd / 4;
    long mm = ( 5 * e + 2 ) / 153;
    d = e - ( 153 * mm + 2 ) / 5 + 1;
    m = mm + 3 - 12 * ( mm / 10 );
    y = 100 * b + dd - 4800 + mm / 10;
}

static std::string iso( const CDate &d, bool ms = true )
{
    if ( ! d.IsValid() )
        return "INVALID";
    return std::string( (const char *)d.FormatISO8601( true, ms ) );
}

    // Historical calendar label and time: "YYYY-MM-DD hh:mm:ss.mmm".
static std::string hist( const CDate &d )
{
    if ( ! d.IsValid() )
        return "INVALID";
    char buffer[ 64 ];
    snprintf( buffer, sizeof( buffer ), "%04d-%02d-%02d %02d:%02d:%02d.%03d", d.GetYear(), d.GetMonth() + 1, d.GetDay(),
              d.GetHours(), d.GetMinutes(), d.GetSeconds(), d.GetMillisecond() );
    return buffer;
}

/***********************************************************************
 * Calendar
 ***********************************************************************/

    // Every day from 1000 BC to AD 9999 in the historical calendar: consecutive
    // day numbers, round trips, weekdays and the ISO (proleptic Gregorian) label.
static void testCalendarSweep( void )
{
    printf( "Calendar sweep, 1000 BC to AD 9999\n" );
    long errors = 0, days = 0;
    long prevJdn = 0, prevDay = 0;
    bool first = true;

    for ( long y = -1000; y <= 9999; y++ )
    {
        if ( y == 0 )
            continue;
        long astro = ( y < 0 ) ? y + 1 : y;
        for ( long m = 1; m <= 12; m++ )
        {
            bool julian = ( astro < 1582 ) || ( astro == 1582 && m < 10 );
            int nd = monthLength( astro, m, astro < 1582 );
            for ( long d = 1; d <= nd; d++ )
            {
                bool before = ( astro < 1582 ) || ( astro == 1582 && ( m < 10 || ( m == 10 && d <= 4 ) ) );
                if ( astro == 1582 && m == 10 && d > 4 && d < 15 )
                {
                    CHECK( ! CDate::IsValidDate( (int)d, 9, 1582 ) );     // the reform gap
                    continue;
                }
                (void)julian;
                long jdn = before ? jdnJulian( astro, m, d ) : jdnGregorian( astro, m, d );
                CDate date( (int)d, (int)( m - 1 ), (int)y );
                long gy, gm, gd;
                gregorianFromJdn( jdn, gy, gm, gd );
                long iy;
                int im, id;
                date.GetISODate( iy, im, id );

                bool ok = date.IsValid() && date.GetYear() == y && date.GetMonth() == m - 1 && date.GetDay() == d &&
                          date.GetDOW() == (int)( ( jdn + 1 ) % 7 ) && iy == gy && im == gm && id == gd &&
                          date.GetJulianDayNumber() == jdn;
                if ( ! first )
                    ok = ok && ( date.GetEpochDay() - prevDay == jdn - prevJdn );
                if ( ! ok )
                {
                    if ( errors < 5 )
                        printf( "    %ld-%02ld-%02ld came back as %s (weekday %d, ISO %ld-%02d-%02d)\n", y, m, d, hist( date ).c_str(), date.GetDOW(), iy, im, id );
                    errors++;
                }
                prevJdn = jdn;
                prevDay = date.GetEpochDay();
                first = false;
                days++;
            }
        }
    }
    printf( "  %ld days checked\n", days );
    CHECK( errors == 0 );
}

static void testCalendarFacts( void )
{
    printf( "Calendar facts\n" );
    CDate probe( 1, 0, 2000 );

        // Leap years: Julian (every 4 years, 1 BC = year -1 is one) and Gregorian.
    CHECK( probe.IsLeapYear( -1 ) );
    CHECK( probe.IsLeapYear( -5 ) );
    CHECK( ! probe.IsLeapYear( -4 ) );
    CHECK( probe.IsLeapYear( 4 ) );
    CHECK( probe.IsLeapYear( 1500 ) );
    CHECK( ! probe.IsLeapYear( 1582 ) );
    CHECK( ! probe.IsLeapYear( 1700 ) );
    CHECK( ! probe.IsLeapYear( 1900 ) );
    CHECK( probe.IsLeapYear( 2000 ) );
    CHECK( probe.IsLeapYear( 4000 ) );
    CHECK( ! probe.IsLeapYear( 0 ) );

        // The reform: Thursday 4 October 1582 was followed by Friday 15 October.
    CDate oct4( 4, 9, 1582 ), oct15( 15, 9, 1582 );
    CHECK( oct15.GetEpochDay() - oct4.GetEpochDay() == 1 );
    CHECK( oct4.GetDOW() == 4 && oct15.GetDOW() == 5 );
    CHECK( CDate( 1, 0, 1583 ).GetEpochDay() - CDate( 1, 0, 1582 ).GetEpochDay() == 355 );
    CHECK( ! CDate( 10, 9, 1582 ).IsValid() );
    CHECK( CDate( 15, 9, 1582 ).GetDOY() == 278 );

        // There is no year 0: 31 December 1 BC is followed by 1 January AD 1.
    CHECK( CDate( 1, 0, 1 ).GetEpochDay() - CDate( 31, 11, -1 ).GetEpochDay() == 1 );
    CHECK( ! CDate( 1, 0, 0 ).IsValid() );
    CHECK( CDate( 1, 0, 1 ).GetDOW() == 6 );                          // Saturday

        // Day numbers from November 1582 on are unchanged from earlier versions.
    CHECK( CDate( 1, 0, 1970 ).GetEpochDay() == 719164 );
    CHECK( CDate( 1, 10, 1582 ).GetEpochDay() == 577754 );
    CHECK( CDate( 1, 2, 2000 ).GetEpochDay() == 730181 );

        // Dates that do not exist.
    CHECK( ! CDate( 29, 1, 1900 ).IsValid() );
    CHECK( CDate( 29, 1, 1500 ).IsValid() );                           // Julian leap year
    CHECK( ! CDate( 31, 3, 2001 ).IsValid() );
    CHECK( ! CDate( 1, 12, 2001 ).IsValid() );
    CHECK( ! CDate( 1, -1, 2001 ).IsValid() );
    CHECK( ! CDate( 1, 0, 2001, 24, 0, 0 ).IsValid() );
    CHECK( ! CDate( 1, 0, 2001, 0, 60, 0 ).IsValid() );
    CHECK( ! CDate( 1, 0, 2000000 ).IsValid() );
    CHECK( CDate( 1, 0, 20000 ).IsValid() && CDate( 1, 0, 20000 ).GetYear() == 20000 );
    CHECK( CDate( 1, 0, -20000 ).IsValid() && CDate( 1, 0, -20000 ).GetYear() == -20000 );

        // SetDate agrees with the constructor, including 1582.
    CDate set( 1, 0, 2000 );
    set.SetDate( 1, 11, 1582 );
    CHECK( set.GetEpochDay() == CDate( 1, 11, 1582 ).GetEpochDay() );

        // The weekday is printed for early dates too.
    CHECK_EQ_STR( "AD 1", std::string( (const char *)CDate( 15, 5, 1 ).FormatDate( CDate::DF_RFC1123 ) ), "Wed, 15 Jun 1 00:00:00 GMT" );      // 1 Jan AD 1 was a Saturday
}

/***********************************************************************
 * Herschel correction
 ***********************************************************************/

static void testHerschel( void )
{
    printf( "Herschel correction\n" );
    CHECK( ! CDate::GetHerschelCorrection() );
    long lStandardMar1 = CDate( 1, 2, 4000 ).GetEpochDay();
    CHECK( CDate( 29, 1, 4000 ).IsValid() );
    CHECK( CDate( 31, 11, 9999 ).GetDOW() == 5 );

    CDate::SetHerschelCorrection( true );
    CDate probe( 1, 0, 2000 );
    CHECK( ! probe.IsLeapYear( 4000 ) && ! probe.IsLeapYear( 8000 ) && probe.IsLeapYear( 4400 ) && probe.IsLeapYear( 2000 ) );
    CHECK( ! CDate( 29, 1, 4000 ).IsValid() );
    CHECK( CDate( 1, 2, 4000 ).GetEpochDay() == lStandardMar1 - 1 );
    CHECK( CDate( 1, 2, 4000 ).GetEpochDay() - CDate( 28, 1, 4000 ).GetEpochDay() == 1 );
    CHECK( CDate( 31, 11, 9999 ).GetDOW() == 3 );                     // two days fewer
    CHECK( CDate( 1, 0, 1970 ).GetEpochDay() == 719164 );

        // Round trips across both dropped days.
    long errors = 0;
    for ( long lDay = CDate( 1, 0, 3999 ).GetEpochDay(); lDay <= CDate( 31, 11, 8001 ).GetEpochDay(); lDay++ )
    {
        CDate date( lDay, 0L );
        CDate again( date.GetDay(), date.GetMonth(), date.GetYear() );
        if ( again.GetEpochDay() != lDay )
            errors++;
    }
    CHECK( errors == 0 );

        // ISO 8601 has no Herschel rule: the same day keeps its ISO label.
    CHECK_EQ_STR( "Herschel ISO", std::string( (const char *)CDate( 1, 2, 4000 ).FormatISO8601( false ) ), "4000-02-29" );

    CDate::SetHerschelCorrection( false );
    CHECK( CDate( 1, 2, 4000 ).GetEpochDay() == lStandardMar1 );
}

/***********************************************************************
 * Parsing
 ***********************************************************************/

struct ParseCase
{
    const char *input;
    const char *expected;           // historical calendar, "INVALID" for none
};

static void runParseCases( const char *title, const ParseCase *cases, size_t count )
{
    printf( "%s\n", title );
    for ( size_t n = 0; n < count; n++ )
    {
            // Compare in UTC: dates with a zone keep their local time.
        CDate d = CDate::Parse( cases[ n ].input );
        CHECK_EQ_STR( cases[ n ].input, hist( d.ToUTC() ), cases[ n ].expected );
    }
}

static void testParsing( void )
{
    CDate now;
    char year[ 8 ];
    snprintf( year, sizeof( year ), "%04d", now.GetYear() % 10000 );
    std::string Y = year;
    std::string jan1 = Y + "-01-01 01:00:00.000";
    std::string jan17 = Y + "-01-17 04:25:00.000";

    const ParseCase documented[] = {
        { "01/01/1998  12:24:44", "1998-01-01 12:24:44.000" },
        { "1/1/2099", "2099-01-01 00:00:00.000" },
        { "1/1/99", "1999-01-01 00:00:00.000" },
        { "1/1/1998 1:1:1 AM", "1998-01-01 01:01:01.000" },
        { "1-1-98 10:13 PM", "1998-01-01 22:13:00.000" },
        { "1-1 1 AM", jan1.c_str() },
        { "1-17  10:05 GMT +5:40", jan17.c_str() },
        { "August 13th, 1998 at 13:05:20 GMT", "1998-08-13 13:05:20.000" },
        { "Aug 14 1998 5:03 PM", "1998-08-14 17:03:00.000" },
        { "1998-08-13T13:05:20Z", "1998-08-13 13:05:20.000" },
        { "13.08.1998", "1998-08-13 00:00:00.000" },
    };
    runParseCases( "Documented examples", documented, sizeof( documented ) / sizeof( documented[0] ) );

    const ParseCase formats[] = {
        { "Sat Jun  2 12:14:11 2001", "2001-06-02 12:14:11.000" },
        { "Sat Jun  2 12:14:11 GMT 2001", "2001-06-02 12:14:11.000" },
        { "Saturday, 02-Jun-01 12:14:11 GMT", "2001-06-02 12:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 GMT", "2001-06-02 12:14:11.000" },
        { "02/Jun/2001:12:14:11", "2001-06-02 12:14:11.000" },
        { "02/Jun/2001:12:14:11 -0700", "2001-06-02 19:14:11.000" },
        { "06/02/2001  12:14:11.374", "2001-06-02 12:14:11.374" },
        { "Jun 02 2001 12:14 PM", "2001-06-02 12:14:00.000" },
        { "2001/06/02 12:14:11", "2001-06-02 12:14:11.000" },
        { "02-JUN-2001, 12:14:11 PM", "2001-06-02 12:14:11.000" },
        { "20010602", "2001-06-02 00:00:00.000" },
        { "2001-06-02 12:14:11.374", "2001-06-02 12:14:11.374" },
        { "20010602121411.374", "2001-06-02 12:14:11.374" },
        { "June 2, 2001", "2001-06-02 00:00:00.000" },
        { "Jun 2 2001", "2001-06-02 00:00:00.000" },
        { "2 June 2001", "2001-06-02 00:00:00.000" },
        { "Saturday, June 2, 2001", "2001-06-02 00:00:00.000" },
        { "June 2nd, 2001 12:14:11", "2001-06-02 12:14:11.000" },
        { "02.06.2001 12:14", "2001-06-02 12:14:00.000" },
        { "October 19 1961 04:15:31:111 pm", "1961-10-19 16:15:31.111" },
        { "1/1/1998 12:00 AM", "1998-01-01 00:00:00.000" },
        { "1/1/1998 12:00 PM", "1998-01-01 12:00:00.000" },
    };
    runParseCases( "Formats", formats, sizeof( formats ) / sizeof( formats[0] ) );

    const ParseCase zones[] = {
        { "Sat, 02 Jun 2001 12:14:11 EST", "2001-06-02 17:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 PDT", "2001-06-02 19:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 +0500", "2001-06-02 07:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 -0500", "2001-06-02 17:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 +0530", "2001-06-02 06:44:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 BST", "2001-06-02 11:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 CET", "2001-06-02 11:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 CEST", "2001-06-02 10:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 A", "2001-06-02 11:14:11.000" },
        { "Sat, 02 Jun 2001 12:14:11 Y", "2001-06-03 00:14:11.000" },
        { "Sat, 02 Jun 2001 23:30:00 GMT-1", "2001-06-03 00:30:00.000" },
        { "06/02/2001 12:14:11 EST", "2001-06-02 17:14:11.000" },
        { "06/02/2001 12:14 GMT+2", "2001-06-02 10:14:00.000" },
    };
    runParseCases( "Time zones (to UTC)", zones, sizeof( zones ) / sizeof( zones[0] ) );

    const ParseCase years[] = {
        { "10/14/1066", "1066-10-14 00:00:00.000" },
        { "14 Oct 1066", "1066-10-14 00:00:00.000" },
        { "July 4 1776", "1776-07-04 00:00:00.000" },
        { "3/15/0044", "0044-03-15 00:00:00.000" },
        { "1/1/0999", "0999-01-01 00:00:00.000" },
        { "1/1/500", "0500-01-01 00:00:00.000" },
        { "0005/03/15", "0005-03-15 00:00:00.000" },
        { "1/1/99999", "99999-01-01 00:00:00.000" },
            // ISO text is proleptic Gregorian: 1066-10-14 is Julian 8 October.
        { "1066-10-14", "1066-10-08 00:00:00.000" },
    };
    runParseCases( "Years", years, sizeof( years ) / sizeof( years[0] ) );

    const ParseCase invalid[] = {
        { "02/30/2001", "INVALID" },
        { "13/01/2001", "INVALID" },
        { "14/13/2001", "INVALID" },
        { "99/99", "INVALID" },
        { "1/1/2001 25:61:61", "INVALID" },
        { "1/1/2001 13:00 PM", "INVALID" },
        { "hello", "INVALID" },
        { "32", "INVALID" },
        { "10/10/1582", "INVALID" },
        { "06/02/2001 garbage", "INVALID" },
        { "2001-02-29", "INVALID" },
    };
    runParseCases( "Invalid input", invalid, sizeof( invalid ) / sizeof( invalid[0] ) );

    printf( "Parsing does not depend on earlier calls\n" );
    std::string a = hist( CDate::Parse( "06/02/2001 12:14:11 GMT" ) );
    CDate::Parse( "Sat, 02 Dec 2001 12:14:11 GMT" );
    std::string b = hist( CDate::Parse( "06/02/2001 12:14:11 GMT" ) );
    CDate::Parse( "Sat, 02 Jun 2001 12:14:11 GMT" );
    std::string c = hist( CDate::Parse( "06/02/2001 12:14:11 GMT" ) );
    CHECK( a == "2001-06-02 12:14:11.000" && b == a && c == a );

    printf( "Relative dates\n" );
    CDate today = CDate::Parse( "Today" );
    CHECK( today.GetEpochDay() == now.GetEpochDay() && today.GetSOD() == 0 );
    CHECK( CDate::Parse( "Today +6" ).GetEpochDay() - today.GetEpochDay() == 6 );
    CHECK( today.GetEpochDay() - CDate::Parse( "Today - 3" ).GetEpochDay() == 3 );
    CDate n = CDate::Parse( "Now" );
    long long plus = CDate::Parse( "Now + 120" ).Elapsed( n );
    long long minus = CDate::Parse( "Now - 4:55" ).Elapsed( n );
    CHECK( plus >= 4800000 && plus < 4802000 );
    CHECK( minus <= -17700000 + 2000 && minus > -17702000 );
    CHECK( CDate::Parse( "Start of day" ).GetSOD() == 0 );
    CHECK( CDate::Parse( "" ).IsValid() && CDate::Parse( "" ).Elapsed( n ) < 2000 );
    CHECK( ! CDate::Parse( "November 2" ).GetSOD() || CDate::Parse( "November 2" ).GetMonth() == 10 );

    printf( "Parsed time zones are kept\n" );
    CDate est = CDate::Parse( "Sat, 02 Jun 2001 12:14:11 EST" );
    CHECK( est.HasTimeZone() && est.GetUTCOffset() == -18000 && ! est.IsGMT() );
    CHECK( est.GetHour() == 12 && est.ToUTC().GetHour() == 17 );
    CHECK( CDate::Parse( "Sat, 02 Jun 2001 12:14:11 GMT" ).IsGMT() );
    CHECK( ! CDate::Parse( "06/02/2001 12:14:11" ).HasTimeZone() );

    printf( "ParseHTTP and ParseSybase\n" );
    CHECK_EQ_STR( "rfc1123", hist( CDate::ParseHTTP( "Sat, 02 Jun 2001 12:14:11 GMT" ) ), "2001-06-02 12:14:11.000" );
    CHECK_EQ_STR( "rfc850", hist( CDate::ParseHTTP( "Saturday, 02-Jun-01 12:14:11 GMT" ) ), "2001-06-02 12:14:11.000" );
    CHECK_EQ_STR( "asctime", hist( CDate::ParseHTTP( "Sat Jun  2 12:14:11 2001" ) ), "2001-06-02 12:14:11.000" );
    CHECK( ! CDate::ParseHTTP( "Sat, 02 Foo 2001 12:14:11 GMT" ).IsValid() );
    CHECK( CDate::ParseHTTP( "Sat, 02 Jun 2001 12:14:11 GMT" ).IsGMT() );
    CHECK_EQ_STR( "sybase", hist( CDate::ParseSybase( "October 19 1961 04:15:31:111 pm" ) ), "1961-10-19 16:15:31.111" );
    CHECK_EQ_STR( "sybase", hist( CDate::ParseSybase( "Oct 19 1961 04:15:31:111PM" ) ), "1961-10-19 16:15:31.111" );
    CHECK( ! CDate::ParseSybase( "Foo 19 1961 04:15:31:111 pm" ).IsValid() );

    printf( "Every output format reads back\n" );
    CDate sample( 2, 5, 2001, 12, 14, 11, 374 );
    struct { CDate::FormatFlags format; const char *expected; } outputs[] = {
        { CDate::DF_PRETTY, "2001-06-02 12:14:11.000" },
        { CDate::DF_UNIX, "2001-06-02 12:14:11.000" },
        { CDate::DF_RFC850, "2001-06-02 12:14:11.000" },
        { CDate::DF_RFC1123, "2001-06-02 12:14:11.000" },
        { CDate::DF_NETSCAPE, "2001-06-02 12:14:11.000" },
        { CDate::DF_EXACT, "2001-06-02 12:14:11.374" },
        { CDate::DF_SYBASE, "2001-06-02 12:14:00.000" },
        { CDate::DF_SQLITE, "2001-06-02 12:14:11.000" },
        { CDate::DF_ORACLE, "2001-06-02 12:14:11.000" },
        { CDate::DF_YYYYMMDD, "2001-06-02 00:00:00.000" },
        { CDate::DF_ISO_8601, "2001-06-02 12:14:11.000" },
        { CDate::DF_ISO_8601_MS, "2001-06-02 12:14:11.374" },
        { CDate::DF_ISO_8601_MS_PACKED, "2001-06-02 12:14:11.374" },
        { CDate::DF_RFC7231, "2001-06-02 12:14:11.000" },
        { CDate::DF_ISO_8601_FULL, "2001-06-02 12:14:11.000" },
        { CDate::DF_ISO_8601_FULL_MS, "2001-06-02 12:14:11.374" },
        { CDate::DF_ISO_8601_DATE, "2001-06-02 00:00:00.000" },
    };
    for ( size_t k = 0; k < sizeof( outputs ) / sizeof( outputs[0] ); k++ )
    {
        CString text = sample.FormatDate( outputs[ k ].format );
        CHECK_EQ_STR( (const char *)text, hist( CDate::Parse( text ) ), outputs[ k ].expected );
    }
}

/***********************************************************************
 * ISO 8601
 ***********************************************************************/

static void testISO8601( void )
{
    printf( "ISO 8601 parsing\n" );
    struct { const char *input; const char *expected; } cases[] = {
        { "2001-06-02", "2001-06-02T00:00:00.000" },
        { "20010602", "2001-06-02T00:00:00.000" },
        { "2001-06", "2001-06-01T00:00:00.000" },
        { "2001", "2001-01-01T00:00:00.000" },
        { "2001-153", "2001-06-02T00:00:00.000" },
        { "2001153", "2001-06-02T00:00:00.000" },
        { "2000-366", "2000-12-31T00:00:00.000" },
        { "2001-W22-6", "2001-06-02T00:00:00.000" },
        { "2001W226", "2001-06-02T00:00:00.000" },
        { "2001-W22", "2001-05-28T00:00:00.000" },
        { "2004-W53-6", "2005-01-01T00:00:00.000" },
        { "2008-W01-1", "2007-12-31T00:00:00.000" },
        { "2009-W53-7", "2010-01-03T00:00:00.000" },
        { "2001-06-02T12:14:11", "2001-06-02T12:14:11.000" },
        { "2001-06-02T12:14:11Z", "2001-06-02T12:14:11.000Z" },
        { "2001-06-02 12:14:11.374Z", "2001-06-02T12:14:11.374Z" },
        { "2001-06-02T12:14:11,5Z", "2001-06-02T12:14:11.500Z" },
        { "20010602T121411Z", "2001-06-02T12:14:11.000Z" },
        { "2001-06-02T12:14", "2001-06-02T12:14:00.000" },
        { "2001-06-02T12", "2001-06-02T12:00:00.000" },
        { "2001-06-02T12.5", "2001-06-02T12:30:00.000" },
        { "2001-06-02T12:14.5", "2001-06-02T12:14:30.000" },
        { "2001-06-02T24:00", "2001-06-03T00:00:00.000" },
        { "2001-06-02T23:59:60Z", "2001-06-02T23:59:59.999Z" },
        { "2001-06-02T12:14:11+05:30", "2001-06-02T12:14:11.000+05:30" },
        { "2001-06-02T12:14:11-0700", "2001-06-02T12:14:11.000-07:00" },
        { "2001-06-02T01:00:00+02", "2001-06-02T01:00:00.000+02:00" },
        { "2001-06-02T01:00:00+05:30:15", "2001-06-02T01:00:00.000+05:30:15" },
        { "0000-01-01", "0000-01-01T00:00:00.000" },
        { "-0043-03-15", "-0043-03-15T00:00:00.000" },
        { "+12345-01-01", "+12345-01-01T00:00:00.000" },
        { "1582-10-15", "1582-10-15T00:00:00.000" },
        { "1582-10-10", "1582-10-10T00:00:00.000" },        // proleptic Gregorian has these days
    };
    for ( size_t n = 0; n < sizeof( cases ) / sizeof( cases[0] ); n++ )
    {
        bool bValid = false;
        CDate d = CDate::ParseISO8601( cases[ n ].input, &bValid );
        CHECK( bValid );
        CHECK_EQ_STR( cases[ n ].input, iso( d ), cases[ n ].expected );
    }

        // Proleptic Gregorian dates before the reform are Julian dates in the
        // historical calendar, and BC years shift by one.
    CHECK_EQ_STR( "1582-10-10", hist( CDate::ParseISO8601( "1582-10-10" ) ), "1582-09-30 00:00:00.000" );
    CHECK_EQ_STR( "-0043-03-15", hist( CDate::ParseISO8601( "-0043-03-15" ) ), "-044-03-17 00:00:00.000" );
    CHECK_EQ_STR( "0000-12-31", hist( CDate::ParseISO8601( "0000-12-31" ) ), "0001-01-02 00:00:00.000" );     // Julian runs 2 days ahead here

    printf( "ISO 8601 rejects malformed text\n" );
    const char *bad[] = { "2001-02-29", "2001-13-01", "2001-00-10", "2001-06-31", "2001-6-2", "01-06-02", "2001-06-02T25:00",
                          "2001-06-02T24:30", "2001-06-02T12:60", "2001-06-02T12:14:61", "2001-W54", "2001-W53-1", "2001-W22-8",
                          "2001-366", "2001-000", "200106", "2001-06-02T12:14:11+24:00", "2001-06-02T12:14:11 junk",
                          "2001-06-02T", "2001-06-02T12:14:11.", "June 2 2001", "", "+123", "2001-06-02Z" };
    for ( size_t n = 0; n < sizeof( bad ) / sizeof( bad[0] ); n++ )
    {
        bool bValid = true;
        CDate d = CDate::ParseISO8601( bad[ n ], &bValid );
        CHECK_EQ_STR( bad[ n ], ( bValid || d.IsValid() ) ? "accepted" : "rejected", "rejected" );
    }

    printf( "ISO 8601 formatting\n" );
    CDate local( 2, 5, 2001, 12, 14, 11, 374 );
    CHECK_EQ_STR( "local", std::string( (const char *)local.FormatISO8601() ), "2001-06-02T12:14:11" );
    CHECK_EQ_STR( "local ms", std::string( (const char *)local.FormatISO8601( true, true ) ), "2001-06-02T12:14:11.374" );
    CHECK_EQ_STR( "date", std::string( (const char *)local.FormatISO8601( false ) ), "2001-06-02" );
    CHECK_EQ_STR( "utc", std::string( (const char *)CDate::ParseUnix( 991483651 ).FormatISO8601() ), "2001-06-02T12:07:31Z" );
    CHECK_EQ_STR( "julian", std::string( (const char *)CDate( 14, 9, 1066 ).FormatISO8601( false ) ), "1066-10-20" );
    CHECK_EQ_STR( "bc", std::string( (const char *)CDate( 15, 2, -44 ).FormatISO8601( false ) ), "-0043-03-13" );
    CHECK_EQ_STR( "big", std::string( (const char *)CDate( 1, 0, 12345 ).FormatISO8601( false ) ), "+12345-01-01" );
    CHECK_EQ_STR( "invalid", std::string( (const char *)CDate( 30, 1, 2001 ).FormatISO8601() ), "Invalid Date" );

    printf( "ISO 8601 round trips\n" );
    std::mt19937 generator( 12345 );
    std::uniform_int_distribution<long> dayDist( CDate( 1, 0, -5000 ).GetEpochDay(), CDate( 31, 11, 99999 ).GetEpochDay() );
    std::uniform_int_distribution<long> msDist( 0, 86399999 );
    long errors = 0;
    for ( int n = 0; n < 20000; n++ )
    {
        CDate d = CDate::ParseUnix( 0 );
        d = CDate( dayDist( generator ), msDist( generator ) );
        CString text = d.FormatISO8601( true, true );
        CDate back = CDate::ParseISO8601( text );
        CString date = d.FormatISO8601( false );
        if ( back != d || CDate::ParseISO8601( date ).GetEpochDay() != d.GetEpochDay() )
        {
            if ( errors < 5 )
                printf( "    %s did not read back\n", (const char *)text );
            errors++;
        }
    }
    CHECK( errors == 0 );
}

/***********************************************************************
 * Unix times, timestamps and arithmetic
 ***********************************************************************/

static void testUnixAndTimestamps( void )
{
    printf( "ParseUnix against gmtime\n" );
    std::mt19937_64 generator( 99 );
    std::uniform_int_distribution<long long> dist( -30000000000LL, 30000000000LL );
    long errors = 0;
    for ( int n = 0; n < 20000; n++ )
    {
        long long v = ( n < 4 ) ? ( n == 0 ? 0 : n == 1 ? -1 : n == 2 ? -86401 : 2147483648LL ) : dist( generator );
        time_t t = (time_t)v;
        struct tm g;
        if ( gmtime_r( &t, &g ) == NULL )
            continue;
        char ref[ 64 ];
        snprintf( ref, sizeof( ref ), "%04d-%02d-%02dT%02d:%02d:%02d.000Z", g.tm_year + 1900, g.tm_mon + 1, g.tm_mday, g.tm_hour, g.tm_min, g.tm_sec );
        std::string got = iso( CDate::ParseUnix( t ) );
        if ( got != ref )
        {
            if ( errors < 5 )
                printf( "    %lld gave %s, gmtime %s\n", v, got.c_str(), ref );
            errors++;
        }
    }
    CHECK( errors == 0 );

    printf( "Timestamps\n" );
    long long epoch = atoll( (const char *)CDate::EpochTimestamp() );
    long long ntp = atoll( (const char *)CDate::NTPTimestamp() );
    long long t = (long long)time( NULL );
    CHECK( epoch >= t - 1 && epoch <= t + 1 );
    CHECK( ntp >= t + 2208988800LL - 1 && ntp <= t + 2208988800LL + 1 );

    printf( "Time server offset\n" );
    unsigned long ulServer = (unsigned long)( time( NULL ) + 2208988800LL + 3600 );
    CDate before;
    CDate::SynchronizeToTimeServer( ulServer );
    CDate after;
    CHECK( CDate::GetOffsetSeconds() >= 3599 && CDate::GetOffsetSeconds() <= 3601 );
    CHECK( after.Elapsed( before ) > 3598000 && after.Elapsed( before ) < 3602000 );
    CDate::SynchronizeToTimeServer( (unsigned long)( time( NULL ) + 2208988800LL ) );
    CHECK( CDate::GetOffsetSeconds() >= -1 && CDate::GetOffsetSeconds() <= 1 );

    printf( "Arithmetic\n" );
    CDate jan( 1, 0, 2001 ), feb( 1, 1, 2001 );
    CHECK( feb.Elapsed( jan ) == 2678400000LL );
    CHECK( feb.ElapsedDays( jan ) == 31 );
    CHECK( CDate( 1, 0, 3001 ).Elapsed( CDate( 1, 0, 2001 ) ) == 365242LL * 86400000LL );
    CDate late( 31, 11, 2001, 23, 0, 0 );
    late.Add( 0, 90, 0 );
    CHECK_EQ_STR( "add", hist( late ), "2002-01-01 00:30:00.000" );
    late.Add( -1, 0, -1800 );
    CHECK_EQ_STR( "add back", hist( late ), "2001-12-31 00:00:00.000" );
    CDate mid( 15, 5, 2001, 13, 45, 30, 250 );
    CDate hour = mid;
    hour.StartOfHour();
    CHECK_EQ_STR( "hour", hist( hour ), "2001-06-15 13:00:00.000" );
    mid.StartOfDay();
    CHECK_EQ_STR( "day", hist( mid ), "2001-06-15 00:00:00.000" );
    CHECK( CDate::GetOffsetSeconds() >= -1 && CDate::GetOffsetSeconds() <= 1 );
}

/***********************************************************************
 * Julian Day
 ***********************************************************************/

static bool near( double a, double b, double tolerance ) { return fabs( a - b ) <= tolerance; }

static void testJulianDay( void )
{
    printf( "Julian Day\n" );
    const double ms = 1.0 / 86400000.0;

        // Reference values; the historical ones are from Meeus, Astronomical
        // Algorithms, chapter 7 (Julian calendar before 1582).
    CHECK( near( CDate( 1, 0, 2000, 12, 0, 0 ).GetJulianDate(), 2451545.0, ms ) );          // J2000.0
    CHECK( near( CDate( 1, 0, 1970 ).GetJulianDate(), 2440587.5, ms ) );                     // Unix epoch
    CHECK( near( CDate( 4, 9, 1957, 19, 26, 24 ).GetJulianDate(), 2436116.31, ms ) );        // Sputnik 1
    CHECK( near( CDate( 27, 0, 333, 12, 0, 0 ).GetJulianDate(), 1842713.0, ms ) );
    CHECK( near( CDate( 12, 6, -1001, 12, 0, 0 ).GetJulianDate(), 1356001.0, ms ) );        // -1000 July 12.5
    CHECK( near( CDate( 1, 0, -4713, 12, 0, 0 ).GetJulianDate(), 0.0, ms ) );                // the zero point
    CHECK( CDate( 4, 9, 1582 ).GetJulianDayNumber() == 2299160 );
    CHECK( CDate( 15, 9, 1582 ).GetJulianDayNumber() == 2299161 );
    CHECK( CDate( 1, 0, -4713 ).GetJulianDayNumber() == 0 );
    CHECK( CDate( 31, 11, -4714 ).GetJulianDayNumber() == -1 );

        // The day number is the JD at noon: it does not change through the
        // civil day, while the JD crosses a whole number at noon.
    CHECK( CDate( 2, 5, 2001, 0, 0, 0 ).GetJulianDayNumber() == CDate( 2, 5, 2001, 23, 59, 59 ).GetJulianDayNumber() );
    CHECK( near( CDate( 2, 5, 2001, 0, 0, 0 ).GetJulianDate(), 2452062.5, ms ) );
    CHECK( near( CDate( 2, 5, 2001, 18, 0, 0 ).GetJulianDate(), 2452063.25, ms ) );

        // The other direction.
    CHECK_EQ_STR( "J2000", iso( CDate::FromJulianDate( 2451545.0 ) ), "2000-01-01T12:00:00.000Z" );
    CHECK_EQ_STR( "sputnik", iso( CDate::FromJulianDate( 2436116.31 ) ), "1957-10-04T19:26:24.000Z" );
    CHECK_EQ_STR( "jd 0", hist( CDate::FromJulianDate( 0.0 ) ), "-4713-01-01 12:00:00.000" );
    CHECK_EQ_STR( "jd -0.5", hist( CDate::FromJulianDate( -0.5 ) ), "-4713-01-01 00:00:00.000" );
    CHECK_EQ_STR( "jd -0.75", hist( CDate::FromJulianDate( -0.75 ) ), "-4714-12-31 18:00:00.000" );
    CHECK_EQ_STR( "jdn", hist( CDate::FromJulianDayNumber( 2299161 ) ), "1582-10-15 12:00:00.000" );
    CHECK( CDate::FromJulianDate( 2451545.0 ).IsGMT() );

        // Round trips at millisecond precision across the supported range.
    std::mt19937 generator( 4713 );
    std::uniform_int_distribution<long> dayDist( CDate( 1, 0, -10000 ).GetEpochDay(), CDate( 31, 11, 10000 ).GetEpochDay() );
    std::uniform_int_distribution<long> msDist( 0, 86399999 );
    long errors = 0;
    for ( int n = 0; n < 100000; n++ )
    {
        CDate d( dayDist( generator ), msDist( generator ) );
        CDate back = CDate::FromJulianDate( d.GetJulianDate() );
        if ( back != d || CDate::FromJulianDayNumber( d.GetJulianDayNumber() ).GetEpochDay() != d.GetEpochDay() )
        {
            if ( errors < 5 )
                printf( "    %s (JD %.8f) came back as %s\n", iso( d ).c_str(), d.GetJulianDate(), iso( back ).c_str() );
            errors++;
        }
    }
    CHECK( errors == 0 );
}

/***********************************************************************
 * Time zones
 ***********************************************************************/

static void setZone( const char *tz )
{
    if ( tz )
        setenv( "TZ", tz, 1 );
    else
        unsetenv( "TZ" );
    tzset();
}

static void testTimeZones( void )
{
    printf( "Time zones\n" );
    const double ms = 1.0 / 86400000.0;

        // Local time is kept; the instant uses the offset.
    CDate est = CDate::Parse( "Sat, 02 Jun 2001 12:14:11 EST" );
    CHECK_EQ_STR( "est iso", std::string( (const char *)est.FormatISO8601() ), "2001-06-02T12:14:11-05:00" );
    CHECK_EQ_STR( "est rfc1123", std::string( (const char *)est.FormatDate( CDate::DF_RFC1123 ) ), "Sat, 02 Jun 2001 17:14:11 GMT" );
    CHECK_EQ_STR( "est pretty", std::string( (const char *)est.FormatDate( CDate::DF_PRETTY ) ), "06/02/2001  12:14:11" );
    CHECK_EQ_STR( "est utc", iso( est.ToUTC(), false ), "2001-06-02T17:14:11Z" );
    CHECK_EQ_STR( "est in india", iso( est.ToTimeZone( 19800 ), false ), "2001-06-02T22:44:11+05:30" );
    CHECK( est.ToTimeZone( 19800 ) == est );

        // Comparisons and Elapsed() use instants.
    CDate paris = CDate::Parse( "2001-06-02T19:14:11+02:00" );
    CHECK( paris == est && ! ( paris < est ) && ! ( paris > est ) && paris.Elapsed( est ) == 0 );
    CHECK( CDate::Parse( "2001-06-02T19:14:12+02:00" ) > est );
    CHECK( paris.GetHour() == 19 );
    CHECK( CDate::Parse( "2001-06-02T00:30:00+14:00" ).ElapsedDays( CDate::Parse( "2001-06-01T23:30:00Z" ) ) == 1 );

        // Julian Days are UT: a zoned local time is converted.
    CDate india( 1, 0, 2000, 17, 30, 0 );
    CHECK( india.SetTimeZone( 19800 ) );
    CHECK( near( india.GetJulianDate(), 2451545.0, ms ) );
    CDate kiritimati( 2, 0, 2000, 0, 30, 0 );       // UTC+14: still 1 January in UT
    kiritimati.SetTimeZone( 14 * 3600 );
    CHECK( kiritimati.GetJulianDayNumber() == 2451545 );
    CHECK( near( kiritimati.GetJulianDate(), 2451544.9375, ms ) );
    CHECK( near( CDate::Parse( "Sat, 01 Jan 2000 07:00:00 EST" ).GetJulianDate(), 2451545.0, ms ) );
    CHECK( near( CDate( 1, 0, 2000, 12, 0, 0 ).GetJulianDate(), 2451545.0, ms ) );     // no zone: system zone, UTC here

        // Zone edits.
    CDate plain( 2, 5, 2001, 12, 0, 0 );
    CHECK( ! plain.SetTimeZone( 86400 ) && ! plain.HasTimeZone() );
    CHECK( plain.SetTimeZone( -3600 ) && plain.GetHour() == 12 && plain.ToUTC().GetHour() == 13 );
    plain.ClearTimeZone();
    CHECK( ! plain.HasTimeZone() && plain.ToUTC().GetHour() == 12 );

        // The older interface (minutes).
    CDate legacy( 2, 5, 2001, 12, 0, 0 );
    legacy.SetToGMT( -300 );
    CHECK( legacy.IsGMT() && legacy.GetHour() == 17 );
    legacy.TZAdjust( 60 );
    CHECK( legacy.GetUTCOffset() == 3600 && legacy.GetHour() == 18 );
    legacy.TZAdjust( 60 );
    CHECK( legacy.IsGMT() && legacy.GetHour() == 17 );
    CDate gmt = CDate::ParseGMT( "2001-06-02 12:00:00" );
    CHECK( gmt.IsGMT() && gmt.GetHour() == 12 );
    gmt = CDate::ParseGMT( "06/02/2001 12:00 EST" );
    CHECK( gmt.IsGMT() && gmt.GetHour() == 17 );

    printf( "System time zone (America/Denver, Europe/London)\n" );
    setZone( "America/Denver" );

    CDate summer( 2, 5, 2001, 12, 0, 0 );
    CHECK( summer.AssumeLocalTimeZone() && summer.GetUTCOffset() == -21600 && summer.GetHour() == 12 );
    CDate winter( 2, 0, 2001, 12, 0, 0 );
    CHECK( winter.AssumeLocalTimeZone() && winter.GetUTCOffset() == -25200 );
    CHECK_EQ_STR( "to local", iso( CDate::ParseUnix( 991483651 ).ToLocalTime(), false ), "2001-06-02T06:07:31-06:00" );
    CHECK( CDate::ParseUnix( 991483651 ).ToLocalTime() == CDate::ParseUnix( 991483651 ) );

        // 1 April 2001 02:30 never happened (clocks went from 02:00 to 03:00).
    CDate gap( 1, 3, 2001, 2, 30, 0 );
    CHECK( gap.AssumeLocalTimeZone() );
    CHECK_EQ_STR( "gap", iso( gap, false ), "2001-04-01T03:30:00-06:00" );
        // 28 October 2001 01:30 happened twice; the first is in daylight time.
    CDate overlap( 28, 9, 2001, 1, 30, 0 );
    CHECK( overlap.AssumeLocalTimeZone() );
    CHECK_EQ_STR( "overlap", iso( overlap, false ), "2001-10-28T01:30:00-06:00" );
        // Before standard time (1883) Denver kept local mean time.
    CDate lmt( 4, 6, 1850, 12, 0, 0 );
    CHECK( lmt.AssumeLocalTimeZone() && lmt.GetUTCOffset() == -25196 );
    CHECK_EQ_STR( "lmt", iso( lmt, false ), "1850-07-04T12:00:00-06:59:56" );
    CHECK( CDate::ParseISO8601( "1850-07-04T12:00:00-06:59:56" ) == lmt );

    setZone( "Europe/London" );
    CDate london( 2, 5, 2001, 12, 0, 0 );
    CHECK( london.AssumeLocalTimeZone() && london.GetUTCOffset() == 3600 );
    CHECK( near( london.GetJulianDate(), CDate::Parse( "2001-06-02T11:00:00Z" ).GetJulianDate(), ms ) );

    setZone( "UTC" );
}

    // Dates without a zone use the system's zone when an instant is needed.
static void testUnknownZoneDefault( void )
{
    printf( "Unknown zones default to the system zone (America/Denver)\n" );
    const double ms = 1.0 / 86400000.0;
    CHECK( CDate::GetUnknownTimeZoneLocal() );
    setZone( "America/Denver" );

    CDate noon( 1, 0, 2000, 12, 0, 0 );                 // 12:00 MST = 19:00 UT
    CHECK( ! noon.HasTimeZone() && noon.GetUTCOffset() == -25200 && noon.GetHour() == 12 );
    CHECK( near( noon.GetJulianDate(), 2451545.0 + 7.0 / 24.0, ms ) );
    CHECK( noon.GetJulianDayNumber() == 2451545 );
    CHECK_EQ_STR( "utc", iso( noon.ToUTC(), false ), "2000-01-01T19:00:00Z" );
    CHECK_EQ_STR( "rfc1123", std::string( (const char *)noon.FormatDate( CDate::DF_RFC1123 ) ), "Sat, 01 Jan 2000 19:00:00 GMT" );
    CHECK_EQ_STR( "iso", std::string( (const char *)noon.FormatISO8601() ), "2000-01-01T12:00:00" );     // local time, no designator
    CHECK( noon == CDate::Parse( "2000-01-01T19:00:00Z" ) );
    CHECK( CDate::Parse( "2000-01-01 12:00:00" ) == noon );

        // Daylight saving applies: summer is -06:00, and the day the clocks went
        // forward (1 April 2001) was 23 hours long.
    CHECK( CDate( 2, 5, 2001, 12, 0, 0 ).GetUTCOffset() == -21600 );
    CHECK( CDate( 2, 3, 2001 ).Elapsed( CDate( 1, 3, 2001 ) ) == 23LL * 3600000LL );
    CHECK( CDate( 2, 3, 2001 ).ElapsedDays( CDate( 1, 3, 2001 ) ) == 1 );

        // A stored zone wins over the default.
    CDate utc( 1, 0, 2000, 12, 0, 0 );
    utc.SetTimeZone( 0 );
    CHECK( near( utc.GetJulianDate(), 2451545.0, ms ) );

        // UTC instead of the system zone.
    CDate::SetUnknownTimeZoneLocal( false );
    CHECK( ! CDate::GetUnknownTimeZoneLocal() );
    CHECK( noon.GetUTCOffset() == 0 && near( noon.GetJulianDate(), 2451545.0, ms ) );
    CDate::SetUnknownTimeZoneLocal( true );

        // With the system zone set to UTC the default changes nothing.
    setZone( "UTC" );
    CHECK( noon.GetUTCOffset() == 0 && near( noon.GetJulianDate(), 2451545.0, ms ) );
    printf( "\"Today\" and missing date parts use the local date\n" );
    bool bDiffersFromUtc = false;
    const char *zones[] = { "Etc/GMT-14", "Etc/GMT+12", "America/Denver", "Asia/Tokyo" };   // UTC+14, UTC-12, ...
    for ( size_t z = 0; z < sizeof( zones ) / sizeof( zones[0] ); z++ )
    {
        setZone( zones[ z ] );
        time_t t = time( NULL );
        struct tm local, utc;
        localtime_r( &t, &local );
        gmtime_r( &t, &utc );
        CDate localDate( local.tm_mday, local.tm_mon, local.tm_year + 1900 );
        CDate utcDate( utc.tm_mday, utc.tm_mon, utc.tm_year + 1900 );
        if ( localDate.GetEpochDay() != utcDate.GetEpochDay() )
            bDiffersFromUtc = true;

        CDate today = CDate::Parse( "Today" );
        CHECK_EQ_STR( zones[ z ], hist( today ), hist( localDate ) );
        CHECK( ! today.HasTimeZone() && today.GetSOD() == 0 );
        CHECK( CDate::Parse( "Start of day" ) == today );
        CHECK( CDate::Parse( "Today + 1" ).GetEpochDay() == today.GetEpochDay() + 1 );
        long long sinceMidnight = CDate().Elapsed( today );
        CHECK( sinceMidnight >= 0 && sinceMidnight < 25LL * 3600000LL );
        CDate timeOnly = CDate::Parse( "10:05" );
        CHECK( timeOnly.GetEpochDay() == localDate.GetEpochDay() && timeOnly.GetHour() == 10 );
        CHECK( CDate::Parse( "1-17" ).GetYear() == local.tm_year + 1900 );

        CDate::SetUnknownTimeZoneLocal( false );
        CDate utcToday = CDate::Parse( "Today" );
        CHECK_EQ_STR( zones[ z ], hist( utcToday ), hist( utcDate ) );
        CHECK( utcToday.IsGMT() );
        CDate::SetUnknownTimeZoneLocal( true );
    }
        // At any moment UTC+14 or UTC-12 is on a different date from UTC, so
        // the checks above really did tell the local date from the UTC one.
    CHECK( bDiffersFromUtc );
    setZone( "UTC" );
}

static void testDefaultTimezone( void )
{
    printf( "SetDefaultTimezone\n" );
    const double ms = 1.0 / 86400000.0;
    setZone( "America/Denver" );
    CHECK( CDate::IsDefaultTimezoneSystem() && CDate::GetDefaultTimezoneOffset() == 0 );

        // A fixed offset applies to dates without a zone.
    CHECK( CDate::SetDefaultTimezone( 19800 ) );                       // UTC+5:30
    CHECK( ! CDate::IsDefaultTimezoneSystem() && CDate::GetDefaultTimezoneOffset() == 19800 );
    CDate india( 1, 0, 2000, 17, 30, 0 );
    CHECK( ! india.HasTimeZone() && india.GetUTCOffset() == 19800 );
    CHECK( near( india.GetJulianDate(), 2451545.0, ms ) );
    CHECK_EQ_STR( "india utc", iso( india.ToUTC(), false ), "2000-01-01T12:00:00Z" );
    CHECK( india == CDate::Parse( "2000-01-01T12:00:00Z" ) );
        // Stored zones are not affected.
    CHECK( CDate::Parse( "2000-01-01T12:00:00-05:00" ).GetUTCOffset() == -18000 );

        // "Today" and parser defaults follow the default zone.
    CDate expected = CDate().ToTimeZone( 19800 );
    CDate today = CDate::Parse( "Today" );
    CHECK( today.GetEpochDay() == expected.GetEpochDay() && today.GetSOD() == 0 && today.GetUTCOffset() == 19800 );
    CHECK( CDate::Parse( "10:05" ).GetEpochDay() == expected.GetEpochDay() );

        // Negative and half-hour offsets.
    CHECK( CDate::SetDefaultTimezone( -12600 ) );                      // Newfoundland standard time
    CHECK( CDate( 1, 0, 2000, 8, 30, 0 ).ToUTC().GetHour() == 12 );

        // Zone text.
    struct { const char *text; int offset; } texts[] = {
        { "UTC", 0 }, { "Z", 0 }, { "GMT", 0 }, { "+05:30", 19800 }, { "-07:00", -25200 }, { "+0100", 3600 },
        { "-0330", -12600 }, { "EST", -18000 }, { "pdt", -25200 }, { "CEST", 7200 }, { "GMT-1", -3600 }, { "UTC+9", 32400 },
    };
    for ( size_t n = 0; n < sizeof( texts ) / sizeof( texts[0] ); n++ )
    {
        bool bSet = CDate::SetDefaultTimezone( texts[ n ].text );
        CHECK_EQ_STR( texts[ n ].text, ( bSet && ! CDate::IsDefaultTimezoneSystem() ) ? std::to_string( CDate::GetDefaultTimezoneOffset() ) : "not set",
                      std::to_string( texts[ n ].offset ) );
    }

        // Bad input leaves the default as it was.
    CHECK( CDate::SetDefaultTimezone( "+02:00" ) );
    const char *bad[] = { "", "Mars/Olympus_Mons", "+25:00", "12:00", "am" };
    for ( size_t n = 0; n < sizeof( bad ) / sizeof( bad[0] ); n++ )
    {
        CHECK_EQ_STR( bad[ n ], CDate::SetDefaultTimezone( bad[ n ] ) ? "accepted" : "rejected", "rejected" );
    }
    CHECK( ! CDate::SetDefaultTimezone( 86400 ) && ! CDate::SetDefaultTimezone( -90000 ) );
    CHECK( CDate::GetDefaultTimezoneOffset() == 7200 );

        // Back to the system zone (America/Denver here), in each way.
    CHECK( CDate::SetDefaultTimezone( "system" ) && CDate::IsDefaultTimezoneSystem() );
    CHECK( CDate( 2, 5, 2001, 12, 0, 0 ).GetUTCOffset() == -21600 );
    CHECK( CDate::SetDefaultTimezone( "UTC" ) && CDate( 2, 5, 2001, 12, 0, 0 ).GetUTCOffset() == 0 );
    CHECK( CDate::SetDefaultTimezone( "local" ) && CDate::IsDefaultTimezoneSystem() );
    CDate::SetUnknownTimeZoneLocal( false );
    CHECK( ! CDate::IsDefaultTimezoneSystem() && CDate::GetDefaultTimezoneOffset() == 0 );
    CDate::SetDefaultTimezoneToSystem();
    CHECK( CDate::IsDefaultTimezoneSystem() && CDate::GetUnknownTimeZoneLocal() );

    setZone( "UTC" );
}

    // The system's offset at a Unix time, for checking CTimeZone against it.
static bool systemOffset( long long t, int &offset )
{
    time_t tt = (time_t)t;
    struct tm local;
    if ( localtime_r( &tt, &local ) == NULL )
        return false;
    offset = (int)( (long long)timegm( &local ) - t );
    return true;
}

static void testNamedZones( void )
{
    printf( "Named time zones\n" );
    if ( ! CTimeZone::Find( "America/New_York" ) )
    {
        printf( "  (no zone database; skipped)\n" );
        return;
    }
    setZone( "UTC" );

        // Offsets at instants, from transitions and (after the last one) the rule.
    std::shared_ptr<const CTimeZone> ny = CTimeZone::Find( "America/New_York" );
    CHECK( ny->GetName() == "America/New_York" );
    CHECK( ny == CTimeZone::Find( "America/New_York" ) );              // loaded once
    CHECK( ny->OffsetAt( 979560000LL ) == -18000 );                     // 2001-01-15 12:00Z
    CHECK( ny->OffsetAt( 995198400LL ) == -14400 );                     // 2001-07-15 12:00Z
    CHECK( ny->OffsetAt( 986108399LL ) == -18000 );                     // 2001-04-01 06:59:59Z
    CHECK( ny->OffsetAt( 986108400LL ) == -14400 );                     // 2001-04-01 07:00Z, clocks forward
    CHECK( ny->OffsetAt( 1004248799LL ) == -14400 );                    // 2001-10-28 05:59:59Z
    CHECK( ny->OffsetAt( 1004248800LL ) == -18000 );                    // 2001-10-28 06:00Z, clocks back
    CHECK( ny->OffsetAt( -5364662400LL ) == -17762 );                   // 1800: local mean time
    CHECK( ny->OffsetAt( 4118400000LL ) == -14400 );                    // 2100-07-04: the rule
    CHECK( ny->OffsetAt( 4105166400LL ) == -18000 );                    // 2100-01-31
    std::shared_ptr<const CTimeZone> sydney = CTimeZone::Find( "Australia/Sydney" );
    CHECK( sydney && sydney->OffsetAt( 4103179200LL ) == 39600 );      // 2100-01-08, summer
    CHECK( sydney && sydney->OffsetAt( 4118400000LL ) == 36000 );      // 2100-07-04, winter
    std::shared_ptr<const CTimeZone> kolkata = CTimeZone::Find( "Asia/Kolkata" );
    CHECK( kolkata && kolkata->OffsetAt( 946728000LL ) == 19800 );

        // Local times: repeated and skipped hours.
    int offset = 0;
    bool gap = false;
    ny->OffsetForLocal( 986092200LL, offset, gap );                     // 2001-04-01 02:30 never happened
    CHECK( offset == -18000 && gap );
    ny->OffsetForLocal( 1004232600LL, offset, gap );                    // 2001-10-28 01:30 happened twice
    CHECK( offset == -14400 && ! gap );
    ny->OffsetForLocal( 1004236200LL, offset, gap );                    // 02:30 after the change
    CHECK( offset == -18000 && ! gap );

        // Against the system's own reading of the same zone files, over many
        // instants from 1900 to 2100 and every hour of 2045 (rule years).
    const char *zones[] = { "America/New_York", "Europe/London", "Australia/Sydney", "Asia/Kolkata",
                            "America/Sao_Paulo", "Pacific/Chatham", "Africa/Casablanca", "Europe/Dublin",
                            "America/St_Johns", "Pacific/Apia", "America/Nuuk", "Asia/Jerusalem",
                            "Antarctica/Troll", "Australia/Lord_Howe", "America/Santiago", "Asia/Tehran" };
    std::mt19937_64 random( 20261117 );
    std::uniform_int_distribution<long long> instants( -2208988800LL, 4102444800LL );
    for ( size_t z = 0; z < sizeof( zones ) / sizeof( zones[0] ); z++ )
    {
        std::shared_ptr<const CTimeZone> zone = CTimeZone::Find( zones[ z ] );
        if ( ! zone )
        {
            printf( "  (%s not installed)\n", zones[ z ] );
            continue;
        }
        setZone( zones[ z ] );
        int mismatches = 0;
        long long first = 0;
        std::vector<long long> times;
        for ( int n = 0; n < 4000; n++ )
            times.push_back( instants( random ) );
        for ( long long t = 2366755200LL; t < 2398291200LL; t += 3600 )  // 2045
            times.push_back( t );
        for ( size_t n = 0; n < times.size(); n++ )
        {
            int expected = 0;
            if ( systemOffset( times[ n ], expected ) && expected != zone->OffsetAt( times[ n ] ) )
            {
                if ( mismatches++ == 0 )
                    first = times[ n ];
            }
        }
        if ( mismatches )
            printf( "  %s: %d differ from the system, first at %lld\n", zones[ z ], mismatches, first );
        CHECK( mismatches == 0 );

            // Local-time resolution agrees with AssumeLocalTimeZone in that zone.
        int differ = 0;
        for ( int n = 0; n < 300; n++ )
        {
            CDate local( (long)( instants( random ) / 86400 ) + 719164L, (long)( instants( random ) % 86400000LL + 86400000LL ) % 86400000L );
            CDate viaSystem = local;
            CDate viaZone = local;
            if ( viaSystem.AssumeLocalTimeZone() && ( ! viaZone.AssumeTimeZone( zones[ z ] ) ||
                 iso( viaSystem ) != iso( viaZone ) ) )
                differ++;
        }
        CHECK( differ == 0 );
    }
    setZone( "UTC" );

        // A named default zone follows daylight saving, whatever TZ says.
    CHECK( CDate::SetDefaultTimezone( "America/New_York" ) );
    CHECK( ! CDate::IsDefaultTimezoneSystem() && CDate::GetDefaultTimezoneOffset() == 0 );
    CHECK_EQ_STR( "name", (const char *)CDate::GetDefaultTimezoneName(), "America/New_York" );
    CHECK( CDate( 15, 0, 2001, 12, 0, 0 ).GetUTCOffset() == -18000 );
    CHECK( CDate( 15, 6, 2001, 12, 0, 0 ).GetUTCOffset() == -14400 );
    CHECK( CDate( 1, 3, 2001, 2, 30, 0 ).GetUTCOffset() == -18000 );   // skipped: pre-change offset
    CHECK( CDate( 28, 9, 2001, 1, 30, 0 ).GetUTCOffset() == -14400 );  // repeated: the earlier one
    CHECK_EQ_STR( "ny utc", iso( CDate( 15, 6, 2001, 12, 0, 0 ).ToUTC(), false ), "2001-07-15T16:00:00Z" );
    CHECK( CDate( 15, 6, 2001, 12, 0, 0 ) == CDate::Parse( "2001-07-15T16:00:00Z" ) );
    CHECK( CDate::Parse( "2001-07-15T12:00:00" ).GetUTCOffset() == -14400 );
    CDate expected = CDate().ToTimeZone( "America/New_York" );
    CDate today = CDate::Parse( "Today" );
    CHECK( today.GetEpochDay() == expected.GetEpochDay() && today.GetSOD() == 0 && ! today.HasTimeZone() );

        // Abbreviations and offsets keep their fixed meaning; names are exact.
    CHECK( CDate::SetDefaultTimezone( "EST" ) && CDate::GetDefaultTimezoneOffset() == -18000 );
    CHECK_EQ_STR( "est name", (const char *)CDate::GetDefaultTimezoneName(), "" );
    CHECK( CDate( 15, 6, 2001, 12, 0, 0 ).GetUTCOffset() == -18000 );
    CHECK( CDate::SetDefaultTimezone( "CET" ) && CDate( 15, 6, 2001, 12, 0, 0 ).GetUTCOffset() == 3600 );
    CHECK( CDate::SetDefaultTimezone( "  Europe/Paris  " ) && CDate( 15, 6, 2001, 12, 0, 0 ).GetUTCOffset() == 7200 );
    const char *bad[] = { "america/new_york", "Mars/Base", "../etc/passwd", "/etc/passwd", "America//New_York",
                          "America/../Europe/Paris", "America/New York", "America", "America/New_York/" };
    for ( size_t n = 0; n < sizeof( bad ) / sizeof( bad[0] ); n++ )
    {
        CHECK_EQ_STR( bad[ n ], CDate::SetDefaultTimezone( bad[ n ] ) ? "accepted" : "rejected", "rejected" );
    }
    CHECK_EQ_STR( "unchanged", (const char *)CDate::GetDefaultTimezoneName(), "Europe/Paris" );
    CDate::SetDefaultTimezoneToSystem();
    CHECK( CDate::IsDefaultTimezoneSystem() && CDate::GetDefaultTimezoneName() == "" );

        // Converting to, and assuming, a named zone.
    CDate noon = CDate::Parse( "2000-01-01T12:00:00Z" );
    CDate india = noon.ToTimeZone( "Asia/Kolkata" );
    CHECK_EQ_STR( "kolkata", iso( india, false ), "2000-01-01T17:30:00+05:30" );
    CHECK( india == noon );
    CHECK_EQ_STR( "paris winter", iso( noon.ToTimeZone( "Europe/Paris" ), false ), "2000-01-01T13:00:00+01:00" );
    CHECK_EQ_STR( "paris summer", iso( CDate::Parse( "2000-07-01T12:00:00Z" ).ToTimeZone( "Europe/Paris" ), false ), "2000-07-01T14:00:00+02:00" );
    CHECK_EQ_STR( "fixed text", iso( noon.ToTimeZone( "+02:00" ), false ), "2000-01-01T14:00:00+02:00" );
    CHECK( ! noon.ToTimeZone( "Mars/Base" ).IsValid() );

    CDate paris( 1, 6, 2021, 12, 0, 0 );
    CHECK( paris.AssumeTimeZone( "Europe/Paris" ) && paris.GetUTCOffset() == 7200 && paris.GetHour() == 12 );
    CDate skipped( 28, 2, 2021, 2, 30, 0 );                             // Paris skipped 02:00-03:00
    CHECK( skipped.AssumeTimeZone( "Europe/Paris" ) );
    CHECK_EQ_STR( "paris gap", iso( skipped, false ), "2021-03-28T03:30:00+02:00" );
    CDate repeated( 31, 9, 2021, 2, 30, 0 );                            // and repeated 02:00-03:00
    CHECK( repeated.AssumeTimeZone( "Europe/Paris" ) );
    CHECK_EQ_STR( "paris overlap", iso( repeated, false ), "2021-10-31T02:30:00+02:00" );
        // The system zone resolves the repeated hour the same way (east of UTC,
        // where the summer offset is not found from the plain UTC reading).
    setZone( "Europe/Paris" );
    CDate systemRepeated( 31, 9, 2021, 2, 30, 0 );
    CHECK( systemRepeated.GetUTCOffset() == 7200 );
    CHECK( systemRepeated.AssumeLocalTimeZone() && systemRepeated.GetUTCOffset() == 7200 );
    setZone( "UTC" );

    CDate fixed( 1, 6, 2021, 12, 0, 0 );
    CHECK( fixed.AssumeTimeZone( "EST" ) && fixed.GetUTCOffset() == -18000 );
    CDate unknown( 1, 6, 2021, 12, 0, 0 );
    CHECK( ! unknown.AssumeTimeZone( "Mars/Base" ) && ! unknown.HasTimeZone() );

        // Julian Day of a local time in a named zone.
    CDate nyNoon( 1, 0, 2000, 7, 0, 0 );
    CHECK( nyNoon.AssumeTimeZone( "America/New_York" ) && near( nyNoon.GetJulianDate(), 2451545.0, 1.0 / 86400000.0 ) );
}

int main( int argc, char *argv[] )
{
        // Dates without a zone take the system zone, so run in UTC unless a
        // test sets another zone.
    const char *pszTestZone = getenv( "TZ" );
    std::string savedTestZone = pszTestZone ? pszTestZone : "";
    setZone( "UTC" );

#ifdef IASLIB_HERSCHEL_CORRECTION__
    CHECK( CDate::GetHerschelCorrection() );
    CDate::SetHerschelCorrection( false );
#else
    CHECK( ! CDate::GetHerschelCorrection() );
#endif

    testCalendarSweep();
    testCalendarFacts();
    testHerschel();
    testParsing();
    testISO8601();
    testUnixAndTimestamps();
    testJulianDay();
    testTimeZones();
    testUnknownZoneDefault();
    testDefaultTimezone();
    testNamedZones();

    setZone( savedTestZone.empty() ? NULL : savedTestZone.c_str() );

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
