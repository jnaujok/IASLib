/**
 *  @file
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
        /// A pool of blocks of one size.
        ///
        /// Chunks of blocks come from the fallback allocator; each block
        /// is aligned to 16 bytes. Requests larger than the block size are
        /// passed straight to the fallback. Thread-safe (one mutex).
        ///
        /// @code
        /// CMemoryFixedHeap heap( sizeof( CNode ), 1024 );
        /// void *p = heap.Allocate( sizeof( CNode ) );
        /// heap.Release( p, sizeof( CNode ) );
        /// heap.Compact();       // gives the empty chunk back
        /// @endcode
    class CMemoryFixedHeap : public CMemoryManager
    {
        protected:
            size_t              m_nBlockSize;           ///< Bytes per block (a multiple of 16).
            size_t              m_nBlocksPerChunk;      ///< Blocks in each new chunk.
            CMemoryManager     *m_pFallback;            ///< Supplies chunks and large requests (not owned).
            CMemoryBlock       *m_pChunks;              ///< Every chunk.
            CMemoryBlock       *m_pChunksWithFree;      ///< Chunks with at least one free block.
            size_t              m_nChunks;              ///< How many chunks there are.
            std::map<char *, CMemoryBlock *> m_mapChunks;   ///< Chunks by first block, to find a block's chunk.
            mutable std::mutex  m_mutex;                ///< Guards everything above.

        public:
                                /// Creates an empty pool (no chunk is allocated yet).
                                /// @param nBlockSize      The block size; rounded up to a multiple of 16
                                ///                        (and to at least a pointer).
                                /// @param nBlocksPerChunk Blocks in each chunk (0 is treated as 1).
                                /// @param pFallback       Takes larger requests and supplies the
                                ///                        chunks (NULL: malloc()). Not owned.
                                CMemoryFixedHeap( size_t nBlockSize, size_t nBlocksPerChunk = 256, CMemoryManager *pFallback = NULL );
                                /// Gives every chunk back, whatever is in use.
            virtual            ~CMemoryFixedHeap( void );

                /// A block, or the fallback's memory if nSize is larger than a block.
                /// @param nSize   The bytes wanted.
                /// @param strType Passed on to the fallback for large requests.
                /// @param strFile Passed on to the fallback for large requests.
                /// @param nLine   Passed on to the fallback for large requests.
                /// @return The memory, or NULL if a new chunk (or the large
                ///         request) could not be allocated.
            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 );
                /// Gives back memory from Allocate().
                /// A pointer that is not a block of this heap, or a block of a
                /// chunk with nothing in use, is reported through
                /// ReportError() and not freed.
                /// @param pMemory The memory (NULL does nothing).
                /// @param nSize   The size it was asked for (decides block or fallback).
            virtual void        Release( void *pMemory, size_t nSize );
            /// "fixed".
            virtual const char *GetName( void ) const { return "fixed"; }

            /// The block size (after rounding).
            size_t              GetBlockSize( void ) const { return m_nBlockSize; }
                /// How many chunks the pool holds.
            size_t              GetChunkCount( void ) const;
                /// How many blocks are handed out.
            size_t              GetBlocksInUse( void ) const;
                /// How many blocks are free in the chunks held.
            size_t              GetFreeBlocks( void ) const;
                /// Whether p is in one of this heap's chunks.
            bool                Owns( const void *p ) const;
                /// Frees every chunk with no blocks in use.
                /// @return How many chunks were freed.
            size_t              Compact( void );

        private:
            /// Allocates a chunk and threads its free list (NULL if there is no memory).
            CMemoryBlock       *NewChunk( void );
            /// Unlinks a chunk and gives it back to the fallback.
            void                FreeChunk( CMemoryBlock *pChunk );
            /// The chunk p lies in (NULL: none).
            CMemoryBlock       *ChunkOf( const void *p ) const;
            /// Puts a chunk on the list of chunks with free blocks.
            void                LinkWithFree( CMemoryBlock *pChunk );
            /// Takes a chunk off the list of chunks with free blocks.
            void                UnlinkWithFree( CMemoryBlock *pChunk );
    };
} // namespace IASLib

#endif // IASLIB_MEMORYFIXEDHEAP_H__
