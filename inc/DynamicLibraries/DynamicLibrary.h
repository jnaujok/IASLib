/**
 * @file
 * @brief Dynamic Library Class.
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
    /// The platform's handle for a loaded library.
    typedef HMODULE DYN_LIB_TYPE;
#else
    #include <dlfcn.h>
    /// The platform's handle for a loaded library.
    typedef void *DYN_LIB_TYPE;
#endif

namespace IASLib
{
    /// A shared library (.so, .dylib or .dll), loaded for as long as the
    /// object lives, and the functions in it.
    ///
    /// A name with a directory or an extension is used as given; a bare
    /// name is tried in the platform's form first (see PlatformName()), then
    /// as given. The library is unloaded when the object is destroyed, so
    /// pointers from GetFunction() must not be used after that. Not
    /// copyable.
    ///
    /// @code
    /// CDynamicLibrary oLib( "m" );            // libm.so, libm.dylib or m.dll
    /// if ( oLib.IsLoaded() )
    /// {
    ///     typedef double (*CosFn)( double );
    ///     CosFn pCos = (CosFn)oLib.FindFunction( "cos" );
    /// }
    /// @endcode
    class CDynamicLibrary : public CObject
    {
        protected:
            DYN_LIB_TYPE        m_LibHandle;    ///< The loaded library, or NULL.
            CString             m_strPath;      ///< the name that loaded
            CString             m_strError;     ///< Why the last load or lookup failed ("" if it worked).

        public:
                                /// Loads the library; IsLoaded() says whether
                                /// it did, and GetError() why not.
                                /// @note A failure used to be silent, and every
                                /// later call then used a NULL handle: dlclose()
                                /// crashed, and dlsym() searched the program itself.
                                /// Only "lib%s.so" was tried, on macOS too.
                                CDynamicLibrary( const char *strLibPath );
                                /// Unloads it.
            virtual            ~CDynamicLibrary( void );

                                DECLARE_OBJECT(CDynamicLibrary,CObject);

            bool                IsLoaded( void ) const { return m_LibHandle != NULL; }    ///< Whether the library loaded.
                /// The name that was loaded (after the platform's form was
                /// applied), or "".
            const char         *GetPath( void ) const { return m_strPath; }
                /// Why the library or the last function could not be found.
            const char         *GetError( void ) const { return m_strError; }

                /// The address of an exported function or variable.
                /// @throws CException (CWindowsSystemException on Windows) when it
                /// is not there or the library is not loaded.
            void               *GetFunction( const char *strFunctionName );
                /// The same, but NULL instead of throwing (GetError() says why).
            void               *FindFunction( const char *strFunctionName );

                /// The platform's file name for a bare library name
                /// ("foo" -> "libfoo.so", "libfoo.dylib" or "foo.dll").
            static CString      PlatformName( const char *strName );

        private:
                                CDynamicLibrary( const CDynamicLibrary & );     ///< Not copyable.
            CDynamicLibrary    &operator =( const CDynamicLibrary & );         ///< Not copyable.
            bool                TryLoad( const char *strName );     ///< Loads @p strName; false (with m_strError set) if it cannot.
    };
} // namespace IASLib

#endif // IASLIB_DYNAMICLIBRARY_H__
