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

#ifndef IASLIB_CLIENTSOCKET_H__
#define IASLIB_CLIENTSOCKET_H__

#ifdef IASLIB_NETWORKING__

#include "Socket.h"

namespace IASLib
{
    class CClientSocket : public CSocket
    {
        public:
	                    CClientSocket( const char *strConnectTo, int nPort );
	        virtual    ~CClientSocket();

                        DEFINE_OBJECT( CClientSocket )
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_CLIENTSOCKET_H__
