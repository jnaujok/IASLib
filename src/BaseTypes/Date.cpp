/***********************************************************************
**  Date
**
**  Description:
**      This object encapsulates the functionality of a date/time combo.
** It also includes the most rediculously robust date parsing routine
** ever written.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include "Date.h"
#include "StringTokenizer.h"
#include "Mutex.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef IASLIB_WIN32__
    #include <sys/time.h>
#else
    #include <sys/types.h>
    #include <sys/timeb.h>
#endif

#define MAX_DATE 10001
#define MIN_DATE (-MAX_DATE)
#define DATE_RANGE (MAX_DATE*2)

// We have to define this constant this way to avoid errors under linux about
// the size of the decimal constant.
static const unsigned long DATE_OFFSET_FOR_TIME_SERVER=(unsigned long)(22089888) * ((unsigned long)100);


namespace IASLib
{
#ifndef IASLIB_WIN32__
#ifdef IASLIB_MULTI_THREADED__
static CMutex g_mutexSingleThread;
#endif
#endif
    long CDate::m_lOffsetSeconds = 0;

    static bool bInitialized = false;
    static long anYearDays[DATE_RANGE + 1];
    static long anMonthDays[ 12 ]    = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    static long anLeapDays[ 12 ]     = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    static long an1582Days[ 12 ]     = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 20, 30, 31 };
    static long anSumMonthDays[ 12 ] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    static long anSumLeapDays[ 12 ]  = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
    static long anSum1582Days[ 12 ]  = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 293, 323 };

    static CString strMonths         = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ";
    static CString strHTTPMonths     = "JanFebMarAprMayJunJulAugSepOctNovDec";
    static CString strWeekDays       = "SunMonTueWedThuFriSat";
    static CString strWeekDaysLong   = "Sunday   Monday   Tuesday  WednesdayThursday Friday   Saturday ";

    IMPLEMENT_OBJECT( CDate, CObject );

    /////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CDate::CDate()
    {
        if ( ! bInitialized )
            Initialize();

        SetToCurrent();
        m_bIsGMT = true;
    }

    CDate::CDate( int nDay, int nMonth, int nYear, int nHour, int nMinute, int nSecond, int nMilli )
    {
        if ( ! bInitialized )
            Initialize();

        m_lMilliseconds = (nHour * 3600000) + (nMinute * 60000) + (nSecond * 1000 ) + nMilli;
        m_lEpochDay = GetYearDays( nYear - 1 );
        if ( IsLeapYear( nYear ) )
        {
            m_lEpochDay += anSumLeapDays[ nMonth ];
        }
        else
        {
            if ( nYear == 1582 )
            {
                m_lEpochDay += anSum1582Days[ nMonth ];
            }
            else
            {
                m_lEpochDay += anSumMonthDays[ nMonth ];
            }
        }

        m_lEpochDay += nDay;

        m_bIsGMT = false;
    }

    CDate::CDate( const CDate &oSource )
    {
        if ( ! bInitialized )
            Initialize();

        m_lMilliseconds = oSource.m_lMilliseconds;
        m_lEpochDay = oSource.m_lEpochDay;
        m_bIsGMT = oSource.m_bIsGMT;
    }

    CDate::CDate( const char *strParse )
    {
        if ( ! bInitialized )
            Initialize();

        *this = GetSafeDate( strParse );
    }

    CDate::CDate( tm_type &tmSource )
    {
        if ( ! bInitialized )
            Initialize();

        if ( ( tmSource.tm_year == 0 ) && ( tmSource.tm_mon == 0 ) && ( tmSource.tm_mday == 0 ) && ( tmSource.tm_msec == -1 ) )
        {
            m_lEpochDay = 0;
            m_lMilliseconds = -1;
            return;
        }

        m_lMilliseconds = (tmSource.tm_hour * 3600000) + (tmSource.tm_min * 60000) + (tmSource.tm_sec * 1000 ) + tmSource.tm_msec;
        m_lEpochDay = GetYearDays( tmSource.tm_year - 1 );
        if ( IsLeapYear( tmSource.tm_year ) )
        {
            m_lEpochDay += anSumLeapDays[ tmSource.tm_mon ];
        }
        else
        {
            if ( tmSource.tm_year == 1582 )
            {
                m_lEpochDay += anSum1582Days[ tmSource.tm_mon ];
            }
            else
            {
                m_lEpochDay += anSumMonthDays[ tmSource.tm_mon ];
            }
        }

        m_lEpochDay += tmSource.tm_mday;

            // Add the GMT Offset
        m_lMilliseconds += tmSource.tm_gmtoffset * 1000;

        while ( m_lMilliseconds >= 86400000 )
        {
            m_lEpochDay++;
            m_lMilliseconds -= 86400000;
        }

        while ( m_lMilliseconds < 0 )
        {
            m_lEpochDay--;
            m_lMilliseconds += 86400000;
        }

        m_bIsGMT = false;
    }


    CDate::~CDate()
    {

    }

    CDate &CDate::operator =( const CDate &oSource )
    {
        m_lMilliseconds = oSource.m_lMilliseconds;
        m_lEpochDay = oSource.m_lEpochDay;
        m_bIsGMT = oSource.m_bIsGMT;

        return *this;
    }

    bool CDate::IsLeapYear( int nYear ) const
    {
        if ( ( nYear < MIN_DATE ) || ( nYear > MAX_DATE ) )
        {
            return false;
        }

        if ( ( GetYearDays( nYear ) - GetYearDays(  nYear - 1 ) ) == 366 )
        {
            return true;
        }
        return false;
    }

    bool CDate::operator >( const CDate &oCompare )
    {
        if ( m_lEpochDay > oCompare.m_lEpochDay )
            return true;

        if ( ( m_lEpochDay == oCompare.m_lEpochDay ) && ( m_lMilliseconds > oCompare.m_lMilliseconds ) )
            return true;

        return false;
    }

    bool CDate::operator <( const CDate &oCompare )
    {
        if ( m_lEpochDay < oCompare.m_lEpochDay )
            return true;

        if ( ( m_lEpochDay == oCompare.m_lEpochDay ) && ( m_lMilliseconds < oCompare.m_lMilliseconds ) )
            return true;

        return false;
    }

    bool CDate::operator >=( const CDate &oCompare )
    {
        if ( m_lEpochDay < oCompare.m_lEpochDay )
            return false;

        if ( ( m_lEpochDay == oCompare.m_lEpochDay ) && ( m_lMilliseconds < oCompare.m_lMilliseconds ) )
            return false;

        return true;
    }

    bool CDate::operator <=( const CDate &oCompare )
    {
        if ( m_lEpochDay > oCompare.m_lEpochDay )
            return false;

        if ( ( m_lEpochDay == oCompare.m_lEpochDay ) && ( m_lMilliseconds > oCompare.m_lMilliseconds ) )
            return false;

        return true;
    }

    bool CDate::operator ==( const CDate &oCompare )
    {
        if ( ( m_lEpochDay == oCompare.m_lEpochDay ) && ( m_lMilliseconds == oCompare.m_lMilliseconds ) )
            return true;

        return false;
    }

    bool CDate::operator !=( const CDate &oCompare )
    {
        if ( ( m_lEpochDay != oCompare.m_lEpochDay ) || ( m_lMilliseconds != oCompare.m_lMilliseconds ) )
            return true;

        return false;
    }

    void CDate::SetDate( int nDay, int nMonth, int nYear, int nHour, int nMinute, int nSecond, int nMilli )
    {
        m_lMilliseconds = (nHour * 3600000) + (nMinute * 60000) + (nSecond * 1000 ) + nMilli;
        m_lEpochDay = GetYearDays( nYear - 1 );
        if ( IsLeapYear( nYear ) )
        {
            if ( nYear == 1582 )
            {
                m_lEpochDay += anSum1582Days[ nMonth ];
            }
            else
            {
                m_lEpochDay += anSumLeapDays[ nMonth ];
            }
        }
        else
        {
            m_lEpochDay += anSumMonthDays[ nMonth ];
        }

        m_lEpochDay += nDay;
        m_bIsGMT = false;
    }

    void CDate::SetDate( tm_type &tmTime )
    {
        int nYear = tmTime.tm_year + 1900;
        m_lMilliseconds = (tmTime.tm_hour * 3600000) + (tmTime.tm_min * 60000) + (tmTime.tm_sec * 1000 );
        m_lEpochDay = GetYearDays( nYear - 1 );

        if ( IsLeapYear( tmTime.tm_year ) )
        {
            if ( nYear == 1582 )
            {
                m_lEpochDay += anSum1582Days[ tmTime.tm_mon ];
            }
            else
            {
                m_lEpochDay += anSumLeapDays[ tmTime.tm_mon ];
            }
        }
        else
        {
            m_lEpochDay += anSumMonthDays[ tmTime.tm_mon ];
        }

        m_lEpochDay += tmTime.tm_mday;

        m_bIsGMT = false;
    }

    void CDate::SetDate( void )
    {
        time_t       nCurrent;
        struct tm   *ptmCurrent;
        int          nMilliseconds;

#ifndef IASLIB_WIN32__
        struct timeval tvCurrent;

        gettimeofday( &tvCurrent, NULL);
        nCurrent = tvCurrent.tv_sec;
        nCurrent += m_lOffsetSeconds;
        nMilliseconds = tvCurrent.tv_usec / 1000;
#else
        struct _timeb tstruct;

        time( &nCurrent );
        nCurrent += m_lOffsetSeconds;
        _ftime64_s( &tstruct );
        nMilliseconds = tstruct.millitm;
#endif
#ifdef IASLIB_WIN32__
        ptmCurrent = new struct tm;
		gmtime_s( ptmCurrent, &nCurrent );
#else
#ifdef IASLIB_MULTI_THREADED__
        g_mutexSingleThread.Lock();
#endif
        ptmCurrent = gmtime( &nCurrent );
#endif
        if ( ptmCurrent )
        {
            m_lMilliseconds = (ptmCurrent->tm_hour * 3600000) + (ptmCurrent->tm_min * 60000) + (ptmCurrent->tm_sec * 1000 ) + nMilliseconds;
            m_lEpochDay = GetYearDays(  ptmCurrent->tm_year + 1899 ) + ptmCurrent->tm_yday + 1; // Year day starts at zero.
        }
#ifdef IASLIB_WIN32__
		delete ptmCurrent;
#else
#ifdef IASLIB_MULTI_THREADED__
        g_mutexSingleThread.Unlock();
#endif
#endif
        m_bIsGMT = true;
    }

    int CDate::GetDay( void ) const
    {
        int nYear = GetYear();
        long nRemainder = m_lEpochDay - GetYearDays( nYear - 1 );
        int nCount = 0;

        if ( nYear == 1582 )
        {
            while ( nRemainder > an1582Days[ nCount ] )
            {
                nRemainder -= an1582Days[ nCount ];
                nCount++;
            }
        }
        else
        {
            if ( IsLeapYear( nYear ) )
            {
                while ( nRemainder > anLeapDays[ nCount ] )
                {
                    nRemainder -= anLeapDays[ nCount ];
                    nCount++;
                }
            }
            else
            {
                while ( nRemainder > anMonthDays[ nCount ] )
                {
                    nRemainder -= anMonthDays[ nCount ];
                    nCount++;
                }
            }
        }

        return (int)nRemainder;
    }

    int CDate::GetDOY( void ) const
    {
        int nYear = GetYear();
        long nRemainder = m_lEpochDay - GetYearDays( nYear - 1 );

        return (int)nRemainder;
    }

    int CDate::GetMonth( void ) const
    {
        int nYear = GetYear();
        long nRemainder = m_lEpochDay - GetYearDays( nYear - 1 );
        int nCount = 0;

        if ( nYear == 1582 )
        {
            while ( nRemainder > an1582Days[ nCount ] )
            {
                nRemainder -= an1582Days[ nCount ];
                nCount++;
            }
        }
        else
        {
            if ( IsLeapYear( nYear ) )
            {
                while ( nRemainder > anLeapDays[ nCount ] )
                {
                    nRemainder -= anLeapDays[ nCount ];
                    nCount++;
                }
            }
            else
            {
                while ( nRemainder > anMonthDays[ nCount ] )
                {
                    nRemainder -= anMonthDays[ nCount ];
                    nCount++;
                }
            }
        }

        return nCount;
    }

    /******************************************************************************
    **  Get Year
    **
    **      This function basically performs a binary search of our valid years to
    ** find a year bracket that surrounds our epoch date. This is about the fastest
    ** way that we can pull a year "out of the hat" to determine what year this
    ** epoch day represents.
    **
    ******************************************************************************/

    int CDate::GetYear( void ) const
    {
        int nCount = 1;
        int nStep = MAX_DATE;

        bool bFound = false;

        while ( ! bFound )
        {
            long nThis = GetYearDays( nCount );
            long nNext = GetYearDays( nCount + 1 );
            if ( ( nThis < m_lEpochDay ) && ( nNext >= m_lEpochDay ) )
            {
                nCount += 1; // Years are off by one?
                bFound = true;
            }
            else
            {
                nStep /= 2;
                if ( nStep == 0 )
                {
                    nStep = 1;
                }

                    // We're still at too small of a date
                if ( nThis < m_lEpochDay )
                {
                        // Move forward.
                    nCount += nStep;
                    if ( nCount > MAX_DATE )
                    {
                        nCount = MAX_DATE;
                        if ( nStep == 1 )
                        {
                                // We've gone past the end of what we represent, and we're stepping by one year. This
                                // date is beyond our far end.
                            bFound = true;
                        }
                    }
                }
                else
                {
                        // We're at too large of a date, step back.
                    nCount -= nStep;
                    if ( nCount < MIN_DATE )
                    {
                        nCount = MIN_DATE;
                        if ( nStep == 1 )
                        {
                                // We've gone past the end of what we represent, and we're stepping by one year. This
                                // date is beyond our far end.
                            bFound = true;
                        }
                    }
                }
            }
        }
        return nCount;
    }

    int CDate::GetHours( void ) const
    {
        return (int)(m_lMilliseconds / 3600000);
    }

    int CDate::GetMinutes( void ) const
    {
        return (int)( (m_lMilliseconds % 3600000) / 60000 );
    }

    int CDate::GetSeconds( void ) const
    {
        return (int)( ( (m_lMilliseconds % 3600000) % 60000 ) / 1000 );
    }

    int CDate::GetMillisecond( void ) const
    {
        return (int)(m_lMilliseconds % 1000);
    }

    /***********************************************************************************
    **  PrettyDate
    **
    **      Given a date value, this function returns a date in the standard year 2K
    ** compliant "MM/DD/YYYY  HH:MM:SS" format.
    **
    ***********************************************************************************/

    CString CDate::PrettyDate( void ) const
    {
        return FormatDate( CDate::DF_PRETTY );
    }

    /***********************************************************************************
    **  FormatDate
    **
    **      Given a date value, this function returns a date in one of the standard
    ** formats that are defined in the header file.
    **
    ***********************************************************************************/

    CString CDate::FormatDate( CDate::FormatFlags nDateFormat ) const
    {
        CString strRetVal = "";
        CString strMonth;
        CString strDOWShort;
        CString strDOWLong;
        if ( m_lMilliseconds == -1 )
        {
            strRetVal = "Invalid Date";
            return strRetVal;
        }

        int nMonth = GetMonth();
        int nDay = GetDate();
        int nYear = GetFullYear();
        int nYearShort = nYear % 100;
        int nHour = GetHours();
        int nMinute = GetMinutes();
        int nSecond = GetSeconds();
        int nMillisecond = GetMillisecond();
        int nDOW = GetDOW();
        strDOWShort = strWeekDays.Substring( nDOW * 3, 3 );
        strDOWLong = strWeekDaysLong.Substring( nDOW * 9, 9 );
        strDOWLong.Trim();
        strMonth = strHTTPMonths.Substring( nMonth * 3, 3 );
        int n12Hour = nHour % 12;
        if ( n12Hour == 0 )
        {
            n12Hour = 12;
        }
        CString strAMPM;
        strAMPM = ( nHour < 12 ) ? "AM" : "PM";
            // Turn the month into 1-12 instead of 0-11
        nMonth++;


        switch ( nDateFormat )
        {
            case CDate::DF_PRETTY:
                strRetVal.Format( "%02d/%02d/%d  %02d:%02d:%02d", nMonth, nDay, nYear, nHour, nMinute, nSecond );
                break;

            case CDate::DF_UNIX:
                strRetVal.Format( "%3s %3s %2d %02d:%02d:%02d %d", (const char *)strDOWShort, (const char *)strMonth, nDay, nHour, nMinute, nSecond, nYear );
                break;

            case CDate::DF_RFC850:
                strRetVal.Format( "%s, %02d-%3s-%02d %02d:%02d:%02d GMT", (const char *)strDOWLong, nDay, (const char *)strMonth, nYearShort, nHour, nMinute, nSecond );
                break;

            case CDate::DF_RFC1123:
                strRetVal.Format( "%3s, %02d %3s %d %02d:%02d:%02d GMT", (const char *)strDOWShort, nDay, (const char *)strMonth, nYear, nHour, nMinute, nSecond );
                break;

            case CDate::DF_NETSCAPE:
                strRetVal.Format( "%02d/%3s/%d:%02d:%02d:%02d", nDay, (const char *)strMonth, nYear, nHour, nMinute, nSecond );
                break;

            case CDate::DF_EXACT:
                strRetVal.Format( "%02d/%02d/%d  %02d:%02d:%02d.%03d", nMonth, nDay, nYear, nHour, nMinute, nSecond, nMillisecond );
                break;

            case CDate::DF_SYBASE:
                strRetVal.Format( "%3s %02d %d %02d:%02d %s", (const char *)strMonth, nDay, nYear, n12Hour, nMinute, (const char *)strAMPM );
                break;

            case CDate::DF_SQLITE:
			    strRetVal.Format( "%d/%02d/%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond );
			    break;

            case CDate::DF_ORACLE:
                strMonth.ToUpperCase();
                strRetVal.Format( "%02d-%3s-%04d, %02d:%02d:%02d %s", nDay, (const char *)strMonth, nYear, n12Hour, nMinute, nSecond, (const char *)strAMPM );
                break;

            case CDate::DF_YYYYMMDD:
                strRetVal.Format( "%04d%02d%02d", nYear, nMonth, nDay );
                break;

            case CDate::DF_ISO_9601:
                strRetVal.Format( "%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond );
                break;

            case CDate::DF_ISO_9601_MS:
                strRetVal.Format( "%04d-%02d-%02d %02d:%02d:%02d.%03d", nYear, nMonth, nDay, nHour, nMinute, nSecond, nMillisecond );
                break;

            case CDate::DF_HHMMSS:
                strRetVal.Format( "%02d%02d%02d", nHour, nMinute, nSecond );
                break;

            case CDate::DF_ISO_9601_MS_PACKED:
                strRetVal.Format( "%04d%02d%02d%02d%02d%02d%03d", nYear, nMonth, nDay, nHour, nMinute, nSecond, nMillisecond );
                break;

            default:
                strRetVal = "Unknown Format";
                break;
        }
            // We return the final string value.
        return strRetVal;
    }

    /***********************************************************************************
    **  DB Date
    **
    **      Converts a date to a string, based on the current database type.
    **
    ***********************************************************************************/

    CString CDate::DBDate( int nDBType ) const
    {
	    switch ( nDBType )
	    {
		    case DB_SYBASE:
                return FormatDate( CDate::DF_SYBASE );

		    case DB_ORACLE:
                return FormatDate( CDate::DF_EXACT );

		    case DB_SQLITE:
                return FormatDate( CDate::DF_SQLITE );

		    default:
                return FormatDate( CDate::DF_PRETTY );
	    }
    }

    /***********************************************************************************
    **  SybaseDate
    **
    **      Given a date value, this function returns a date in the Sybase date format
    ** of "Mmm DD YYYY HH:MM AM/PM" format.
    **
    ***********************************************************************************/

    CString CDate::SybaseDate( void ) const
    {
        return FormatDate( CDate::DF_SYBASE );
    }

    /***********************************************************************************
    **  HTTPDate
    **
    **      Given a date value, this function returns a date in the standard HTTP format
    ** accepted and preferred format laid out by RFC 822 and updated in RFC 1123. The
    ** format is "Www, dd Mmm yyyy hh:mm:ss GMT"
    **
    ***********************************************************************************/

    CString CDate::HTTPDate( void ) const
    {
        CString strRetVal = "";

        if ( m_lMilliseconds == -1 )
        {
            strRetVal = "Tue, 25 Jan 2000 06:06:06 GMT";
            return strRetVal;
        }

        strRetVal = FormatDate( CDate::DF_RFC1123 );
        return strRetVal;
    }

    /***********************************************************************************
    **  GetSafeDate
    **
    **      This function takes a string date and time and converts it to a "legitimate"
    ** date. Dates that contain letters in addition to numbers are accepted and parsed.
    ** Dates that are acceptable are of the following forms, with their interpreted
    ** value given after the equals sign. For these examples, the current date is
    ** assumed as August 13th, 1998 at 13:05:20 GMT. All whitespace in the dates is
    ** ignored. The function returns the date and time in the string format given
    ** which can be parsed without fail by the standard date routines.
    **
    **          ""                                  = "08/13/1998  13:05:20"
    **          "01/01/1998  12:24:44"              = "01/01/1998  12:24:44"
    **          "1/1/2099"                          = "01/01/2099  00:00:00"
    **          "1/1/99"                            = "01/01/1999  00:00:00"
    **          "1/1/65"                            = "01/01/2065  00:00:00"
    **          "1/1/73"                            = "01/01/1973  00:00:00"
    **          "1/1/1998 1:1:1 AM"                 = "01/01/1998  01:01:01"
    **          "1-1-98 10:13 PM"                   = "01/01/1998  22:13:00"
    **          "1-1 1 AM"                          = "01/01/1998  01:00:00"
    **          "1-17  10:05 GMT +5:40"             = "01/17/1998  04:25:00"
    **          "Today"                             = "08/13/1998  00:00:00"
    **          "Today +6"                          = "08/19/1998  00:00:00"
    **          "Now"                               = "08/13/1998  13:05:20"
    **          "Now - 4:55"                        = "08/13/1998  08:10:20"
    **          "Now + 120"                         = "08/13/1998  14:25:20"
    **          "Start of day"                      = "08/13/1998  00:00:00"
    **          "August 13th, 1998 at 13:05:20 GMT" = "08/13/1998  13:05:20"
    **          "Aug 14 1998 5:03 PM"               = "08/14/1998  17:03:00"
    **
    ***********************************************************************************/
    CDate CDate::GetSafeDate( const char *strValue )
    {
        CString     strWork = "";
        CString     strRetVal = "";
        CString     strParse = "";
        CString     strTime = "";
        CString     strMonth = "";
        CString     strDate = "";
        CString     strYear = "";
        CString     strHour = "";
        CString     strMinute = "";
        CString     strSecond = "";
        CDate       dttDate;
        CString     strTemp = strValue;

        strWork = strValue;
        strWork.Trim();
        strWork.ToUpperCase();
        strTemp.ToLowerCase();
        strTemp.Trim();

            // Get rid of nasty hyphen-style dates.
        strWork.Replace( '-', '/' );

        if ( strWork != strTemp )
        {
            return GetAlphaDate( strWork );
        }

        return CDate::DateParse( strWork );
    }

    /***********************************************************************************
    **  GetHMS
    **
    **      Given a string of the form "hh:mm:ss" or "hh:mm" or "hhmmss" or "hmmss" or
    ** "hhmm" or "hmm" or "hh" or "mm" or "h", this function returns the number of seconds
    ** defined by the time period.
    **
    ***********************************************************************************/
    long CDate::GetHMS( const char *strParse, tm_type &tmTime )
    {
        CString     strTime = strParse;
        CString     strHour;
        CString     strMinute;
        CString     strSecond;
        CString     strWork;
        int         nHour = 0;
        int         nMinute = 0;
        int         nSecond = 0;

        if ( strTime.IndexOf( ":" ) != NOT_FOUND )
        {
            strHour = strTime.Substring( 0, (int)strTime.IndexOf( ":" ) );

            nHour = atoi( strHour );

            strWork = strTime.Substring( strTime.IndexOf( ":" ) + 1 );

            if ( strWork.IndexOf( ":" ) != NOT_FOUND )
            {
                strMinute = strWork.Substring( 0, (int)strWork.IndexOf( ":" ) );
                nMinute = atoi( strMinute );

                strSecond = strWork.Substring( strWork.IndexOf( ":" ) + 1 );
                nSecond = atoi( strSecond );
            }
            else
            {
                nMinute = atoi( strWork );
            }
        }
        else
        {
                // hhmmss
            if ( strTime.GetLength() == 6 )
            {
                nHour = atoi( strTime.Substring( 0, 2 ) );
                nMinute = atoi( strTime.Substring( 2, 2 ) );
                nSecond = atoi( strTime.Substring( 4, 2 ) );
            }

                // hmmss
            if ( strTime.GetLength() == 5 )
            {
                nHour = atoi( strTime.Substring( 0, 1 ) );
                nMinute = atoi( strTime.Substring( 1, 2 ) );
                nSecond = atoi( strTime.Substring( 3, 2 ) );
            }

                // hhmm
            if ( strTime.GetLength() == 4 )
            {
                nHour = atoi( strTime.Substring( 0, 2 ) );
                nMinute = atoi( strTime.Substring( 2, 2 ) );
            }

                // hmm
            if ( strTime.GetLength() == 3 )
            {
                nHour = atoi( strTime.Substring( 0, 1 ) );
                nMinute = atoi( strTime.Substring( 1, 2 ) );
            }

                // mm or hh
            if ( strTime.GetLength() == 2 )
            {
                int nTemp = atoi( strTime );

                if ( nTemp < 24 )
                    nHour = nTemp;
                else
                    nMinute = nTemp;
            }

                // h
            if ( strTime.GetLength() == 1 )
            {
                nHour = atoi( strTime );
            }
        }

        tmTime.tm_hour = nHour;
        tmTime.tm_min = nMinute;
        tmTime.tm_sec = nSecond;

        return ( nHour * 3600000 ) + ( nMinute * 60000 ) + ( nSecond * 1000 );
    }

    /***********************************************************************************
    **  GetAlphaDate
    **
    **      This function takes a date that contains letters in addition to numbers.
    ** Dates that are acceptable are of the following forms:
    **          "1/1/1998 1:1:1 AM"     = "01/01/1998  01:01:01"
    **          "1-1-98 10:13 PM"       = "01/01/1998  22:13:00"
    **          "1-1 1 AM"              = "01/01/1998  01:00:00"
    **          "1-17  10:05 GMT +5:40" = "01/17/1998  04:25:00"
    **          "Today"                 = "08/13/1998  00:00:00"
    **          "Today +6"              = "08/19/1998  00:00:00"
    **          "Now"                   = "08/13/1998  13:05:20"
    **          "Now - 4:55"            = "08/13/1998  08:10:20"
    **          "Now + 120"             = "08/13/1998  14:25:20"
    **          "Start of day"          = "08/13/1998  00:00:00"
    **
    ***********************************************************************************/
    CDate CDate::GetAlphaDate( const char *pszValue )
    {
        CString strValue = pszValue;
        strValue.ToUpperCase();
        CDate dttRetVal = CDate( );

        if ( ( strValue.IndexOf( "GMT-" ) != NOT_FOUND ) || ( strValue.IndexOf( "GMT+" ) != NOT_FOUND ) || ( strValue.IndexOf( "GMT -" ) != NOT_FOUND ) || ( strValue.IndexOf( "GMT +" ) != NOT_FOUND ) || ( strValue.IndexOf( "GMT" ) != NOT_FOUND ) )
        {
            return CDate::DateParse( strValue );
        }

            // Check for the "NOW" format which is just the current time
        if ( strValue.IndexOf( "NOW" ) != NOT_FOUND )
        {
            return GetNowDate( strValue );
        }

        if ( strValue.IndexOf( "TODAY" ) != NOT_FOUND )
        {
            return GetTodayDate( strValue );
        }

        if ( strValue.IndexOf( "START OF DAY" ) != NOT_FOUND )
        {
            return GetTodayDate( "TODAY" );
        }

        return CDate::DateParse( strValue );

    }


    /***********************************************************************************
    **  GetNowDate
    **
    **      Part of the GetAlphaDate hierarchy, this function returns the current time
    ** and date with a possible offset given in hours, minutes and seconds. The offset
    ** is optional, and can be either addition (+) or subtraction (-) from the current
    ** time and date.
    **
    ***********************************************************************************/
    CDate CDate::GetNowDate( const char *strValue )
    {
        CDate dttNow;
        bool bAdd = true;

            // Cut off the "NOW" at the start of the value.
        CString strWork = strValue;
        strWork = strWork.Substring( 3 );
        strWork.Trim();

        if ( strWork.GetLength() > 0 )
        {
            if ( strWork[ 0 ] == '/' )
            {
                bAdd=false;
                strWork = strWork.Substring( 1 );
            }
            else
            {
                if ( strWork[ 0 ] == '+' )
                {
                    strWork = strWork.Substring( 1 );
                }
            }
            tm_type tmTemp;
            tmTemp.tm_hour = 0;
            int nOffset = GetHMS( strWork, tmTemp );

            if ( bAdd )
            {
                dttNow.m_lMilliseconds += nOffset;
                if ( dttNow.m_lMilliseconds > 86399999 )
                {
                    dttNow.m_lEpochDay++;
                    dttNow.m_lMilliseconds -= 86400000;
                }
            }
            else
            {
                dttNow.m_lMilliseconds -= nOffset;
                if ( dttNow.m_lMilliseconds < 0 )
                {
                    dttNow.m_lEpochDay--;
                    dttNow.m_lMilliseconds += 86400000;
                }
            }
        }

        return dttNow;
    }

    /***********************************************************************************
    **  GetTodayDate
    **
    **      Part of the GetAlphaDate hierarchy, this function returns the current date
    ** at midnight and allows offsets in days either forward (+) or backward (-) from
    ** today's date.
    **
    ***********************************************************************************/
    CDate CDate::GetTodayDate( const char *strValue )
    {
            // Get Today's Date
        CDate dttNow;

            // Cut off the "Today" at the start of the value.
        CString strWork = strValue;
        strWork = strWork.Substring( 5 );
        strWork.Trim();

        if ( strWork.GetLength() > 0 )
        {
            bool bAdd = true;

                // Note: We aren't looking for "-" because we already converted hyphens to slashes.
            if ( strWork[ 0 ] == '/' )
            {
                bAdd = false;
                strWork = strWork.Substring( 1 );
                strWork.Trim();
            }
            else
            {
                if ( strWork[ 0 ] == '+' )
                {
                    strWork = strWork.Substring( 1 );
                    strWork.Trim();
                }
            }
            int nOffset = atoi( strWork );

            if ( bAdd )
                dttNow.m_lEpochDay += nOffset;
            else
                dttNow.m_lEpochDay -= nOffset;
        }

            // Move it to midnight
        dttNow.m_lMilliseconds = 0;
        return dttNow;
    }

    int CDate::AdjustTwoDigitYear( int nYear )
    {
        nYear = nYear % 100;

        CDate dttNow;
        int nNowYear = dttNow.GetFullYear() % 100;
        int nCentury = ( dttNow.GetFullYear() / 100 ) * 100;
        int nLess = 0;
        int nMore = 0;

        if ( nNowYear < 30 )
        {
            nNowYear += 70;
            nLess = 0;
            nMore = -100;
        }
        else
        {
            nNowYear -= 30;
            nLess = 100;
            nMore = 0;
        }

        if ( nYear < nNowYear )
            nYear += nCentury + nLess;
        else
            nYear += nCentury + nMore;

        return nYear;
    }

    int CDate::Elapsed( const CDate &oThen )
    {
        int nDays = 0;
        int nMillis;

        nDays = m_lEpochDay - oThen.m_lEpochDay;
        nMillis = m_lMilliseconds - oThen.m_lMilliseconds;

        return (nDays * 86400000) + nMillis;
    }

    CDate &CDate::Add( int nDays, int nMinutes, int nSeconds )
    {
        m_lEpochDay += nDays;
        while ( nMinutes > 1440 )
        {
            m_lEpochDay += 1;
            nMinutes -= 1440;
        }

        while ( nMinutes < -1440 )
        {
            m_lEpochDay -= 1;
            nMinutes += 1440;
        }

        m_lMilliseconds += nMinutes * 60000;

        while ( nSeconds > 86400 )
        {
            m_lEpochDay += 1;
            nSeconds -= 86400;
        }

        while ( nSeconds < -86400 )
        {
            m_lEpochDay -= 1;
            nSeconds += 86400;
        }

        m_lMilliseconds += nSeconds * 1000;

        while ( m_lMilliseconds >= 86400000 )
        {
            m_lEpochDay++;
            m_lMilliseconds -= 86400000;
        }

        while ( m_lMilliseconds < 0 )
        {
            m_lEpochDay--;
            m_lMilliseconds += 86400000;
        }

        return *this;
    }

    /***********************************************************************************
    **  ParseSybase
    **
    **      This function is made solely to tear apart the Sybase Short Date format.
    **
    **   CS_DATES_LONG
    **     mth dd yyyy hh:mm:ss:zzz[am|pm]
    **     October 19 1961 04:15:31:111 pm
    **
    ***********************************************************************************/
    CDate CDate::ParseSybase( const char *strValue )
    {
        CString     strWork = strValue;
        CStringTokenizer stToken( strWork, " :", false );
        CString     strMonth = stToken.NextToken();
        CString     strDay = stToken.NextToken();
        CString     strYear = stToken.NextToken();
        CString     strHour = stToken.NextToken();
        CString     strMinute = stToken.NextToken();
        CString     strSecond = stToken.NextToken();
        CString     strMillisecond = stToken.NextToken();
        CString     strAMPM = strMillisecond.Substring( 3 );
        if ( ( strAMPM.GetLength() == 0 ) && ( stToken.HasMoreTokens() ) )
        {
            strAMPM = stToken.NextToken();
        }
        strMillisecond = strMillisecond.Substring( 0, 3 );
        CDate       dttDate;
        CString     strMonths="JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";

        strMonth.ToUpperCase();
        strMonth = strMonth.Substring( 0, 3 );
        int nMonth = (int)strMonths.IndexOf( strMonth );
        nMonth /= 3;
        int nDay = atoi( strDay );
        int nYear = atoi( strYear );

        int nHour = atoi( strHour );
        strAMPM.ToUpperCase();
        if ( ( strAMPM == "PM" ) && ( nHour != 12 ) )
        {
            nHour += 12;
        }
        else
        {
            if ( ( strAMPM == "AM" ) && ( nHour == 12 ) )
            {
                nHour = 0;
            }
        }

        int nMinute = atoi( strMinute );
        int nSecond = atoi( strSecond );
        int nMillisecond = atoi( strMillisecond );

        dttDate.SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, nMillisecond );

        return dttDate;
    }

    /***********************************************************************************
    **  ParseUnix
    **
    **      This function is made to convert a Unix based Time_T value into our date
    ** format. This is actually a pretty easy conversion. All we need to do is find out
    ** how many days it represents (by dividing by 86400), add that to the day for
    ** January 1, 1970 (the last day of 1969 + 1 ), and multiply the remaining seconds
    ** by 1000 to make them into milliseconds.
    **
    ***********************************************************************************/
    CDate CDate::ParseUnix( time_t tValue )
    {
        long    lValue = (long) tValue;
        long    lDays = lValue / 86400;
        long    lSeconds = lValue % 86400;
        CDate   dttDate;

        dttDate.m_lEpochDay = lDays + GetYearDays( 1969 ) + 1;
        dttDate.m_lMilliseconds = lSeconds * 1000;

        return dttDate;
    }

    /***********************************************************************************
    **  ParseHTTP
    **
    **      This function is made solely to tear apart the three accepted HTTP date
    ** formats.
    **
    ***********************************************************************************/
    CDate CDate::ParseHTTP( const char *strValue )
    {
        CDate       dttDate;
        CString     strWork;
        CString     strTemp;
        int         nTemp;
        bool        bAsctime = false;
        CString     strWkday = "SUN|MON|TUE|WED|THU|FRI|SAT|";
        CString     strWeekday = "SUNDAY|MONDAY|TUESDAY|WEDNESDAY|THURSDAY|FRIDAY|SATURDAY|";
        CString     strMonth = "JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC|";
        int         nDay;
        int         nMonth;
        int         nYear;
        int         nHour;
        int         nMinute;
        int         nSecond;

        strWork = strValue;
        strWork.Trim();
        strWork.ToUpperCase();
        nTemp = (int)strWork.IndexOf( ',' );
        if ( nTemp == (int)IASLib::NOT_FOUND )
        {
            nTemp = (int)strWork.IndexOf( ' ' );
            bAsctime = true;
        }
        if ( nTemp != (int)IASLib::NOT_FOUND )
        {
            strTemp = strWork.Substring( 0, nTemp );
            strTemp += "|";
                // Remove the tell-tale.
            strWork = strWork.Substring( nTemp + 1 );
            strWork.Trim();

            if ( ( ! bAsctime ) && ( strWkday.IndexOf( strTemp ) != NOT_FOUND ) )
            {
                // This is an RFC 1123 Date: Wkday "," SP 2DIGIT SP month SP 4DIGIT SP 2DIGIT ":" 2DIGIT ":" 2DIGIT SP "GMT" (Mon, 02 Jun 2001 12:14:11 GMT)
                    // Find the end of the date.
                nTemp = (int)strWork.IndexOf( ' ' );
                strTemp = strWork.Substring( 0, nTemp );
                nDay = atoi( strTemp );

                    // Trim the day off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ' ' );
                strTemp = strWork.Substring( 0, nTemp );
                nMonth = (int)strMonth.IndexOf( strTemp ) / 4;

                    // Trim the month off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ' ' );
                strTemp = strWork.Substring( 0, nTemp );
                nYear = atoi( strTemp );

                    // Trim the year off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ':' );
                strTemp = strWork.Substring( 0, nTemp );
                nHour = atoi( strTemp );

                    // Trim the Hour off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ':' );
                strTemp = strWork.Substring( 0, nTemp );
                nMinute = atoi( strTemp );

                    // Trim the Minute off.
                strWork = strWork.Substring( nTemp + 1 );
                nTemp = (int)strWork.IndexOf( ' ' );
                strTemp = strWork.Substring( 0, nTemp );
                nSecond = atoi( strTemp );
                dttDate.SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, 0 );
            }
            else
            {
                if ( strWeekday.IndexOf( strTemp ) != IASLib::NOT_FOUND )
                {
                    // This is an RFC 850 Date: Weekday "," SP 2DIGIT "-" month "-" 2DIGIT SP 2DIGIT ":" 2DIGIT ":" 2DIGIT SP "GMT" (Monday, 02-Jun-01 12:14:11 GMT)
                        // Find the end of the date.
                    nTemp = (int)strWork.IndexOf( '-' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nDay = atoi( strTemp );

                        // Trim the day off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( '-' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nMonth = (int)strMonth.IndexOf( strTemp ) / 4;

                        // Trim the month off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( ' ' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nYear = atoi( strTemp );
                    nYear = AdjustTwoDigitYear( nYear );

                        // Trim the year off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( ':' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nHour = atoi( strTemp );

                        // Trim the Hour off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( ':' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nMinute = atoi( strTemp );

                        // Trim the Minute off.
                    strWork = strWork.Substring( nTemp + 1 );
                    nTemp = (int)strWork.IndexOf( ' ' );
                    strTemp = strWork.Substring( 0, nTemp );
                    nSecond = atoi( strTemp );
                    dttDate.SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, 0 );
                }
                else
                {
                    if ( strWkday.IndexOf( strTemp ) != NOT_FOUND )
                    {
                        // This is an ASC-Time date: Wkday SP month SP 1*DIGIT SP 2DIGIT ":" 2DIGIT ":" 2DIGIT SP 4DIGIT (Mon Jun  2 12:14:11 2001)
                            // Find the end of the Month.
                        nTemp = (int)strWork.IndexOf( ' ' );
                        strTemp = strWork.Substring( 0, nTemp );
                        nMonth = (int)strMonth.IndexOf( strTemp ) / 4;

                            // Trim the Month off.
                        strWork = strWork.Substring( nTemp + 1 );
                        nTemp = (int)strWork.IndexOf( ' ', 1 );
                        strTemp = strWork.Substring( 0, nTemp );
                        strTemp.Trim();
                        nDay = atoi( strTemp );

                            // Trim the Day off.
                        strWork = strWork.Substring( nTemp + 1 );
                        nTemp = (int)strWork.IndexOf( ':' );
                        strTemp = strWork.Substring( 0, nTemp );
                        nHour = atoi( strTemp );

                            // Trim the Hour off.
                        strWork = strWork.Substring( nTemp + 1 );
                        nTemp = (int)strWork.IndexOf( ':' );
                        strTemp = strWork.Substring( 0, nTemp );
                        nMinute = atoi( strTemp );

                            // Trim the Minute off.
                        strWork = strWork.Substring( nTemp + 1 );
                        nTemp = (int)strWork.IndexOf( ' ' );
                        strTemp = strWork.Substring( 0, nTemp );
                        nSecond = atoi( strTemp );

                            // Trim the seconds off.
                        strWork = strWork.Substring( nTemp + 1 );
                        strTemp = strWork;
                        nYear = atoi( strTemp );

                        dttDate.SetDate( nDay, nMonth, nYear, nHour, nMinute, nSecond, 0 );
                    }
                    else
                    {
                            // This is some other format. We'll try, but there's no guarantee.
                        dttDate = Parse( strValue );
                    }
                }
            }
        }

        return dttDate;
    }

    /***********************************************************************************
    **  SynchronizeToTimeServer
    **
    **      This function allows the system to synchronize our current date and time to
    ** a reference time server. This allows us to keep our time and date accurate to
    ** plus or minus one second. This is especially useful since most of the real world
    ** servers do not seem to be NTP slaved to a reference server.
    **      To use this function, some outside source must get the value of the reference
    ** time server and then send it to this function. The function then determines the
    ** difference (in seconds) between the current time and date, and the date sent by
    ** the reference server. This is then loaded into the static "m_lOffsetSeconds" value
    ** that is used whenever the current time and date are retrieved.
    **      This whole thing seemed like a good idea after I noticed that the production
    ** servers were running up to 15 minutes out of sync with the real time and up to
    ** ten minutes out of sync with each other. It seems that the situation is totally
    ** unacceptable for a system that deals mainly with times and dates.
    **
    ***********************************************************************************/
    void CDate::SynchronizeToTimeServer( unsigned long lServerTime )
    {
        unsigned long  nCurrent;
#ifndef IASLIB_WIN32__
        struct timeval tvCurrent;
#else
        struct _timeb tvCurrent;
#endif

        // First we need to correct the long value to put it into the same terms as the
        // value we get from gettimeofday (Time servers give you seconds since Jan 1, 1900,
        // gettimeofday, the seconds since Jan 1, 1970)
        lServerTime -= DATE_OFFSET_FOR_TIME_SERVER;

#ifndef IASLIB_WIN32__
        // Next, we try to set the time on this machine. Hey, why not, we might be running as
        // root. If that's the case, then we won't need the offset seconds.

        tvCurrent.tv_sec = (time_t)lServerTime;
        tvCurrent.tv_usec = 0;
        if ( settimeofday( &tvCurrent, NULL ) != 0 )
        {
                // Didn't work, we need to set up the offset.
            gettimeofday( &tvCurrent, NULL);
            nCurrent = (unsigned long)tvCurrent.tv_sec;
#else
            _ftime64_s( &tvCurrent );
            nCurrent = (unsigned long)tvCurrent.time;
#endif
            m_lOffsetSeconds = (long)((long)lServerTime - (long)nCurrent);
#ifndef IASLIB_WIN32__
        }
        else
        {
            m_lOffsetSeconds = 0;
        }
#endif
    }

    void CDate::SetToCurrent( void )
    {
        time_t       nCurrent;
        struct tm   *ptmCurrent;
        int          nMilliseconds;

#ifndef IASLIB_WIN32__
        struct timeval tvCurrent;

        gettimeofday( &tvCurrent, NULL);
        nCurrent = tvCurrent.tv_sec;
        nCurrent += m_lOffsetSeconds;
        nMilliseconds = tvCurrent.tv_usec / 1000;
#else
        struct _timeb tstruct;

        time( &nCurrent );
        nCurrent += m_lOffsetSeconds;
        _ftime64_s( &tstruct );
        nMilliseconds = tstruct.millitm;
#endif
#ifdef IASLIB_WIN32__
        ptmCurrent = new struct tm;
		gmtime_s( ptmCurrent, &nCurrent );
#else
#ifdef IASLIB_MULTI_THREADED__
        g_mutexSingleThread.Lock();
#endif
		ptmCurrent = gmtime( &nCurrent );
#endif
        if ( ptmCurrent )
        {
            m_lMilliseconds = (ptmCurrent->tm_hour * 3600000) + (ptmCurrent->tm_min * 60000) + (ptmCurrent->tm_sec * 1000 ) + nMilliseconds;
            m_lEpochDay = GetYearDays(  ptmCurrent->tm_year + 1899 ) + ptmCurrent->tm_yday + 1; // Year day starts at zero.
        }
#ifdef IASLIB_WIN32__
		delete ptmCurrent;
#else
#ifdef IASLIB_MULTI_THREADED__
        g_mutexSingleThread.Unlock();
#endif
#endif
    }

    long CDate::GetYearDays( int nYear )
    {
        if ( anYearDays[ nYear + MAX_DATE ] != -1 )
        {
            return anYearDays[ nYear + MAX_DATE ];
        }

        long lDay = 0;
        int  nCount;

            // THERE IS NO YEAR ZERO! That's why the millenium starts on 2001.
        if ( nYear == 0 )
            nYear = -1;

        if ( nYear < MIN_DATE )
        {
            nYear = MIN_DATE;
        }

        if ( nYear > MAX_DATE )
        {
            nYear = MAX_DATE;
        }

        if ( nYear < 0 )
        {
            nCount = 0;
            while ( nCount != nYear )
            {
                if ( ( (-nCount) % 4 ) == 0 )
                    lDay -= 366;
                else
                    lDay -= 365;

                anYearDays[ nCount + MAX_DATE ] = lDay;

                nCount--;
            }
        }
        else
        {
            nCount = 1;
            while ( nCount <= nYear )
            {
                if ( ( nCount % 4 ) == 0 )
                {
                    if ( nCount > 1582 )
                    {
                        // We have to do the Gregorian Shuffle
                        if ( ( nCount % 100 ) == 0 )
                        {
                            if ( ( nCount % 400 ) != 0 )
                            {
                                lDay += 365;
                            }
                            else
                            {
                                if ( ( nCount % 4000 ) == 0 )
                                {
                                    lDay += 365;
                                }
                                else
                                {
                                    lDay += 366;
                                }
                            }
                        }
                        else
                        {
                            lDay += 366;
                        }
                    }
                    else
                    {
                        lDay += 366;
                    }
                }
                else
                {
                    if ( nCount == 1582 )
                    {
                        lDay += 354;
                    }
                    else
                    {
                        lDay += 365;
                    }
                }

                anYearDays[ nCount + MAX_DATE ] = lDay;

                nCount++;
            }
        }

        return lDay;
    }

    void CDate::Initialize( void )
    {
        for ( int nCount = 0; nCount <= DATE_RANGE ; nCount++ )
        {
            anYearDays[ nCount ] = -1;
        }
        GetYearDays( MIN_DATE );
        GetYearDays( MAX_DATE );
        bInitialized = true;
    }

    int CDate::FixAMPM( int nHour, int nAMPM )
    {
        switch ( nAMPM )
	    {
            case AMPM_NONE:
	            break;

            case AMPM_AM:
	            if ( nHour == 12 )
	                nHour = 0;
	            break;

            case AMPM_PM:
                if ( nHour != 12 )
	                nHour += 12;
	            break;
	    }

        return nHour;
    }

    CDate CDate::DateParse( const char *strRawDate )
    {
        CString     strDate = strRawDate;
        CDate       dttNow;
        char        str_mon[500], str_wday[500], str_gmtoff[500], str_ampm[500], str_trash[500];
        int         tm_sec, tm_min, tm_hour, tm_mday, tm_year;
        long        tm_mon, tm_wday, ampm, gmtoff;
        tm_type     tmVals;

            // sscanf_s has problems with leading/trailing spaces.
        strDate.Trim();

        tmVals.tm_msec = 0;
        tmVals.tm_sec = 0;
        tmVals.tm_min = 0;
        tmVals.tm_hour = 0;
        tmVals.tm_mday = dttNow.GetDay();
        tmVals.tm_mon = dttNow.GetMonth();
        tmVals.tm_year = dttNow.GetYear();
        tmVals.tm_isdst = 0;
        tmVals.tm_gmtoffset = 0;
        ampm = AMPM_NONE;

        /* And do the sscanfs.  WARNING: you can add more formats here,
        ** but be careful!  You can easily screw up the parsing of existing
        ** formats when you add new ones.  The order is important.
        */

            /* DD/mth/YY:HH:MM:SS zone */
        if (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d:%d:%d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d:%d:%d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
        &&
            ( ScanMonths( str_mon, tm_mon )  )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* DD/mth/YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8  )
#else
                ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* DD/mth/YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[apmAPM] %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#else
                ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[apmAPM] %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD/mth/YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD/mth/YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d %d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d %d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD mth YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* mth DD??, YY at HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[apmAPM] %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[apmAPM] %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* mth DD??, YY at HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* mth DD??, YY at HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[apmAPM] %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[apmAPM] %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* mth DD??, YY at HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[apmAPM]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d %400[apmAPM]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-z,] %d at %d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* mth DD YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[apmAPM] %400[^\t:\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[apmAPM] %400[^\t:\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-zA-Z], %d at %d:%d:%d %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-zA-Z], %d at %d:%d:%d %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* mth DD YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-zA-Z], %d at %d:%d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-zA-Z], %d at %d:%d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* mth DD YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d %400[apmAPM] %400[^\t:\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d %400[apmAPM] %400[^\t:\n\r ]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-zA-Z], %d at %d:%d %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-zA-Z], %d at %d:%d %400[^\t:\n\r ]", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* mth DD YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d %d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d %d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d%400[a-zA-Z], %d at %d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min ) == 5 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d%400[a-zA-Z], %d at %d:%d", str_mon, &tm_mday, str_trash, &tm_year, &tm_hour, &tm_min ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD mth YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d %400[^\t:\n\r ]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* DD mth YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }
            /* DD mth YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d %d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d %d:%d", &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD mth HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d:%d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d:%d:%d", &tm_mday, str_mon, &tm_hour, &tm_min, &tm_sec ) == 5 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d:%d:%d", &tm_mday, str_mon, &tm_hour, &tm_min, &tm_sec ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD mth HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %400[a-zA-Z] %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_hour, &tm_min, str_ampm ) == 5 )
#else
                ( sscanf( strDate, "%d %400[a-zA-Z] %d:%d %400[apmAPM]", &tm_mday, str_mon, &tm_hour, &tm_min, str_ampm ) == 5 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d:%d", &tm_mday, str_mon, &tm_hour, &tm_min ) == 4 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d:%d", &tm_mday, str_mon, &tm_hour, &tm_min ) == 4 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* HH:MM:SS ampm zone DD/mth/YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM] %400[^\t:\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 8 )
#else
                ( sscanf( strDate, "%d:%d:%d %400[apmAPM] %400[^\t:\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %400[^\t:\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#else
            ( sscanf( strDate, "%d:%d:%d %400[^\t:\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* HH:MM ampm zone DD/mth/YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d %400[apmAPM] %400[^\t:\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#else
                ( sscanf( strDate, "%d:%d %400[apmAPM] %400[^\t:\n\r ] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %400[^:  \n] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 6 )
#else
            ( sscanf( strDate, "%d:%d %400[^:  \n] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* HH:MM:SS ampm DD/mth/YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon, &tm_year ) == 7 )
#else
                ( sscanf( strDate, "%d:%d:%d %400[apmAPM] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon, &tm_year ) == 6 )
#else
            ( sscanf( strDate, "%d:%d:%d %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* HH:MM ampm DD/mth/YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d %400[apmAPM] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_ampm, &tm_mday, str_mon, &tm_year ) == 6 )
#else
                ( sscanf( strDate, "%d:%d %400[apmAPM] %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, str_ampm, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 )
#else
            ( sscanf( strDate, "%d:%d %d/%400[a-zA-Z]/%d", &tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* HH:MM:SS ampm zone DD mth YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM] %400[^\t:\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 8 )
#else
                ( sscanf( strDate, "%d:%d:%d %400[apmAPM] %400[^\t:\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %400[^\t:\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#else
            ( sscanf( strDate, "%d:%d:%d %400[^\t:\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* HH:MM ampm zone DD mth YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d %400[apmAPM] %400[^\t:\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#else
                ( sscanf( strDate, "%d:%d %400[apmAPM] %400[^\t:\n\r ] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %400[^:  \n] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 6 )
#else
            ( sscanf( strDate, "%d:%d %400[^:  \n] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* HH:MM:SS ampm DD mth YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon, &tm_year ) == 7 )
#else
                ( sscanf( strDate, "%d:%d:%d %400[apmAPM] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon, &tm_year ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon, &tm_year ) == 6 )
#else
            ( sscanf( strDate, "%d:%d:%d %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* HH:MM ampm DD mth YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d:%d %400[apmAPM] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_ampm, &tm_mday, str_mon, &tm_year ) == 6 )
#else
                ( sscanf( strDate, "%d:%d %400[apmAPM] %d %400[a-zA-Z] %d", &tm_hour, &tm_min, str_ampm, &tm_mday, str_mon, &tm_year ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 )
#else
            ( sscanf( strDate, "%d:%d %d %400[a-zA-Z] %d", &tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* wdy, DD/mth/YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM] %400[^\t:\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM] %400[^\t:\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[^:  \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[^:  \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* wdy, DD/mth/YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[apmAPM] %400[^:  \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[apmAPM] %400[^:  \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[^:     \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[^:     \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* wdy, DD/mth/YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* wdy, DD/mth/YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d/%400[a-zA-Z]/%d %d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 6 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* wdy, DD mth YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t:\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t:\n\r ]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 9 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[^:  \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[^:  \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 8 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* wdy, DD mth YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^:  \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^:  \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[^:     \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[^:     \n]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* wdy, DD mth YY HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* wdy, DD mth YY HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#else
                ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d %400[apmAPM]", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 7 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 6 )
#else
            ( sscanf( strDate, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d", str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 6 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* wdy mth DD HH:MM:SS ampm zone YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t:\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_year ) == 9 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %400[apmAPM] %400[^\t:\n\r ] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_year ) == 9 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %400[^:  \n] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_year ) == 8 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %400[^:  \n] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_year ) == 8 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* wdy mth DD HH:MM ampm zone YY */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^:  \n] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_year ) == 8 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d %400[apmAPM] %400[^:  \n] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_year ) == 8 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d %400[^:     \n] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff, &tm_year ) == 7 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d %400[^:     \n] %d", str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff, &tm_year ) == 7 )
#endif
            )
        &&
            ( ScanWday( str_wday, tm_wday ) )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_wday = tm_wday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = 0;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

            /* mth DD HH:MM:SS ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d %d:%d:%d", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 5 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d:%d", str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 5 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* mth DD HH:MM ampm */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%400[a-zA-Z] %d %d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 5 )
#else
                ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d %400[apmAPM]", str_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 5 )
#endif
            &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d %d:%d", str_mon, &tm_mday, &tm_hour, &tm_min ) == 4 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d %d:%d", str_mon, &tm_mday, &tm_hour, &tm_min ) == 4 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD/mth/YY */
        if (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d/%400[a-zA-Z]/%d", &tm_mday, str_mon, &tm_year ) == 3 )
#else
            ( sscanf( strDate, "%d/%400[a-zA-Z]/%d", &tm_mday, str_mon, &tm_year ) == 3 )
#endif
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD mth YY */
        if (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z] %d", &tm_mday, str_mon, &tm_year ) == 3 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z] %d", &tm_mday, str_mon, &tm_year ) == 3 )
#endif
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* MM/DD/YY HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d/%d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8  )
#else
                ( sscanf( strDate, "%ld/%d/%d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d:%d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d:%d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* MM/DD/YYYY HH:MM:SS ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d:%d", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d:%d", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
#endif
        )
        {
            if ( tm_mon > 12 )
            {
                    // Format YYYY/MM/DD Cannot be determined by any other means.
                int nTemp = tm_year;
                tm_year = tm_mon;
                tm_mon = tm_mday;
                tm_mday = nTemp;
            }
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* MM/DD HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 7  )
#else
                ( sscanf( strDate, "%ld/%d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 7  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d:%d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d:%d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 6 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* MM/DD HH:MM:SS ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 6 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 5 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD HH:MM:SS ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 6  )
#else
                ( sscanf( strDate, "%d %d:%d:%d %400[apmAPM] %400[^:     \n]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 6  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d:%d %400[^\t:\n\r ]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 5 )
#else
            ( sscanf( strDate, "%d %d:%d:%d %400[^\t:\n\r ]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 5 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* DD HH:MM:SS ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 5 )
#else
            ( sscanf( strDate, "%d %d:%d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 5 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d:%d", &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 4 )
#else
            ( sscanf( strDate, "%d %d:%d:%d", &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 4 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* MM/DD/YY HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d/%d %d:%d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7  )
#else
                ( sscanf( strDate, "%ld/%d/%d %d:%d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* MM/DD/YYYY HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d:%d", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d:%d", &tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min ) == 5 )
#endif
        )
        {
            if ( tm_mon > 12 )
            {
                    // Format YYYY/MM/DD Cannot be determined by any other means.
                int nTemp = tm_year;
                tm_year = tm_mon;
                tm_mon = tm_mday;
                tm_mday = nTemp;
            }
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* MM/DD HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d %d:%d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 6  )
#else
                ( sscanf( strDate, "%ld/%d %d:%d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 6  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 5 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* MM/DD HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 5 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min ) == 4 )
#else
            ( sscanf( strDate, "%ld/%d %d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min ) == 4 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD HH:MM ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %d:%d %400[apmAPM] %400[^:     \n]", &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 5  )
#else
                ( sscanf( strDate, "%d %d:%d %400[apmAPM] %400[^:     \n]", &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 5  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d %400[^\t:\n\r ]", &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 4 )
#else
            ( sscanf( strDate, "%d %d:%d %400[^\t:\n\r ]", &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 4 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* DD HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, str_ampm ) == 4 )
#else
            ( sscanf( strDate, "%d %d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, str_ampm ) == 4 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d", &tm_mday, &tm_hour, &tm_min ) == 3 )
#else
            ( sscanf( strDate, "%d %d:%d", &tm_mday, &tm_hour, &tm_min ) == 3 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* MM/DD/YY HH ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d/%d %d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_ampm, str_gmtoff ) == 6  )
#else
                ( sscanf( strDate, "%ld/%d/%d %d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_ampm, str_gmtoff ) == 6  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_gmtoff ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_gmtoff ) == 5 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* MM/DD/YYYY HH ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_ampm ) == 5 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d %400[apmAPM]", &tm_mon, &tm_mday, &tm_year, &tm_hour, str_ampm ) == 5 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d/%d %d", &tm_mon, &tm_mday, &tm_year, &tm_hour ) == 4 )
#else
            ( sscanf( strDate, "%ld/%d/%d %d", &tm_mon, &tm_mday, &tm_year, &tm_hour ) == 4 )
#endif
        )
        {
            if ( tm_mon > 12 )
            {
                    // Format YYYY/MM/DD Cannot be determined by any other means.
                int nTemp = tm_year;
                tm_year = tm_mon;
                tm_mon = tm_mday;
                tm_mday = nTemp;
            }
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* MM/DD HH ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%ld/%d %d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_hour, str_ampm, str_gmtoff ) == 5  )
#else
                ( sscanf( strDate, "%ld/%d %d %400[apmAPM] %400[^:     \n]", &tm_mon, &tm_mday, &tm_hour, str_ampm, str_gmtoff ) == 5  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_hour, str_gmtoff ) == 4 )
#else
            ( sscanf( strDate, "%ld/%d %d %400[^\t:\n\r ]", &tm_mon, &tm_mday, &tm_hour, str_gmtoff ) == 4 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* MM/DD HH ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, str_ampm ) == 4 )
#else
            ( sscanf( strDate, "%ld/%d %d %400[apmAPM]", &tm_mon, &tm_mday, &tm_hour, str_ampm ) == 4 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%ld/%d %d", &tm_mon, &tm_mday, &tm_hour ) == 3 )
#else
            ( sscanf( strDate, "%ld/%d %d", &tm_mon, &tm_mday, &tm_hour ) == 3 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD HH ampm zone */
        if (
            (
            (
#ifdef IASLIB_WIN32__
                ( sscanf_s( strDate, "%d %d %400[apmAPM] %400[^:     \n]", &tm_mday, &tm_hour, str_ampm, str_gmtoff ) == 4  )
#else
                ( sscanf( strDate, "%d %d %400[apmAPM] %400[^:     \n]", &tm_mday, &tm_hour, str_ampm, str_gmtoff ) == 4  )
#endif
                &&
                ( ScanAMPM( str_ampm, ampm ) )
            )
            ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d %400[^\t:\n\r ]", &tm_mday, &tm_hour, str_gmtoff ) == 3 )
#else
            ( sscanf( strDate, "%d %d %400[^\t:\n\r ]", &tm_mday, &tm_hour, str_gmtoff ) == 3 )
#endif
            )
            &&
            ( ScanMonths( str_mon, tm_mon ) )
            &&
            ( ScanGMTOffset( str_gmtoff, gmtoff ) )
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            AdjustTime( tmVals, gmtoff );
            return CDate( tmVals );
        }

        /* DD HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, str_ampm ) == 4 )
#else
            ( sscanf( strDate, "%d %d:%d %400[apmAPM]", &tm_mday, &tm_hour, &tm_min, str_ampm ) == 4 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %d:%d", &tm_mday, &tm_hour, &tm_min ) == 3 )
#else
            ( sscanf( strDate, "%d %d:%d", &tm_mday, &tm_hour, &tm_min ) == 3 )
#endif
        )
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

        /* MM/DD/YYYY */
#ifdef IASLIB_WIN32__
        if ( sscanf_s( strDate, "%ld/%d/%d", &tm_mon, &tm_mday, &tm_year ) == 3 )
#else
        if ( sscanf( strDate, "%ld/%d/%d", &tm_mon, &tm_mday, &tm_year ) == 3 )
#endif
        {
            if ( tm_mon > 12 )
            {
                    // Format YYYY/MM/DD Cannot be determined by any other means.
                int nTemp = tm_year;
                tm_year = tm_mon;
                tm_mon = tm_mday;
                tm_mday = nTemp;
            }
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            tmVals.tm_year = tm_year;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

        /* MM/DD */
#ifdef IASLIB_WIN32__
        if ( sscanf_s( strDate, "%ld/%d", &tm_mon, &tm_mday ) == 2 )
#else
        if ( sscanf( strDate, "%ld/%d", &tm_mon, &tm_mday ) == 2 )
#endif
        {
            tmVals.tm_mday = tm_mday;
            tmVals.tm_mon = tm_mon;
                // Months go from 0 to 11, not 1 to 12
            tmVals.tm_mon--;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

        /* HH:MM:SS ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d %400[apmAPM]", &tm_hour, &tm_min, &tm_sec, str_ampm ) == 4 )
#else
            ( sscanf( strDate, "%d:%d:%d %400[apmAPM]", &tm_hour, &tm_min, &tm_sec, str_ampm ) == 4 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d:%d", &tm_hour, &tm_min, &tm_sec ) == 3 )
#else
            ( sscanf( strDate, "%d:%d:%d", &tm_hour, &tm_min, &tm_sec ) == 3 )
#endif
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            tmVals.tm_sec = tm_sec;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* HH:MM ampm */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d %400[apmAPM]", &tm_hour, &tm_min, str_ampm ) == 3 )
#else
            ( sscanf( strDate, "%d:%d %400[apmAPM]", &tm_hour, &tm_min, str_ampm ) == 3 )
#endif
            &&
            ( ScanAMPM( str_ampm, ampm ) )
            )
        ||
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d:%d", &tm_hour, &tm_min ) == 2 )
#else
            ( sscanf( strDate, "%d:%d", &tm_hour, &tm_min ) == 2 )
#endif
        )
        {
            tmVals.tm_hour = FixAMPM( tm_hour, ampm );
            tmVals.tm_min = tm_min;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* DD mth */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%d %400[a-zA-Z]", &tm_mday, str_mon ) == 2 )
#else
            ( sscanf( strDate, "%d %400[a-zA-Z]", &tm_mday, str_mon ) == 2 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

        /* YYYYMMDD */
        if (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%4d%2ld%2d", &tm_year, &tm_mon, &tm_mday ) == 2 )
#else
            ( sscanf( strDate, "%4d%2ld%2d", &tm_year, &tm_mon, &tm_mday ) == 2 )
#endif
        )
        {
            tmVals.tm_year = tm_year;
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

        /* DD */
#ifdef IASLIB_WIN32__
        if ( sscanf_s( strDate, "%d", &tm_mday ) == 1 )
#else
        if ( sscanf( strDate, "%d", &tm_mday ) == 1 )
#endif
        {
            tmVals.tm_mday = tm_mday;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

            /* mth DD */
        if (
            (
#ifdef IASLIB_WIN32__
            ( sscanf_s( strDate, "%400[a-zA-Z] %d", str_mon, &tm_mday ) == 2 )
#else
            ( sscanf( strDate, "%400[a-zA-Z] %d", str_mon, &tm_mday ) == 2 )
#endif
            )
        &&
            ( ScanMonths( str_mon, tm_mon ) )
        )
        {
            tmVals.tm_mon = tm_mon;
            tmVals.tm_mday = tm_mday;
            AdjustTime( tmVals, 0 );
            return CDate( tmVals );
        }

        tmVals.tm_msec = -1;
        tmVals.tm_mday = 0;
        tmVals.tm_mon = 0;
        tmVals.tm_year = 0;

        return CDate( tmVals );
    }

    bool CDate::ScanTokens( Token_Type *aTokens, int nLength, CString strCompare, long &lValue )
    {
        int nCount = 0;
        while ( nCount < nLength )
        {
            if ( strCompare == aTokens->strData )
            {
                lValue = aTokens->lToken;
                return true;
            }
            aTokens++;
            nCount++;
        }
        return false;
    }

    bool CDate::ScanAMPM( const char *pszAMPM, long  &lValue )
    {
        static Token_Type AMPMTable[2] = {
                                            { "am", AMPM_AM },
                                            { "pm", AMPM_PM },
                                        };
        CString strAMPM = pszAMPM;
        strAMPM.ToLowerCase();
        return ScanTokens( AMPMTable, 2, strAMPM, lValue );
    }


    bool CDate::ScanWday( const char *pszWday, long &lValue )
    {
        static Token_Type WdayTable[14] = {
                                            { "sun", 0 },
                                            { "sunday", 0 },
                                            { "mon", 1 },
                                            { "monday", 1 },
                                            { "tue", 2 },
                                            { "tuesday", 2 },
                                            { "wed", 3 },
                                            { "wednesday", 3 },
                                            { "thu", 4 },
                                            { "thursday", 4 },
                                            { "fri", 5 },
                                            { "friday", 5 },
                                            { "sat", 6 },
                                            { "saturday", 6 },
                                        };
        CString strWday = pszWday;
        strWday.ToLowerCase();
        return ScanTokens( WdayTable, 14, strWday, lValue );
    }

    bool CDate::ScanMonths( const char *pszMonth, long &lValue )
    {
        static Token_Type MonthTable[23] = {
                                            { "jan", 0 },
                                            { "january", 0 },
                                            { "feb", 1 },
                                            { "february", 1 },
                                            { "mar", 2 },
                                            { "march", 2 },
                                            { "apr", 3 },
                                            { "april", 3 },
                                            { "may", 4 },
                                            { "jun", 5 },
                                            { "june", 5 },
                                            { "jul", 6 },
                                            { "july", 6 },
                                            { "aug", 7 },
                                            { "august", 7 },
                                            { "sep", 8 },
                                            { "september", 8 },
                                            { "oct", 9 },
                                            { "october", 9 },
                                            { "nov", 10 },
                                            { "november", 10 },
                                            { "dec", 11 },
                                            { "december", 11 },
                                        };
        CString strMonth = pszMonth;
        strMonth.ToLowerCase();
        return ScanTokens( MonthTable, 23, strMonth, lValue );
    }

    bool CDate::ScanGMTOffset( const char *pszOffset, long &lValue )
    {
        static Token_Type GMTOffsetTable[] = {
                                                { "gmt", 0L }, { "utc", 0L }, { "ut", 0L },
                                                { "0000", 0L }, { "+0000", 0L }, { "/0000", 0L },
                                                { "0100", 3600L }, { "+0100", 3600L }, { "/0100", -3600L },
                                                { "0200", 7200L }, { "+0200", 7200L }, { "/0200", -7200L },
                                                { "0300", 10800L }, { "+0300", 10800L }, { "/0300", -10800L },
                                                { "0400", 14400L }, { "+0400", 14400L }, { "/0400", -14400L },
                                                { "0500", 18000L }, { "+0500", 18000L }, { "/0500", -18000L },
                                                { "0600", 21600L }, { "+0600", 21600L }, { "/0600", -21600L },
                                                { "0700", 25200L }, { "+0700", 25200L }, { "/0700", -25200L },
                                                { "0800", 28800L }, { "+0800", 28800L }, { "/0800", -28800L },
                                                { "0900", 32400L }, { "+0900", 32400L }, { "/0900", -32400L },
                                                { "1000", 36000L }, { "+1000", 36000L }, { "/1000", -36000L },
                                                { "1100", 39600L }, { "+1100", 39600L }, { "/1100", -39600L },
                                                { "1200", 43200L }, { "+1200", 43200L }, { "/1200", -43200L },
                                                { "cet", 3600L }, { "ced", 7200L }, /* Central European time */
                                                { "mez", 3600L }, { "mesz", 7200L },    /* Mitteleuropdische Zeit */
                                                { "jst", 32400L }, { "jdt", 36000L },   /* Japan time */
                                                { "bst", -3600L },
                                                { "nst", -12600L },
                                                { "ast", -14400L }, { "adt", -10800L },
                                                { "est", -18000L }, { "edt", -14400L },
                                                { "cst", -21600L }, { "cdt", -18000L },
                                                { "mst", -25200L }, { "mdt", -21600L },
                                                { "pst", -28800L }, { "pdt", -25200L },
                                                { "yst", -32400L }, { "ydt", -28800L },
                                                { "hst", -36000L }, { "hdt", -32400L },
                                                { "a", -3600L }, { "b", -7200L }, { "c", -10800L }, { "d", -14400L },
                                                { "e", -18000L }, { "f", -21600L }, { "g", -25200L }, { "h", -28800L },
                                                { "i", -32400L }, { "k", -36000L }, { "l", -39600L }, { "m", -43200L },
                                                { "n", 3600L }, { "o", 7200L }, { "p", 10800L }, { "q", 14400L },
                                                { "r", 18000L }, { "s", 21600L }, { "t", 25200L }, { "u", 28800L },
                                                { "v", 32400L }, { "w", 36000L }, { "x", 39600L }, { "y", 43200L },
                                                { "z", 0L },
                                            };
        CString strOffset = pszOffset;
        strOffset.ToLowerCase();
        if ( ScanTokens( GMTOffsetTable, sizeof( GMTOffsetTable ) / sizeof( Token_Type ), strOffset, lValue ) )
            return true;

        if ( strOffset.IndexOf( "gmt" ) != NOT_FOUND )
        {
            strOffset = strOffset.Substring( strOffset.IndexOf( "gmt" ) + 3 );
            bool bAdd = true;

                // Remember, we converted "-" to "/"
            if ( strOffset.IndexOf( "/" ) != NOT_FOUND )
            {
                bAdd = false;
                strOffset = strOffset.Substring( strOffset.IndexOf( "/" ) + 1 );
            }

            if ( strOffset.IndexOf( "+" ) != NOT_FOUND )
            {
                bAdd = true;
                strOffset = strOffset.Substring( strOffset.IndexOf( "+" ) + 1 );
            }

            strOffset.Trim();

            // What's left should be an offset (HHMMSS) or (HH:MM:SS) or some variation thereof.
            tm_type tmTime;
            tmTime.tm_hour=0;

            long lMilliseconds = GetHMS( strOffset, tmTime );

            lValue = lMilliseconds / 1000;

            if ( ! bAdd )
            {
                lValue = -lValue;
            }
            return true;
        }
        else
            lValue = 0;

        return false;
    }

    void CDate::AdjustTime( tm_type &tmVals, long gmtoff )
    {
        if ( tmVals.tm_year < 100 )
        {
            tmVals.tm_year = AdjustTwoDigitYear( tmVals.tm_year );
        }
        if ( tmVals.tm_year < 1000 )
        {
            tmVals.tm_year += 1900;
        }
        tmVals.tm_gmtoffset = gmtoff;
    }

    void CDate::TZAdjust( int nOffset )
    {
        if ( m_bIsGMT )
        {
            Add( 0, nOffset, 0 );
            m_bIsGMT = false;
        }
        else
        {
            Add( 0, -nOffset, 0 );
            m_bIsGMT = true;
        }
    }

    void CDate::SetToGMT( int nOffset )
    {
        if ( ! m_bIsGMT )
        {
            Add( 0, - nOffset, 0 );
            m_bIsGMT = true;
        }
    }
    
    // need the long/long conversion methods
#include <string>

    CString CDate::EpochTimestamp( void )
    {
        CDate dttNow;
        dttNow.SetToCurrent();
        static CDate dttEpoch( 01, 01, 1970, 00, 00, 00 );

        int days = dttNow.ElapsedDays( dttEpoch );
        unsigned long long epoch = ( (unsigned long long)days * (unsigned long long)86400 ) + (unsigned long long)dttNow.m_lOffsetSeconds;

        CString retVal( (const char *)(std::to_string( epoch )).c_str() );

        return retVal;
    }

    CString CDate::NTPTimestamp( void )
    {
        CDate dttNow;
        dttNow.SetToCurrent();
        static CDate dttNTPEpoch( 01, 01, 1900, 00, 00, 00 );

        int days = dttNow.ElapsedDays( dttNTPEpoch );
        unsigned long long epoch = ( (unsigned long long)days * (unsigned long long)86400 ) + (unsigned long long)dttNow.m_lOffsetSeconds;

        CString retVal( (const char *)(std::to_string( epoch )).c_str() );

        return retVal;
    }
} // namespace IASLib
