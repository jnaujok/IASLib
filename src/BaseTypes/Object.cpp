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
#include "../../inc/MemoryManager/MemoryManager.h"

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

        // (The fencepost allocator that was here is now CMemoryDebugHeap.)
    void *CObject::allocate( size_t size, const char *name )
    {
        return CMemoryManager::AllocateObject( size, name, NULL, 0 );
    }

    void *CObject::allocateAt( size_t size, const char *name, const char *strFile, int nLine )
    {
        return CMemoryManager::AllocateObject( size, name, strFile, nLine );
    }

    void CObject::deallocate( void *p, const char *name )
    {
        CMemoryManager::ReleaseObject( p, name );
    }
} // End of Namespace
