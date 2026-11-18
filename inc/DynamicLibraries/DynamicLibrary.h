/**
 * @file
 * @brief Dynamic Library Class.
 *
 *      This class gives us access to dynamic libraries in a cross-
 * platform, architecture independent manor. This will allow us to
 * build applications which can use dynamic libraries without worrying
 * about the underlying mechanism.
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
    /// Platform handle of a loaded library.
    typedef HMODULE DYN_LIB_TYPE;
#endif

#ifdef IASLIB_SUN__
#include <dlfcn.h>
    /// Platform handle of a loaded library.
    typedef void *DYN_LIB_TYPE;
#endif

#ifdef IASLIB_DEC__
#include <dlfcn.h>
    /// Platform handle of a loaded library.
    typedef void *DYN_LIB_TYPE;
#endif

#if defined( IASLIB_LINUX__ ) || defined( IASLIB_MAC__ )
    #include <dlfcn.h>
    /// Platform handle of a loaded library (a dlopen() handle here; an HMODULE on Windows).
    typedef void *DYN_LIB_TYPE;
    //#define __cdecl
#endif

namespace IASLib
{
    /// Loads a shared library (.so or .dll) and looks up functions in it.
    ///
    /// The library stays loaded for the life of the object and is unloaded
    /// by the destructor. Pointers from GetFunction() are invalid after that.
    ///
    /// @code
    /// CDynamicLibrary oLib( "sqlite3" );    // loads libsqlite3.so or sqlite3.dll
    /// typedef const char *(*VersionFn)( void );
    /// VersionFn pfn = (VersionFn)oLib.GetFunction( "sqlite3_libversion" );
    /// @endcode
    class CDynamicLibrary : public CObject
    {
        protected:
            DYN_LIB_TYPE        m_LibHandle;    ///< Handle of the loaded library, or NULL if loading failed.

        public:

                                /// Loads a library by base name.
                                /// On Linux, macOS and Solaris it opens "libNAME.so" with
                                /// dlopen( RTLD_LAZY ); on Windows it opens "NAME.dll" with
                                /// LoadLibrary(). The normal system search path is used.
                                /// A failure does not throw: the handle is left NULL and
                                /// GetFunction() will then fail.
                                /// @param strLibPath Library base name, without "lib" or extension.
                                /// @note The DEC branch never opens a library.
                                CDynamicLibrary( const char *strLibPath );
            /// Unloads the library (dlclose() or FreeLibrary()).
            virtual            ~CDynamicLibrary( void );

                                DECLARE_OBJECT(CDynamicLibrary,CObject);

            /// Looks up an exported symbol.
            /// @param strFunctionName Name of the exported function.
            /// @return Address of the function; cast it to the right function pointer type.
            /// @throws CException with the dlerror() text if the lookup fails (POSIX).
            /// @throws CWindowsSystemException if GetProcAddress() fails (Windows).
            void               *GetFunction( const char *strFunctionName );
    };
} // namespace IASLib

#endif // IASLIB_DYNAMICLIBRARY_H__

