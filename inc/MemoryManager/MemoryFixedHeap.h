/**
 *  Fixed-size Heap
 *
 *      A pool of blocks of one size, for programs that make and free many
 * small objects of the same size. Memory is taken from the system a chunk
 * of blocks at a time; allocating and freeing a block are constant time.
 * Requests larger than the block size go to the fallback allocator
 * (malloc() by default). Compact() returns chunks with no blocks in use.
 * Thread-safe.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MEMORYFIXEDHEAP_H__
#define IASLIB_MEMORYFIXEDHEAP_H__

#include <map>
#include <mutex>
#include "MemoryManager.h"
#include "MemoryBlock.h"

namespace IASLib
{
    class CMemoryFixedHeap : public CMemoryManager
    {
        protected:
            size_t              m_nBlockSize;
            size_t              m_nBlocksPerChunk;
            CMemoryManager     *m_pFallback;
            CMemoryBlock       *m_pChunks;
            CMemoryBlock       *m_pChunksWithFree;
            size_t              m_nChunks;
            std::map<char *, CMemoryBlock *> m_mapChunks;   // by first block
            mutable std::mutex  m_mutex;

        public:
                                // nBlockSize is rounded up to a multiple of 16.
                                // pFallback (NULL: malloc()) takes larger
                                // requests and supplies the chunks.
                                CMemoryFixedHeap( size_t nBlockSize, size_t nBlocksPerChunk = 256, CMemoryManager *pFallback = NULL );
                                // Every chunk goes back, whatever is in use.
            virtual            ~CMemoryFixedHeap( void );

            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 );
            virtual void        Release( void *pMemory, size_t nSize );
            virtual const char *GetName( void ) const { return "fixed"; }

            size_t              GetBlockSize( void ) const { return m_nBlockSize; }
            size_t              GetChunkCount( void ) const;
            size_t              GetBlocksInUse( void ) const;
            size_t              GetFreeBlocks( void ) const;
                // Whether p is a block of this heap.
            bool                Owns( const void *p ) const;
                // Frees every chunk with no blocks in use; returns how many.
            size_t              Compact( void );

        private:
            CMemoryBlock       *NewChunk( void );
            void                FreeChunk( CMemoryBlock *pChunk );
            CMemoryBlock       *ChunkOf( const void *p ) const;
            void                LinkWithFree( CMemoryBlock *pChunk );
            void                UnlinkWithFree( CMemoryBlock *pChunk );
    };
} // namespace IASLib

#endif // IASLIB_MEMORYFIXEDHEAP_H__
