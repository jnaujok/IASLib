/**
 *  Debugging Heap
 *
 *      See MemoryDebugHeap.h. The layout of each allocation:
 *
 *      [ header ][ front fence ] user data [ padding ][ back fence ]
 *      |<------ FRONT ------->|                       |<- FENCE ->|
 *
 *      The user data starts FRONT bytes in (a multiple of 16), and the
 * padding up to a multiple of 16 is checked with the back fence: the user
 * should not write there either.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "../../inc/MemoryManager/MemoryDebugHeap.h"

#include <string.h>

namespace IASLib
{
    struct CMemoryDebugHeap::CDebugHeader
    {
        size_t          nSize;
        size_t          nMagic;
        const char     *pszType;
        const char     *pszFile;
        int             nLine;
        bool            bTracked;
        unsigned long   ulSequence;
        CDebugHeader   *pPrev;
        CDebugHeader   *pNext;
    };

    namespace
    {
        const size_t DEBUG_MAGIC = (size_t)1784324;     // (the value CObject used)
        const size_t DEBUG_FREED = (size_t)0x0DEAD0FFUL;
        const size_t FRONT = 96;
        const size_t FENCE = 16;
        const unsigned char SPOIL = 'X';

        size_t PaddedSize( size_t nSize )
        {
            return ( nSize + 15 ) & ~(size_t)15;
        }

        const char *nameOf( const char *pszType )
        {
            return ( pszType ) ? pszType : "?";
        }
    }

    static_assert( sizeof( CMemoryDebugHeap::CDebugHeader ) + FENCE <= FRONT, "the header must leave room for a front fencepost" );

    size_t CMemoryDebugHeap::FrontSize( void ) { return FRONT; }
    size_t CMemoryDebugHeap::FenceSize( void ) { return FENCE; }

    CMemoryDebugHeap::CMemoryDebugHeap( CMemoryManager *pUnderlying, bool bTrackLeaks )
        : m_bTrackLeaks( bTrackLeaks )
    {
        m_pUnderlying = ( pUnderlying ) ? pUnderlying : &CMemoryManager::Default();
        m_pTracked = NULL;
        m_nTracked = 0;
        m_nTrackedBytes = 0;
        m_ulSequence = 0;
    }

    CMemoryDebugHeap::~CMemoryDebugHeap( void )
    {
    }

    void *CMemoryDebugHeap::Allocate( size_t nSize, const char *strType, const char *strFile, int nLine )
    {
        size_t nPadded = PaddedSize( nSize );
        if ( nPadded < nSize || nPadded > (size_t)-1 - FRONT - FENCE )
            return NULL;

        size_t nTotal = FRONT + nPadded + FENCE;
        char *pBase = (char *)m_pUnderlying->Allocate( nTotal, strType, strFile, nLine );
        if ( pBase == NULL )
            return NULL;
        memset( pBase, 0, nTotal );

        CDebugHeader *pHeader = (CDebugHeader *)pBase;
        pHeader->nSize = nSize;
        pHeader->nMagic = DEBUG_MAGIC;
        pHeader->pszType = strType;
        pHeader->pszFile = strFile;
        pHeader->nLine = nLine;
        pHeader->bTracked = false;
        pHeader->pPrev = NULL;
        pHeader->pNext = NULL;

        if ( m_bTrackLeaks )
        {
            std::lock_guard<std::mutex> lock( m_mutexTracked );
            pHeader->bTracked = true;
            pHeader->ulSequence = ++m_ulSequence;
            pHeader->pNext = m_pTracked;
            if ( m_pTracked )
                m_pTracked->pPrev = pHeader;
            m_pTracked = pHeader;
            m_nTracked++;
            m_nTrackedBytes += nSize;
        }

        CountAllocation( nSize );
        return pBase + FRONT;
    }

    bool CMemoryDebugHeap::CheckFences( const CDebugHeader *pHeader, const char *strWhen )
    {
        const unsigned char *pData = (const unsigned char *)pHeader + FRONT;
        const char *strType = nameOf( pHeader->pszType );
        bool bOk = true;

            // Report each fence once, with how much of it was damaged. (Every
            // damaged byte got a line of its own.)
        const unsigned char *pFront = (const unsigned char *)pHeader + FRONT - FENCE;
        size_t nDamaged = 0, nFirst = 0;
        for ( size_t n = 0; n < FENCE; n++ )
        {
            if ( pFront[n] != 0 && nDamaged++ == 0 )
                nFirst = n;
        }
        if ( nDamaged )
        {
            ReportError( "[%s] Front fencepost damaged (%u bytes, first at %u: %02x) %s",
                         strType, (unsigned)nDamaged, (unsigned)nFirst, pFront[ nFirst ], strWhen );
            bOk = false;
        }

        size_t nBack = PaddedSize( pHeader->nSize ) - pHeader->nSize + FENCE;
        const unsigned char *pBack = pData + pHeader->nSize;
        nDamaged = 0;
        for ( size_t n = 0; n < nBack; n++ )
        {
            if ( pBack[n] != 0 && nDamaged++ == 0 )
                nFirst = n;
        }
        if ( nDamaged )
        {
            ReportError( "[%s] Back fencepost damaged (%u bytes, first at %u: %02x) %s",
                         strType, (unsigned)nDamaged, (unsigned)nFirst, pBack[ nFirst ], strWhen );
            bOk = false;
        }
        return bOk;
    }

    void CMemoryDebugHeap::Release( void *pMemory, size_t nSize )
    {
        if ( pMemory == NULL )
            return;

        CDebugHeader *pHeader = (CDebugHeader *)( (char *)pMemory - FRONT );
        if ( pHeader->nMagic != DEBUG_MAGIC )
        {
                // Not ours, or freed already: nothing in it can be trusted.
            if ( pHeader->nMagic == DEBUG_FREED )
                ReportError( "Memory freed twice [%p]", pMemory );
            else
                ReportError( "MAGIC NUMBER MISMATCH! [%p]", pMemory );
            return;
        }
        if ( pHeader->nSize != nSize )
        {
            ReportError( "[%s] Freed with size %u, allocated with %u", nameOf( pHeader->pszType ),
                         (unsigned)nSize, (unsigned)pHeader->nSize );
        }

        CheckFences( pHeader, "when freed" );

        if ( pHeader->bTracked )
        {
            std::lock_guard<std::mutex> lock( m_mutexTracked );
            if ( pHeader->pPrev )
                pHeader->pPrev->pNext = pHeader->pNext;
            else
                m_pTracked = pHeader->pNext;
            if ( pHeader->pNext )
                pHeader->pNext->pPrev = pHeader->pPrev;
            m_nTracked--;
            m_nTrackedBytes -= pHeader->nSize;
        }

        size_t nAllocated = pHeader->nSize;
        size_t nTotal = FRONT + PaddedSize( nAllocated ) + FENCE;
        CountRelease( nAllocated );

            // Spoil everything after the header, so use after free shows up;
            // the header keeps the freed mark for a second free to find.
        pHeader->nMagic = DEBUG_FREED;
        memset( (char *)pHeader + sizeof( CDebugHeader ), SPOIL, nTotal - sizeof( CDebugHeader ) );
        m_pUnderlying->Release( pHeader, nTotal );
    }

    size_t CMemoryDebugHeap::GetLeakCount( void )
    {
        std::lock_guard<std::mutex> lock( m_mutexTracked );
        return m_nTracked;
    }

    size_t CMemoryDebugHeap::GetLeakBytes( void )
    {
        std::lock_guard<std::mutex> lock( m_mutexTracked );
        return m_nTrackedBytes;
    }

    size_t CMemoryDebugHeap::ReportLeaks( FILE *pOut )
    {
        std::lock_guard<std::mutex> lock( m_mutexTracked );
        if ( pOut )
        {
            for ( const CDebugHeader *p = m_pTracked; p; p = p->pNext )
            {
                fprintf( pOut, "Leak #%lu: %u bytes as %s at %s:%d\n", p->ulSequence, (unsigned)p->nSize,
                         nameOf( p->pszType ), ( p->pszFile ) ? p->pszFile : "?", p->nLine );
            }
            fflush( pOut );
        }
        return m_nTracked;
    }

    size_t CMemoryDebugHeap::CheckAll( void )
    {
        std::lock_guard<std::mutex> lock( m_mutexTracked );
        size_t nDamaged = 0;
        for ( const CDebugHeader *p = m_pTracked; p; p = p->pNext )
        {
            if ( ! CheckFences( p, "(checked while in use)" ) )
                nDamaged++;
        }
        return nDamaged;
    }
} // namespace IASLib
