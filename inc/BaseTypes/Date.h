/*
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

    typedef struct My_tm
    {
        int     tm_msec;
        int     tm_sec;
        int     tm_min;
        int     tm_hour;
        int     tm_mday;
        int     tm_mon;
        int     tm_year;
        int     tm_wday;
        int     tm_yday;
        int     tm_isdst;
        int     tm_gmtoffset;
    } tm_type;

    typedef struct My_Token
    {
        const char *strData;
        long        lToken;
    } Token_Type;


    #define AMPM_NONE 0
    #define AMPM_AM 1
    #define AMPM_PM 2

    /*
    **  DATE FORMATS
    **
    **  PRETTY   - 02/06/2001  12:14:11
    **  UNIX     - Mon Jun  2 12:14:11 2001
    **  RFC850   - Monday, 02-Jun-01 12:14:11 GMT
    **  RFC1123  - Mon, 02 Jun 2001 12:14:11 GMT
    **  NETSCAPE - 02/Jun/2001:12:14:11
    **  EXACT    - 02/06/2001 12:14:11.374 GMT
    **  SYBASE   - Jun 02 2001 12:14 PM
    **  SQLITE   - 2001/06/02 12:14:11
    **  ORACLE   - 02-JUN-2001, 12:14:11 PM
    **  ISO_8601_FULL    - 2001-06-02T12:14:11Z  (Z only for dates known to be UTC)
    **  ISO_8601_FULL_MS - 2001-06-02T12:14:11.374Z
    **  ISO_8601_DATE    - 2001-06-02
    **
    **  The ISO 8601 based formats (ISO_8601*, YYYYMMDD) use the proleptic
    **  Gregorian calendar with astronomical years, as ISO 8601 requires; the
    **  others use the historical calendar (Julian before 15 October 1582).
    **
    */

    #define DATE_FORMAT_PRETTY      1
    #define DATE_FORMAT_UNIX        2
    #define DATE_FORMAT_RFC850      3
    #define DATE_FORMAT_RFC1123     4
    #define DATE_FORMAT_NETSCAPE    5
    #define DATE_FORMAT_EXACT       6
    #define DATE_FORMAT_SYBASE      7
    #define DATE_FORMAT_SQLITE		8
    #define DATE_FORMAT_ORACLE      9
    #define DATE_FORMAT_YYYYMMDD    10
    #define DATE_FORMAT_ISO_9601    11
    #define DATE_FORMAT_ISO_9601_MS 12
    #define DATE_FORMAT_HHMMSS      13
    #define DATE_FORMAT_ISO_9601_MS_PACKED 14
    #define DATE_FORMAT_RFC7231     15
    #define DATE_FORMAT_ISO_8601_FULL    16
    #define DATE_FORMAT_ISO_8601_FULL_MS 17
    #define DATE_FORMAT_ISO_8601_DATE    18

    #define DB_UNKNOWN				0
    #define DB_ORACLE				1
    #define DB_SYBASE				2
    #define DB_SQLITE				3
    #define DB_MYSQL                4

    class CDate : public CObject
    {
        protected:
            static long         m_lOffsetSeconds;
            long                m_lEpochDay;
            long                m_lMilliseconds;
            bool                m_bHasZone;     // Whether the offset below is known.
            int                 m_nUTCOffset;   // Seconds east of UTC (UTC+5:30 is 19800).
            CString             m_castBuffer;
        public:
            enum FormatFlags {
                DF_UNDEFINED            = 0x0000,
		        DF_PRETTY               = 0x0001,
                DF_UNIX                 = 0x0002,
                DF_RFC850               = 0x0003,
                DF_RFC1123              = 0x0004,
                DF_NETSCAPE             = 0x0005,
                DF_EXACT                = 0x0006,
                DF_SYBASE               = 0x0007,
                DF_SQLITE		        = 0x0008,
                DF_ORACLE               = 0x0009,
                DF_YYYYMMDD             = 0x000A,
                DF_ISO_8601             = 0x000B,
                DF_ISO_8601_MS          = 0x000C,
                DF_HHMMSS               = 0x000D,
                DF_ISO_8601_MS_PACKED   = 0x000E,
                DF_RFC7231              = 0x000F,
                DF_ISO_8601_FULL        = 0x0010,
                DF_ISO_8601_FULL_MS     = 0x0011,
                DF_ISO_8601_DATE        = 0x0012,
		    };

                // The current time (UTC).
                                CDate();
                // A date in the historical calendar: nMonth is 0-11, years have no
                // year zero (1 BC is -1). A date or time that does not exist (30
                // February, 10 October 1582, hour 24) gives an invalid date.
                                CDate( int nDay, int nMonth, int nYear, int nHour = 0, int nMinute = 0, int nSecond = 0, int nMillisecond = 0 );
                                CDate( const CDate &oSource );
                                CDate( const char *strSource );
                                CDate( tm_type &tmSource );
                // A day number and milliseconds since midnight, with no time zone.
                                CDate( long lEpochDay, long lMilliseconds ) { m_lEpochDay = lEpochDay; m_lMilliseconds = lMilliseconds; m_bHasZone = false; m_nUTCOffset = 0; }
            virtual            ~CDate();

                                DEFINE_OBJECT( CDate )

            CDate              &operator =( const CDate &oSource );

            bool                operator >( const CDate &oCompare ) const;
            bool                operator <( const CDate &oCompare ) const;
            bool                operator >=( const CDate &oCompare ) const;
            bool                operator <=( const CDate &oCompare ) const;
            bool                operator ==( const CDate &oCompare ) const;
            bool                operator !=( const CDate &oCompare ) const;

            int                 GetDay( void ) const;
                // Day of the week, 0 (Sunday) to 6 (Saturday).
            int                 GetDOW( void ) const { long lDow = ( m_lEpochDay + 6 ) % 7; return (int)( ( lDow < 0 ) ? lDow + 7 : lDow ); }
            int                 GetDate( void ) const { return GetDay(); }
            int                 GetMonth( void ) const;
            int                 GetYear( void ) const;
            int                 GetFullYear( void ) const { return GetYear(); }
            int                 GetHours( void ) const;
            int                 GetMinutes( void ) const;
            int                 GetSeconds( void ) const;
            int                 GetHour( void ) const { return GetHours(); }
            int                 GetMinute( void ) const { return GetMinutes(); }
            int                 GetSecond( void ) const { return GetSeconds(); }
            void                SetMillisecond( int nMilliseconds ) { m_lMilliseconds -= m_lMilliseconds % 1000; nMilliseconds %= 1000; m_lMilliseconds += nMilliseconds; }
            int                 GetMillisecond( void ) const;
            long                GetEpochDay( void ) const { return m_lEpochDay; }

            int                 GetDOY( void ) const;
            int                 GetSOD( void ) const { return m_lMilliseconds / 1000; }

            void                ClearTime( void ) { m_lMilliseconds = 0; }

            bool                IsDate( void ) const { return ( m_lMilliseconds != -1 ); }
            bool                IsValid( void ) const { return ( m_lMilliseconds >= 0 ) ? true:false; }
            bool                IsLeapYear( int nYear ) const;
                // True if the day, month (0-11) and year form a real date.
            static bool         IsValidDate( int nDay, int nMonth, int nYear );
                // Time zones. A date holds its local (wall clock) time and, when
                // known, its offset from UTC in seconds east of UTC (-18000 for
                // EST, 19800 for UTC+5:30). The day, month, hour and so on are
                // always the local time. Anything that needs the instant uses the
                // offset automatically: comparisons, Elapsed(), ToUTC(), the Julian
                // Day methods and the formats that say "GMT". A date without a
                // zone (from numbers, or text without a zone) is taken to be in
                // the default time zone for those: the system's local time zone
                // unless SetDefaultTimezone() chose another, using the offset in
                // force at that local time (UTC if the system has none). Use
                // SetTimeZone() for a date in some other zone.
                //
                // The current time, Unix times and Julian Days are UTC. Parsed text
                // keeps its zone ("12:14 EST" is 12:14 at -05:00).
            bool                HasTimeZone( void ) const { return m_bHasZone; }
                // The offset in seconds east of UTC used for conversions: the
                // date's own, or for a date without a zone the default above.
            int                 GetUTCOffset( void ) const;
                // The default time zone: the zone assumed for dates without one,
                // and the zone of "Today" and of dates the parser fills in. It is
                // the system's local time zone (with its daylight saving rules)
                // unless set to a fixed offset, in seconds east of UTC, or to zone
                // text the parser reads: "UTC", "Z", "+05:30", "-0700", "EST",
                // "GMT-1", or to a named IANA zone such as "America/New_York",
                // which follows that zone's daylight saving rules (read from the
                // system's zone database). Abbreviations and offsets are tried
                // before names, so "EST" is always UTC-5; names are case-sensitive.
                // "system" or "local" goes back to the system zone. Returns
                // false, leaving the default unchanged, for text it cannot read or
                // an offset of a day or more. It applies to every date (including
                // existing ones without a zone), so set it at startup.
            static bool         SetDefaultTimezone( int nOffsetSeconds );
            static bool         SetDefaultTimezone( const char *pszZone );
            static void         SetDefaultTimezoneToSystem( void );
            static bool         IsDefaultTimezoneSystem( void );
                // The fixed default offset (0 when the default is the system zone).
            static int          GetDefaultTimezoneOffset( void );
                // The named default zone ("America/New_York"), or "" if the
                // default is the system zone or a fixed offset.
            static CString      GetDefaultTimezoneName( void );
                // Shorthand: true for the system zone, false for UTC.
            static void         SetUnknownTimeZoneLocal( bool bLocal );
            static bool         GetUnknownTimeZoneLocal( void );
                // True if the date is known to be UTC.
            bool                IsGMT( void ) const { return m_bHasZone && m_nUTCOffset == 0; }
                // Sets the zone the local time is in, without changing the local
                // time. False for an offset of a day or more.
            bool                SetTimeZone( int nOffsetSeconds );
            void                ClearTimeZone( void );
                // The same instant as local time in another zone.
            CDate               ToTimeZone( int nOffsetSeconds ) const;
                // The same, for zone text as SetDefaultTimezone() reads it,
                // including named zones ("Asia/Kolkata"). An invalid date if the
                // zone is unknown.
            CDate               ToTimeZone( const char *pszZone ) const;
            CDate               ToUTC( void ) const;
                // The same instant as local time in the system's time zone (with
                // its daylight saving rules for that date).
            CDate               ToLocalTime( void ) const;
                // Takes the local time to be in the system's time zone and records
                // the offset in force then. A local time that happens twice (clocks
                // going back) is taken as the earlier one; one that never happened
                // (clocks going forward) moves forward by the gap. Returns false if
                // the system has no offset for the date.
            bool                AssumeLocalTimeZone( void );
                // The same for zone text as SetDefaultTimezone() reads it: a
                // named zone ("Europe/Paris") resolves its daylight saving as
                // above; a fixed offset or abbreviation is simply recorded. False
                // for an unknown zone or an invalid date.
            bool                AssumeTimeZone( const char *pszZone );

            void                SetInvalid( void ) { m_lMilliseconds = -1; m_lEpochDay = 0; }
            void                SetDate( int nDay, int nMonth, int nYear, int nHour = 0, int nMinute = 0, int nSecond = 0, int nMillisecond = 0 );
            void                SetDate( tm_type &tmTime );
            void                SetDate( void );
            CString             PrettyDate( void ) const;
            CString             FormatDate( FormatFlags nDateFormat = CDate::DF_PRETTY ) const;
		    CString				DBDate( int nDBType ) const;
            CString             SybaseDate( void ) const;
            CString             HTTPDate( void ) const;

                // ISO 8601 extended format: "YYYY-MM-DD", or with bTime
                // "YYYY-MM-DDTHH:MM:SS[.sss]" plus the zone ("Z", "+05:30", or
                // nothing when the zone is unknown). Proleptic
                // Gregorian calendar; years outside 0000-9999 get a sign
                // ("-0043" is 44 BC, "+10000").
            CString             FormatISO8601( bool bTime = true, bool bMilliseconds = false ) const;
                // The date in the proleptic Gregorian calendar used by ISO 8601:
                // astronomical year (0 is 1 BC), month 1-12, day.
            void                GetISODate( long &lYear, int &nMonth, int &nDay ) const;
                // Reads ISO 8601 (and RFC 3339) dates and times: calendar dates
                // (YYYY-MM-DD, YYYYMMDD, YYYY-MM, YYYY), ordinal dates (YYYY-DDD),
                // week dates (YYYY-Www-D), expanded years (+YYYYY, -YYYY), times
                // after "T" or a space (hh, hh:mm, hh:mm:ss, basic forms, a
                // fraction on the last part, 24:00), and zones (Z, +hh, +hh:mm,
                // +hhmm). The time is kept as written, with its zone. Returns an
                // invalid date, and sets *pbValid to false, if the text is not
                // exactly an ISO 8601 date.
            static CDate        ParseISO8601( const char *strValue, bool *pbValid = NULL );
            static CDate        GetSafeDate( const char *strValue );
            static CDate        Parse( const char *strValue ) { return GetSafeDate( strValue ); }
                // Parses text whose times are UTC unless it gives a zone; the result
                // is in UTC.
            static CDate        ParseGMT( const char *strValue ) { CDate dttTemp = GetSafeDate( strValue ); if ( ! dttTemp.m_bHasZone ) dttTemp.SetTimeZone( 0 ); return dttTemp.ToUTC(); }
            static CDate        ParseSybase( const char *strValue );
            static CDate        ParseUnix( time_t tValue );

                // Julian Day: the astronomical count of days since noon UT on
                // 1 January 4713 BC (Julian calendar). GetJulianDate() includes
                // the time of day as a fraction (days start at noon, so midnight
                // is .5); GetJulianDayNumber() is the whole number for the day,
                // the JD at noon, of the UT date. A date with a time zone is
                // converted to UT first; one without uses the default zone (see
                // SetDefaultTimezone). A double gives about a
                // microsecond of resolution for current dates.
            double              GetJulianDate( void ) const;
            long                GetJulianDayNumber( void ) const;
                // The date and time (UT, rounded to the millisecond) of a Julian
                // Date or Day Number. The result is marked as UTC.
            static CDate        FromJulianDate( double dJulianDate );
            static CDate        FromJulianDayNumber( long lJulianDayNumber );
            static CDate        ParseHTTP( const char *strHTTPDate );

                // Milliseconds and days from oThen to this date (positive when
                // this date is later).
            long long           Elapsed( const CDate &oThen ) const;
            int                 ElapsedDays( const CDate &oThen ) const;
            CDate               OffsetDays( int nDays ) { CDate dttRet( *this ); dttRet.m_lEpochDay += (long)nDays; return dttRet; }
            CDate              &Add( int nDays, int nMinutes = 0, int nSeconds = 0 );

                // Herschel's proposed 4000-year rule: when enabled, years divisible
                // by 4000 (4000, 8000) are common years instead of leap years. Off
                // by default, which matches the Gregorian calendar as decreed, ISO
                // 8601 and other date libraries; build with -DUSE_HERSCHEL_CORRECTION=ON
                // to make it the default. Set it once at startup, before creating
                // dates: a date stores a day count, so dates after 28 February 4000
                // created under one setting read one day differently under the other.
            static void         SetHerschelCorrection( bool bEnabled );
            static bool         GetHerschelCorrection( void );

                // Records the difference between a time server's time (seconds
                // since 1900) and the system clock, and applies it to the current
                // time from then on. The system clock is not changed.
            static void         SynchronizeToTimeServer( unsigned long lServerTime );
            static long         GetOffsetSeconds( void ) { return m_lOffsetSeconds; }

            void                SetToCurrent( void );

            void                TestFunction( long &lEpoch, long &lMillis ) { lEpoch = m_lEpochDay; lMillis = m_lMilliseconds; }

                // Older interface; offsets in minutes east of UTC. TZAdjust converts
                // a UTC date to that zone and anything else to UTC; SetToGMT
                // converts to UTC, taking a date without a zone to be in that zone.
            void                TZAdjust( int nOffset );
            void                SetToGMT( int nOffset );

                // Moves the time back to midnight, or to the start of the hour.
            CDate              &StartOfDay( void );
            CDate              &StartOfHour( void );

                // The current time in seconds since 1970 (Unix) or 1900 (NTP).
            static CString      EpochTimestamp( void );
            static CString      NTPTimestamp( void );

        private:
            static long         GetHMS( const char *strTime, tm_type &tmTemp );
            static CDate        GetAlphaDate( const char *strValue );
            static CDate        GetNowDate( const char *strValue );
            static CDate        GetTodayDate( const char *strValue );
            static int          AdjustTwoDigitYear( int nYear );
            static bool         DateToDay( int nDay, int nMonth, int nYear, long &lDay );
            static void         DayToDate( long lDay, int &nDay, int &nMonth, int &nYear );
            static CDate        DateParse( const char *strRawDate );
            static CDate        FinishParse( tm_type &tmVals );
            long long           UtcMilliseconds( void ) const;
            static CDate        FromUtcMilliseconds( long long llUtc, int nOffsetSeconds );
            static bool         ScanGMTOffset( const char *pszOffset, long &lValue );
            static bool         ReadZone( const char *pszZone, int &nKind, int &nOffset, std::shared_ptr<const CTimeZone> &pZone );
            bool                AssumeZone( const CTimeZone *pZone );
            static bool         ScanMonths( const char *pszMonth, long &lValue );
            static bool         ScanWday( const char *pszWday, long &lValue );
            static bool         ScanAMPM( const char *pszAMPM, long  &lValue );
            static bool         ScanTokens( Token_Type *aTokens, int nLength, CString strCompare, long &lValue );
            static int          FixAMPM( int nHour, int nAMPM );
            static void         AdjustTime( tm_type &tmVals, long gmtoff );
    };
} // namespace IASLib
#endif // IASLIB_DATE_H__
