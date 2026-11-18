/*
 * Socket Exception Class
 *
 *      This class wraps socket based exceptions. When a socket fails,
 * it can throw an exception of this type that can be caught by a handler
 * at a lower level.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "SocketException.h"
#include <errno.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CSocketException, CException );

    CSocketException::CSocketException( int nErrorNumber ) : CException( "Unknown Socket Exception", CException::FATAL )
    {
        m_nErrorNumber = nErrorNumber;

            // Error numbers are errno values on every platform: CSocket
            // translates Winsock's WSAE* codes before throwing.
        switch ( m_nErrorNumber )
        {
            case EACCES:
                m_strExceptionMessage = "Permission to use the socket or address has been denied.";
                m_nPriority = CException::FATAL;
                break;

            case EMFILE:
                m_strExceptionMessage = "Could not create socket. File descriptor table is full.";
                m_nPriority = CException::FATAL;
                break;

            case ENOMEM:
                m_strExceptionMessage = "Could not create socket. Insufficient Memory.";
                m_nPriority = CException::FATAL;
                break;

            case ENOBUFS:
                m_strExceptionMessage = "No buffer space is available.";
                m_nPriority = CException::FATAL;
                break;

            case EBADF:
                m_strExceptionMessage = "Invalid socket descriptor.";
                m_nPriority = CException::FATAL;
                break;

            case EINVAL:
                m_strExceptionMessage = "Invalid argument (the socket may be in the wrong state).";
                m_nPriority = CException::HIGH;
                break;

            case EINTR:
                m_strExceptionMessage = "Signal interrupted delivery.";
                m_nPriority = CException::WARNING;
                break;

            case ENODEV:
                m_strExceptionMessage = "Could not associate socket with a device.";
                m_nPriority = CException::FATAL;
                break;

            case EDEADLK:
                m_strExceptionMessage = "Blocking would have caused a dead-lock to occur.";
                m_nPriority = CException::NORMAL;
                break;

            case EPIPE:
                m_strExceptionMessage = "The socket pipe is broken.";
                m_nPriority = CException::FATAL;
                break;

            case EPROTONOSUPPORT:
                m_strExceptionMessage = "Could not create socket. Protocol is not supported.";
                m_nPriority = CException::FATAL;
                break;

            case EAFNOSUPPORT:
                m_strExceptionMessage = "The address family is not supported.";
                m_nPriority = CException::FATAL;
                break;

            case ENOTSOCK:
                m_strExceptionMessage = "Descriptor is not a socket.";
                m_nPriority = CException::HIGH;
                break;

            case EOPNOTSUPP:
                m_strExceptionMessage = "Operation is not supported on this socket.";
                m_nPriority = CException::HIGH;
                break;

            case EPROTO:
                m_strExceptionMessage = "A Protocol error occurred on the socket.";
                m_nPriority = CException::HIGH;
                break;

            case EWOULDBLOCK:
                m_strExceptionMessage = "This operation would block execution (or timed out).";
                m_nPriority = CException::WARNING;
                break;

            case ETIMEDOUT:
                m_strExceptionMessage = "The operation timed out.";
                m_nPriority = CException::WARNING;
                break;

            case ECONNRESET:
                m_strExceptionMessage = "Connection Reset by Peer.";
                m_nPriority = CException::FATAL;
                break;

            case ECONNREFUSED:
                m_strExceptionMessage = "The connection was refused.";
                m_nPriority = CException::HIGH;
                break;

            case ECONNABORTED:
                m_strExceptionMessage = "The connection was aborted.";
                m_nPriority = CException::HIGH;
                break;

            case ENOTCONN:
                m_strExceptionMessage = "The socket is not connected.";
                m_nPriority = CException::HIGH;
                break;

            case EADDRINUSE:
                m_strExceptionMessage = "The address is already in use.";
                m_nPriority = CException::HIGH;
                break;

            case EADDRNOTAVAIL:
                m_strExceptionMessage = "The address is not available on this machine.";
                m_nPriority = CException::HIGH;
                break;

            case EHOSTUNREACH:
                m_strExceptionMessage = "The host cannot be reached.";
                m_nPriority = CException::HIGH;
                break;

            case ENETUNREACH:
                m_strExceptionMessage = "The network cannot be reached.";
                m_nPriority = CException::HIGH;
                break;

            case EMSGSIZE:
                m_strExceptionMessage = "The message is too long.";
                m_nPriority = CException::HIGH;
                break;

            case ENOENT:
                m_strExceptionMessage = "No such entry. The socket is already closed.";
                m_nPriority = CException::FATAL;
                break;

    #if defined( ENOSR )
            case ENOSR:
                m_strExceptionMessage = "Could not create socket. No STREAM resources available.";
                m_nPriority = CException::FATAL;
                break;
    #endif

            default:
                m_strExceptionMessage.Format( "Unknown error occurred on socket. Error Number = %d", m_nErrorNumber );
                m_nPriority = CException::NORMAL;
                break;
        }
        m_pCallStack->SetMessage( m_strExceptionMessage );
    }

    CSocketException::~CSocketException( void )
    {
        // Nothing to do for now.
    }
} // namespace IASLib

