/*
 * CPUUsage Class
 *
 *  This class provides a quick means of getting stats for the
 * CPU on just how much CPU time you've used. Since this is such a 
 * platform-specific task, it seemed a good thing to make it a part
 * of the library.
 *  Note that this is a "snapshot" of the current status when the
 * object is created, for the whole process (every thread). To get the
 * time used between two snapshots, call TotalElapsed(), SysElapsed() or
 * UserElapsed() on one and pass the other: the result is the difference
 * whichever is the earlier. Microseconds are always below 1,000,000.
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
                // Microseconds. (They were seconds and microseconds in ints,
                // and the POSIX total's microseconds were never carried, so
                // they could reach 1,999,998.)
            long long   m_llTotal;
            long long   m_llUser;
            long long   m_llSys;

            static void Split( long long llMicroseconds, int &nSeconds, int &nMicroSeconds );
            static void Difference( long long llThis, long long llOther, int &nSeconds, int &nMicroSeconds );

        public:
                        CCPUUsage( void );
                        CCPUUsage( const CCPUUsage &oSource );
            virtual    ~CCPUUsage( void );

            CCPUUsage  &operator =( const CCPUUsage &oSource );

                        DEFINE_OBJECT( CCPUUsage )

            void        TotalElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const;
            void        SysElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const;
            void        UserElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const;

            void        GetTotalTime( int &nSec, int &nMicroSec ) const;
            void        GetUserTime( int &nSec, int &nMicroSec ) const;
            void        GetSystemTime( int &nSec, int &nMicroSec ) const;

                // The same, in microseconds.
            long long   GetTotalMicroseconds( void ) const { return m_llTotal; }
            long long   GetUserMicroseconds( void ) const { return m_llUser; }
            long long   GetSystemMicroseconds( void ) const { return m_llSys; }
    };
} // namespace IASLib

#endif // IASLIB_CPUUSAGE_H__
