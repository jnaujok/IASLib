/**
 *  @file
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
 * itself. (The previous header \#defined new for every file that included
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
        /// The interface of an allocator, and the hub CObject's new and delete go through.
        ///
        /// Not a CObject: CObject's new comes through here. Subclasses
        /// implement Allocate(), Release() and GetName(), and call
        /// CountAllocation() and CountRelease() to keep the statistics. The
        /// static functions are thread-safe. Allocators are not copyable.
        /// Destroying the current allocator makes the initial one (see Get()) current again.
    class CMemoryManager
    {
        public:
            /// What an allocator has done (see GetStatistics()).
            struct CStatistics
            {
                size_t  nAllocations;       ///< Allocations ever made.
                size_t  nReleases;          ///< Releases ever made.
                size_t  nBytesInUse;        ///< Bytes in use, as asked for.
                size_t  nPeakBytesInUse;    ///< The most bytes ever in use at once.
            };

                /// Called with each problem found (a damaged fencepost, a bad
                /// pointer); the default prints it to stderr.
            typedef void ( *ErrorHandler )( const char *strMessage );

                                /// Creates an allocator with zeroed statistics.
                                CMemoryManager( void );
                                /// Destroys the allocator; if it was current, the initial one (see Get()) becomes current.
            virtual            ~CMemoryManager( void );

                /// nSize bytes, aligned for any type, or NULL if there is no
                /// memory.
                /// strType, strFile and nLine describe the request (any may
                /// be NULL / 0); most allocators ignore them.
                /// @param nSize   The bytes wanted.
                /// @param strType The type being allocated.
                /// @param strFile The source file of the request.
                /// @param nLine   The source line of the request.
                /// @return The memory, or NULL.
            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 ) = 0;
                /// Gives back memory from Allocate(), with the size it was
                /// asked for. NULL does nothing.
                /// @param pMemory The memory.
                /// @param nSize   The size given to Allocate().
            virtual void        Release( void *pMemory, size_t nSize ) = 0;
                /// A short name for the allocator ("malloc", "fixed", "pooled", "debug").
            virtual const char *GetName( void ) const = 0;

                /// A snapshot of this allocator's statistics.
            CStatistics         GetStatistics( void ) const;

                /// The allocator CObjects come from (by default
                /// CDefaultMemoryManager, or with IASLIB_MEMORY_DEBUGGING__ a
                /// CMemoryDebugHeap over it). Never NULL.
            static CMemoryManager  *Get( void );
                /// Makes pManager the allocator for new objects.
                /// @param pManager The allocator (NULL: the default). Not owned;
                ///                 it must outlive every object allocated from it.
                /// @return The previous one (never NULL).
            static CMemoryManager  *Set( CMemoryManager *pManager );
                /// malloc() and free(); lives for the whole program.
            static CMemoryManager  &Default( void );

                /// What CObject's operator new calls.
                /// Allocates from the current allocator, with a header that
                /// records the allocator, size and type.
                /// @param nSize   The object's size.
                /// @param strType The object's type (may be NULL).
                /// @param strFile The source file (may be NULL).
                /// @param nLine   The source line.
                /// @return The object's memory (never NULL).
                /// @throws std::bad_alloc when there is no memory.
            static void        *AllocateObject( size_t nSize, const char *strType, const char *strFile, int nLine );
                /// What CObject's operator delete calls.
                /// Gives the memory back to the allocator it came from. An
                /// object deleted twice, or not from AllocateObject(), is
                /// reported and not freed; a type that differs from the one
                /// allocated is reported.
                /// @param pObject The object (NULL does nothing).
                /// @param strType The type being deleted (may be NULL).
            static void         ReleaseObject( void *pObject, const char *strType );
                /// The type an object was allocated as (NULL if unknown).
            static const char  *AllocatedType( const void *pObject );

                /// Sets the function problems are reported to.
                /// @param pfnHandler The handler (NULL: print to stderr).
                /// @return The previous handler.
            static ErrorHandler SetErrorHandler( ErrorHandler pfnHandler );
                /// Reports a problem (printf-style, up to 511 characters) and counts it.
            static void         ReportError( const char *strFormat, ... );
                /// Problems reported since the program started.
            static size_t       GetErrorCount( void );

        protected:
                /// For allocators to keep the statistics: counts an allocation of nSize bytes.
            void                CountAllocation( size_t nSize );
                /// Counts a release of nSize bytes.
            void                CountRelease( size_t nSize );

        private:
            std::atomic<size_t> m_nAllocations;     ///< Allocations ever made.
            std::atomic<size_t> m_nReleases;        ///< Releases ever made.
            std::atomic<size_t> m_nBytesInUse;      ///< Bytes in use.
            std::atomic<size_t> m_nPeakBytesInUse;  ///< Most bytes ever in use.

                                /// Not copyable.
                                CMemoryManager( const CMemoryManager & );
                                /// Not assignable.
            CMemoryManager     &operator =( const CMemoryManager & );
    };

        /// The allocator over malloc() and free().
        /// Use CMemoryManager::Default() rather than making another.
    class CDefaultMemoryManager : public CMemoryManager
    {
        public:
                /// malloc( nSize ) (at least one byte); the description is ignored.
                /// @copydetails CMemoryManager::Allocate()
            virtual void       *Allocate( size_t nSize, const char *strType = NULL, const char *strFile = NULL, int nLine = 0 );
                /// free(); NULL does nothing.
                /// @param pMemory The memory.
                /// @param nSize   The size given to Allocate() (for the statistics).
            virtual void        Release( void *pMemory, size_t nSize );
            /// "malloc".
            virtual const char *GetName( void ) const { return "malloc"; }
    };
} // namespace IASLib

#if defined( IASLIB_MEMORY_MANAGER__ ) || defined( IASLIB_MEMORY_DEBUGGING__ )
    /// new that records the file and line of the allocation (for the debug
    /// heap's leak report): "CThing *p = IASLIB_NEW CThing( 1 );".
    #define IASLIB_NEW new( __FILE__, __LINE__ )
#else
    /// Plain new when the memory manager is not built in.
    #define IASLIB_NEW new
#endif

#endif // IASLIB_MEMORYMANAGER_H__
