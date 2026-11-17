/**
 *  TLS Context Class
 *
 *      The settings shared by secure connections: for a server, its
 * certificate (with any intermediate certificates) and private key; for a
 * client, the certificates it trusts. One context is set up once and then
 * shared, read-only, by every connection made with it, from any thread.
 *
 *      Connections use TLS 1.2 or later. A client verifies the server's
 * certificate and host name by default; a server does not ask for client
 * certificates unless setVerifyPeer( true ) is called.
 *
 *          std::shared_ptr<CTlsContext> tls = std::make_shared<CTlsContext>( CTlsContext::SERVER );
 *          if ( ! tls->setCertificateFile( "server.crt" ) || ! tls->setPrivateKeyFile( "server.key" ) )
 *              printf( "%s\n", (const char *)tls->getError() );
 *          httpServer.setTlsContext( tls );
 *
 *      Needs OpenSSL (IASLIB_TLS__ is defined when the library was built
 * with it). The OpenSSL headers are not needed to use this class.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: November 17, 2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TLSCONTEXT_H__
#define IASLIB_TLSCONTEXT_H__

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_TLS__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

struct ssl_ctx_st;

namespace IASLib
{
    class CTlsContext : public CObject
    {
        public:
            enum Role { SERVER, CLIENT };

                                CTlsContext( Role eRole );
            virtual            ~CTlsContext( void );

                                DEFINE_OBJECT( CTlsContext )

            Role                getRole( void ) const { return m_eRole; }

                // The certificate to present, from a PEM file or PEM text. Any
                // further certificates in it are sent as the chain.
            bool                setCertificateFile( const char *pszPath );
            bool                setCertificatePem( const char *pszPem );
                // The matching private key, from a PEM file or PEM text.
            bool                setPrivateKeyFile( const char *pszPath );
            bool                setPrivateKeyPem( const char *pszPem );

                // Certificates to trust when verifying the other end: from a
                // PEM file, PEM text, or the system's default store (clients
                // start with the system store).
            bool                setTrustedCertificatesFile( const char *pszPath );
            bool                addTrustedCertificatePem( const char *pszPem );
            bool                useSystemTrustStore( void );

                // Clients: verify the server (default true). Servers: require a
                // client certificate (default false).
            void                setVerifyPeer( bool bVerify );
            bool                getVerifyPeer( void ) const { return m_bVerifyPeer; }

                // True when connections can be made: a server needs a
                // certificate and its key.
            bool                isReady( void ) const;

                // What went wrong with the last call that failed.
            const CString      &getError( void ) const { return m_strError; }

                // The OpenSSL context, for settings not covered here.
            struct ssl_ctx_st  *getNativeContext( void ) const { return m_pContext; }

        private:
            Role                m_eRole;
            struct ssl_ctx_st  *m_pContext;
            bool                m_bHaveCertificate;
            bool                m_bHaveKey;
            bool                m_bVerifyPeer;
            CString             m_strError;

            bool                fail( const char *pszWhat );
            bool                checkKey( void );

                                CTlsContext( const CTlsContext & );
            CTlsContext        &operator =( const CTlsContext & );
    };
} // namespace IASLib

#endif // IASLIB_TLS__
#endif // IASLIB_NETWORKING__
#endif // IASLIB_TLSCONTEXT_H__
