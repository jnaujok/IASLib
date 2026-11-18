/**
 * @file Object.h
 * @brief CObject, the base class of the library, and its RTTI, memory and
 * ASSERT() macros.
 *
 * CObject class
 *
 *	This class defines the base (abstract) class that all objects in the
 * IAS Library are based on.
 *
 *      Every object gets an ID, unique for the life of the program (a copy
 * gets its own ID). With IASLIB_RTTI__, classes name themselves with
 * DECLARE_OBJECT( class, base ) in the class, or DEFINE_OBJECT( class ) in
 * the class and IMPLEMENT_OBJECT( class, base ) in its source file, giving
 * GetType() and IsType(). With IASLIB_MEMORY_MANAGER__ or
 * IASLIB_MEMORY_DEBUGGING__, those classes are allocated through
 * CMemoryManager (see MemoryManager/MemoryManager.h); with
 * IASLIB_MEMORY_DEBUGGING__ that is a CMemoryDebugHeap, which puts
 * fenceposts around each object and checks them when it is deleted.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/18/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_OBJECT_H__
#define IASLIB_OBJECT_H__

#ifdef IASLIB_WIN32__
#include "Platform.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#ifndef NULL
/// The null pointer constant, for compilers whose headers lack it.
#define NULL 0L
#endif

    /// The library's character type: char on every platform.
    /// @note It was TCHAR on Windows, which is wchar_t in UNICODE builds,
    /// but the string code has only ever handled char.
typedef char IASLibChar__;

/// The IAS Library: every class of the library lives in this namespace.
namespace IASLib
{
        /// "Not found": what searches return when they find nothing. This
        /// constant is used in a lot of places.
    const size_t NOT_FOUND = (size_t)-1;

    /**
     * The base (abstract) class that all objects in the IAS Library are
     * based on.
     *
     * Every object gets an ID, unique for the life of the program. A copy
     * gets its own ID, and assignment keeps the target's ID. IDs come from an
     * atomic counter, so objects can be made on any thread.
     *
     * With IASLIB_RTTI__, a class names itself with DECLARE_OBJECT() in the
     * class, or DEFINE_OBJECT() in the class and IMPLEMENT_OBJECT() in its
     * source file; that gives it GetType() and IsType(). With the memory
     * manager built in (IASLIB_MEMORY_MANAGER__ or IASLIB_MEMORY_DEBUGGING__)
     * the same macros route its new and delete through allocate() and
     * deallocate().
     *
     * @code
     * class CThing : public IASLib::CObject
     * {
     *     public:
     *         DECLARE_OBJECT( CThing, CObject );
     * };
     *
     * CThing thing;
     * if ( thing.IsType( "CObject" ) )    // true: a CThing is a CObject
     *     printf( "%s #%lu\n", thing.GetType(), thing.GetID() );
     * @endcode
     */
    class CObject
    {
        private:
            unsigned long           m_nObjectID;    ///< This object's ID (-1 once destroyed).

        public:
                /// The function ASSERT() calls when its condition is false,
                /// with the source file, the line and the condition's text.
            typedef void          (*AssertHandler)( const char *strFile, int nLine, const char *strCondition );

#ifdef IASLIB_RTTI__
                /// The class's name, as given to DECLARE_OBJECT() or
                /// DEFINE_OBJECT() ("CObject" for CObject itself).
            virtual const char     *GetType( void ) const = 0;
                /// Whether this object is of the named class or of a class
                /// derived from it (names compared with strcmp()).
                /// @param strName A class name; NULL gives false.
                /// @return true if the object is a strName.
            virtual bool            IsType( const char *strName ) const;
#endif
                                    /// Makes an object with a new ID.
                                    CObject( void );
                                        /// A copy is a different object: it
                                        /// gets a new ID.
                                    CObject( const CObject & );
                                    /// Destroys the object; its ID becomes -1.
            virtual                ~CObject( void );

                                        /// Assignment copies the value, not
                                        /// the identity: the ID stays.
            CObject                &operator =( const CObject & ) { return *this; }

                /// The object's ID, unique for the life of the program.
            unsigned long           GetID( void ) const { return m_nObjectID; }

                /// Replaces the handler ASSERT() calls.
                /// @param pfnHandler The new handler. NULL restores the
                ///     default, which prints the failure to stderr.
                /// @return The previous handler.
            static AssertHandler    SetAssertHandler( AssertHandler pfnHandler );
                /// Calls the current assert handler. This is what ASSERT()
                /// does when its condition is false.
                /// @param strFile The source file of the assertion.
                /// @param nLine The line of the assertion.
                /// @param strCondition The condition's text.
            static void             CallAssertHandler( const char *strFile, int nLine, const char *strCondition );

                /// A hash for hashed collections. Objects are equal only to
                /// themselves unless a class says otherwise, so the default is
                /// the ID; classes with values (CString) hash the value.
            virtual int             hashCode( void ) const { return (int)m_nObjectID; }

                /// What the classes' operator new calls when the memory
                /// manager is built in: memory from the current
                /// CMemoryManager.
                /// @param size The number of bytes.
                /// @param name The class's name, for the manager's reports.
                /// @return The memory, never NULL.
                /// @throws std::bad_alloc When memory runs out.
            static void            *allocate( size_t size, const char *name );
                /// Same as allocate(), recording where the allocation was
                /// made (the form IASLIB_NEW uses).
                /// @param size The number of bytes.
                /// @param name The class's name, for the manager's reports.
                /// @param strFile The source file of the allocation.
                /// @param nLine The line of the allocation.
                /// @return The memory, never NULL.
                /// @throws std::bad_alloc When memory runs out.
            static void            *allocateAt( size_t size, const char *name, const char *strFile, int nLine );
                /// What the classes' operator delete calls: gives memory from
                /// allocate() back to the CMemoryManager it came from, which
                /// reports memory freed as a different class than it was
                /// allocated as. deallocate( NULL ) does nothing.
                /// @param p The memory, or NULL.
                /// @param name The class's name.
            static void             deallocate( void *p, const char *name );
    };
} // End of Namespace IASLib

    /// @def IASLIB_MEMORY_DECLARE__(x)
    /// The classes' own new and delete, through the memory manager; empty
    /// when the memory manager is not built in. The (file, line) forms are
    /// what IASLIB_NEW uses; the placement forms keep "new( p ) T" working.
    /// DEFINE_OBJECT() and DECLARE_OBJECT() include it.
#if defined( IASLIB_MEMORY_MANAGER__ ) || defined( IASLIB_MEMORY_DEBUGGING__ )
    #define IASLIB_MEMORY_DECLARE__(x) \
        static void        *operator new( size_t size ) { return IASLib::CObject::allocate( size, #x ); } \
        static void        *operator new( size_t size, const char *strFile, int nLine ) { return IASLib::CObject::allocateAt( size, #x, strFile, nLine ); } \
        static void         operator delete( void *p ) { IASLib::CObject::deallocate( p, #x ); } \
        static void         operator delete( void *p, const char *, int ) { IASLib::CObject::deallocate( p, #x ); } \
        static void        *operator new( size_t, void *pPlace ) { return pPlace; } \
        static void         operator delete( void *, void * ) {}
#else
    #define IASLIB_MEMORY_DECLARE__(x)
#endif

    /// @def DEFINE_OBJECT(x)
    /// Names class x: goes in the class declaration, with
    /// IMPLEMENT_OBJECT( x, base ) in its source file. Declares GetType() and
    /// IsType() and, with the memory manager, the class's new and delete.
    /// (Without RTTI the memory manager was not used either; then it is only
    /// IASLIB_MEMORY_DECLARE__().)

    /// @def DECLARE_OBJECT(x,y)
    /// Names class x, derived from y: goes in the class declaration, by
    /// itself. Defines GetType() and IsType() and, with the memory manager,
    /// the class's new and delete.

    /// @def IMPLEMENT_OBJECT(x,y)
    /// Defines x::IsType() (x derived from y) in the source file of a class
    /// that uses DEFINE_OBJECT( x ). Empty without IASLIB_RTTI__.
#ifdef IASLIB_RTTI__
        /// The body of x::IsType(): true for the name x, otherwise whatever
        /// the base class y says.
    #define IASLIB_RTTI_IS_TYPE_BODY__(x,y) \
        { return ( strName != NULL ) && ( ( strcmp( strName, #x ) == 0 ) || y::IsType( strName ) ); }

        // In a class declaration, with IMPLEMENT_OBJECT( x, y ) in its source.
    #define DEFINE_OBJECT(x) \
        virtual const char *GetType( void ) const { return #x; } \
        virtual bool        IsType( const char *strName ) const; \
        IASLIB_MEMORY_DECLARE__(x)

        // In a class declaration, by itself.
    #define DECLARE_OBJECT(x,y) \
        virtual const char *GetType( void ) const { return #x; } \
        virtual bool        IsType( const char *strName ) const IASLIB_RTTI_IS_TYPE_BODY__(x,y) \
        IASLIB_MEMORY_DECLARE__(x)

    #define IMPLEMENT_OBJECT(x,y) \
        bool x::IsType( const char *strName ) const IASLIB_RTTI_IS_TYPE_BODY__(x,y)
#else
        // (Without RTTI the memory manager was not used either.)
    #define DEFINE_OBJECT(x) IASLIB_MEMORY_DECLARE__(x)
    #define DECLARE_OBJECT(x,y) IASLIB_MEMORY_DECLARE__(x)
    #define IMPLEMENT_OBJECT(x,y)
#endif

/// @def ASSERT(x)
/// In IASLIB_DEBUG__ builds, calls CObject::CallAssertHandler() when x is
/// false. Otherwise it does nothing and x is not evaluated.
#ifdef IASLIB_DEBUG__
#define ASSERT(x) do { if ( !(x) ) IASLib::CObject::CallAssertHandler( __FILE__, __LINE__, #x ); } while ( 0 )
#else
#define ASSERT(x) do { } while ( 0 )
#endif

#endif
