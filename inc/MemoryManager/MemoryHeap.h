/**
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
    class CMemoryHeap : public CMemoryManager
    {
        public:
            static const size_t SIZE_CLASSES = 6;
            static const size_t LARGEST_POOLED = 512;

        protected:
            CMemoryManager     *m_pFallback;
            CMemoryFixedHeap   *m_apPools[ SIZE_CLASSES ];

        public:
                                CMemoryHeap( size_t nBlocksPerChunk = 128, CMemoryManager *pFallback = NULL );
            virtual            ~CMemoryHeap( void );

            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 );
            virtual void        Release( void *pMemory, size_t nSize );
            virtual const char *GetName( void ) const { return "pooled"; }

                // The pool a size is served from (NULL: the fallback).
            CMemoryFixedHeap   *PoolFor( size_t nSize ) const;
                // Compacts every pool; returns the chunks freed.
            size_t              Compact( void );

        private:
                                CMemoryHeap( const CMemoryHeap & );
            CMemoryHeap        &operator =( const CMemoryHeap & );
    };
} // namespace IASLib

#endif // IASLIB_MEMORYHEAP_H__
