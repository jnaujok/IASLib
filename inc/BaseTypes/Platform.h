/**
 * @file Platform.h
 * @brief The operating system headers the library needs, in the right order.
 *
 ***********************************************************************
 **  Platform
 **
 **  Description:
 **      The operating system headers the library needs, chosen only by the
 **  platform flag (IASLIB_WIN32__, IASLIB_LINUX__, IASLIB_MAC__, ...). On
 **  Windows the order matters: winsock2.h and ws2tcpip.h must come before
 **  windows.h, which is included lean (otherwise it pulls in the old
 **  winsock.h, which conflicts with winsock2.h) and without the min/max
 **  macros. Every library header that needs a Windows header includes this
 **  one instead.
 **
 **  Copyright (c) 2026, The Irene Adler Software Group, all rights reserved.
 ***********************************************************************/

#ifndef IASLIB_PLATFORM_H__
#define IASLIB_PLATFORM_H__

#ifdef IASLIB_WIN32__
        // Windows 10 or later, unless the program asks for another version:
        // the socket code needs at least Vista (getaddrinfo, inet_ntop).
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0A00
    #endif
    #ifndef WINVER
        #define WINVER _WIN32_WINNT
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
        // No GDI: the library draws nothing, and wingdi.h defines macros
        // such as ERROR, OPAQUE and TRANSPARENT that clash with ordinary names.
    #ifndef NOGDI
        #define NOGDI
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    #ifdef _MSC_VER
        #pragma comment( lib, "ws2_32.lib" )
    #endif

        /// POSIX names the socket code uses.
    typedef ADDRESS_FAMILY sa_family_t;
#endif

#endif // IASLIB_PLATFORM_H__
