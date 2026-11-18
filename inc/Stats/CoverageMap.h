/**
 * Coverage Map Class
 *
 *  This class provides a means of producing a map or snapshot of usage
 * of a limited resource based on the use of small, non-overlapping parts
 * of that range. For the main example, this is used to analyze memory
 * and disk fragmentation. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/31/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_COVERAGEMAP_H__
#define IASLIB_COVERAGEMAP_H__

#include <map>
#include "../BaseTypes/Object.h"
#include "../Streams/Stream.h"

namespace IASLib
{
        // Which parts of a range are in use: Allocate() and Release() mark
        // and clear runs [nBase, nBase + nSize), which are kept merged. The
        // size is the extent of everything ever allocated; the free space is
        // the gaps inside it. Dump() writes a row per block of the block size
        // with how much of it is used. Not thread-safe.
        //
        // (The previous version compared its blocks by pointer, so nothing
        // was ever found: Allocate() never merged, Release() never released,
        // the counters stayed 0, and Dump() wrote no data.)
    class CCoverageMap : public CObject
    {
        public:
            enum OutputFormatSpecifier
            {
                CSV         = 0x0001,
                XML         = 0x0002,
            };
        protected:
            std::map<size_t, size_t> m_mapRuns;     // start -> end (exclusive)
            size_t          m_nMinimum;
            size_t          m_nMaximum;
            size_t          m_nUsed;
            size_t          m_nBlockSize;
        public:
                            CCoverageMap( void );
            virtual        ~CCoverageMap( void );

                            DEFINE_OBJECT( CCoverageMap )

                // Marks a run used. True if none of it was in use already
                // (false for an empty run, or one that overlaps; the run is
                // marked either way).
            bool            Allocate( size_t nBase, size_t nSize );
                // Clears a run; with no size, from nBase to the end of the
                // used run it is in. True if anything was in use.
            bool            Release( size_t nBase, size_t nSize = IASLib::NOT_FOUND );
                // Whether every byte of a run is in use.
            bool            IsUsed( size_t nBase, size_t nSize = 1 ) const;

            size_t          GetSize( void ) const { return ( m_nMaximum > m_nMinimum ) ? m_nMaximum - m_nMinimum : 0; }
            size_t          GetUsed( void ) const { return m_nUsed; }
            size_t          GetFree( void ) const { return GetSize() - m_nUsed; }
                // The number of separate used runs.
            size_t          GetRunCount( void ) const { return m_mapRuns.size(); }
                // 0 when the free space is all in one gap, towards 1 as it is
                // split into many small ones (1 - largest gap / free space).
            double          GetFragmentation( void ) const;

            void            SetBlockSize( size_t nBlockSize ) { m_nBlockSize = ( nBlockSize ) ? nBlockSize : 1; }
            size_t          GetBlockSize( void ) const { return m_nBlockSize; }

                // Start, used and free bytes of each block, from the first
                // used block to the last; with bFillInGaps false, blocks with
                // nothing in use are left out.
            void            Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps = false );

        private:
            size_t          UsedIn( size_t nStart, size_t nEnd ) const;

                            CCoverageMap( const CCoverageMap & );
            CCoverageMap   &operator =( const CCoverageMap & );
    };
} // namespace IASLib

#endif // IASLIB_COVERAGEMAP_H__
