/**
 *  Debugging Heap
 *
 *      An allocator over another one that finds memory bugs:
 *
 *      - fenceposts of zeros before and after each allocation, checked
 *        when it is freed (overruns and underruns);
 *      - a magic number, so freeing memory that did not come from here (or
 *        freeing it twice) is reported rather than corrupting the heap;
 *      - new memory is zeroed, and freed memory is filled with 'X' before
 *        it goes back, so use after free shows up;
 *      - with leak tracking on, every live allocation is listed with its
 *        type, file and line (IASLIB_NEW records those), and ReportLeaks()
 *        prints what is still allocated.
 *
 *      Problems go to CMemoryManager::ReportError(). Without leak tracking
 * it takes no lock. Leak tracking can be switched on and off at any time;
 * only allocations made while it is on are listed.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MEMORYDEBUGHEAP_H__
#define IASLIB_MEMORYDEBUGHEAP_H__

#include <stdio.h>
#include <atomic>
#include <mutex>
#include "MemoryManager.h"

namespace IASLib
{
    class CMemoryDebugHeap : public CMemoryManager
    {
        public:
            struct CDebugHeader;

        protected:
            CMemoryManager     *m_pUnderlying;
            std::atomic<bool>   m_bTrackLeaks;
            std::mutex          m_mutexTracked;
            CDebugHeader       *m_pTracked;         // live tracked allocations
            size_t              m_nTracked;
            size_t              m_nTrackedBytes;
            unsigned long       m_ulSequence;

        public:
                                // pUnderlying: where the memory comes from
                                // (NULL: malloc()).
                                CMemoryDebugHeap( CMemoryManager *pUnderlying = NULL, bool bTrackLeaks = false );
            virtual            ~CMemoryDebugHeap( void );

            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 );
            virtual void        Release( void *pMemory, size_t nSize );
            virtual const char *GetName( void ) const { return "debug"; }

            void                SetLeakTracking( bool bTrack ) { m_bTrackLeaks = bTrack; }
            bool                IsLeakTracking( void ) const { return m_bTrackLeaks; }
                // Tracked allocations still live, and their bytes.
            size_t              GetLeakCount( void );
            size_t              GetLeakBytes( void );
                // Prints each live tracked allocation ("<size> bytes as
                // <type> at <file>:<line>"), returning how many there were.
            size_t              ReportLeaks( FILE *pOut = stderr );
                // Checks the fenceposts of every live tracked allocation now,
                // returning how many are damaged.
            size_t              CheckAll( void );

                // The header and fence sizes, for tests.
            static size_t       FrontSize( void );
            static size_t       FenceSize( void );

        private:
            bool                CheckFences( const CDebugHeader *pHeader, const char *strWhen );

                                CMemoryDebugHeap( const CMemoryDebugHeap & );
            CMemoryDebugHeap   &operator =( const CMemoryDebugHeap & );
    };
} // namespace IASLib

#endif // IASLIB_MEMORYDEBUGHEAP_H__
