/**
 * Fixed Connection Pool Class
 *
 *      This class is a client socket source which provides a fixed number of
 * connections that can be allocated and returned to the pool at will. The
 * number of connections cannot exceed the fixed size of the pool.
 *      Note that connections will still be opened as needed, and maintained
 * for as long as possible.
 *      Connections that exceed the longest idle time will be closed. If a
 * connection request is made for a connection to a host or port that is not
 * already available, then a new connection will be made. If no additional
 * connections are available, then an idle connection will be closed and
 * re-purposed to connect to the new host/port.
 *      If no idle connections are available, the call to get a connection
 * will either block and wait for a connection to become available, or it will
 * return immediately with a NULL connection, if the flag was set to non-
 * blocking.
 *
 * 	Author: Jeffrey R. Naujok
 *	Created: 6/04/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef FIXEDCONNECTIONPOOL_H__
#define FIXEDCONNECTIONPOOL_H__

#ifdef IASLIB_NETWORKING__

#include "ClientSocketSource.h"
#include "../Collections/Array.h"

namespace IASLib
{
    class CFixedConnectionPool : public CClientSocketSource
    {
        private:
            CArray  m_aIdleConnections;
            int     m_nSize;
            CArray  m_aInUse;
            bool    m_bShutdown;

        public:
            CFixedConnectionPool( int nMaxConnections );

            DEFINE_OBJECT( CFixedConnectionPool );

            virtual CClientSocket *getClientSocket();
            virtual CClientSocket *getClientSocket( const char *hostname );
            virtual CClientSocket *getClientSocket( const char *hostname, int port );
    };
}

#endif // IASLIB_NETWORKING__

#endif // FIXEDCONNECTIONPOOL_H__

