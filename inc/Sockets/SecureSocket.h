/**
 * @file SecureSocket.h
 * @brief CSecureSocket, a TCP connection protected by TLS.
 *
 *  Secure Socket Class
 *
 *      A TCP connection protected by TLS, used exactly like a CSocket: Read,
 * Send, ReadWithTimeout, HasData, Close and Shutdown behave as they do on a
 * plain socket, with the encryption done underneath. So code written for
 * CSocket (such as the HTTP and SIP servers) works unchanged over TLS.
 *
 *      A server wraps a connection it has accepted; a client connects to a
 * host. Either way the TLS handshake runs on the first read or write, or
 * earlier by calling Handshake() with a time limit (servers should, on the
 * connection's own thread, so a slow client cannot hold anything else up).
 *
 *          CSocket *pAccepted = listener.Accept( 200 );
 *          CSecureSocket *pSecure = new CSecureSocket( pAccepted, tls );   // takes pAccepted over
 *          if ( ! pSecure->Handshake( 10000 ) ) ... pSecure->GetTlsError() ...
 *
 *      One thread may read while another writes; the TLS state is guarded
 * internally. Writes never raise SIGPIPE.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/22/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SECURE_SOCKET_H__
#define IASLIB_SECURE_SOCKET_H__

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_TLS__

#include "Socket.h"
#include "TlsContext.h"

#include <memory>
#include <mutex>
#include <string>

struct ssl_st;

namespace IASLib
{
    /// A TCP connection protected by TLS, used exactly like a CSocket.
    ///
    /// Read, Send, ReadWithTimeout, HasData, Close and Shutdown behave as
    /// they do on a plain socket, with the encryption done underneath, so
    /// code written for CSocket works unchanged over TLS. Unlike CSocket,
    /// Read() and Send() do not throw: they return -1 (or 0 for the
    /// one-byte Read()), with the reason in GetTlsError().
    ///
    /// A server wraps a connection it has accepted; a client connects to a
    /// host. Either way the TLS handshake runs on the first read or write,
    /// or earlier by calling Handshake() with a time limit (servers should,
    /// on the connection's own thread, so a slow client cannot hold anything
    /// else up). The handle is made non-blocking; waits are done with poll.
    ///
    /// One thread may read while another writes; the TLS state is guarded
    /// internally. Writes never raise SIGPIPE.
    ///
    /// @code
    /// CSocket *pAccepted = listener.Accept( 200 );
    /// CSecureSocket *pSecure = new CSecureSocket( pAccepted, tls );   // takes pAccepted over
    /// if ( ! pSecure->Handshake( 10000 ) )
    ///     printf( "%s\n", (const char *)pSecure->GetTlsError() );
    /// @endcode
    class CSecureSocket : public CSocket
    {
        public:
                /// Server side: takes over a connection from Accept() (which is
                /// deleted). The context must be a ready SERVER context.
                ///
                /// If it is not, the socket keeps the connection but cannot
                /// handshake; GetTlsError() says why.
                /// @param pAccepted  The accepted connection; this object deletes it.
                /// @param pContext  A ready SERVER context, shared.
                            CSecureSocket( CSocket *pAccepted, const std::shared_ptr<CTlsContext> &pContext );
                /// Client side: connects to strHost:nPort. The server's
                /// certificate is checked against strHost when the context
                /// verifies peers.
                ///
                /// Does not throw: check IsConnected().
                /// @param strHost  A host name, or an address.
                /// @param nPort  The port.
                /// @param pContext  A CLIENT context, shared.
                            CSecureSocket( const char *strHost, int nPort, const std::shared_ptr<CTlsContext> &pContext );
                /// Closes the connection (see Close()).
	        virtual        ~CSecureSocket();

                            DEFINE_OBJECT( CSecureSocket )

                /// Completes the TLS handshake, waiting at most nTimeoutMs.
                /// @param nTimeoutMs  The limit; a negative value waits as long
                ///                    as it takes.
                /// @return true once the handshake is done (at once if it already
                ///         was); false, with the reason in GetTlsError(), if it
                ///         fails or runs out of time.
            bool                    Handshake( int nTimeoutMs );
                /// Whether the TLS handshake is done.
            virtual bool            IsHandshakeComplete( void );

                /// Reads one byte.
                /// @return The byte, or 0 if nothing could be read (closed or failed).
            virtual unsigned char   Read( void );
                /// Reads up to nBufferSize bytes, waiting as long as it takes.
                /// @return How many were read, 0 if the peer closed the
                ///         connection, or -1 on an error.
            virtual int             Read( char *pchBuffer, int nBufferSize );
                /// Reads up to nBufferSize bytes, waiting at most nTimeoutMs.
                ///
                /// Completes the handshake first if needed (within nTimeoutMs, or
                /// the handshake timeout if nTimeoutMs is negative).
                /// @return As CSocket::ReadWithTimeout(): bytes read, 0 if the peer
                ///         closed, READ_TIMEOUT or READ_ERROR.
            virtual int             ReadWithTimeout( char *pchBuffer, int nBufferSize, int nTimeoutMs );
                /// Sends one byte.
                /// @return 1, or -1 on failure.
            virtual int             Send( unsigned char chSend );
                /// Sends all of the bytes, completing the handshake first if needed.
                /// @return nBufferSize, or -1 on failure or if the send timeout
                ///         runs out (see GetTlsError()).
            virtual int             Send( const char *pchBuffer, int nBufferSize );
                /// Whether decrypted data is waiting, or the socket can be read now.
            virtual bool            HasData( void );
                /// Sends the TLS close notification (if it can without waiting)
                /// and closes the connection.
            virtual void            Close( void );

                /// How long a Send() may wait for the peer to take data (default
                /// 30 seconds); it fails after that.
                /// @param nTimeoutMs  The limit; negative for none.
            void                    SetSendTimeout( int nTimeoutMs ) { m_nSendTimeoutMs = nTimeoutMs; }
                /// How long a handshake started by Read() or Send() may take
                /// (default 30 seconds).
                /// @param nTimeoutMs  The limit; negative for none.
            void                    SetHandshakeTimeout( int nTimeoutMs ) { m_nHandshakeTimeoutMs = nTimeoutMs; }

                /// Clients: the host name to ask for (SNI) and to check the
                /// certificate against, when it differs from the host connected
                /// to (for example, connecting to an address). Before the
                /// handshake; ignored afterwards and on servers.
                /// @param pszName  The host name.
            void                    SetServerName( const char *pszName );
                /// The host name the client asked for (on either side); "" if
                /// none.
            CString                 GetServerName( void );

                /// Whether the connection is protected by TLS: true.
            virtual bool            IsSecure( void ) { return true; }
                /// The same as GetServerName().
            virtual CString         GetTlsServerName( void ) { return GetServerName(); }

                /// What went wrong with the last TLS operation that failed ("" if nothing).
            CString                 GetTlsError( void );
                /// The protocol in use, such as "TLSv1.3" ("" before the handshake).
            CString                 GetProtocolVersion( void );
                /// The cipher in use ("" before the handshake).
            CString                 GetCipher( void );
                /// Whether the peer presented a certificate (false before the handshake).
            bool                    HasPeerCertificate( void );
                /// The peer certificate's subject name ("" if none, or before the handshake).
            CString                 GetPeerSubject( void );

        private:
            std::shared_ptr<CTlsContext>    m_pContext;     ///< The shared settings.
            struct ssl_st          *m_pSsl;                 ///< The OpenSSL connection, or NULL.
            std::mutex              m_mutex;                ///< Guards m_pSsl.
            bool                    m_bServer;              ///< The server side of the connection.
            bool                    m_bHandshakeDone;       ///< The handshake has completed.
            int                     m_nSendTimeoutMs;       ///< See SetSendTimeout().
            int                     m_nHandshakeTimeoutMs;  ///< See SetHandshakeTimeout().
            std::string             m_strHost;              ///< Clients: the name asked for and checked.
            std::string             m_strTlsError;          ///< What GetTlsError() returns.

                /// Creates the OpenSSL connection over the handle.
            void                    setUp( void );
                /// Sets the name to ask for (SNI) and to check the certificate against.
            void                    applyServerName( void );
                /// Waits for what the TLS layer asked for: 1 ready, 0 timed out, -1 error.
            int                     waitFor( int nSslError, int nTimeoutMs );
                /// Records the reason for a failure in m_strTlsError.
            void                    recordError( const char *pszWhat, int nSslError );

                                    CSecureSocket( const CSecureSocket & );    ///< Not copyable.
            CSecureSocket          &operator =( const CSecureSocket & );      ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_TLS__
#endif // IASLIB_NETWORKING__
#endif // IASLIB_SECURE_SOCKET_H__
