/**
 * @file Date.h
 * @brief CDate, a date and time with an optional time zone, and its
 * format and database constants.
 *
 * Date
 *
 *	This class encapsulates the functionality of a date/time combo.
 * It also includes the most rediculously robust date parsing routine
 * ever written.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/01/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_DATE_H__
#define IASLIB_DATE_H__

#include "Object.h"
#include "String_.h"
#ifdef IASLIB_DEFINE_BOOL__
#include "Bool.h"
#endif

#include <ctime>
#include <memory>

namespace IASLib
{
    class CTimeZone;

    /// The parts of a date and time, like the C library's struct tm (with
    /// milliseconds and a UTC offset added). How tm_year and tm_mon are read
    /// depends on the call: see CDate( tm_type & ) and SetDate( tm_type & ).
    typedef struct My_tm
    {
        int     tm_msec;        ///< Milliseconds, 0-999.
        int     tm_sec;         ///< Seconds, 0-59.
        int     tm_min;         ///< Minutes, 0-59.
        int     tm_hour;        ///< Hours, 0-23.
        int     tm_mday;        ///< Day of the month, 1-31.
        int     tm_mon;         ///< Month, 0-11.
        int     tm_year;        ///< Year (the full year, or years since 1900; see above).
        int     tm_wday;        ///< Day of the week, 0 (Sunday) to 6.
        int     tm_yday;        ///< Day of the year.
        int     tm_isdst;       ///< Daylight saving flag, as in struct tm.
        int     tm_gmtoffset;   ///< Offset from UTC in seconds east of UTC (0: none).
    } tm_type;

    /// A word and the number it stands for, in the date parser's tables.
    typedef struct My_Token
    {
        const char *strData;    ///< The word, in lower case.
        long        lToken;     ///< Its value.
    } Token_Type;


    #define AMPM_NONE 0     ///< The parser saw no AM or PM.
    #define AMPM_AM 1       ///< The parser saw AM.
    #define AMPM_PM 2       ///< The parser saw PM.

    /*
    **  DATE FORMATS
    **
    **  PRETTY   - 06/02/2001  12:14:11
    **  UNIX     - Mon Jun  2 12:14:11 2001
    **  RFC850   - Monday, 02-Jun-01 12:14:11 GMT
    **  RFC1123  - Mon, 02 Jun 2001 12:14:11 GMT
    **  NETSCAPE - 02/Jun/2001:12:14:11
    **  EXACT    - 06/02/2001  12:14:11.374
    **  SYBASE   - Jun 02 2001 12:14 PM
    **  SQLITE   - 2001/06/02 12:14:11
    **  ORACLE   - 02-Jun-2001, 12:14:11 PM
    **  ISO_8601_FULL    - 2001-06-02T12:14:11Z  (Z only for dates known to be UTC)
    **  ISO_8601_FULL_MS - 2001-06-02T12:14:11.374Z
    **  ISO_8601_DATE    - 2001-06-02
    **
    **  The ISO 8601 based formats (ISO_8601*, YYYYMMDD) use the proleptic
    **  Gregorian calendar with astronomical years, as ISO 8601 requires; the
    **  others use the historical calendar (Julian before 15 October 1582).
    **
    */

    /// @name Date format numbers
    /// The numbers of the CDate::FormatFlags values, for code that predates
    /// the enumeration. ("ISO_9601" is ISO 8601.)
    ///@{
    #define DATE_FORMAT_PRETTY      1   ///< CDate::DF_PRETTY.
    #define DATE_FORMAT_UNIX        2   ///< CDate::DF_UNIX.
    #define DATE_FORMAT_RFC850      3   ///< CDate::DF_RFC850.
    #define DATE_FORMAT_RFC1123     4   ///< CDate::DF_RFC1123.
    #define DATE_FORMAT_NETSCAPE    5   ///< CDate::DF_NETSCAPE.
    #define DATE_FORMAT_EXACT       6   ///< CDate::DF_EXACT.
    #define DATE_FORMAT_SYBASE      7   ///< CDate::DF_SYBASE.
    #define DATE_FORMAT_SQLITE		8   ///< CDate::DF_SQLITE.
    #define DATE_FORMAT_ORACLE      9   ///< CDate::DF_ORACLE.
    #define DATE_FORMAT_YYYYMMDD    10  ///< CDate::DF_YYYYMMDD.
    #define DATE_FORMAT_ISO_9601    11  ///< CDate::DF_ISO_8601.
    #define DATE_FORMAT_ISO_9601_MS 12  ///< CDate::DF_ISO_8601_MS.
    #define DATE_FORMAT_HHMMSS      13  ///< CDate::DF_HHMMSS.
    #define DATE_FORMAT_ISO_9601_MS_PACKED 14   ///< CDate::DF_ISO_8601_MS_PACKED.
    #define DATE_FORMAT_RFC7231     15  ///< CDate::DF_RFC7231.
    #define DATE_FORMAT_ISO_8601_FULL    16     ///< CDate::DF_ISO_8601_FULL.
    #define DATE_FORMAT_ISO_8601_FULL_MS 17     ///< CDate::DF_ISO_8601_FULL_MS.
    #define DATE_FORMAT_ISO_8601_DATE    18     ///< CDate::DF_ISO_8601_DATE.
    ///@}

    /// @name Database types
    /// The databases CDate::DBDate() formats dates for.
    ///@{
    #define DB_UNKNOWN				0   ///< Any other database: DF_PRETTY.
    #define DB_ORACLE				1   ///< Oracle: DF_EXACT.
    #define DB_SYBASE				2   ///< Sybase: DF_SYBASE.
    #define DB_SQLITE				3   ///< SQLite: DF_SQLITE.
    #define DB_MYSQL                4   ///< MySQL: DF_ISO_8601_MS.
    ///@}

    /**
     * A date and time, with an optional time zone.
     *
     * A date is a day number and the milliseconds since midnight, in local
     * (wall clock) time, plus its offset from UTC when that is known. Dates
     * use the historical calendar (Julian before 15 October 1582) with no
     * year zero, months 0-11; the ISO 8601 methods use the proleptic
     * Gregorian calendar. An invalid date (IsValid() false) is what parsing
     * gives for text that is not a date. A date is a value: copy it freely,
     * but do not change one object on two threads at once. The default time
     * zone settings are shared by every thread.
     *
     * @code
     * CDate dttNow;                                  // now, UTC
     * CDate dttDue( "Aug 14 1998 5:03 PM EST" );     // kept at -05:00
     * if ( dttDue.IsValid() && dttDue < dttNow )
     *     printf( "%s\n", dttDue.FormatISO8601().c_str() );   // 1998-08-14T17:03:00-05:00
     * CDate dttDay( 2, 5, 2001 );                    // 2 June 2001 (month 0-11)
     * @endcode
     */
    class CDate : public CObject
    {
        protected:
            static long         m_lOffsetSeconds;   ///< Seconds added to the system clock (see SynchronizeToTimeServer()).
            long                m_lEpochDay;        ///< The day number (719164 is 1 January 1970).
            long                m_lMilliseconds;    ///< Milliseconds since local midnight; -1 for an invalid date.
            bool                m_bHasZone;     ///< Whether the offset below is known.
            int                 m_nUTCOffset;   ///< Seconds east of UTC (UTC+5:30 is 19800).
            CString             m_castBuffer;   ///< Scratch text (not used by the library).
        public:
            /// The text formats FormatDate() writes (examples are for 2 June
            /// 2001, 12:14:11.374).
            enum FormatFlags {
                DF_UNDEFINED            = 0x0000,   ///< No format: "Unknown Format".
		        DF_PRETTY               = 0x0001,   ///< "06/02/2001  12:14:11" (MM/DD/YYYY).
                DF_UNIX                 = 0x0002,   ///< "Sat Jun  2 12:14:11 2001".
                DF_RFC850               = 0x0003,   ///< "Saturday, 02-Jun-01 12:14:11 GMT" (in UTC).
                DF_RFC1123              = 0x0004,   ///< "Sat, 02 Jun 2001 12:14:11 GMT" (in UTC).
                DF_NETSCAPE             = 0x0005,   ///< "02/Jun/2001:12:14:11".
                DF_EXACT                = 0x0006,   ///< "06/02/2001  12:14:11.374".
                DF_SYBASE               = 0x0007,   ///< "Jun 02 2001 12:14 PM".
                DF_SQLITE		        = 0x0008,   ///< "2001/06/02 12:14:11".
                DF_ORACLE               = 0x0009,   ///< "02-Jun-2001, 12:14:11 PM".
                DF_YYYYMMDD             = 0x000A,   ///< "20010602" (proleptic Gregorian).
                DF_ISO_8601             = 0x000B,   ///< "2001-06-02 12:14:11".
                DF_ISO_8601_MS          = 0x000C,   ///< "2001-06-02 12:14:11.374".
                DF_HHMMSS               = 0x000D,   ///< "121411".
                DF_ISO_8601_MS_PACKED   = 0x000E,   ///< "20010602121411.374".
                DF_RFC7231              = 0x000F,   ///< Same as DF_RFC1123 (the HTTP date).
                DF_ISO_8601_FULL        = 0x0010,   ///< FormatISO8601(): "2001-06-02T12:14:11Z".
                DF_ISO_8601_FULL_MS     = 0x0011,   ///< FormatISO8601() with milliseconds.
                DF_ISO_8601_DATE        = 0x0012,   ///< FormatISO8601() without the time: "2001-06-02".
		    };

                /// The current time (UTC).
                                CDate();
                /// A date in the historical calendar: nMonth is 0-11, years have no
                /// year zero (1 BC is -1). A date or time that does not exist (30
                /// February, 10 October 1582, hour 24) gives an invalid date. The
                /// date has no time zone.
                                CDate( int nDay, int nMonth, int nYear, int nHour = 0, int nMinute = 0, int nSecond = 0, int nMillisecond = 0 );
                /// A copy (time and zone).
                                CDate( const CDate &oSource );
                /// Parses text, as GetSafeDate() does (an invalid date if it is
                /// not a date).
                                CDate( const char *strSource );
                /// A date from its parts: tm_year is the full year and tm_mon is
                /// 0-11. A non-zero tm_gmtoffset becomes the date's time zone. All
                /// of year, month and day 0 with tm_msec -1 gives an invalid date.
                                CDate( tm_type &tmSource );
                /// A day number and milliseconds since midnight, with no time zone.
                                CDate( long lEpochDay, long lMilliseconds ) { m_lEpochDay = lEpochDay; m_lMilliseconds = lMilliseconds; m_bHasZone = false; m_nUTCOffset = 0; }
            virtual            ~CDate();

                                DEFINE_OBJECT( CDate )

                /// Copies the time and the zone.
            CDate              &operator =( const CDate &oSource );

                /// Whether this date is later than oCompare. Dates compare as
                /// instants, so their time zones may differ.
            bool                operator >( const CDate &oCompare ) const;
                /// Whether this date is earlier than oCompare.
            bool                operator <( const CDate &oCompare ) const;
                /// Whether this date is not earlier than oCompare.
            bool                operator >=( const CDate &oCompare ) const;
                /// Whether this date is not later than oCompare.
            bool                operator <=( const CDate &oCompare ) const;
                /// Whether both are the same instant.
            bool                operator ==( const CDate &oCompare ) const;
                /// Whether they are different instants.
            bool                operator !=( const CDate &oCompare ) const;

                /// Day of the month, 1-31.
            int                 GetDay( void ) const;
                /// Day of the week, 0 (Sunday) to 6 (Saturday).
            int                 GetDOW( void ) const { long lDow = ( m_lEpochDay + 6 ) % 7; return (int)( ( lDow < 0 ) ? lDow + 7 : lDow ); }
                /// Day of the month; same as GetDay().
            int                 GetDate( void ) const { return GetDay(); }
                /// Month, 0 (January) to 11.
            int                 GetMonth( void ) const;
                /// Year in the historical calendar (1 BC is -1).
            int                 GetYear( void ) const;
                /// The year; same as GetYear().
            int                 GetFullYear( void ) const { return GetYear(); }
                /// Hour, 0-23.
            int                 GetHours( void ) const;
                /// Minute, 0-59.
            int                 GetMinutes( void ) const;
                /// Second, 0-59.
            int                 GetSeconds( void ) const;
                /// Hour; same as GetHours().
            int                 GetHour( void ) const { return GetHours(); }
                /// Minute; same as GetMinutes().
            int                 GetMinute( void ) const { return GetMinutes(); }
                /// Second; same as GetSeconds().
            int                 GetSecond( void ) const { return GetSeconds(); }
                /// Sets the milliseconds of the current second (nMilliseconds % 1000).
            void                SetMillisecond( int nMilliseconds ) { m_lMilliseconds -= m_lMilliseconds % 1000; nMilliseconds %= 1000; m_lMilliseconds += nMilliseconds; }
                /// Millisecond, 0-999.
            int                 GetMillisecond( void ) const;
                /// The day number (719164 is 1 January 1970).
            long                GetEpochDay( void ) const { return m_lEpochDay; }

                /// Day of the year, 1 for 1 January.
            int                 GetDOY( void ) const;
                /// Seconds since midnight.
            int                 GetSOD( void ) const { return m_lMilliseconds / 1000; }

                /// Moves the time to midnight (does not check validity; see StartOfDay()).
            void                ClearTime( void ) { m_lMilliseconds = 0; }

                /// Whether the date is not the "no date" value (milliseconds -1).
            bool                IsDate( void ) const { return ( m_lMilliseconds != -1 ); }
                /// Whether the date is valid.
            bool                IsValid( void ) const { return ( m_lMilliseconds >= 0 ) ? true:false; }
                /// Whether nYear (historical calendar, no year zero) is a leap
                /// year. 1582, the reform year, is not.
            bool                IsLeapYear( int nYear ) const;
                /// True if the day, month (0-11) and year form a real date.
            static bool         IsValidDate( int nDay, int nMonth, int nYear );
                /// Whether the date has a time zone.
                ///
                /// Time zones. A date holds its local (wall clock) time and, when
                /// known, its offset from UTC in seconds east of UTC (-18000 for
                /// EST, 19800 for UTC+5:30). The day, month, hour and so on are
                /// always the local time. Anything that needs the instant uses the
                /// offset automatically: comparisons, Elapsed(), ToUTC(), the Julian
                /// Day methods and the formats that say "GMT". A date without a
                /// zone (from numbers, or text without a zone) is taken to be in
                /// the default time zone for those: the system's local time zone
                /// unless SetDefaultTimezone() chose another, using the offset in
                /// force at that local time (UTC if the system has none). Use
                /// SetTimeZone() for a date in some other zone.
                ///
                /// The current time, Unix times and Julian Days are UTC. Parsed text
                /// keeps its zone ("12:14 EST" is 12:14 at -05:00).
            bool                HasTimeZone( void ) const { return m_bHasZone; }
                /// The offset in seconds east of UTC used for conversions: the
                /// date's own, or for a date without a zone the default above.
            int                 GetUTCOffset( void ) const;
                /// Sets the default time zone to a fixed offset.
                ///
                /// The default time zone: the zone assumed for dates without one,
                /// and the zone of "Today" and of dates the parser fills in. It is
                /// the system's local time zone (with its daylight saving rules)
                /// unless set to a fixed offset, in seconds east of UTC, or to zone
                /// text the parser reads: "UTC", "Z", "+05:30", "-0700", "EST",
                /// "GMT-1", or to a named IANA zone such as "America/New_York",
                /// which follows that zone's daylight saving rules (read from the
                /// system's zone database). Abbreviations and offsets are tried
                /// before names, so "EST" is always UTC-5; names are case-sensitive.
                /// "system" or "local" goes back to the system zone. Returns
                /// false, leaving the default unchanged, for text it cannot read or
                /// an offset of a day or more. It applies to every date (including
                /// existing ones without a zone), so set it at startup.
                /// @param nOffsetSeconds Seconds east of UTC.
                /// @return false for an offset of a day or more.
            static bool         SetDefaultTimezone( int nOffsetSeconds );
                /// Sets the default time zone from zone text (see above).
                /// @param pszZone "UTC", "+05:30", "EST", "America/New_York",
                ///     "system" and the like.
                /// @return false, leaving the default unchanged, for text it cannot read.
            static bool         SetDefaultTimezone( const char *pszZone );
                /// Makes the system's local time zone the default again.
            static void         SetDefaultTimezoneToSystem( void );
                /// Whether the default time zone is the system's.
            static bool         IsDefaultTimezoneSystem( void );
                /// The fixed default offset (0 when the default is the system zone
                /// or a named zone).
            static int          GetDefaultTimezoneOffset( void );
                /// The named default zone ("America/New_York"), or "" if the
                /// default is the system zone or a fixed offset.
            static CString      GetDefaultTimezoneName( void );
                /// Shorthand: true for the system zone, false for UTC.
            static void         SetUnknownTimeZoneLocal( bool bLocal );
                /// Whether the default zone is the system's (see SetUnknownTimeZoneLocal()).
            static bool         GetUnknownTimeZoneLocal( void );
                /// True if the date is known to be UTC.
            bool                IsGMT( void ) const { return m_bHasZone && m_nUTCOffset == 0; }
                /// Sets the zone the local time is in, without changing the local
                /// time. False for an offset of a day or more.
            bool                SetTimeZone( int nOffsetSeconds );
                /// Forgets the time zone; the local time stays.
            void                ClearTimeZone( void );
                /// The same instant as local time in another zone (the date
                /// unchanged if it is invalid or the offset is a day or more).
            CDate               ToTimeZone( int nOffsetSeconds ) const;
                /// The same, for zone text as SetDefaultTimezone() reads it,
                /// including named zones ("Asia/Kolkata"). An invalid date if the
                /// zone is unknown.
            CDate               ToTimeZone( const char *pszZone ) const;
                /// The same instant in UTC.
            CDate               ToUTC( void ) const;
                /// The same instant as local time in the system's time zone (with
                /// its daylight saving rules for that date).
            CDate               ToLocalTime( void ) const;
                /// Takes the local time to be in the system's time zone and records
                /// the offset in force then. A local time that happens twice (clocks
                /// going back) is taken as the earlier one; one that never happened
                /// (clocks going forward) moves forward by the gap. Returns false if
                /// the system has no offset for the date.
            bool                AssumeLocalTimeZone( void );
                /// The same for zone text as SetDefaultTimezone() reads it: a
                /// named zone ("Europe/Paris") resolves its daylight saving as
                /// above; a fixed offset or abbreviation is simply recorded. False
                /// for an unknown zone or an invalid date.
            bool                AssumeTimeZone( const char *pszZone );

                /// Makes the date invalid.
            void                SetInvalid( void ) { m_lMilliseconds = -1; m_lEpochDay = 0; }
                /// Sets the date and time as the constructor with the same
                /// arguments does (invalid if it does not exist), with no time zone.
            void                SetDate( int nDay, int nMonth, int nYear, int nHour = 0, int nMinute = 0, int nSecond = 0, int nMillisecond = 0 );
                /// Sets the date from C library struct tm style parts: tm_year is
                /// years since 1900, tm_mon 0-11; milliseconds and the offset are
                /// ignored.
            void                SetDate( tm_type &tmTime );
                /// Sets the date to now (same as SetToCurrent()).
            void                SetDate( void );
                /// The date as DF_PRETTY text ("MM/DD/YYYY  HH:MM:SS").
            CString             PrettyDate( void ) const;
                /// The date as text in one of the FormatFlags formats; "Invalid
                /// Date" for an invalid date. The formats that say "GMT" convert
                /// to UTC first.
            CString             FormatDate( FormatFlags nDateFormat = CDate::DF_PRETTY ) const;
                /// The date as a database type (DB_ORACLE and the like) writes it.
		    CString				DBDate( int nDBType ) const;
                /// The date as DF_SYBASE text ("Mmm DD YYYY HH:MM AM").
            CString             SybaseDate( void ) const;
                /// The date as an HTTP header date (DF_RFC1123, in UTC). An invalid
                /// date gives a fixed, well-formed date so the header stays readable.
            CString             HTTPDate( void ) const;

                /// ISO 8601 extended format: "YYYY-MM-DD", or with bTime
                /// "YYYY-MM-DDTHH:MM:SS[.sss]" plus the zone ("Z", "+05:30", or
                /// nothing when the zone is unknown). Proleptic
                /// Gregorian calendar; years outside 0000-9999 get a sign
                /// ("-0043" is 44 BC, "+10000").
            CString             FormatISO8601( bool bTime = true, bool bMilliseconds = false ) const;
                /// The date in the proleptic Gregorian calendar used by ISO 8601:
                /// astronomical year (0 is 1 BC), month 1-12, day.
            void                GetISODate( long &lYear, int &nMonth, int &nDay ) const;
                /// Reads ISO 8601 (and RFC 3339) dates and times: calendar dates
                /// (YYYY-MM-DD, YYYYMMDD, YYYY-MM, YYYY), ordinal dates (YYYY-DDD),
                /// week dates (YYYY-Www-D), expanded years (+YYYYY, -YYYY), times
                /// after "T" or a space (hh, hh:mm, hh:mm:ss, basic forms, a
                /// fraction on the last part, 24:00), and zones (Z, +hh, +hh:mm,
                /// +hhmm). The time is kept as written, with its zone. Returns an
                /// invalid date, and sets *pbValid to false, if the text is not
                /// exactly an ISO 8601 date.
            static CDate        ParseISO8601( const char *strValue, bool *pbValid = NULL );
                /// Parses almost any written date: numeric forms ("1/1/98
                /// 10:13 PM", "13.08.1998"), month and day names ("August 13th,
                /// 1998 at 13:05:20 GMT"), ISO 8601, and "Now", "Today",
                /// "Today +6", "Now - 4:55", "Start of day". Empty text (or NULL)
                /// is now. One- and two-digit years fall from 30 years before to
                /// 70 years after the current year. A zone in the text is kept.
                /// @return The date, or an invalid date for text that is not a date.
            static CDate        GetSafeDate( const char *strValue );
                /// Same as GetSafeDate().
            static CDate        Parse( const char *strValue ) { return GetSafeDate( strValue ); }
                /// Parses text whose times are UTC unless it gives a zone; the result
                /// is in UTC.
            static CDate        ParseGMT( const char *strValue ) { CDate dttTemp = GetSafeDate( strValue ); if ( ! dttTemp.m_bHasZone ) dttTemp.SetTimeZone( 0 ); return dttTemp.ToUTC(); }
                /// Parses Sybase's long date format ("October 19 1961 04:15:31:111 pm").
            static CDate        ParseSybase( const char *strValue );
                /// The date of a Unix time (seconds since 1970, UTC; may be
                /// negative), marked as UTC.
            static CDate        ParseUnix( time_t tValue );

                /// The Julian Date, with the time of day as a fraction.
                ///
                /// Julian Day: the astronomical count of days since noon UT on
                /// 1 January 4713 BC (Julian calendar). GetJulianDate() includes
                /// the time of day as a fraction (days start at noon, so midnight
                /// is .5); GetJulianDayNumber() is the whole number for the day,
                /// the JD at noon, of the UT date. A date with a time zone is
                /// converted to UT first; one without uses the default zone (see
                /// SetDefaultTimezone). A double gives about a
                /// microsecond of resolution for current dates.
            double              GetJulianDate( void ) const;
                /// The Julian Day Number of the UT date (see GetJulianDate()).
            long                GetJulianDayNumber( void ) const;
                /// The date and time (UT, rounded to the millisecond) of a Julian
                /// Date. The result is marked as UTC.
            static CDate        FromJulianDate( double dJulianDate );
                /// The date (at noon UT, when that Julian Day starts) of a Julian Day
                /// Number, marked as UTC.
            static CDate        FromJulianDayNumber( long lJulianDayNumber );
                /// Parses an HTTP date in any of its three forms (RFC 1123, RFC
                /// 850 and asctime()); other text goes to Parse().
            static CDate        ParseHTTP( const char *strHTTPDate );

                /// Milliseconds from oThen to this date (positive when this date
                /// is later), comparing instants.
            long long           Elapsed( const CDate &oThen ) const;
                /// Calendar days from oThen's date to this date (positive when
                /// this is later), each date in its own time zone.
            int                 ElapsedDays( const CDate &oThen ) const;
                /// A copy of this date nDays later (earlier if negative).
            CDate               OffsetDays( int nDays ) { CDate dttRet( *this ); dttRet.m_lEpochDay += (long)nDays; return dttRet; }
                /// Adds days, minutes and seconds (any of them negative) to the
                /// local time. An invalid date stays as it is.
                /// @return This date.
            CDate              &Add( int nDays, int nMinutes = 0, int nSeconds = 0 );

                /// Turns Herschel's 4000-year rule on or off.
                ///
                /// Herschel's proposed 4000-year rule: when enabled, years divisible
                /// by 4000 (4000, 8000) are common years instead of leap years. Off
                /// by default, which matches the Gregorian calendar as decreed, ISO
                /// 8601 and other date libraries; build with -DUSE_HERSCHEL_CORRECTION=ON
                /// to make it the default. Set it once at startup, before creating
                /// dates: a date stores a day count, so dates after 28 February 4000
                /// created under one setting read one day differently under the other.
            static void         SetHerschelCorrection( bool bEnabled );
                /// Whether Herschel's 4000-year rule is on.
            static bool         GetHerschelCorrection( void );

                /// Records the difference between a time server's time (seconds
                /// since 1900) and the system clock, and applies it to the current
                /// time from then on. The system clock is not changed.
            static void         SynchronizeToTimeServer( unsigned long lServerTime );
                /// The seconds added to the system clock (see SynchronizeToTimeServer()).
            static long         GetOffsetSeconds( void ) { return m_lOffsetSeconds; }

                /// Sets the date to now, in UTC.
            void                SetToCurrent( void );

                /// Gives the stored day number and milliseconds (for tests).
            void                TestFunction( long &lEpoch, long &lMillis ) { lEpoch = m_lEpochDay; lMillis = m_lMilliseconds; }

                /// Toggles between UTC and a zone.
                ///
                /// Older interface; offsets in minutes east of UTC. TZAdjust converts
                /// a UTC date to that zone and anything else to UTC; SetToGMT
                /// converts to UTC, taking a date without a zone to be in that zone.
                /// @param nOffset Minutes east of UTC.
            void                TZAdjust( int nOffset );
                /// Converts to UTC, taking a date without a zone to be nOffset
                /// minutes east of UTC (see TZAdjust()).
            void                SetToGMT( int nOffset );

                /// Moves the time back to midnight (an invalid date stays as it is).
                /// @return This date.
            CDate              &StartOfDay( void );
                /// Moves the time back to the start of the hour.
                /// @return This date.
            CDate              &StartOfHour( void );

                /// The current time in seconds since 1970 (Unix), as text.
            static CString      EpochTimestamp( void );
                /// The current time in seconds since 1900 (NTP), as text.
            static CString      NTPTimestamp( void );

        private:
                /// Reads "hh:mm:ss", "hh:mm", "hhmmss" and shorter forms into
                /// tmTemp, returning the time in milliseconds.
            static long         GetHMS( const char *strTime, tm_type &tmTemp );
                /// Parses text with letters in it ("Now", month names, ...).
            static CDate        GetAlphaDate( const char *strValue );
                /// "Now", with an optional "+ h:mm:ss" or "- h:mm:ss" offset.
            static CDate        GetNowDate( const char *strValue );
                /// "Today" (local midnight), with an optional "+ n" or "- n" days.
            static CDate        GetTodayDate( const char *strValue );
                /// Puts a one- or two-digit year in the window from 30 years before
                /// to 70 years after the current year.
            static int          AdjustTwoDigitYear( int nYear );
                /// Converts a historical calendar date to a day number; false if
                /// the date does not exist.
            static bool         DateToDay( int nDay, int nMonth, int nYear, long &lDay );
                /// Converts a day number to a historical calendar date.
            static void         DayToDate( long lDay, int &nDay, int &nMonth, int &nYear );
                /// The numeric date parser behind GetSafeDate().
            static CDate        DateParse( const char *strRawDate );
                /// Makes the parsed parts a date, with the zone the parser found.
            static CDate        FinishParse( tm_type &tmVals );
                /// The instant, in milliseconds on the day numbering, in UTC.
            long long           UtcMilliseconds( void ) const;
                /// The date at an instant, as local time nOffsetSeconds east of UTC.
            static CDate        FromUtcMilliseconds( long long llUtc, int nOffsetSeconds );
                /// Reads a zone abbreviation or offset into seconds east of UTC.
            static bool         ScanGMTOffset( const char *pszOffset, long &lValue );
                /// Reads zone text: the system zone, a fixed offset or a named zone.
            static bool         ReadZone( const char *pszZone, int &nKind, int &nOffset, std::shared_ptr<const CTimeZone> &pZone );
                /// Takes the local time to be in pZone (NULL: the system's zone).
            bool                AssumeZone( const CTimeZone *pZone );
                /// Reads a month name into 0-11.
            static bool         ScanMonths( const char *pszMonth, long &lValue );
                /// Reads a day name into 0 (Sunday) to 6.
            static bool         ScanWday( const char *pszWday, long &lValue );
                /// Reads "am" or "pm" into AMPM_AM or AMPM_PM.
            static bool         ScanAMPM( const char *pszAMPM, long  &lValue );
                /// Looks strCompare up in a table of nLength tokens.
            static bool         ScanTokens( Token_Type *aTokens, int nLength, CString strCompare, long &lValue );
                /// Converts a 12-hour hour to 0-23 (-1 for an impossible one).
            static int          FixAMPM( int nHour, int nAMPM );
                /// Records the parsed zone and resolves short years.
            static void         AdjustTime( tm_type &tmVals, long gmtoff );
    };
} // namespace IASLib
#endif // IASLIB_DATE_H__
