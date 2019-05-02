/**
 *  Secure Client Socket Class
 *
 *      This class provides an interface for connecting
 * to a remote server through a secure socket connection.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/23/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "SecureClientSocket.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSecureClientSocket, CSecureSocket );

    /**
     * Client Socket Constructor
     *
     * This constructor builds a Client socket.
     *
     * @param strConnectTo
     *      The host (name or IP Address) to connect to.
     * @param nPort
     *      The Port to connect to.
     */
    CSecureClientSocket::CSecureClientSocket( const char *strConnectTo, int nPort )
    : CSecureSocket( strConnectTo, nPort )
    {
    }

    /**
     * Client Socket Destructor
     *
     * The destructor cleans up the socket object.
     */
    CSecureClientSocket::~CSecureClientSocket()
    {
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__

