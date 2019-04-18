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

#ifndef IASLIB_DYNAMICLIBRARY_H__
#define IASLIB_DYNAMICLIBRARY_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

#ifdef IASLIB_WIN32__
#include <windows.h>
    typedef HMODULE DYN_LIB_TYPE;
#endif

#ifdef IASLIB_SUN__
#include <dlfcn.h>
    typedef void *DYN_LIB_TYPE;
#endif

#ifdef IASLIB_DEC__
#include <dlfcn.h>
    typedef void *DYN_LIB_TYPE;
#endif

#ifdef IASLIB_LINUX__
#include <dlfcn.h>
    typedef void *DYN_LIB_TYPE;
#endif

namespace IASLib
{
    class CDynamicLibrary : public CObject
    {
        protected:
            DYN_LIB_TYPE        m_LibHandle;
            
        public:

                                CDynamicLibrary( const char *strLibPath );
            virtual            ~CDynamicLibrary( void );

                                DECLARE_OBJECT(CDynamicLibrary,CObject);

            void               *GetFunction( const char *strFunctionName );
    };
} // namespace IASLib

#endif // IASLIB_DYNAMICLIBRARY_H__

