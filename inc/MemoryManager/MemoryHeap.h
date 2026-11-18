/**
 *  @file
 *  Pooled Heap
 *
 *      A general allocator built on CMemoryFixedHeap: requests up to 512
 * bytes come from pools of 16, 32, 64, 128, 256 and 512-byte blocks;
 * larger ones go to the fallback (malloc() by default). Thread-safe.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MEMORYHEAP_H__
#define IASLIB_MEMORYHEAP_H__

#include "MemoryFixedHeap.h"

namespace IASLib
{
        /// A general allocator: pools for small sizes, the fallback for large ones.
        ///
        /// Each request goes to the smallest pool whose blocks fit it. The
        /// size given to Release() must be the size asked for, since it
        /// picks the pool. Thread-safe. Not copyable.
        ///
        /// @code
        /// CMemoryHeap heap;
        /// CMemoryManager::Set( &heap );     // new CObjects come from the pools
        /// @endcode
    class CMemoryHeap : public CMemoryManager
    {
        public:
            static const size_t SIZE_CLASSES = 6;       ///< How many pools (16 to 512 bytes).
            static const size_t LARGEST_POOLED = 512;   ///< The largest request served from a pool.

        protected:
            CMemoryManager     *m_pFallback;                ///< Takes larger requests and supplies the pools' chunks (not owned).
            CMemoryFixedHeap   *m_apPools[ SIZE_CLASSES ];  ///< The pools, smallest first (owned).

        public:
                                /// Creates the pools.
                                /// @param nBlocksPerChunk Blocks in each chunk of every pool.
                                /// @param pFallback       Takes larger requests and supplies the chunks
                                ///                        (NULL: malloc()). Not owned.
                                CMemoryHeap( size_t nBlocksPerChunk = 128, CMemoryManager *pFallback = NULL );
                                /// Deletes the pools, giving back every chunk whatever is in use.
            virtual            ~CMemoryHeap( void );

                /// Memory from the right pool, or from the fallback for large sizes.
                /// @param nSize   The bytes wanted.
                /// @param strType Describes the request; passed on.
                /// @param strFile Describes the request; passed on.
                /// @param nLine   Describes the request; passed on.
                /// @return The memory, or NULL if there is none.
            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 );
                /// Gives back memory from Allocate().
                /// @param pMemory The memory (NULL does nothing).
                /// @param nSize   The size it was asked for.
            virtual void        Release( void *pMemory, size_t nSize );
            /// "pooled".
            virtual const char *GetName( void ) const { return "pooled"; }

                /// The pool a size is served from (NULL: the fallback).
            CMemoryFixedHeap   *PoolFor( size_t nSize ) const;
                /// Compacts every pool (see CMemoryFixedHeap::Compact()).
                /// @return The chunks freed.
            size_t              Compact( void );

        private:
                                /// Not copyable.
                                CMemoryHeap( const CMemoryHeap & );
                                /// Not assignable.
            CMemoryHeap        &operator =( const CMemoryHeap & );
    };
} // namespace IASLib

#endif // IASLIB_MEMORYHEAP_H__
