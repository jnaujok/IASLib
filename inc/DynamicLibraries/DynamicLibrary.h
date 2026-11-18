/*
 *  Dynamic Library Class
 *
 *      Loads a shared library (.so, .dylib or .dll) and finds functions in
 * it, the same way on every platform.
 *
 *      The name is used as given when it has a directory or an extension
 * ("./plugins/libfoo.so", "libm.so.6", "user32.dll"). A bare name is
 * tried in the platform's form first - "foo" is libfoo.so on Linux,
 * libfoo.dylib on macOS, foo.dll on Windows - and then as given, through
 * the system's usual search.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/15/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_DYNAMICLIBRARY_H__
#define IASLIB_DYNAMICLIBRARY_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

#ifdef IASLIB_WIN32__
#include "../BaseTypes/Platform.h"
    typedef HMODULE DYN_LIB_TYPE;
#else
    #include <dlfcn.h>
    typedef void *DYN_LIB_TYPE;
#endif

namespace IASLib
{
    class CDynamicLibrary : public CObject
    {
        protected:
            DYN_LIB_TYPE        m_LibHandle;
            CString             m_strPath;      // the name that loaded
            CString             m_strError;

        public:
                                // Loads the library; IsLoaded() says whether
                                // it did, and GetError() why not. (A failure
                                // was silent, and every later call then
                                // used a NULL handle: dlclose() crashed, and
                                // dlsym() searched the program itself.)
                                CDynamicLibrary( const char *strLibPath );
                                // Unloads it.
            virtual            ~CDynamicLibrary( void );

                                DECLARE_OBJECT(CDynamicLibrary,CObject);

            bool                IsLoaded( void ) const { return m_LibHandle != NULL; }
                // The name that was loaded (after the platform's form was
                // applied), or "".
            const char         *GetPath( void ) const { return m_strPath; }
                // Why the library or the last function could not be found.
            const char         *GetError( void ) const { return m_strError; }

                // The address of an exported function or variable. Throws a
                // CException (CWindowsSystemException on Windows) when it
                // is not there or the library is not loaded.
            void               *GetFunction( const char *strFunctionName );
                // The same, but NULL instead of throwing.
            void               *FindFunction( const char *strFunctionName );

                // The platform's file name for a bare library name
                // ("foo" -> "libfoo.so", "libfoo.dylib" or "foo.dll").
            static CString      PlatformName( const char *strName );

        private:
                                CDynamicLibrary( const CDynamicLibrary & );
            CDynamicLibrary    &operator =( const CDynamicLibrary & );
            bool                TryLoad( const char *strName );
    };
} // namespace IASLib

#endif // IASLIB_DYNAMICLIBRARY_H__
