/*
 * Time Zone
 *
 *      A named time zone from the IANA time zone database (for example
 * "America/New_York"), read from the compiled zone files the system keeps in
 * $TZDIR or /usr/share/zoneinfo. A zone gives the offset from UTC in force at
 * any instant: from the recorded transitions, and after the last of them from
 * the zone's daylight saving rule. Zones are loaded once and shared; they are
 * immutable and safe to use from any thread.
 *
 *      Zone files with leap seconds (the "right/" zones) are not accepted,
 * since their times are not UTC.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: November 17, 2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TIMEZONE_H__
#define IASLIB_TIMEZONE_H__

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace IASLib
{
    class CTimeZone
    {
        public:
                // The zone with this IANA name, or NULL if there is no such zone
                // (or no zone database). Names are case-sensitive.
            static std::shared_ptr<const CTimeZone> Find( const char *pszName );

                // The zone's name, for example "America/New_York".
            const std::string  &GetName( void ) const { return m_strName; }

                // The offset in seconds east of UTC in force at an instant
                // (seconds since 1970 UTC).
            int                 OffsetAt( long long llUtcSeconds ) const;

                // The offset in force at a local time (seconds since 1970 on the
                // local clock). A local time that happened twice (clocks going
                // back) gives the earlier instant's offset; one that never
                // happened (clocks going forward) gives the offset from before the
                // change and sets bGap.
            void                OffsetForLocal( long long llLocalSeconds, int &nOffset, bool &bGap ) const;

                // The algorithm behind OffsetForLocal, for any source of offsets.
                // offsetAt returns false if it has no offset for an instant.
            static bool         ResolveLocal( long long llLocalSeconds, const std::function<bool( long long, int & )> &offsetAt, int &nOffset, bool &bGap );

        private:
            struct CRule;

            std::string                 m_strName;
            std::vector<long long>      m_aTransitions;     // UTC seconds, ascending
            std::vector<int>            m_aOffsets;         // offset after each transition
            int                         m_nInitialOffset;   // before the first transition
            std::shared_ptr<CRule>      m_pRule;            // after the last, if any

            CTimeZone( void );
            static std::shared_ptr<const CTimeZone> Load( const std::string &strName );
            bool                Parse( const std::vector<unsigned char> &aData );
            static std::shared_ptr<CRule> ParseRule( const std::string &strRule );
            static int          RuleOffsetAt( const CRule &rule, long long llUtcSeconds );
    };
} // namespace IASLib

#endif // IASLIB_TIMEZONE_H__
