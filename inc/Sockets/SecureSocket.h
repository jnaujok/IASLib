/**
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
    class CSecureSocket : public CSocket
    {
        public:
                // Server side: takes over a connection from Accept() (which is
                // deleted). The context must be a ready SERVER context.
                            CSecureSocket( CSocket *pAccepted, const std::shared_ptr<CTlsContext> &pContext );
                // Client side: connects to strHost:nPort. The server's
                // certificate is checked against strHost when the context
                // verifies peers.
                            CSecureSocket( const char *strHost, int nPort, const std::shared_ptr<CTlsContext> &pContext );
	        virtual        ~CSecureSocket();

                            DEFINE_OBJECT( CSecureSocket )

                // Completes the TLS handshake, waiting at most nTimeoutMs (a
                // negative value waits as long as it takes). False, with the
                // reason in GetTlsError(), if it fails or runs out of time.
            bool                    Handshake( int nTimeoutMs );
            virtual bool            IsHandshakeComplete( void );

            virtual unsigned char   Read( void );
            virtual int             Read( char *pchBuffer, int nBufferSize );
            virtual int             ReadWithTimeout( char *pchBuffer, int nBufferSize, int nTimeoutMs );
            virtual int             Send( unsigned char chSend );
            virtual int             Send( const char *pchBuffer, int nBufferSize );
            virtual bool            HasData( void );
                // Sends the TLS close notification (if it can without waiting)
                // and closes the connection.
            virtual void            Close( void );

                // How long a Send() may wait for the peer to take data (default
                // 30 seconds); it fails after that.
            void                    SetSendTimeout( int nTimeoutMs ) { m_nSendTimeoutMs = nTimeoutMs; }
                // How long a handshake started by Read() or Send() may take
                // (default 30 seconds).
            void                    SetHandshakeTimeout( int nTimeoutMs ) { m_nHandshakeTimeoutMs = nTimeoutMs; }

                // About the connection, once the handshake is done.
            CString                 GetTlsError( void );
            CString                 GetProtocolVersion( void );     // "TLSv1.3"
            CString                 GetCipher( void );
            bool                    HasPeerCertificate( void );
            CString                 GetPeerSubject( void );         // the peer certificate's subject name

        private:
            std::shared_ptr<CTlsContext>    m_pContext;
            struct ssl_st          *m_pSsl;
            std::mutex              m_mutex;            // guards m_pSsl
            bool                    m_bServer;
            bool                    m_bHandshakeDone;
            int                     m_nSendTimeoutMs;
            int                     m_nHandshakeTimeoutMs;
            std::string             m_strHost;
            std::string             m_strTlsError;

            void                    setUp( void );
            int                     waitFor( int nSslError, int nTimeoutMs );
            void                    recordError( const char *pszWhat, int nSslError );

                                    CSecureSocket( const CSecureSocket & );
            CSecureSocket          &operator =( const CSecureSocket & );
    };
} // namespace IASLib

#endif // IASLIB_TLS__
#endif // IASLIB_NETWORKING__
#endif // IASLIB_SECURE_SOCKET_H__
