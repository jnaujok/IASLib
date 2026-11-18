/**
 * @file SipClient.h
 * @brief A placeholder for a SIP client (CSipClient); not implemented.
 *
 *  SIP Client Class
 *
 *      This class provides an interface for connecting as a 
 * client to a SIP server. 
 *      This class works by taking a SipRequest, and executing
 * it against a SIP server, and returning a SipResponse object.
 *      It should be noted that this is a simple client, and does
 * not build all calls needed to actually properly perform a 
 * conversation between the client and a SIP server, it simply
 * performs a single request/response interaction.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: October 3, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPCLIENT_H__
#define IASLIB_SIPCLIENT_H__

#include "Collections/Array.h"
#include "Sockets/Socket.h"
#include "Sockets/UDPSocket.h"
#include "NetworkServices/GenericClient.h"

namespace IASLib
{
    /// A SIP client that would send one request to a SIP server and return
    /// the response.
    ///
    /// @warning This class is not implemented: its constructor and destructor
    ///          are declared but not defined (SipClient.cpp is empty), and it
    ///          does not override executeRequest(), so it is abstract. It
    ///          cannot be used yet.
    class CSipClient : public CGenericClient
    {
        public:
                            DEFINE_OBJECT( CSipClient );
                            /// Would store the server's address (not defined).
                            /// @param strRemoteHost The server's host name or address.
                            /// @param nPort         The port (5060 by default).
                            /// @param bUseSSL       Whether to use TLS.
                            /// @param bUseUDP       Whether to use UDP instead of TCP.
                            CSipClient( const char *strRemoteHost, int nPort=5060, bool bUseSSL = false, bool bUseUDP = false );
                            /// Destructor (not defined).
            virtual        ~CSipClient( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPCLIENT_H__

#endif // IASLIB_NETWORKING__
