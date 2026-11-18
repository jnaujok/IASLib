/**
 * @file TimeZone.h
 * @brief CTimeZone, a named time zone from the IANA time zone database.
 *
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
 *      Windows has no zone files. There, zones not found in $TZDIR (which
 * can point at zone files shipped with a program) come from the copy of the
 * IANA data in Windows' own ICU library (icu.dll, Windows 10 version 1903 and
 * later; icuin.dll on some earlier versions of Windows 10), loaded when first
 * needed. Without either, named zones are not found. Other systems can use
 * ICU too when built with IASLIB_TIMEZONE_ICU__ (CMake USE_TIMEZONE_ICU),
 * which is mainly for testing.
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
    /**
     * A named time zone from the IANA time zone database.
     *
     * A zone gives the offset from UTC in force at any instant: from the
     * recorded transitions, and after the last of them from the zone's
     * daylight saving rule. Zones come from the system's zone files (or ICU
     * on Windows), are loaded once and shared through std::shared_ptr, and
     * are immutable, so they are safe to use from any thread. CDate uses
     * this class for its named zones.
     *
     * @code
     * std::shared_ptr<const CTimeZone> pZone = CTimeZone::Find( "America/New_York" );
     * if ( pZone )
     *     printf( "%d\n", pZone->OffsetAt( 0 ) );   // -18000 (EST, on 1 January 1970)
     * @endcode
     */
    class CTimeZone
    {
        public:
                /// The zone with this IANA name, or NULL if there is no such zone
                /// (or no zone database). Names are case-sensitive.
            static std::shared_ptr<const CTimeZone> Find( const char *pszName );

                /// The zone from ICU only, or NULL if ICU is not available or has
                /// no such zone. Find() uses this where there are no zone files.
            static std::shared_ptr<const CTimeZone> FindInICU( const char *pszName );
                /// True if this build can use ICU and the library is present.
            static bool         IsICUAvailable( void );

                /// The system's zone, as the C library would choose it: the zone
                /// TZ names (an IANA name, a file path, or a POSIX rule such as
                /// "EST5EDT,M3.2.0,M11.1.0"), or /etc/localtime if TZ is not set.
                /// NULL if it cannot be read, as on Windows without TZ, where the
                /// C library's own zone is used. Read again when TZ changes.
            static std::shared_ptr<const CTimeZone> System( void );
                /// A zone read from a compiled zone file (TZif).
                /// @param pszPath The file.
                /// @param pszName The zone's name; NULL uses the path.
                /// @return The zone, or NULL if the file cannot be read or is not a zone file.
            static std::shared_ptr<const CTimeZone> FromFile( const char *pszPath, const char *pszName );
                /// A zone from a POSIX TZ rule ("EST5EDT,M3.2.0,M11.1.0"), named
                /// after the rule.
                /// @return The zone, or NULL if the rule cannot be read.
            static std::shared_ptr<const CTimeZone> FromRule( const char *pszRule );

                /// The zone's name, for example "America/New_York".
            const std::string  &GetName( void ) const { return m_strName; }

                /// The offset in seconds east of UTC in force at an instant
                /// (seconds since 1970 UTC).
            int                 OffsetAt( long long llUtcSeconds ) const;

                /// The offset in force at a local time (seconds since 1970 on the
                /// local clock). A local time that happened twice (clocks going
                /// back) gives the earlier instant's offset; one that never
                /// happened (clocks going forward) gives the offset from before the
                /// change and sets bGap.
            void                OffsetForLocal( long long llLocalSeconds, int &nOffset, bool &bGap ) const;

                /// The algorithm behind OffsetForLocal, for any source of offsets.
                /// offsetAt returns false if it has no offset for an instant.
                /// @return false if offsetAt had no offset.
            static bool         ResolveLocal( long long llLocalSeconds, const std::function<bool( long long, int & )> &offsetAt, int &nOffset, bool &bGap );

        private:
            struct CRule;       ///< A POSIX TZ daylight saving rule.
            struct CICUZone;    ///< An ICU calendar, for a zone from ICU.

            std::string                 m_strName;          ///< The zone's name.
            std::vector<long long>      m_aTransitions;     ///< UTC seconds, ascending.
            std::vector<int>            m_aOffsets;         ///< Offset after each transition.
            int                         m_nInitialOffset;   ///< Before the first transition.
            std::shared_ptr<CRule>      m_pRule;            ///< After the last, if any.
            std::shared_ptr<CICUZone>   m_pICU;             ///< Set for a zone from ICU.

            /// An empty zone (UTC); made only by the loaders.
            CTimeZone( void );
                /// Reads the named zone from the zone file directories.
            static std::shared_ptr<const CTimeZone> Load( const std::string &strName );
                /// Reads the named zone from ICU.
            static std::shared_ptr<const CTimeZone> LoadICU( const std::string &strName );
                /// Reads a compiled zone file's contents; false if it is not one.
            bool                Parse( const std::vector<unsigned char> &aData );
                /// Reads a POSIX TZ rule; NULL if it is not one.
            static std::shared_ptr<CRule> ParseRule( const std::string &strRule );
                /// The offset a rule gives at an instant.
            static int          RuleOffsetAt( const CRule &rule, long long llUtcSeconds );
    };
} // namespace IASLib

#endif // IASLIB_TIMEZONE_H__
