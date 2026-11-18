/**
 *  Memory Manager
 *
 *      The allocator hub and the malloc() allocator. See MemoryManager.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "../../inc/MemoryManager/MemoryManager.h"
#include "../../inc/MemoryManager/MemoryDebugHeap.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>

namespace IASLib
{
    namespace
    {
            // Before every object: where it came from, so it can go back there
            // whatever allocator is current when it is deleted.
        struct CObjectHeader
        {
            CMemoryManager *pOwner;
            size_t          nSize;
            size_t          nMagic;
            const char     *pszType;
        };

        const size_t OBJECT_HEADER = 32;        // keeps the object 16-byte aligned
        const size_t OBJECT_MAGIC = (size_t)0x0B1EC7A1UL;
        const size_t OBJECT_FREED = (size_t)0xDEADBEEFUL;

        static_assert( sizeof( CObjectHeader ) <= OBJECT_HEADER, "the object header must fit" );

        std::atomic<CMemoryManager *>               g_pCurrent( NULL );
        std::atomic<CMemoryManager::ErrorHandler>   g_pfnError( NULL );
        std::atomic<size_t>                         g_nErrors( 0 );

            // The allocators that live for the whole program are built in
            // static storage and never destroyed, so objects freed during
            // exit still have somewhere to go.
        CMemoryManager &InitialManager( void )
        {
#ifdef IASLIB_MEMORY_DEBUGGING__
            alignas( CMemoryDebugHeap ) static unsigned char s_abyStorage[ sizeof( CMemoryDebugHeap ) ];
            static CMemoryManager *s_pInitial = new ( s_abyStorage ) CMemoryDebugHeap( &CMemoryManager::Default() );
            return *s_pInitial;
#else
            return CMemoryManager::Default();
#endif
        }
    }

    CMemoryManager::CMemoryManager( void )
        : m_nAllocations( 0 ), m_nReleases( 0 ), m_nBytesInUse( 0 ), m_nPeakBytesInUse( 0 )
    {
    }

    CMemoryManager::~CMemoryManager( void )
    {
        CMemoryManager *pThis = this;
        g_pCurrent.compare_exchange_strong( pThis, NULL );
    }

    CMemoryManager::CStatistics CMemoryManager::GetStatistics( void ) const
    {
        CStatistics stats;
        stats.nAllocations = m_nAllocations.load();
        stats.nReleases = m_nReleases.load();
        stats.nBytesInUse = m_nBytesInUse.load();
        stats.nPeakBytesInUse = m_nPeakBytesInUse.load();
        return stats;
    }

    void CMemoryManager::CountAllocation( size_t nSize )
    {
        m_nAllocations++;
        size_t nInUse = ( m_nBytesInUse += nSize );
        size_t nPeak = m_nPeakBytesInUse.load();
        while ( nInUse > nPeak && ! m_nPeakBytesInUse.compare_exchange_weak( nPeak, nInUse ) )
        {
        }
    }

    void CMemoryManager::CountRelease( size_t nSize )
    {
        m_nReleases++;
        m_nBytesInUse -= nSize;
    }

    CMemoryManager &CMemoryManager::Default( void )
    {
        alignas( CDefaultMemoryManager ) static unsigned char s_abyStorage[ sizeof( CDefaultMemoryManager ) ];
        static CMemoryManager *s_pDefault = new ( s_abyStorage ) CDefaultMemoryManager();
        return *s_pDefault;
    }

    CMemoryManager *CMemoryManager::Get( void )
    {
        CMemoryManager *pManager = g_pCurrent.load();
        return ( pManager ) ? pManager : &InitialManager();
    }

    CMemoryManager *CMemoryManager::Set( CMemoryManager *pManager )
    {
        CMemoryManager *pPrevious = g_pCurrent.exchange( pManager );
        return ( pPrevious ) ? pPrevious : &InitialManager();
    }

    void *CMemoryManager::AllocateObject( size_t nSize, const char *strType, const char *strFile, int nLine )
    {
        if ( nSize > (size_t)-1 - OBJECT_HEADER )
            throw std::bad_alloc();

        CMemoryManager *pManager = Get();
        char *pBase = (char *)pManager->Allocate( nSize + OBJECT_HEADER, strType, strFile, nLine );
        if ( pBase == NULL )
            throw std::bad_alloc();

        CObjectHeader *pHeader = (CObjectHeader *)pBase;
        pHeader->pOwner = pManager;
        pHeader->nSize = nSize;
        pHeader->nMagic = OBJECT_MAGIC;
        pHeader->pszType = strType;
        return pBase + OBJECT_HEADER;
    }

    void CMemoryManager::ReleaseObject( void *pObject, const char *strType )
    {
        if ( pObject == NULL )
            return;

        CObjectHeader *pHeader = (CObjectHeader *)( (char *)pObject - OBJECT_HEADER );
        if ( pHeader->nMagic != OBJECT_MAGIC )
        {
                // Not an object from here, or deleted already: nothing in it
                // can be trusted, so it is not freed.
            if ( pHeader->nMagic == OBJECT_FREED )
                ReportError( "Object deleted twice [%s]", ( strType ) ? strType : "?" );
            else
                ReportError( "MAGIC NUMBER MISMATCH! [%s]", ( strType ) ? strType : "?" );
            return;
        }

        if ( strType != NULL && pHeader->pszType != NULL && strType != pHeader->pszType &&
             strcmp( strType, pHeader->pszType ) != 0 )
        {
            ReportError( "Delete mismatch. Expected [%s], got [%s]", strType, pHeader->pszType );
        }

        CMemoryManager *pOwner = pHeader->pOwner;
        size_t nSize = pHeader->nSize;
        pHeader->nMagic = OBJECT_FREED;
        pOwner->Release( pHeader, nSize + OBJECT_HEADER );
    }

    const char *CMemoryManager::AllocatedType( const void *pObject )
    {
        if ( pObject == NULL )
            return NULL;
        const CObjectHeader *pHeader = (const CObjectHeader *)( (const char *)pObject - OBJECT_HEADER );
        return ( pHeader->nMagic == OBJECT_MAGIC ) ? pHeader->pszType : NULL;
    }

    CMemoryManager::ErrorHandler CMemoryManager::SetErrorHandler( ErrorHandler pfnHandler )
    {
        return g_pfnError.exchange( pfnHandler );
    }

    void CMemoryManager::ReportError( const char *strFormat, ... )
    {
        char achMessage[ 512 ];
        va_list vaArgs;
        va_start( vaArgs, strFormat );
        vsnprintf( achMessage, sizeof( achMessage ), strFormat, vaArgs );
        va_end( vaArgs );

        g_nErrors++;
        ErrorHandler pfnHandler = g_pfnError.load();
        if ( pfnHandler )
        {
            pfnHandler( achMessage );
        }
        else
        {
            fprintf( stderr, "%s\n", achMessage );
            fflush( stderr );
        }
    }

    size_t CMemoryManager::GetErrorCount( void )
    {
        return g_nErrors.load();
    }

    void *CDefaultMemoryManager::Allocate( size_t nSize, const char *, const char *, int )
    {
        void *p = malloc( ( nSize ) ? nSize : 1 );
        if ( p )
            CountAllocation( nSize );
        return p;
    }

    void CDefaultMemoryManager::Release( void *pMemory, size_t nSize )
    {
        if ( pMemory == NULL )
            return;
        CountRelease( nSize );
        free( pMemory );
    }
} // namespace IASLib
