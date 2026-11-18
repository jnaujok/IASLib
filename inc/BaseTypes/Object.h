/**
 * CObject class
 *
 *	This class defines the base (abstract) class that all objects in the
 * IAS Library are based on.
 *
 *      Every object gets an ID, unique for the life of the program (a copy
 * gets its own ID). With IASLIB_RTTI__, classes name themselves with
 * DECLARE_OBJECT( class, base ) in the class, or DEFINE_OBJECT( class ) in
 * the class and IMPLEMENT_OBJECT( class, base ) in its source file, giving
 * GetType() and IsType(). With IASLIB_MEMORY_DEBUGGING__ as well, those
 * classes are allocated with fenceposts that are checked when they are
 * deleted (see allocate()).
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
#define NULL 0L
#endif

    // The library's character type: char on every platform. (It was TCHAR
    // on Windows, which is wchar_t in UNICODE builds, but the string code
    // has only ever handled char.)
typedef char IASLibChar__;

namespace IASLib
{
        // This constant is used in a lot of places
    const size_t NOT_FOUND = (size_t)-1;

    class CObject
    {
        private:
            unsigned long           m_nObjectID;

        public:
                // Called by ASSERT() when its condition is false.
            typedef void          (*AssertHandler)( const char *strFile, int nLine, const char *strCondition );

#ifdef IASLIB_RTTI__
            virtual const char     *GetType( void ) const = 0;
            virtual bool            IsType( const char *strName ) const;
#endif
                                    CObject( void );
                                        // A copy is a different object: it
                                        // gets a new ID.
                                    CObject( const CObject & );
            virtual                ~CObject( void );

                                        // Assignment copies the value, not
                                        // the identity: the ID stays.
            CObject                &operator =( const CObject & ) { return *this; }

            unsigned long           GetID( void ) const { return m_nObjectID; }

                // Replaces the handler ASSERT() calls (NULL restores the
                // default, which prints the failure to stderr), returning the
                // previous one.
            static AssertHandler    SetAssertHandler( AssertHandler pfnHandler );
            static void             CallAssertHandler( const char *strFile, int nLine, const char *strCondition );

                // A hash for hashed collections. Objects are equal only to
                // themselves unless a class says otherwise, so the default is
                // the ID; classes with values (CString) hash the value.
            virtual int             hashCode( void ) const { return (int)m_nObjectID; }

                // The memory-debugging allocator: memory with a header and
                // fenceposts before and after, zeroed. deallocate() reports
                // (on stderr) a damaged fencepost, or memory freed as a
                // different class than it was allocated as. Throws
                // std::bad_alloc when memory runs out; deallocate( NULL ) does
                // nothing.
            static void            *allocate( size_t size, const char *name );
            static void             deallocate( void *p, const char *name );
    };
} // End of Namespace IASLib

#ifdef IASLIB_RTTI__
    #define IASLIB_RTTI_IS_TYPE_BODY__(x,y) \
        { return ( strName != NULL ) && ( ( strcmp( strName, #x ) == 0 ) || y::IsType( strName ) ); }

    #ifdef IASLIB_MEMORY_DEBUGGING__
        #define IASLIB_MEMORY_DECLARE__(x) \
            static void        *operator new( size_t size ) { return IASLib::CObject::allocate( size, #x ); } \
            static void         operator delete( void *p ) { IASLib::CObject::deallocate( p, #x ); } \
            static void        *operator new( size_t, void *pPlace ) { return pPlace; } \
            static void         operator delete( void *, void * ) {}
    #else
        #define IASLIB_MEMORY_DECLARE__(x)
    #endif

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
    #define DEFINE_OBJECT(x)
    #define DECLARE_OBJECT(x,y)
    #define IMPLEMENT_OBJECT(x,y)
#endif

#ifdef IASLIB_DEBUG__
#define ASSERT(x) do { if ( !(x) ) IASLib::CObject::CallAssertHandler( __FILE__, __LINE__, #x ); } while ( 0 )
#else
#define ASSERT(x) do { } while ( 0 )
#endif

#endif
