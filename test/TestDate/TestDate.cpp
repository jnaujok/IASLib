/**
 * CDate tests
 *
 * Checks CDate's Gregorian arithmetic from 1583 to 9999 against Julian Day
 * Numbers, with and without the Herschel 4000-year correction.
 */

#include "IASLib.h"

#include <cstdio>

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

static bool isLeap( long y, bool herschel )
{
    if ( herschel && y % 4000 == 0 )
        return false;
    return ( y % 4 == 0 && y % 100 != 0 ) || y % 400 == 0;
}

    // Days since a fixed origin for the Gregorian calendar with the chosen
    // rule. With herschel off this differs from the Julian Day Number only by
    // a constant.
static long dayNumber( long y, long m, long d, bool herschel )
{
    long a = ( 14 - m ) / 12, y2 = y + 4800 - a, m2 = m + 12 * a - 3;
    long jdn = d + ( 153 * m2 + 2 ) / 5 + 365 * y2 + y2 / 4 - y2 / 100 + y2 / 400 - 32045;
    if ( herschel )
    {
            // One fewer day for each year divisible by 4000 whose 29 February
            // has passed.
        long dropped = y / 4000;
        if ( y % 4000 == 0 && m <= 2 )
            dropped--;
        jdn -= dropped;
    }
    return jdn;
}

    // Every day from 1 Jan 1583 to 31 Dec 9999: consecutive day counts,
    // round trips through day/month/year, and weekdays.
static void sweep( bool herschel )
{
    static const int monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    long errors = 0;
    long days = 0;
    long prevNumber = 0;
    long prevEpoch = 0;
    bool first = true;

    for ( long y = 1583; y <= 9999; y++ )
    {
        for ( long m = 1; m <= 12; m++ )
        {
            int nd = ( m == 2 && isLeap( y, herschel ) ) ? 29 : monthDays[ m - 1 ];
            for ( long d = 1; d <= nd; d++ )
            {
                long number = dayNumber( y, m, d, herschel );
                CDate date( (int)d, (int)( m - 1 ), (int)y );
                bool ok = ( date.GetYear() == y && date.GetMonth() == m - 1 && date.GetDay() == d );
                if ( ! first )
                    ok = ok && ( date.GetEpochDay() - prevEpoch == number - prevNumber );
                    // Weekdays follow the real (rule-less) day count only while
                    // both rules agree, i.e. before the first dropped day.
                if ( ! herschel || y < 4000 || ( y == 4000 && m <= 2 ) )
                    ok = ok && ( date.GetDOW() == (int)( ( number + 1 ) % 7 ) );
                if ( ! ok )
                {
                    if ( errors < 5 )
                        printf( "    %s: %04ld-%02ld-%02ld came back as %04d-%02d-%02d (day %ld, weekday %d)\n",
                                herschel ? "Herschel" : "Gregorian", y, m, d, date.GetYear(), date.GetMonth() + 1,
                                date.GetDay(), date.GetEpochDay(), date.GetDOW() );
                    errors++;
                }
                prevNumber = number;
                prevEpoch = date.GetEpochDay();
                first = false;
                days++;
            }
        }
    }
    printf( "  %s: %ld days checked\n", herschel ? "With the Herschel correction" : "Standard Gregorian", days );
    CHECK( errors == 0 );
}

static void testStandardGregorian( void )
{
    printf( "Standard Gregorian (default)\n" );
    CDate::SetHerschelCorrection( false );
    CHECK( ! CDate::GetHerschelCorrection() );

    CDate probe( 1, 0, 2000 );
    CHECK( ! probe.IsLeapYear( 1900 ) );
    CHECK( probe.IsLeapYear( 2000 ) );
    CHECK( ! probe.IsLeapYear( 2100 ) );
    CHECK( probe.IsLeapYear( 4000 ) );
    CHECK( probe.IsLeapYear( 8000 ) );

    CDate leapDay( 29, 1, 4000 );
    CHECK( leapDay.GetYear() == 4000 && leapDay.GetMonth() == 1 && leapDay.GetDay() == 29 );
    CHECK( leapDay.GetDOW() == 2 );                                   // Tuesday
    CDate feb28( 28, 1, 4000 ), mar1( 1, 2, 4000 );
    CHECK( mar1.GetEpochDay() - feb28.GetEpochDay() == 2 );
    CHECK( CDate( 31, 11, 9999 ).GetDOW() == 5 );                     // Friday
    CHECK( CDate( 1, 0, 1970 ).GetDOW() == 4 );                       // Thursday

    sweep( false );
}

static void testHerschel( void )
{
    printf( "Herschel correction\n" );
    CDate before( 1, 2, 4000 );                                       // 1 Mar 4000, standard rules
    long standardEpoch = before.GetEpochDay();

    CDate::SetHerschelCorrection( true );
    CHECK( CDate::GetHerschelCorrection() );

    CDate probe( 1, 0, 2000 );
    CHECK( probe.IsLeapYear( 2000 ) );
    CHECK( ! probe.IsLeapYear( 2100 ) );
    CHECK( ! probe.IsLeapYear( 4000 ) );
    CHECK( ! probe.IsLeapYear( 8000 ) );
    CHECK( probe.IsLeapYear( 4400 ) );

    CDate feb28( 28, 1, 4000 ), mar1( 1, 2, 4000 );
    CHECK( mar1.GetEpochDay() - feb28.GetEpochDay() == 1 );
    CHECK( mar1.GetEpochDay() == standardEpoch - 1 );                  // one day fewer from here on
    CHECK( CDate( 31, 11, 9999 ).GetDOW() == 3 );                     // Wednesday: two days dropped
    CHECK( CDate( 1, 0, 1970 ).GetDOW() == 4 );                       // earlier dates unchanged

    sweep( true );

        // Switching back restores the standard rules.
    CDate::SetHerschelCorrection( false );
    CHECK( CDate( 1, 2, 4000 ).GetEpochDay() == standardEpoch );
    CHECK( CDate( 29, 1, 4000 ).GetDay() == 29 );
}

int main( int argc, char *argv[] )
{
#ifdef IASLIB_HERSCHEL_CORRECTION__
    CHECK( CDate::GetHerschelCorrection() );
#else
    CHECK( ! CDate::GetHerschelCorrection() );
#endif

    testStandardGregorian();
    testHerschel();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
