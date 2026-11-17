/**
 * SIP Transport classes
 *
 * A SIP transport sends messages for the SIP server. The server core only
 * deals with transports, which keeps the protocol logic independent of
 * sockets (and testable without them):
 *
 *   CSipUdpTransport     sends datagrams to an address from a UDP socket.
 *   CSipStreamTransport  sends on one connected TCP (or TLS) socket; the
 *                        target address is ignored, since responses go back
 *                        on the connection the request arrived on.
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPTRANSPORT_H__
#define IASLIB_SIPTRANSPORT_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"
#include "Sockets/InternetAddress.h"
#include "Sockets/Socket.h"
#include "Sockets/UDPSocket.h"

#include <mutex>

namespace IASLib
{
    class CSipTransport : public CObject
    {
        public:
            DEFINE_OBJECT( CSipTransport );

            CSipTransport( void ) {}
            virtual ~CSipTransport( void ) {}

                // Sends a complete message. Returns false if it could not be sent.
            virtual bool send( const CString &data, const CInternetAddress &target ) = 0;

                // True for transports that deliver reliably (TCP, TLS), which
                // changes the RFC 3261 retransmission rules.
            virtual bool isReliable( void ) = 0;

                // The transport name as used in a Via header: "UDP", "TCP", ...
            virtual const char *getProtocol( void ) = 0;
    };

    class CSipUdpTransport : public CSipTransport
    {
        protected:
            CUDPSocket *m_pSocket;

        public:
            DEFINE_OBJECT( CSipUdpTransport );

                // The socket is not owned by the transport.
            CSipUdpTransport( CUDPSocket *pSocket );
            virtual ~CSipUdpTransport( void );

            virtual bool send( const CString &data, const CInternetAddress &target );
            virtual bool isReliable( void ) { return false; }
            virtual const char *getProtocol( void ) { return "UDP"; }
    };

    class CSipStreamTransport : public CSipTransport
    {
        protected:
            CSocket    *m_pSocket;
            bool        m_bSecure;
            std::mutex  m_sendMutex;

        public:
            DEFINE_OBJECT( CSipStreamTransport );

                // The socket is not owned by the transport.
            CSipStreamTransport( CSocket *pSocket, bool bSecure = false );
            virtual ~CSipStreamTransport( void );

            virtual bool send( const CString &data, const CInternetAddress &target );
            virtual bool isReliable( void ) { return true; }
            virtual const char *getProtocol( void ) { return m_bSecure ? "TLS" : "TCP"; }
    };
} // namespace IASLib

#endif // IASLIB_SIPTRANSPORT_H__

#endif // IASLIB_NETWORKING__
