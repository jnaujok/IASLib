/**
 * Client Socket Source class
 *
 *      This class provides a base-line interface for providing a source of
 * client sockets. While it remains possible to simply grab a single socket
 * and use it, it's often the case that such a use/discard cycle will rapidly
 * exhaust the available set of socket handles, and the overhead of the socket
 * connection can become the majority of the time spent in the connection.
 *      This class, therefore provides the baseline for potentially reusing
 * or pooling of sockets. Additionally, it makes it simple to generate a
 * large number of identically configured sockets.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 6/03/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef CLIENTSOCKETSOURCE_H__
#define CLIENTSOCKETSOURCE_H__

#ifdef IASLIB_NETWORKING__

#include "ClientSocket.h"

namespace IASLib
{
    class CClientSocketSource : public CObject
    {
        protected:
            CClientSocketSource( void );
        public:
            DECLARE_OBJECT( CClientSocketSource, CObject );

            virtual CClientSocket *getClientSocket() = 0;
            virtual CClientSocket *getClientSocket( const char *hostname ) = 0;
            virtual CClientSocket *getClientSocket( const char *hostname, int port ) = 0;

            virtual bool init( CSocketSettings *settings );
    };
} // end of namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // CLIENTSOCKETSOURCE_H__
