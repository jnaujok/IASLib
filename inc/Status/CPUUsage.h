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

#ifndef IASLIB_CPUUSAGE_H__
#define IASLIB_CPUUSAGE_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
    class CCPUUsage : public CObject
    {
        protected:
            int         m_nTotalSeconds;
            int         m_nTotaluSeconds;
            int         m_nUserSeconds;
            int         m_nUseruSeconds;
            int         m_nSysSeconds;
            int         m_nSysuSeconds;

        public:
                        CCPUUsage( void );
                        CCPUUsage( const CCPUUsage &oSource );
            virtual    ~CCPUUsage( void );

            CCPUUsage  &operator =( const CCPUUsage &oSource );

                        DECLARE_OBJECT( CCPUUsage, CObject );

            void        TotalElapsed( CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds );
            void        SysElapsed( CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds );
            void        UserElapsed( CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds );

            void        GetTotalTime( int &nSec, int &nMicroSec );
            void        GetUserTime( int &nSec, int &nMicroSec );
            void        GetSystemTime( int &nSec, int &nMicroSec );
    };
} // namespace IASLib

#endif // IASLIB_CPUUSAGE_H__
