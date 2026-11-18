/**
 *  Pooled Heap
 *
 *      See MemoryHeap.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "../../inc/MemoryManager/MemoryHeap.h"

namespace IASLib
{
    CMemoryHeap::CMemoryHeap( size_t nBlocksPerChunk, CMemoryManager *pFallback )
    {
        m_pFallback = ( pFallback ) ? pFallback : &CMemoryManager::Default();
        for ( size_t n = 0; n < SIZE_CLASSES; n++ )
            m_apPools[n] = new CMemoryFixedHeap( (size_t)16 << n, nBlocksPerChunk, m_pFallback );
    }

    CMemoryHeap::~CMemoryHeap( void )
    {
        for ( size_t n = 0; n < SIZE_CLASSES; n++ )
        {
            delete m_apPools[n];
            m_apPools[n] = NULL;
        }
    }

    CMemoryFixedHeap *CMemoryHeap::PoolFor( size_t nSize ) const
    {
        for ( size_t n = 0; n < SIZE_CLASSES; n++ )
        {
            if ( nSize <= m_apPools[n]->GetBlockSize() )
                return m_apPools[n];
        }
        return NULL;
    }

    void *CMemoryHeap::Allocate( size_t nSize, const char *strType, const char *strFile, int nLine )
    {
        CMemoryFixedHeap *pPool = PoolFor( nSize );
        void *p = ( pPool ) ? pPool->Allocate( nSize, strType, strFile, nLine )
                            : m_pFallback->Allocate( nSize, strType, strFile, nLine );
        if ( p )
            CountAllocation( nSize );
        return p;
    }

    void CMemoryHeap::Release( void *pMemory, size_t nSize )
    {
        if ( pMemory == NULL )
            return;
        CountRelease( nSize );
        CMemoryFixedHeap *pPool = PoolFor( nSize );
        if ( pPool )
            pPool->Release( pMemory, nSize );
        else
            m_pFallback->Release( pMemory, nSize );
    }

    size_t CMemoryHeap::Compact( void )
    {
        size_t nFreed = 0;
        for ( size_t n = 0; n < SIZE_CLASSES; n++ )
            nFreed += m_apPools[n]->Compact();
        return nFreed;
    }
} // namespace IASLib
