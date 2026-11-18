/*
 * CPUUsage Class
 *
 *  This class provides a quick means of getting stats for the
 * CPU on just how much CPU time you've used. Since this is such a 
 * platform-specific task, it seemed a good thing to make it a part
 * of the library.
 *  Note that this is a "snapshot" of the current status when the
 * object is created. See CPUUsage.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2/24/2005
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "CPUUsage.h"

#ifndef IASLIB_WIN32__
#include <sys/time.h>
#include <sys/resource.h>
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CCPUUsage, CObject );

    CCPUUsage::CCPUUsage( void )
    {
        m_llTotal = 0;
        m_llUser = 0;
        m_llSys = 0;
#ifdef IASLIB_WIN32__
        FILETIME        CreateTime;
        FILETIME        ExitTime;
        FILETIME        KernelTime;
        FILETIME        UserTime;

            // (Neither call was checked, which left the fields undefined.)
        if ( GetProcessTimes( GetCurrentProcess(), &CreateTime, &ExitTime, &KernelTime, &UserTime ) )
        {
            ULARGE_INTEGER  nTemp;      // 100-nanosecond units

            nTemp.HighPart = KernelTime.dwHighDateTime;
            nTemp.LowPart = KernelTime.dwLowDateTime;
            m_llSys = (long long)( nTemp.QuadPart / 10 );

            nTemp.HighPart = UserTime.dwHighDateTime;
            nTemp.LowPart = UserTime.dwLowDateTime;
            m_llUser = (long long)( nTemp.QuadPart / 10 );
        }
#else
        struct rusage stUsageData;

        if ( getrusage( RUSAGE_SELF, &stUsageData ) == 0 )
        {
            m_llUser = (long long)stUsageData.ru_utime.tv_sec * 1000000 + stUsageData.ru_utime.tv_usec;
            m_llSys = (long long)stUsageData.ru_stime.tv_sec * 1000000 + stUsageData.ru_stime.tv_usec;
        }
#endif
        m_llTotal = m_llUser + m_llSys;
    }

    CCPUUsage::CCPUUsage( const CCPUUsage &oSource ) : CObject()
    {
        m_llTotal = oSource.m_llTotal;
        m_llUser = oSource.m_llUser;
        m_llSys = oSource.m_llSys;
    }

    CCPUUsage::~CCPUUsage( void )
    {
    }

    CCPUUsage &CCPUUsage::operator =( const CCPUUsage &oSource )
    {
        m_llTotal = oSource.m_llTotal;
        m_llUser = oSource.m_llUser;
        m_llSys = oSource.m_llSys;
        return *this;
    }

    void CCPUUsage::Split( long long llMicroseconds, int &nSeconds, int &nMicroSeconds )
    {
        nSeconds = (int)( llMicroseconds / 1000000 );
        nMicroSeconds = (int)( llMicroseconds % 1000000 );
    }

    void CCPUUsage::Difference( long long llThis, long long llOther, int &nSeconds, int &nMicroSeconds )
    {
        Split( ( llThis >= llOther ) ? llThis - llOther : llOther - llThis, nSeconds, nMicroSeconds );
    }

    void CCPUUsage::TotalElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const
    {
        Difference( m_llTotal, oPrevious.m_llTotal, nSeconds, nMicroSeconds );
    }

    void CCPUUsage::SysElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const
    {
        Difference( m_llSys, oPrevious.m_llSys, nSeconds, nMicroSeconds );
    }

    void CCPUUsage::UserElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const
    {
        Difference( m_llUser, oPrevious.m_llUser, nSeconds, nMicroSeconds );
    }

    void CCPUUsage::GetTotalTime( int &nSec, int &nMicroSec ) const
    {
        Split( m_llTotal, nSec, nMicroSec );
    }

    void CCPUUsage::GetUserTime( int &nSec, int &nMicroSec ) const
    {
        Split( m_llUser, nSec, nMicroSec );
    }

    void CCPUUsage::GetSystemTime( int &nSec, int &nMicroSec ) const
    {
        Split( m_llSys, nSec, nMicroSec );
    }
} // namespace IASLib
