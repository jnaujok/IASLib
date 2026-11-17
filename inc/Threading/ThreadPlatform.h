/***********************************************************************
**  Thread Platform
**
**  Description:
**      Chooses the threading API from the platform flag alone: Win32
**  threads on Windows, POSIX threads everywhere else (Linux, macOS and
**  other Unix systems). Included by the threading headers, so that
**  IASLIB_PTHREAD__ never needs to be set by hand.
**
**  Copyright (c) 2026, The Irene Adler Software Group, all rights reserved.
***********************************************************************/

#ifndef IASLIB_THREADPLATFORM_H__
#define IASLIB_THREADPLATFORM_H__

#if defined( IASLIB_MULTI_THREADED__ ) && ! defined( IASLIB_WIN32__ ) && ! defined( IASLIB_PTHREAD__ )
    #define IASLIB_PTHREAD__
#endif

#endif // IASLIB_THREADPLATFORM_H__
