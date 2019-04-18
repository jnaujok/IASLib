/*
 * CPUUsage Class
 *
 *  This class provides a quick means of getting stats for the
 * CPU on just how much CPU time you've used. Since this is such a 
 * platform-specific task, it seemed a good thing to make it a part
 * of the library.
 *  Note that this is a "snapshot" of the current status when the 
 * object is created. To get elapsed time, use the Elapsed function
 * and pass it the previous or the current snapshot to get the 
 * difference.
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

    CCPUUsage::CCPUUsage( void )
    {
#ifdef IASLIB_WIN32__
        FILETIME        CreateTime;
        FILETIME        ExitTime;
        FILETIME        KernelTime;
        FILETIME        UserTime;
        ULARGE_INTEGER  nTemp;

        GetProcessTimes( GetCurrentProcess(), &CreateTime, &ExitTime, &KernelTime, &UserTime );

        nTemp.HighPart = KernelTime.dwHighDateTime;
        nTemp.LowPart = KernelTime.dwLowDateTime;

        m_nSysSeconds = (int)(nTemp.QuadPart / 10000000);
        m_nSysuSeconds = (int)(( nTemp.QuadPart % 10000000 ) / 10);

        nTemp.HighPart = UserTime.dwHighDateTime;
        nTemp.LowPart = UserTime.dwLowDateTime;

        m_nUserSeconds = (int)(nTemp.QuadPart / 10000000);
        m_nUseruSeconds = (int)(( nTemp.QuadPart % 10000000 ) / 10);

        m_nTotalSeconds = m_nSysSeconds + m_nUserSeconds;
        m_nTotaluSeconds = m_nSysuSeconds + m_nUseruSeconds;

        while ( m_nTotaluSeconds > 1000000 )
        {
            m_nTotalSeconds++;
            m_nTotaluSeconds -= 1000000;
        }
#else
        struct rusage stUsageData;

        getrusage( RUSAGE_SELF, &stUsageData );

        m_nTotalSeconds = stUsageData.ru_utime.tv_sec + stUsageData.ru_stime.tv_sec;
        m_nTotaluSeconds = stUsageData.ru_utime.tv_usec + stUsageData.ru_stime.tv_usec;
        m_nUserSeconds = stUsageData.ru_utime.tv_sec;
        m_nUseruSeconds = stUsageData.ru_utime.tv_usec;
        m_nSysSeconds = stUsageData.ru_stime.tv_sec;
        m_nSysuSeconds = stUsageData.ru_stime.tv_usec;
#endif
    }

    CCPUUsage::CCPUUsage( const CCPUUsage &oSource )
    {
        m_nTotalSeconds = oSource.m_nTotalSeconds;
        m_nTotaluSeconds = oSource.m_nTotaluSeconds;
        m_nUserSeconds = oSource.m_nUserSeconds;
        m_nUseruSeconds = oSource.m_nUseruSeconds;
        m_nSysSeconds = oSource.m_nSysSeconds;
        m_nSysuSeconds = oSource.m_nSysuSeconds;
    }

    CCPUUsage::~CCPUUsage( void )
    {
    }

    CCPUUsage &CCPUUsage::operator =( const CCPUUsage &oSource )
    {
        if ( this != &oSource )
        {
            m_nTotalSeconds = oSource.m_nTotalSeconds;
            m_nTotaluSeconds = oSource.m_nTotaluSeconds;
            m_nUserSeconds = oSource.m_nUserSeconds;
            m_nUseruSeconds = oSource.m_nUseruSeconds;
            m_nSysSeconds = oSource.m_nSysSeconds;
            m_nSysuSeconds = oSource.m_nSysuSeconds;
        }
        return *this;
    }

    void CCPUUsage::TotalElapsed( CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds )
    {
        if ( ( oPrevious.m_nTotalSeconds > m_nTotalSeconds ) ||
            ( ( oPrevious.m_nTotalSeconds == m_nTotalSeconds ) && ( oPrevious.m_nTotaluSeconds > m_nTotaluSeconds ) ) )
        {
            nSeconds = oPrevious.m_nTotalSeconds - m_nTotalSeconds;
            nMicroSeconds = oPrevious.m_nTotaluSeconds - m_nTotaluSeconds;
        }
        else
        {
            nSeconds = m_nTotalSeconds - oPrevious.m_nTotalSeconds;
            nMicroSeconds = m_nTotaluSeconds - oPrevious.m_nTotaluSeconds;
        }

        while ( nMicroSeconds < 0 )
        {
            nSeconds--;
            nMicroSeconds += 1000000;
        }
    }

    void CCPUUsage::SysElapsed( CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds )
    {
        if ( ( oPrevious.m_nSysSeconds > m_nSysSeconds ) ||
            ( ( oPrevious.m_nSysSeconds == m_nSysSeconds ) && ( oPrevious.m_nSysuSeconds > m_nSysuSeconds ) ) )
        {
            nSeconds = oPrevious.m_nSysSeconds - m_nSysSeconds;
            nMicroSeconds = oPrevious.m_nSysuSeconds - m_nSysuSeconds;
        }
        else
        {
            nSeconds = m_nSysSeconds - oPrevious.m_nSysSeconds;
            nMicroSeconds = m_nSysuSeconds - oPrevious.m_nSysuSeconds;
        }

        while ( nMicroSeconds < 0 )
        {
            nSeconds--;
            nMicroSeconds += 1000000;
        }
    }

    void CCPUUsage::UserElapsed( CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds )
    {
        if ( ( oPrevious.m_nUserSeconds > m_nUserSeconds ) ||
            ( ( oPrevious.m_nUserSeconds == m_nUserSeconds ) && ( oPrevious.m_nUseruSeconds > m_nUseruSeconds ) ) )
        {
            nSeconds = oPrevious.m_nUserSeconds - m_nUserSeconds;
            nMicroSeconds = oPrevious.m_nUseruSeconds - m_nUseruSeconds;
        }
        else
        {
            nSeconds = m_nUserSeconds - oPrevious.m_nUserSeconds;
            nMicroSeconds = m_nUseruSeconds - oPrevious.m_nUseruSeconds;
        }

        while ( nMicroSeconds < 0 )
        {
            nSeconds--;
            nMicroSeconds += 1000000;
        }
    }

    void CCPUUsage::GetTotalTime( int &nSec, int &nMicroSec )
    {
        nSec = m_nTotalSeconds;
        nMicroSec = m_nTotaluSeconds;
    }

    void CCPUUsage::GetUserTime( int &nSec, int &nMicroSec )
    {
        nSec = m_nUserSeconds;
        nMicroSec = m_nUseruSeconds;
    }

    void CCPUUsage::GetSystemTime( int &nSec, int &nMicroSec )
    {
        nSec = m_nSysSeconds;
        nMicroSec = m_nSysuSeconds;
    }

} // namespace IASLib

