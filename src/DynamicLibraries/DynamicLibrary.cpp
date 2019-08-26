/*
 *  Dynamic Library Class
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

#include "DynamicLibrary.h"
#include "WindowsSystemException.h"

namespace IASLib
{
    CDynamicLibrary::CDynamicLibrary( const char *strLibPath )
    {
        m_LibHandle = NULL;
        CString libName;

        // TODO: Update to do intelligent path handling.
    #ifdef IASLIB_SUN__
        libName.Format( "lib%s.so", strLibPath );
        m_LibHandle = dlopen( libName, RTLD_LAZY );
    #endif

    #ifdef IASLIB_LINUX__
        libName.Format( "lib%s.so", strLibPath );
        m_LibHandle = dlopen( libName, RTLD_LAZY );
    #endif

    #ifdef IASLIB_WIN32__
        libName.Format( "%s.dll", strLibPath );
        m_LibHandle = LoadLibrary( libName );
    #endif
    }

    CDynamicLibrary::~CDynamicLibrary( void )
    {
    #ifdef IASLIB_DEC__
        dlclose( m_LibHandle );
    #endif

    #ifdef IASLIB_SUN__
        dlclose( m_LibHandle );
    #endif

    #ifdef IASLIB_LINUX__
        dlclose( m_LibHandle );
    #endif

    #ifdef IASLIB_WIN32__
        FreeLibrary( m_LibHandle );
    #endif
    }

    void *CDynamicLibrary::GetFunction( const char *strFunctionName )
    {
        void       *pRetVal = NULL;

   #ifdef IASLIB_DEC__
        const char *strError = NULL;
        dlerror();
        pRetVal = dlsym( m_LibHandle, strFunctionName );
        strError = dlerror();
        if ( strError )
        {
            IASLIB_THROW_DEFAULT( strError );
        }
    #endif

    #ifdef IASLIB_SUN__
        const char *strError = NULL;
        dlerror();
        pRetVal = dlsym( m_LibHandle, strFunctionName );
        strError = dlerror();
        if ( strError )
        {
            IASLIB_THROW_DEFAULT( strError );
        }
    #endif

    #ifdef IASLIB_LINUX__
        const char *strError = NULL;
        dlerror();
        pRetVal = dlsym( m_LibHandle, strFunctionName );
        strError = dlerror();
        if ( strError )
        {
            IASLIB_THROW_DEFAULT( strError );
        }
    #endif

    #ifdef IASLIB_WIN32__
        pRetVal = GetProcAddress( m_LibHandle, strFunctionName );
        if (! pRetVal )
        {
            IASLIB_THROW_WINDOWS_SYSTEM_ERROR( GetLastError() );
        }
    #endif

      return pRetVal;
    }
} // namespace IASLib
