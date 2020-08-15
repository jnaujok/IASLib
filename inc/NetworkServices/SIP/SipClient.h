/**
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
    class CSipClient : public CGenericClient
    {
        public:
                            DEFINE_OBJECT( CSipClient );
                            CSipClient( const char *strRemoteHost, int nPort=5060, bool bUseSSL = false, bool bUseUDP = false );
            virtual        ~CSipClient( void );
    };
} // namespace IASLib

#endif // IASLIB_SIPCLIENT_H__

#endif // IASLIB_NETWORKING__
