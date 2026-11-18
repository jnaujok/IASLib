/**
 * @file SocketWait.h
 * @brief Waiting for a socket to become readable or writable, with a timeout.
 *
 * Socket Wait
 *
 *      Waits for a socket to become readable, with a timeout, on every
 * platform: poll() on POSIX systems and WSAPoll() on Windows (Vista and
 * later). Used by the timed reads and accepts.
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SOCKETWAIT_H__
#define IASLIB_SOCKETWAIT_H__

#ifdef IASLIB_NETWORKING__

#ifdef IASLIB_WIN32__
    #include "../BaseTypes/Platform.h"
#else
    #include <errno.h>
    #include <poll.h>
#endif

namespace IASLib
{
        /// Waits for a socket to become readable.
        /// @param hSocket  The socket handle.
        /// @param nTimeoutMs  The longest wait in milliseconds; negative waits forever.
        /// @return 1 if the socket can be read (or has an error or hang-up to
        ///         report), 0 if the time ran out (or the wait was
        ///         interrupted), -1 on error.
    template <typename SocketType>
    inline int WaitForReadable( SocketType hSocket, int nTimeoutMs )
    {
#ifdef IASLIB_WIN32__
        WSAPOLLFD pfd;
        pfd.fd = (SOCKET)hSocket;
        pfd.events = POLLRDNORM;
        pfd.revents = 0;
        int nReady = WSAPoll( &pfd, 1, nTimeoutMs );
        if ( nReady == SOCKET_ERROR )
            return ( WSAGetLastError() == WSAEINTR ) ? 0 : -1;
        return ( nReady > 0 ) ? 1 : 0;
#else
        struct pollfd pfd;
        pfd.fd = (int)hSocket;
        pfd.events = POLLIN;
        pfd.revents = 0;
        int nReady = poll( &pfd, 1, nTimeoutMs );
        if ( nReady < 0 )
            return ( errno == EINTR ) ? 0 : -1;
        return ( nReady > 0 ) ? 1 : 0;
#endif
    }

        /// The same for writing: 1 if the socket can take more data.
        /// @param hSocket  The socket handle.
        /// @param nTimeoutMs  The longest wait in milliseconds; negative waits forever.
        /// @return 1 if it can be written, 0 if the time ran out (or the wait
        ///         was interrupted), -1 on error.
    template <typename SocketType>
    inline int WaitForWritable( SocketType hSocket, int nTimeoutMs )
    {
#ifdef IASLIB_WIN32__
        WSAPOLLFD pfd;
        pfd.fd = (SOCKET)hSocket;
        pfd.events = POLLWRNORM;
        pfd.revents = 0;
        int nReady = WSAPoll( &pfd, 1, nTimeoutMs );
        if ( nReady == SOCKET_ERROR )
            return ( WSAGetLastError() == WSAEINTR ) ? 0 : -1;
        return ( nReady > 0 ) ? 1 : 0;
#else
        struct pollfd pfd;
        pfd.fd = (int)hSocket;
        pfd.events = POLLOUT;
        pfd.revents = 0;
        int nReady = poll( &pfd, 1, nTimeoutMs );
        if ( nReady < 0 )
            return ( errno == EINTR ) ? 0 : -1;
        return ( nReady > 0 ) ? 1 : 0;
#endif
    }

        /// True if the last socket call failed only because it would block or
        /// timed out (as opposed to a real error). An interrupted call counts
        /// as well.
    inline bool SocketErrorIsTimeout( void )
    {
#ifdef IASLIB_WIN32__
        int nError = WSAGetLastError();
        return nError == WSAEWOULDBLOCK || nError == WSAETIMEDOUT || nError == WSAEINTR;
#else
        return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR;
#endif
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SOCKETWAIT_H__
