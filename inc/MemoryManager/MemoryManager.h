/**
 *  Memory Manager
 *
 *      CMemoryManager is the interface of an allocator, and the hub that
 * CObject's operator new and delete go through when the library is built
 * with IASLIB_MEMORY_MANAGER__ (or IASLIB_MEMORY_DEBUGGING__). The
 * allocators are:
 *
 *      CDefaultMemoryManager   malloc() and free()
 *      CMemoryFixedHeap        a pool of blocks of one size
 *      CMemoryHeap             pools for several sizes, malloc() for larger
 *      CMemoryDebugHeap        fenceposts, use-after-free spoiling, and
 *                              (optionally) leak tracking by type, file and
 *                              line
 *
 *      Set() picks the allocator new objects come from. Every object
 * remembers the allocator it came from, so the allocator can be changed
 * while objects are alive: each goes back to its own. An allocator must
 * outlive the objects allocated from it.
 *
 *      IASLIB_NEW is new that records the file and line (for the debug
 * heap's leak report): "CThing *p = IASLIB_NEW CThing( 1 );". It is plain
 * new when the memory manager is not built in. Nothing redefines new
 * itself. (The previous header #defined new for every file that included
 * it, and could not be compiled.)
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MEMORYMANAGER_H__
#define IASLIB_MEMORYMANAGER_H__

#include <stddef.h>
#include <atomic>

namespace IASLib
{
        // Not a CObject: CObject's new comes through here.
    class CMemoryManager
    {
        public:
            struct CStatistics
            {
                size_t  nAllocations;       // ever made
                size_t  nReleases;          // ever made
                size_t  nBytesInUse;        // as asked for
                size_t  nPeakBytesInUse;
            };

                // Called with each problem found (a damaged fencepost, a bad
                // pointer); the default prints it to stderr.
            typedef void ( *ErrorHandler )( const char *strMessage );

                                CMemoryManager( void );
            virtual            ~CMemoryManager( void );

                // nSize bytes, aligned for any type, or NULL if there is no
                // memory. strType, strFile and nLine describe the request
                // (any may be NULL / 0); most allocators ignore them.
            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 ) = 0;
                // Gives back memory from Allocate(), with the size it was
                // asked for. NULL does nothing.
            virtual void        Release( void *pMemory, size_t nSize ) = 0;
            virtual const char *GetName( void ) const = 0;

            CStatistics         GetStatistics( void ) const;

                // The allocator CObjects come from (by default
                // CDefaultMemoryManager, or with IASLIB_MEMORY_DEBUGGING__ a
                // CMemoryDebugHeap over it).
            static CMemoryManager  *Get( void );
                // Makes pManager the allocator for new objects (NULL: the
                // default), returning the previous one.
            static CMemoryManager  *Set( CMemoryManager *pManager );
                // malloc() and free(); lives for the whole program.
            static CMemoryManager  &Default( void );

                // What CObject's operator new and delete call. Throws
                // std::bad_alloc when there is no memory.
            static void        *AllocateObject( size_t nSize, const char *strType, const char *strFile, int nLine );
            static void         ReleaseObject( void *pObject, const char *strType );
                // The type an object was allocated as (NULL if unknown).
            static const char  *AllocatedType( const void *pObject );

            static ErrorHandler SetErrorHandler( ErrorHandler pfnHandler );
            static void         ReportError( const char *strFormat, ... );
                // Problems reported since the program started.
            static size_t       GetErrorCount( void );

        protected:
                // For allocators to keep the statistics.
            void                CountAllocation( size_t nSize );
            void                CountRelease( size_t nSize );

        private:
            std::atomic<size_t> m_nAllocations;
            std::atomic<size_t> m_nReleases;
            std::atomic<size_t> m_nBytesInUse;
            std::atomic<size_t> m_nPeakBytesInUse;

                                CMemoryManager( const CMemoryManager & );
            CMemoryManager     &operator =( const CMemoryManager & );
    };

    class CDefaultMemoryManager : public CMemoryManager
    {
        public:
            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 );
            virtual void        Release( void *pMemory, size_t nSize );
            virtual const char *GetName( void ) const { return "malloc"; }
    };
} // namespace IASLib

#if defined( IASLIB_MEMORY_MANAGER__ ) || defined( IASLIB_MEMORY_DEBUGGING__ )
    #define IASLIB_NEW new( __FILE__, __LINE__ )
#else
    #define IASLIB_NEW new
#endif

#endif // IASLIB_MEMORYMANAGER_H__
