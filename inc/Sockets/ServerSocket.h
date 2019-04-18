/**
 *  Server Socket Class
 *
 *      This class provides an interface for creating a server
 * socket connection.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/02/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SERVERSOCKET_H__
#define IASLIB_SERVERSOCKET_H__

#ifdef IASLIB_NETWORKING__

#include "Socket.h"

namespace IASLib
{
    class CServerSocket : public CSocket  
    {
        public:
	                    CServerSocket( int nPort, int nMaxBacklog = 16 );
	        virtual    ~CServerSocket( void );

                        DEFINE_OBJECT( CServerSocket )

            CSocket    *Accept( void );
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SERVERSOCKET_H__

