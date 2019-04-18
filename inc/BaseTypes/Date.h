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

#include <time.h>

namespace IASLib
{
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

    #define DB_UNKNOWN				0
    #define DB_ORACLE				1
    #define DB_SYBASE				2
    #define DB_SQLITE				3

    class CDate : public CObject
    {
        protected:
            static long         m_lOffsetSeconds;
            long                m_lEpochDay;
            long                m_lMilliseconds;
            bool                m_bIsGMT;
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
                DF_YYYYMMDD             = 0x000A
		    };

                                CDate();
                                CDate( int nDay, int nMonth, int nYear, int nHour = 0, int nMinute = 0, int nSecond = 0, int nMillisecond = 0 );
                                CDate( const CDate &oSource );
                                CDate( const char *strSource );
                                CDate( tm_type &tmSource );
                                CDate( long lEpochDay, long lMilliseconds ) { m_lEpochDay = lEpochDay; m_lMilliseconds = lMilliseconds; }
            virtual            ~CDate();

                                DEFINE_OBJECT( CDate )

            CDate              &operator =( const CDate &oSource );

            bool                operator >( const CDate &oCompare );
            bool                operator <( const CDate &oCompare );
            bool                operator >=( const CDate &oCompare );
            bool                operator <=( const CDate &oCompare );
            bool                operator ==( const CDate &oCompare );
            bool                operator !=( const CDate &oCompare );

            int                 GetDay( void ) const;
            int                 GetDOW( void ) const { return ( m_lEpochDay + 6 ) % 7; }
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

            void                SetInvalid( void ) { m_lMilliseconds = -1; m_lEpochDay = 0; }
            void                SetDate( int nDay, int nMonth, int nYear, int nHour = 0, int nMinute = 0, int nSecond = 0, int nMillisecond = 0 );
            void                SetDate( tm_type &tmTime );
            void                SetDate( void );
            CString             PrettyDate( void ) const;
            CString             FormatDate( FormatFlags nDateFormat = CDate::DF_PRETTY ) const;
		    CString				DBDate( int nDBType ) const;
            CString             SybaseDate( void ) const;
            CString             HTTPDate( void ) const;
            static CDate        GetSafeDate( const char *strValue );
            static CDate        Parse( const char *strValue ) { return GetSafeDate( strValue ); }
            static CDate        ParseGMT( const char *strValue ) { CDate dttTemp; dttTemp=GetSafeDate( strValue ); dttTemp.m_bIsGMT = true; return dttTemp; }
            static CDate        ParseSybase( const char *strValue );
            static CDate        ParseUnix( time_t tValue );
            static CDate        ParseHTTP( const char *strHTTPDate );

            int                 Elapsed( const CDate &oThen );
            int                 ElapsedDays( const CDate &oThen ) { return (int)(oThen.m_lEpochDay - m_lEpochDay); }
            CDate               OffsetDays( int nDays ) { CDate dttRet( *this ); dttRet.m_lEpochDay += (long)nDays; return dttRet; }
            void                Add( int nDays, int nMinutes = 0, int nSeconds = 0 );

            static void         SynchronizeToTimeServer( unsigned long lServerTime );
            static long         GetOffsetSeconds( void ) { return m_lOffsetSeconds; }

            void                SetToCurrent( void );

            void                TestFunction( long &lEpoch, long &lMillis ) { lEpoch = m_lEpochDay; lMillis = m_lMilliseconds; }

            void                TZAdjust( int nOffset );
            void                SetToGMT( int nOffset );

        private:
            static long         GetHMS( const char *strTime, tm_type &tmTemp );
            static CDate        GetAlphaDate( const char *strValue );
            static CDate        GetNowDate( const char *strValue );
            static CDate        GetTodayDate( const char *strValue );
            static int          AdjustTwoDigitYear( int nYear );
            static long         GetYearDays( int nYear );
            static void         Initialize( void );
            static CDate        DateParse( const char *strRawDate );
            static bool         ScanGMTOffset( const char *pszOffset, long &lValue );
            static bool         ScanMonths( const char *pszMonth, long &lValue );
            static bool         ScanWday( const char *pszWday, long &lValue );
            static bool         ScanAMPM( const char *pszAMPM, long  &lValue );
            static bool         ScanTokens( Token_Type *aTokens, int nLength, CString strCompare, long &lValue );
            static int          FixAMPM( int nHour, int nAMPM );
            static void         AdjustTime( tm_type &tmVals, long gmtoff );

    };
} // namespace IASLib
#endif // IASLIB_DATE_H__
