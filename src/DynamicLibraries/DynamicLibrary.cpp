/*
 *  Dynamic Library Class
 *
 *      See DynamicLibrary.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/15/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "DynamicLibrary.h"
#include "Exception.h"
#include "WindowsSystemException.h"

#include <cstring>

namespace IASLib
{
    namespace
    {
            // Whether the name already says where or what file it is: a
            // directory, or an extension (".so" may be followed by a
            // version, as in "libm.so.6").
        bool IsExplicitName( const char *strName )
        {
            if ( strchr( strName, '/' ) || strchr( strName, '\\' ) )
                return true;
            if ( strstr( strName, ".so" ) || strstr( strName, ".dylib" ) || strstr( strName, ".dll" ) || strstr( strName, ".DLL" ) )
                return true;
            return false;
        }
    }

    CDynamicLibrary::CDynamicLibrary( const char *strLibPath )
    {
        m_LibHandle = NULL;
        if ( strLibPath == NULL || *strLibPath == '\0' )
        {
            m_strError = "No library name was given.";
            return;
        }
            // (Only "lib%s.so" was tried, on macOS too, so a path or a full
            // file name could never load.)
        if ( IsExplicitName( strLibPath ) )
        {
            TryLoad( strLibPath );
            return;
        }
        CString strPlatform = PlatformName( strLibPath );
        if ( TryLoad( strPlatform ) )
            return;
        CString strFirstError = m_strError;
        if ( ! TryLoad( strLibPath ) )
            m_strError = strFirstError;
    }

    CDynamicLibrary::~CDynamicLibrary( void )
    {
        if ( m_LibHandle == NULL )
            return;
    #ifdef IASLIB_WIN32__
        FreeLibrary( m_LibHandle );
    #else
        dlclose( m_LibHandle );
    #endif
        m_LibHandle = NULL;
    }

    CString CDynamicLibrary::PlatformName( const char *strName )
    {
        CString strResult;
    #if defined( IASLIB_WIN32__ )
        strResult.Format( "%s.dll", strName );
    #elif defined( IASLIB_MAC__ ) || defined( __APPLE__ )
        strResult.Format( "lib%s.dylib", strName );
    #else
        strResult.Format( "lib%s.so", strName );
    #endif
        return strResult;
    }

    bool CDynamicLibrary::TryLoad( const char *strName )
    {
    #ifdef IASLIB_WIN32__
        m_LibHandle = LoadLibraryA( strName );
        if ( m_LibHandle == NULL )
        {
            m_strError.Format( "LoadLibrary(\"%s\") failed with error %lu.", strName, (unsigned long)GetLastError() );
            return false;
        }
    #else
        dlerror();
        m_LibHandle = dlopen( strName, RTLD_LAZY | RTLD_LOCAL );
        if ( m_LibHandle == NULL )
        {
            const char *strError = dlerror();
            m_strError = ( strError ) ? strError : "dlopen() failed.";
            return false;
        }
    #endif
        m_strPath = strName;
        m_strError = "";
        return true;
    }

    void *CDynamicLibrary::FindFunction( const char *strFunctionName )
    {
        if ( m_LibHandle == NULL )
        {
            m_strError = "The library is not loaded.";
            return NULL;
        }
        if ( strFunctionName == NULL || *strFunctionName == '\0' )
        {
            m_strError = "No function name was given.";
            return NULL;
        }
    #ifdef IASLIB_WIN32__
        void *pRetVal = (void *)GetProcAddress( m_LibHandle, strFunctionName );
        if ( pRetVal == NULL )
            m_strError.Format( "\"%s\" was not found (error %lu).", strFunctionName, (unsigned long)GetLastError() );
    #else
        dlerror();
        void *pRetVal = dlsym( m_LibHandle, strFunctionName );
        const char *strError = dlerror();
        if ( strError )
        {
            m_strError = strError;
            pRetVal = NULL;
        }
    #endif
        return pRetVal;
    }

    void *CDynamicLibrary::GetFunction( const char *strFunctionName )
    {
        if ( m_LibHandle == NULL )
        {
            IASLIB_THROW_DEFAULT( "CDynamicLibrary: the library is not loaded." );
        }
    #ifdef IASLIB_WIN32__
        void *pRetVal = FindFunction( strFunctionName );
        if ( pRetVal == NULL )
        {
            IASLIB_THROW_WINDOWS_SYSTEM_ERROR( GetLastError() );
        }
    #else
        void *pRetVal = FindFunction( strFunctionName );
        if ( pRetVal == NULL )
        {
            IASLIB_THROW_DEFAULT( (const char *)m_strError );
        }
    #endif
        return pRetVal;
    }
} // namespace IASLib
