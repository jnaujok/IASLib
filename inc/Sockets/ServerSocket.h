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
        // A listening TCP socket. The constructors throw a CSocketException
        // if the port cannot be bound or listened on.
    class CServerSocket : public CSocket
    {
        protected:
            bool m_bSecure;
            CSocketConfig config;

        public:
                            // Binds nPort (0: any free port) on bindAddress
                            // (NULL: every IPv4 address), with the
                            // configuration's backlog and options (which
                            // accepted connections inherit).
                        CServerSocket( CSocketConfig config, int nPort, const char *bindAddress = NULL );
	                    CServerSocket( int nPort, int nMaxBacklog = 16, bool bSecure = false );
	        virtual    ~CServerSocket( void );

                        DEFINE_OBJECT( CServerSocket )

                // Waits for a connection. Throws a CSocketException on an
                // error; on a non-blocking socket, returns NULL if none is
                // waiting.
            virtual CSocket    *Accept( void );
                // Waits at most nTimeoutMs milliseconds for a connection.
                // Returns NULL if none arrived in time or on error; never throws.
            virtual CSocket    *Accept( int nTimeoutMs );
            virtual bool        isSecure( void ) { return m_bSecure; }

        private:
            void                Listen( int nBacklog );
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SERVERSOCKET_H__

