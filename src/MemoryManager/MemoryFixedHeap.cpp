/**
 *  Fixed-size Heap
 *
 *      See MemoryFixedHeap.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "../../inc/MemoryManager/MemoryFixedHeap.h"

#include <string.h>

namespace IASLib
{
    CMemoryFixedHeap::CMemoryFixedHeap( size_t nBlockSize, size_t nBlocksPerChunk, CMemoryManager *pFallback )
    {
            // Big enough to hold the free-list link, and a multiple of 16 so
            // every block is aligned for any type.
        if ( nBlockSize < sizeof( void * ) )
            nBlockSize = sizeof( void * );
        m_nBlockSize = ( nBlockSize + 15 ) & ~(size_t)15;
        m_nBlocksPerChunk = ( nBlocksPerChunk ) ? nBlocksPerChunk : 1;
        m_pFallback = ( pFallback ) ? pFallback : &CMemoryManager::Default();
        m_pChunks = NULL;
        m_pChunksWithFree = NULL;
        m_nChunks = 0;
    }

    CMemoryFixedHeap::~CMemoryFixedHeap( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        while ( m_pChunks )
        {
            CMemoryBlock *pChunk = m_pChunks;
            m_pChunks = pChunk->pNextChunk;
            m_pFallback->Release( pChunk, CMemoryBlock::HeaderSize() + pChunk->nBlocks * m_nBlockSize );
        }
        m_mapChunks.clear();
        m_pChunksWithFree = NULL;
        m_nChunks = 0;
    }

    void CMemoryFixedHeap::LinkWithFree( CMemoryBlock *pChunk )
    {
        pChunk->pPrevWithFree = NULL;
        pChunk->pNextWithFree = m_pChunksWithFree;
        if ( m_pChunksWithFree )
            m_pChunksWithFree->pPrevWithFree = pChunk;
        m_pChunksWithFree = pChunk;
    }

    void CMemoryFixedHeap::UnlinkWithFree( CMemoryBlock *pChunk )
    {
        if ( pChunk->pPrevWithFree )
            pChunk->pPrevWithFree->pNextWithFree = pChunk->pNextWithFree;
        else if ( m_pChunksWithFree == pChunk )
            m_pChunksWithFree = pChunk->pNextWithFree;
        if ( pChunk->pNextWithFree )
            pChunk->pNextWithFree->pPrevWithFree = pChunk->pPrevWithFree;
        pChunk->pNextWithFree = NULL;
        pChunk->pPrevWithFree = NULL;
    }

    CMemoryBlock *CMemoryFixedHeap::NewChunk( void )
    {
        size_t nBytes = CMemoryBlock::HeaderSize() + m_nBlocksPerChunk * m_nBlockSize;
        CMemoryBlock *pChunk = (CMemoryBlock *)m_pFallback->Allocate( nBytes, "CMemoryFixedHeap chunk" );
        if ( pChunk == NULL )
            return NULL;

        pChunk->nBlocks = m_nBlocksPerChunk;
        pChunk->nUsed = 0;
        pChunk->pFirstBlock = (char *)pChunk + CMemoryBlock::HeaderSize();

            // Thread the free list through the blocks, first block first.
        pChunk->pFreeList = pChunk->pFirstBlock;
        for ( size_t n = 0; n < m_nBlocksPerChunk; n++ )
        {
            char *pBlock = pChunk->pFirstBlock + n * m_nBlockSize;
            *(void **)pBlock = ( n + 1 < m_nBlocksPerChunk ) ? pBlock + m_nBlockSize : NULL;
        }

        pChunk->pPrevChunk = NULL;
        pChunk->pNextChunk = m_pChunks;
        if ( m_pChunks )
            m_pChunks->pPrevChunk = pChunk;
        m_pChunks = pChunk;
        LinkWithFree( pChunk );
        m_mapChunks[ pChunk->pFirstBlock ] = pChunk;
        m_nChunks++;
        return pChunk;
    }

    void CMemoryFixedHeap::FreeChunk( CMemoryBlock *pChunk )
    {
        UnlinkWithFree( pChunk );
        if ( pChunk->pPrevChunk )
            pChunk->pPrevChunk->pNextChunk = pChunk->pNextChunk;
        else
            m_pChunks = pChunk->pNextChunk;
        if ( pChunk->pNextChunk )
            pChunk->pNextChunk->pPrevChunk = pChunk->pPrevChunk;
        m_mapChunks.erase( pChunk->pFirstBlock );
        m_nChunks--;
        m_pFallback->Release( pChunk, CMemoryBlock::HeaderSize() + pChunk->nBlocks * m_nBlockSize );
    }

    CMemoryBlock *CMemoryFixedHeap::ChunkOf( const void *p ) const
    {
        std::map<char *, CMemoryBlock *>::const_iterator it = m_mapChunks.upper_bound( (char *)p );
        if ( it == m_mapChunks.begin() )
            return NULL;
        --it;
        CMemoryBlock *pChunk = it->second;
        const char *pByte = (const char *)p;
        if ( pByte >= pChunk->pFirstBlock + pChunk->nBlocks * m_nBlockSize )
            return NULL;
        return pChunk;
    }

    void *CMemoryFixedHeap::Allocate( size_t nSize, const char *strType, const char *strFile, int nLine )
    {
        if ( nSize > m_nBlockSize )
        {
            void *p = m_pFallback->Allocate( nSize, strType, strFile, nLine );
            if ( p )
                CountAllocation( nSize );
            return p;
        }

        std::lock_guard<std::mutex> lock( m_mutex );
        CMemoryBlock *pChunk = m_pChunksWithFree;
        if ( pChunk == NULL && ( pChunk = NewChunk() ) == NULL )
            return NULL;

        void *pBlock = pChunk->pFreeList;
        pChunk->pFreeList = *(void **)pBlock;
        pChunk->nUsed++;
        if ( pChunk->pFreeList == NULL )
            UnlinkWithFree( pChunk );
        CountAllocation( nSize );
        return pBlock;
    }

    void CMemoryFixedHeap::Release( void *pMemory, size_t nSize )
    {
        if ( pMemory == NULL )
            return;

        if ( nSize > m_nBlockSize )
        {
            CountRelease( nSize );
            m_pFallback->Release( pMemory, nSize );
            return;
        }

        std::lock_guard<std::mutex> lock( m_mutex );
        CMemoryBlock *pChunk = ChunkOf( pMemory );
        if ( pChunk == NULL || ( (char *)pMemory - pChunk->pFirstBlock ) % m_nBlockSize != 0 )
        {
            ReportError( "CMemoryFixedHeap: %p is not a block of this heap", pMemory );
            return;
        }
        if ( pChunk->nUsed == 0 )
        {
            ReportError( "CMemoryFixedHeap: %p freed from a chunk with nothing in use", pMemory );
            return;
        }

        bool bWasFull = ( pChunk->pFreeList == NULL );
        *(void **)pMemory = pChunk->pFreeList;
        pChunk->pFreeList = pMemory;
        pChunk->nUsed--;
        if ( bWasFull )
            LinkWithFree( pChunk );
        CountRelease( nSize );
    }

    size_t CMemoryFixedHeap::GetChunkCount( void ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return m_nChunks;
    }

    size_t CMemoryFixedHeap::GetBlocksInUse( void ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        size_t nUsed = 0;
        for ( CMemoryBlock *pChunk = m_pChunks; pChunk; pChunk = pChunk->pNextChunk )
            nUsed += pChunk->nUsed;
        return nUsed;
    }

    size_t CMemoryFixedHeap::GetFreeBlocks( void ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        size_t nFree = 0;
        for ( CMemoryBlock *pChunk = m_pChunks; pChunk; pChunk = pChunk->pNextChunk )
            nFree += pChunk->nBlocks - pChunk->nUsed;
        return nFree;
    }

    bool CMemoryFixedHeap::Owns( const void *p ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return ChunkOf( p ) != NULL;
    }

    size_t CMemoryFixedHeap::Compact( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        size_t nFreed = 0;
        CMemoryBlock *pChunk = m_pChunks;
        while ( pChunk )
        {
            CMemoryBlock *pNext = pChunk->pNextChunk;
            if ( pChunk->nUsed == 0 )
            {
                FreeChunk( pChunk );
                nFreed++;
            }
            pChunk = pNext;
        }
        return nFreed;
    }
} // namespace IASLib
