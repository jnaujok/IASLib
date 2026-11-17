/***********************************************************************
**  TestSockets
**
**  The few socket calls the tests make directly, on any platform: the
**  POSIX names on Linux and macOS, and their Winsock equivalents on
**  Windows (where Winsock is also started before main runs).
***********************************************************************/

#ifndef IASLIB_TESTSOCKETS_H__
#define IASLIB_TESTSOCKETS_H__

#ifdef IASLIB_WIN32__
    #include "BaseTypes/Platform.h"

    typedef SSIZE_T ssize_t;
    #define poll WSAPoll
    #ifndef MSG_NOSIGNAL
        #define MSG_NOSIGNAL 0
    #endif

    inline void closeSocket( int fd ) { closesocket( (SOCKET)fd ); }

    struct CTestWinsock
    {
        CTestWinsock( void ) { WSADATA data; WSAStartup( MAKEWORD( 2, 2 ), &data ); }
        ~CTestWinsock( void ) { WSACleanup(); }
    };
    static CTestWinsock g_testWinsock;
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <poll.h>
    #include <sys/socket.h>
    #include <unistd.h>

    #ifndef MSG_NOSIGNAL
        #define MSG_NOSIGNAL 0
    #endif

    inline void closeSocket( int fd ) { close( fd ); }
#endif

#endif // IASLIB_TESTSOCKETS_H__
