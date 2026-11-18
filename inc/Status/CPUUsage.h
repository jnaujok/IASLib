/**
 * @file CPUUsage.h
 * @brief CCPUUsage, a snapshot of the CPU time the process has used.
 *
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
    /**
     * A snapshot of the CPU time the process has used.
     *
     * The snapshot is taken when the object is made, for the whole process
     * (every thread): user time, system (kernel) time and their total, from
     * getrusage() or, on Windows, GetProcessTimes() (all zero if the call
     * fails). To get the time used between two snapshots, call
     * TotalElapsed(), SysElapsed() or UserElapsed() on one and pass the
     * other: the result is the difference whichever is the earlier.
     * Microseconds are always below 1,000,000.
     *
     * @code
     * CCPUUsage before;
     * DoWork();
     * CCPUUsage after;
     * int nSec, nMicro;
     * after.TotalElapsed( before, nSec, nMicro );
     * printf( "%d.%06d s of CPU\n", nSec, nMicro );
     * @endcode
     */
    class CCPUUsage : public CObject
    {
        protected:
                /// User plus system time, in microseconds.
                ///
                /// The times are in microseconds. (They were seconds and
                /// microseconds in ints, and the POSIX total's microseconds were
                /// never carried, so they could reach 1,999,998.)
            long long   m_llTotal;
            long long   m_llUser;   ///< User time, in microseconds.
            long long   m_llSys;    ///< System (kernel) time, in microseconds.

                /// Splits microseconds into seconds and microseconds.
            static void Split( long long llMicroseconds, int &nSeconds, int &nMicroSeconds );
                /// Splits the difference between two times (whichever is
                /// larger) into seconds and microseconds.
            static void Difference( long long llThis, long long llOther, int &nSeconds, int &nMicroSeconds );

        public:
                        /// A snapshot of the CPU time used so far.
                        CCPUUsage( void );
                        /// A copy of another snapshot.
                        CCPUUsage( const CCPUUsage &oSource );
            virtual    ~CCPUUsage( void );

                        /// Copies another snapshot's times.
            CCPUUsage  &operator =( const CCPUUsage &oSource );

                        DEFINE_OBJECT( CCPUUsage )

                /// The total CPU time between this snapshot and oPrevious
                /// (either may be the earlier).
                /// @param oPrevious The other snapshot.
                /// @param nSeconds Receives the whole seconds.
                /// @param nMicroSeconds Receives the microseconds (below 1,000,000).
            void        TotalElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const;
                /// The system time between this snapshot and oPrevious (see
                /// TotalElapsed()).
            void        SysElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const;
                /// The user time between this snapshot and oPrevious (see
                /// TotalElapsed()).
            void        UserElapsed( const CCPUUsage &oPrevious, int &nSeconds, int &nMicroSeconds ) const;

                /// The total CPU time at the snapshot, in seconds and microseconds.
            void        GetTotalTime( int &nSec, int &nMicroSec ) const;
                /// The user time at the snapshot, in seconds and microseconds.
            void        GetUserTime( int &nSec, int &nMicroSec ) const;
                /// The system time at the snapshot, in seconds and microseconds.
            void        GetSystemTime( int &nSec, int &nMicroSec ) const;

                /// The total CPU time at the snapshot, in microseconds.
            long long   GetTotalMicroseconds( void ) const { return m_llTotal; }
                /// The user time at the snapshot, in microseconds.
            long long   GetUserMicroseconds( void ) const { return m_llUser; }
                /// The system time at the snapshot, in microseconds.
            long long   GetSystemMicroseconds( void ) const { return m_llSys; }
    };
} // namespace IASLib

#endif // IASLIB_CPUUSAGE_H__
