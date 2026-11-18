/**
 * @file SipTransport.h
 * @brief The transports a SIP server sends on (CSipTransport, CSipUdpTransport, CSipStreamTransport).
 *
 * SIP Transport classes
 *
 * A SIP transport sends messages for the SIP server. The server core only
 * deals with transports, which keeps the protocol logic independent of
 * sockets (and testable without them):
 *
 *   - CSipUdpTransport     sends datagrams to an address from a UDP socket.
 *   - CSipStreamTransport  sends on one connected TCP (or TLS) socket; the
 *                          target address is ignored, since responses go back
 *                          on the connection the request arrived on.
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
    /// Sends complete SIP messages for the SIP server.
    ///
    /// The server core only deals with transports, which keeps the protocol
    /// logic independent of sockets (and testable without them: a test can
    /// supply a transport that records what is sent).
    class CSipTransport : public CObject
    {
        public:
            DEFINE_OBJECT( CSipTransport );

                /// Creates a transport.
            CSipTransport( void ) {}
                /// Destroys the transport.
            virtual ~CSipTransport( void ) {}

                /// Sends a complete message.
                /// @param data   The whole message.
                /// @param target Where to send it (ignored by stream transports).
                /// @return false if it could not be sent.
            virtual bool send( const CString &data, const CInternetAddress &target ) = 0;

                /// True for transports that deliver reliably (TCP, TLS), which
                /// changes the RFC 3261 retransmission rules.
            virtual bool isReliable( void ) = 0;

                /// The transport name as used in a Via header: "UDP", "TCP", ...
            virtual const char *getProtocol( void ) = 0;
    };

    /// Sends datagrams to an address from a UDP socket.
    class CSipUdpTransport : public CSipTransport
    {
        protected:
            CUDPSocket *m_pSocket;  ///< The socket to send from (not owned).

        public:
            DEFINE_OBJECT( CSipUdpTransport );

                /// Creates a transport on the socket. The socket is not owned by
                /// the transport and must outlive it.
            CSipUdpTransport( CUDPSocket *pSocket );
                /// Destroys the transport (not the socket).
            virtual ~CSipUdpTransport( void );

                /// Sends the message as one datagram to the target.
                /// @return false if there is no socket or not all of it was sent.
            virtual bool send( const CString &data, const CInternetAddress &target );
                /// false: UDP is unreliable.
            virtual bool isReliable( void ) { return false; }
                /// "UDP".
            virtual const char *getProtocol( void ) { return "UDP"; }
    };

    /// Sends on one connected TCP (or TLS) socket.
    ///
    /// The target address is ignored, since responses go back on the
    /// connection the request arrived on. Sends are serialised by a mutex, so
    /// several threads may answer on the same connection.
    class CSipStreamTransport : public CSipTransport
    {
        protected:
            CSocket    *m_pSocket;      ///< The connection (not owned).
            bool        m_bSecure;      ///< Whether the connection is TLS.
            std::mutex  m_sendMutex;    ///< Serialises send().

        public:
            DEFINE_OBJECT( CSipStreamTransport );

                /// Creates a transport on the connection. The socket is not
                /// owned by the transport and must outlive it.
                /// @param pSocket The connected socket.
                /// @param bSecure Whether it is a TLS connection.
            CSipStreamTransport( CSocket *pSocket, bool bSecure = false );
                /// Destroys the transport (not the socket).
            virtual ~CSipStreamTransport( void );

                /// Sends the message on the connection; the target is ignored.
                /// @return false if the socket is missing, not connected, or the
                ///         send failed.
            virtual bool send( const CString &data, const CInternetAddress &target );
                /// true: TCP and TLS are reliable.
            virtual bool isReliable( void ) { return true; }
                /// "TLS" for a secure connection, else "TCP".
            virtual const char *getProtocol( void ) { return m_bSecure ? "TLS" : "TCP"; }
    };
} // namespace IASLib

#endif // IASLIB_SIPTRANSPORT_H__

#endif // IASLIB_NETWORKING__
