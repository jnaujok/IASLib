/**
 * CObject class
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

#ifndef IASLIB_OBJECT_H__
#define IASLIB_OBJECT_H__

#ifdef IASLIB_WIN32__
#ifndef WINVER
#define WINVER 0x0500
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#endif

#ifdef IASLIB_MULTI_THREADED__
#include "../Threading/Mutex.h"
#endif

#ifndef NULL
#define NULL 0L
#endif

#ifndef IASLIB_WIN32__
    typedef char IASLibChar__;
    #include <stdlib.h>
#else
    #include <TCHAR.h>
    typedef _TCHAR IASLibChar__;
#endif


namespace IASLib
{
        // This constant is used in a lot of places
    const size_t NOT_FOUND = (size_t)-1;

    class CObject
    {
        private:
            static unsigned long    m_ulCurrentID;
#ifdef IASLIB_MULTI_THREADED__
            static CMutex           m_mutexCreation;
#endif
            unsigned long           m_nObjectID;
        public:
#ifdef IASLIB_RTTI__
            virtual const char     *GetType( void ) = 0;
            virtual bool            IsType( const char *strName );
#endif
                                    CObject( void );
            virtual                ~CObject( void );

            unsigned long           GetID( void ) const { return m_nObjectID; }

            static void             SetAssertHandler();
            static void             CallAssertHandler( const char *strFile, int nLine, const char *strCondition );

            virtual int             hashCode( void ) const { return (int)m_nObjectID; }

            static void            *allocate( size_t size, const char *name );
            static void             deallocate( void *p, const char *name );
    };
} // End of Namespace IASLib

#ifdef IASLIB_RTTI__
#define DEFINE_OBJECT(x) virtual const char *GetType( void ) { return #x; }\
                         virtual bool        IsType( const char *strName ); \
                         void               *operator new( size_t size ); \
                         void                operator delete( void *p );

#define DECLARE_OBJECT(x,y) virtual const char *GetType( void ) { return #x; }\
							virtual bool IsType( const char *strName )\
                              { \
                                  IASLib::CString strTemp = strName; \
                                  if ( strTemp == #x ) \
                                      return true; \
                                  else \
                                      return y::IsType( strName ); \
                              } \
                              void *operator new( size_t size ) \
                              { \
                                return CObject::allocate( size, #x ); \
                              } \
                              void operator delete( void *p ) \
                              { \
                                  CObject::deallocate( p, #x ); \
                              }

#define IMPLEMENT_OBJECT(x,y) bool x::IsType( const char *strName )\
                              { \
                                  IASLib::CString strTemp = strName; \
                                  if ( strTemp == #x ) \
                                      return true; \
                                  else \
                                      return y::IsType( strName ); \
                              } \
                              void *x::operator new( size_t size ) \
                              { \
                                return CObject::allocate( size, #x ); \
                              } \
                              void x::operator delete( void *p ) \
                              { \
                                  CObject::deallocate( p, #x ); \
                              }

#else
#define DEFINE_OBJECT(x)
#define DECLARE_OBJECT(x,y)
#define IMPLEMENT_OBJECT(x,y)
#endif

#ifdef IASLIB_DEBUG__
#define ASSERT(x) { if ( !(x) ) IASLib::CObject::CallAssertHandler( __FILE__, __LINE__, #x ); }
#else
#define ASSERT(x)
#endif

#endif
