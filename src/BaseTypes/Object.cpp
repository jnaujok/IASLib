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
#include "String_.h"
#ifdef IASLIB_MULTI_THREADED__
#include "Mutex.h"
#endif

namespace IASLib
{
    unsigned long   CObject::m_ulCurrentID = 0;
#ifdef IASLIB_MULTI_THREADED__
    CMutex          CObject::m_mutexCreation( false );
#endif

    CObject::CObject( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexCreation.Lock();
#endif
        m_nObjectID = ++m_ulCurrentID;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexCreation.Unlock();
#endif
    }

    CObject::~CObject( void )
    {
        m_nObjectID = 0xFFFFFFFF;
    }

#ifdef IASLIB_RTTI__
    const char *CObject::GetType( void )
    {
        return "CObject";
    }

    bool CObject::IsType( const char *strName )
    {
        if (strcmp( strName, "CObject" ) == 0)
            return true;
        return false;
    }
#endif

    void CObject::SetAssertHandler( )
    {
        // TODO: Implement Assertion Handlers
    }


    void CObject::CallAssertHandler( const char *strFile,
                                     int nLine,
                                     const char *strCondition )
    {
        // TODO: Implement Assertion Handlers
        printf( "Assertion failed: (%s) - %s:%d", strCondition, strFile, nLine );
    }

    void *CObject::allocate( size_t size, const char *name )
    {
        size_t magic = 1784324;
        size_t blocks = (size / 16);
        if ( ( size - (blocks * 16) ) > 0 )
        {
            // Ensure we have an even padding of 16 bytes and cover any overage.
            blocks++;
        }
        // Allocate an extra 32 bytes
        blocks += 3;

        void *p = malloc( blocks * 16 );
        // printf( "New: %u (%u)\n", (unsigned long)p, (unsigned long)size );
        // Pre-allocate the memory to zero.
        memset( p, 0, blocks * 16 );

        // Store the block size in the front of the fencepost.
        size_t *pSize = (size_t *)p;
        size_t *pMagic = pSize + 1;
        *pSize = size;
        *pMagic = magic;
        const char **pName = (const char **)(pMagic + 1);
        *pName = name;

        // Move forward 32 bytes to create a front fencepost at least 16 bytes long
        p = ((char *)p) + 32;

        // Return the now offset block of memory so we can check fenceposts later.
        return p;
    }

    void CObject::deallocate( void *p, const char *name )
    {
        size_t magic = 1784324;
         // Back up 32 bytes
        void *pBase = ((char *)p) - 32;

        size_t *pSize = (size_t *)pBase;
        size_t *pMagic = pSize + 1;
        if ( *pMagic != magic )
        {
            printf( "MAGIC NUMBER MISMATCH! [%s]\n", name );
        }
        const char **pName = (const char **)(pMagic + 1);
        char *pTest = (char *)(pName + 1 );
        size_t size = *pSize;
        size_t blocks = (size / 16);
        if ( ( size - (blocks * 16) ) > 0 )
        {
            // account for padding
            blocks++;
        }

        if ( strcmp( name, *pName ) != 0 )
        {
            printf( "Delete mismatch. Expected [%s], got [%s]\n", name, *pName );
        }

        int nCount = 0;
        while ( pTest < p )
        {
            if ( *pTest != '\0' )
            {
                printf( "[%s] Front fencepost byte %d damaged: %02hhx\n", *pName, nCount, *pTest );
            }
            *pTest = 'X';
            pTest++;
            nCount++;
        }

        char *pEnd = ((char *)p) + (blocks * 16 );
        pTest = pEnd;
        pEnd += 16;

        nCount = 0;
        while ( pTest < pEnd )
        {
            if ( *pTest != '\0' )
            {
                printf( "[%s] Back fencepost byte %d damaged: %02hhx\n", *pName, nCount, *pTest );
            }
            *pTest = 'X';
            pTest++;
            nCount++;
        }

        // Release the memory
        free( pBase );
    }
} // End of Namespace
