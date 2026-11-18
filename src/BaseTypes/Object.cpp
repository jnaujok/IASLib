/*
 * Object
 *
 *	This class defines the base (abstract) class that all objects in the
 * IAS Library are based on.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/18/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Object.h"

#include <atomic>
#include <new>

namespace IASLib
{
    namespace
    {
        std::atomic<unsigned long>              g_ulCurrentID( 0 );

        void DefaultAssertHandler( const char *strFile, int nLine, const char *strCondition )
        {
            fprintf( stderr, "Assertion failed: (%s) - %s:%d\n", strCondition, strFile, nLine );
            fflush( stderr );
        }

        std::atomic<CObject::AssertHandler>     g_pfnAssertHandler( &DefaultAssertHandler );

            // The memory-debugging layout:
            //
            //      [ header: size, magic, name ][ front fence ] user data [ back fence ]
            //      |<---------------- FRONT bytes -------------->|        |<-- FENCE -->|
            //
            // The user data starts FRONT bytes in (a multiple of 16, so it is
            // as aligned as malloc's memory), and is followed by FENCE bytes
            // of zeros, plus the padding up to a multiple of 16.
        const size_t    ALLOCATION_MAGIC = 1784324;
        const size_t    FRONT = 32;
        const size_t    FENCE = 16;

        struct CAllocationHeader
        {
            size_t          nSize;
            size_t          nMagic;
            const char     *pszName;
        };

        static_assert( sizeof( CAllocationHeader ) < FRONT, "the header must leave room for a front fencepost" );

        size_t PaddedSize( size_t nSize )
        {
            return ( nSize + 15 ) & ~(size_t)15;
        }
    }

    CObject::CObject( void )
    {
        m_nObjectID = ++g_ulCurrentID;
    }

    CObject::CObject( const CObject & )
    {
        m_nObjectID = ++g_ulCurrentID;
    }

    CObject::~CObject( void )
    {
        m_nObjectID = (unsigned long)-1;
    }

#ifdef IASLIB_RTTI__
    const char *CObject::GetType( void ) const
    {
        return "CObject";
    }

    bool CObject::IsType( const char *strName ) const
    {
        return ( strName != NULL ) && ( strcmp( strName, "CObject" ) == 0 );
    }
#endif

    CObject::AssertHandler CObject::SetAssertHandler( AssertHandler pfnHandler )
    {
        return g_pfnAssertHandler.exchange( ( pfnHandler != NULL ) ? pfnHandler : &DefaultAssertHandler );
    }

    void CObject::CallAssertHandler( const char *strFile, int nLine, const char *strCondition )
    {
        AssertHandler pfnHandler = g_pfnAssertHandler.load();
        pfnHandler( strFile, nLine, strCondition );
    }

    void *CObject::allocate( size_t size, const char *name )
    {
        size_t nPadded = PaddedSize( size );
        if ( nPadded < size || nPadded > (size_t)-1 - FRONT - FENCE )
        {
            throw std::bad_alloc();
        }

        size_t nTotal = FRONT + nPadded + FENCE;
        char *pBase = (char *)malloc( nTotal );
        if ( pBase == NULL )
        {
            throw std::bad_alloc();
        }
        memset( pBase, 0, nTotal );

        CAllocationHeader *pHeader = (CAllocationHeader *)pBase;
        pHeader->nSize = size;
        pHeader->nMagic = ALLOCATION_MAGIC;
        pHeader->pszName = name;

        return pBase + FRONT;
    }

    void CObject::deallocate( void *p, const char *name )
    {
        if ( p == NULL )
        {
            return;
        }

        char *pBase = ((char *)p) - FRONT;
        CAllocationHeader *pHeader = (CAllocationHeader *)pBase;

        if ( pHeader->nMagic != ALLOCATION_MAGIC )
        {
                // Not memory from allocate() (or its header was overwritten):
                // nothing else in it can be trusted.
            fprintf( stderr, "MAGIC NUMBER MISMATCH! [%s]\n", name );
            fflush( stderr );
            free( pBase );
            return;
        }

        const char *pszAllocatedAs = ( pHeader->pszName != NULL ) ? pHeader->pszName : "?";
        if ( name != NULL && strcmp( name, pszAllocatedAs ) != 0 )
        {
            fprintf( stderr, "Delete mismatch. Expected [%s], got [%s]\n", name, pszAllocatedAs );
        }

        const unsigned char *pFront = (const unsigned char *)( pHeader + 1 );
        for ( size_t nCount = 0; pFront + nCount < (const unsigned char *)p; nCount++ )
        {
            if ( pFront[ nCount ] != 0 )
            {
                fprintf( stderr, "[%s] Front fencepost byte %u damaged: %02x\n", pszAllocatedAs, (unsigned)nCount, pFront[ nCount ] );
            }
        }

            // The padding after the object is checked along with the fence:
            // the object should not have written there either.
        size_t nPadded = PaddedSize( pHeader->nSize );
        const unsigned char *pBack = (const unsigned char *)p + pHeader->nSize;
        size_t nBack = ( nPadded - pHeader->nSize ) + FENCE;
        for ( size_t nCount = 0; nCount < nBack; nCount++ )
        {
            if ( pBack[ nCount ] != 0 )
            {
                fprintf( stderr, "[%s] Back fencepost byte %u damaged: %02x\n", pszAllocatedAs, (unsigned)nCount, pBack[ nCount ] );
            }
        }
        fflush( stderr );

            // Spoil the memory, so use after free shows up.
        memset( pBase, 'X', FRONT + nPadded + FENCE );
        free( pBase );
    }
} // End of Namespace
