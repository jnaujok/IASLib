/**
 *  @file
 *  Memory Block
 *
 *      A chunk of memory that CMemoryFixedHeap carves into blocks of one
 * size: the chunk's header, then its blocks. Free blocks hold the link to
 * the next free block of the same chunk.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MEMORYBLOCK_H__
#define IASLIB_MEMORYBLOCK_H__

#include <stddef.h>

namespace IASLib
{
        /// The header at the start of each chunk of a CMemoryFixedHeap.
        ///
        /// The chunk's blocks follow the header (at HeaderSize()). Only
        /// CMemoryFixedHeap uses this, under its own lock.
        /// @note This was a 4096-byte array inside a \#pragma pack, and
        /// nothing used it.
    struct CMemoryBlock
    {
        CMemoryBlock   *pNextChunk;     ///< Next in the heap's list of chunks.
        CMemoryBlock   *pPrevChunk;     ///< Previous in the heap's list of chunks.
        CMemoryBlock   *pNextWithFree;  ///< Next in the list of chunks with free blocks.
        CMemoryBlock   *pPrevWithFree;  ///< Previous in the list of chunks with free blocks.
        void           *pFreeList;      ///< This chunk's free blocks (NULL: none).
        size_t          nUsed;          ///< Blocks handed out.
        size_t          nBlocks;        ///< Blocks in the chunk.
        char           *pFirstBlock;    ///< The first block, just after the header.

            /// The header's size, rounded up so the blocks are aligned for any type.
        static size_t   HeaderSize( void ) { return ( sizeof( CMemoryBlock ) + 15 ) & ~(size_t)15; }
    };
} // namespace IASLib

#endif // IASLIB_MEMORYBLOCK_H__
