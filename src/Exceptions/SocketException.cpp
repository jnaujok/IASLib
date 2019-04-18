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

        switch ( m_nErrorNumber )
        {
            case EACCES:
                m_strExceptionMessage = "Permission to create socket has been denied.";
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

            case EBADF:
                m_strExceptionMessage = "Invalid socket descriptor.";
                m_nPriority = CException::FATAL;
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

    #ifndef WIN32
            case ENOSR:
                m_strExceptionMessage = "Could not create socket. No STREAM resources available.";
                m_nPriority = CException::FATAL;
                break;
                
            case EPROTONOSUPPORT:
                m_strExceptionMessage = "Could not create socket. Protocol is not supported.";
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
                m_strExceptionMessage = "This operation would block execution.";
                m_nPriority = CException::WARNING;
                break;
                
            case ECONNRESET:
                m_strExceptionMessage = "Connection Reset by Peer.";
                m_nPriority = CException::FATAL;
                break;

            case ENOENT:
                m_strExceptionMessage = "No such entry. The socket is already closed.";
                m_nPriority = CException::FATAL;
                break;
    #endif

            default:
                m_strExceptionMessage = "Unknown error occurred on socket. Error Number = ";
                m_strExceptionMessage += m_nErrorNumber;
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

