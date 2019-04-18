/**
 *  Client Socket Class
 *
 *      This class provides an interface for connecting
 * to a remote server through a socket connection.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/02/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "ClientSocket.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CClientSocket, CSocket );

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
    CClientSocket::CClientSocket( const char *strConnectTo, int nPort ) 
    : CSocket( strConnectTo, nPort )
    {
    }

    /**
     * Client Socket Destructor
     *
     * The destructor cleans up the socket object.
     */
    CClientSocket::~CClientSocket()
    {
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__

