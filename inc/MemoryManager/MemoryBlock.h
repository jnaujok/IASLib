/**
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
        // (This was a 4096-byte array inside a #pragma pack, and nothing
        // used it.)
    struct CMemoryBlock
    {
        CMemoryBlock   *pNextChunk;     // the heap's list of chunks
        CMemoryBlock   *pPrevChunk;
        CMemoryBlock   *pNextWithFree;  // the list of chunks with free blocks
        CMemoryBlock   *pPrevWithFree;
        void           *pFreeList;      // this chunk's free blocks
        size_t          nUsed;          // blocks handed out
        size_t          nBlocks;
        char           *pFirstBlock;

            // The header, rounded up so the blocks are aligned for any type.
        static size_t   HeaderSize( void ) { return ( sizeof( CMemoryBlock ) + 15 ) & ~(size_t)15; }
    };
} // namespace IASLib

#endif // IASLIB_MEMORYBLOCK_H__
