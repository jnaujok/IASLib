/**
 *  @file
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
        /// An allocator that checks for overruns, bad frees and leaks.
        ///
        /// Each allocation gets a header and front fencepost before it and
        /// a back fencepost after it (FrontSize() and FenceSize()); see the
        /// file description for the checks. Problems are reported through
        /// CMemoryManager::ReportError(), never thrown. Thread-safe. Not
        /// copyable.
        ///
        /// @code
        /// CMemoryDebugHeap heap( NULL, true );     // over malloc(), tracking leaks
        /// CMemoryManager *pOld = CMemoryManager::Set( &heap );
        /// RunTests();
        /// CMemoryManager::Set( pOld );
        /// if ( heap.ReportLeaks() ) { ... }
        /// @endcode
    class CMemoryDebugHeap : public CMemoryManager
    {
        public:
            /// The bookkeeping before each allocation (defined in the source file).
            struct CDebugHeader;

        protected:
            CMemoryManager     *m_pUnderlying;      ///< Where the memory comes from (not owned).
            std::atomic<bool>   m_bTrackLeaks;      ///< True: new allocations are tracked.
            std::mutex          m_mutexTracked;     ///< Guards the tracked list and its counts.
            CDebugHeader       *m_pTracked;         ///< Live tracked allocations, newest first.
            size_t              m_nTracked;         ///< How many allocations are tracked.
            size_t              m_nTrackedBytes;    ///< Their bytes, as asked for.
            unsigned long       m_ulSequence;       ///< Number given to the last tracked allocation.

        public:
                                /// Creates a debugging heap.
                                /// @param pUnderlying Where the memory comes from
                                ///                    (NULL: malloc()). Not owned; it must outlive this heap.
                                /// @param bTrackLeaks True to track allocations from the start.
                                CMemoryDebugHeap( CMemoryManager *pUnderlying = NULL, bool bTrackLeaks = false );
                                /// Destroys the heap. Memory still allocated is not freed or reported.
            virtual            ~CMemoryDebugHeap( void );

                /// Allocates nSize zeroed bytes between fenceposts.
                /// @param nSize   The bytes wanted.
                /// @param strType The type allocated (kept for reports; may be NULL).
                /// @param strFile The source file of the request (may be NULL).
                /// @param nLine   The source line of the request.
                /// @return The memory, or NULL if the underlying allocator had none
                ///         (or the size is too large).
            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 );
                /// Checks and frees memory from Allocate().
                /// Reports a bad magic number (not ours, or freed twice; the
                /// memory is then left alone), a size that differs from the
                /// one allocated, and damaged fenceposts. The memory is filled
                /// with 'X' before it goes back.
                /// @param pMemory The memory (NULL does nothing).
                /// @param nSize   The size it was allocated with.
            virtual void        Release( void *pMemory, size_t nSize );
            /// "debug".
            virtual const char *GetName( void ) const { return "debug"; }

            /// Turns leak tracking on or off for allocations made from now on.
            void                SetLeakTracking( bool bTrack ) { m_bTrackLeaks = bTrack; }
            /// True if new allocations are being tracked.
            bool                IsLeakTracking( void ) const { return m_bTrackLeaks; }
                /// Tracked allocations still live.
            size_t              GetLeakCount( void );
                /// The bytes of the tracked allocations still live.
            size_t              GetLeakBytes( void );
                /// Prints each live tracked allocation ("Leak #&lt;n&gt;: &lt;size&gt;
                /// bytes as &lt;type&gt; at &lt;file&gt;:&lt;line&gt;").
                /// @param pOut Where to print (NULL: only count).
                /// @return How many there were.
            size_t              ReportLeaks( FILE *pOut = stderr );
                /// Checks the fenceposts of every live tracked allocation now.
                /// Damage is reported through ReportError().
                /// @return How many are damaged.
            size_t              CheckAll( void );

                /// The bytes before each allocation (header and front fence), for tests.
            static size_t       FrontSize( void );
                /// The size of the back fencepost, for tests.
            static size_t       FenceSize( void );

        private:
                                /// Reports damage to either fencepost of an allocation;
                                /// false if any was found.
            bool                CheckFences( const CDebugHeader *pHeader, const char *strWhen );

                                /// Not copyable.
                                CMemoryDebugHeap( const CMemoryDebugHeap & );
                                /// Not assignable.
            CMemoryDebugHeap   &operator =( const CMemoryDebugHeap & );
    };
} // namespace IASLib

#endif // IASLIB_MEMORYDEBUGHEAP_H__
