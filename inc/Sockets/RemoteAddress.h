/**
 *  Remote Address Class
 *
 *      This class encapsulates a remote IP address, either IPv4 or IPv6,
 * including port if provided. This allows for a C++ style of storage of a
 * remote internet address in a nice, simple, easy-to-use class instead of
 * the 1970's vintage sockaddr structures.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 7/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_REMOTEADDRESS_H__
#define IASLIB_REMOTEADDRESS_H__

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_WIN32__
    #include <winsock2.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#ifndef IASLIB_DEC__
    #include <inttypes.h>
#endif
    #include <unistd.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif

#include "BaseTypes/Object.h"

namespace IASLib
{
    class CRemoteAddress : public CObject
    {
        protected:
        public:
                            CRemoteAddress( const char *hostname, int port = -1 );
                            CRemoteAddress( sockaddr *address );
                            CRemoteAddress( sockaddr_in *address );
	        virtual        ~CRemoteAddress();

                            DEFINE_OBJECT( CRemoteAddress )


    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_REMOTEADDRESS_H__
