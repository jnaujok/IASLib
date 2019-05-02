/**
 *  Client Socket Class
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

#ifndef IASLIB_SECURECLIENTSOCKET_H__
#define IASLIB_SECURECLIENTSOCKET_H__

#ifdef IASLIB_NETWORKING__

#include "SecureSocket.h"

namespace IASLib
{
    class CSecureClientSocket : public CSecureSocket
    {
        public:
	                    CSecureClientSocket( const char *strConnectTo, int nPort );
	        virtual    ~CSecureClientSocket();

                        DEFINE_OBJECT( CSecureClientSocket )
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SECURECLIENTSOCKET_H__
