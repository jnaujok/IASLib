/*
 * Memory Manager Base Class
 *
 *  This class defines a basic set of functionality for a memory manager
 * class that provides memory allocation based on the derived type. Each
 * different type of memory manager can then provide memory usage based
 * on how it needs to be used rather than one global solution.
 *  For example, global objects with long lifetimes are more sensibly 
 * allocated off a static heap, much like the standard malloc or new
 * commmands. On the other hand, small blocks of memory that are rapidly
 * allocated and deallocated should be taken from a fixed-block heap
 * that maintains different "heaps" for each size of block and allocates
 * memory from these fixed-size heaps rapidly and with quick re-use.
 *  All derived memory managers must supply both a production version,
 * which does not need to do range and bounds checking, and a debugging
 * version which *must* identify leaked memory, overwrites, double-frees, 
 * or out-of-range writes. 
 *
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/19/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MEMORYMANAGER_H__
#define IASLIB_MEMORYMANAGER_H__

#include "../BaseTypes/Object.h"

namespace IASLib
{

    class CMemoryManager : public CObject
    {
        protected:
            static CMemoryManager  *m_pNewAllocator;
        public:
            virtual void   *Allocate( size_t size ) = 0;
            virtual void   *Reallocate( void *p, size_t size ) = 0;
            virtual void    Release( void *p ) = 0; 

            virtual void   *AllocateDebug( size_t size, const char *strFile, int nLine ) = 0;
            virtual void   *ReallocateDebug( void *p, size_t size, const char *strFile, int nLine  ) = 0;
            virtual void    ReleaseDebug( void *p ) = 0;

            virtual void    Compact( void );

            virtual void    Collect( void );

            CMemoryManager *GetNewAllocator( void ) { return m_pNewAllocator; }
            void            SetNewAllocator( CMemoryManager *pNewAllocator );
    };
}

#ifdef IASLIB_DEBUG__
inline void * __cdecl operator new(unsigned int size, const char *file, int line)
{
    return IASLib::CMemoryManager::AllocateDebug( size, file, line );
};

inline void __cdecl operator delete(void *p )
{
    IASLib::CMemoryManager::ReleaseDebug( p );
};
#endif

#ifdef IASLIB_DEBUG__
    #define DEBUG_NEW new(__FILE__, __LINE__)
    #define new DEBUG_NEW
#endif

#endif // IASLIB_MEMORYMANAGER_H__
