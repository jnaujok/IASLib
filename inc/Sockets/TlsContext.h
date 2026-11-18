/**
 * @file TlsContext.h
 * @brief CTlsContext, the TLS settings (certificates, keys, trust) shared by secure connections.
 *
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
 *      A server can present different certificates for different host names
 * (Server Name Indication): addHostContext() names another server context
 * for clients that ask for that host.
 *
 *      Clients start with the system's trusted certificates: OpenSSL's
 * default locations, and on Windows the system's Trusted Root store (which
 * OpenSSL does not read by itself).
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

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

struct ssl_ctx_st;
struct ssl_st;

namespace IASLib
{
    /// The settings shared by secure connections (an OpenSSL SSL_CTX).
    ///
    /// For a server: its certificate (with any intermediate certificates)
    /// and private key. For a client: the certificates it trusts. Set a
    /// context up once, then share it (by std::shared_ptr), read-only, with
    /// every connection made with it, from any thread. addHostContext() may
    /// also be called while serving.
    ///
    /// Connections use TLS 1.2 or later. A client verifies the server's
    /// certificate and host name by default; a server does not ask for
    /// client certificates unless setVerifyPeer( true ) is called. Clients
    /// start with the system's trusted certificates.
    ///
    /// The setters return false on failure, with the reason in getError();
    /// nothing here throws.
    ///
    /// @code
    /// std::shared_ptr<CTlsContext> tls = std::make_shared<CTlsContext>( CTlsContext::SERVER );
    /// if ( ! tls->setCertificateFile( "server.crt" ) || ! tls->setPrivateKeyFile( "server.key" ) )
    ///     printf( "%s\n", (const char *)tls->getError() );
    /// @endcode
    class CTlsContext : public CObject
    {
        public:
            /// Which end of a connection the context is for.
            enum Role {
                SERVER,     ///< Accepts connections and presents a certificate.
                CLIENT      ///< Connects, and verifies the server by default.
            };

                /// Creates a context for one role.
                ///
                /// If OpenSSL cannot create it, getError() says so and
                /// isReady() is false.
                /// @param eRole  SERVER or CLIENT.
                                CTlsContext( Role eRole );
                /// Frees the OpenSSL context.
            virtual            ~CTlsContext( void );

                                DEFINE_OBJECT( CTlsContext )

                /// The role given to the constructor.
            Role                getRole( void ) const { return m_eRole; }

                /// The certificate to present, from a PEM file. Any further
                /// certificates in it are sent as the chain.
                /// @param pszPath  The file's path.
                /// @return false if it cannot be read, or does not match a key
                ///         already set (see getError()).
            bool                setCertificateFile( const char *pszPath );
                /// The same, from PEM text.
                /// @param pszPem  The PEM text.
            bool                setCertificatePem( const char *pszPem );
                /// The matching private key, from a PEM file.
                /// @param pszPath  The file's path.
                /// @return false if it cannot be read, or does not match the
                ///         certificate (see getError()).
            bool                setPrivateKeyFile( const char *pszPath );
                /// The same, from PEM text.
                /// @param pszPem  The PEM text.
            bool                setPrivateKeyPem( const char *pszPem );

                /// Certificates to trust when verifying the other end, from a
                /// PEM file.
                /// @param pszPath  The file's path.
                /// @return false if they cannot be read.
            bool                setTrustedCertificatesFile( const char *pszPath );
                /// Trusts the certificates in PEM text.
                /// @param pszPem  The PEM text (one or more certificates).
                /// @return false if none could be read.
            bool                addTrustedCertificatePem( const char *pszPem );
                /// Trusts the system's default store (clients start with it).
                /// @return false if it cannot be loaded.
            bool                useSystemTrustStore( void );

                /// The number of certificates trusted so far (those loaded from
                /// directories are only read when needed, so are not counted).
            int                 getTrustedCertificateCount( void ) const;

                /// Servers: use pContext (another ready SERVER context, with its
                /// own certificate) for clients asking for this host name.
                ///
                /// A name "*.example.com" matches any one label in its place;
                /// names ignore case, and exact names win over patterns. Clients
                /// asking for other names, or none, get this context's
                /// certificate. Adding a name again replaces its context. May
                /// be called while serving.
                /// @param pszHostName  The host name or pattern.
                /// @param pContext  The context for it.
                /// @return false if this is not a server context, the name is
                ///         empty, or pContext is not another ready server context.
            bool                addHostContext( const char *pszHostName, const std::shared_ptr<CTlsContext> &pContext );
                /// Servers: refuse handshakes that ask for a host name with no
                /// context here (default false). Handshakes naming no host still
                /// get this context's certificate.
                /// @param bRequire  true to refuse unknown names.
            void                setRequireKnownHostName( bool bRequire ) { m_bRequireKnownHost = bRequire; }

                /// Clients: verify the server (default true). Servers: require a
                /// client certificate (default false).
                /// @param bVerify  Whether to verify the other end.
            void                setVerifyPeer( bool bVerify );
                /// Whether the other end is verified (see setVerifyPeer()).
            bool                getVerifyPeer( void ) const { return m_bVerifyPeer; }

                /// True when connections can be made: a server needs a
                /// certificate and its key.
            bool                isReady( void ) const;

                /// What went wrong with the last call that failed ("" after one
                /// that worked).
            const CString      &getError( void ) const { return m_strError; }

                /// The OpenSSL context (an SSL_CTX), for settings not covered
                /// here. It still belongs to this object.
            struct ssl_ctx_st  *getNativeContext( void ) const { return m_pContext; }

        private:
            Role                m_eRole;                ///< SERVER or CLIENT.
            struct ssl_ctx_st  *m_pContext;             ///< The OpenSSL context (owned).
            bool                m_bHaveCertificate;     ///< A certificate has been set.
            bool                m_bHaveKey;             ///< A matching key has been set.
            bool                m_bVerifyPeer;          ///< See setVerifyPeer().
            bool                m_bRequireKnownHost;    ///< See setRequireKnownHostName().
            CString             m_strError;             ///< What getError() returns.

            mutable std::mutex  m_hostMutex;            ///< Guards m_aHosts.
            std::vector< std::pair< std::string, std::shared_ptr<CTlsContext> > > m_aHosts;   ///< Host contexts by lower-case name.

                /// Records a failure (with OpenSSL's reasons) in m_strError.
                /// @return false.
            bool                fail( const char *pszWhat );
                /// Checks that the key matches the certificate, once both are set.
            bool                checkKey( void );
                /// The host context for a name, or an empty pointer.
            std::shared_ptr<CTlsContext> findHost( const char *pszHostName ) const;
                /// OpenSSL's server-name callback: picks the host context.
            static int          serverNameCallback( struct ssl_st *pSsl, int *pnAlert, void *pArgument );

                                CTlsContext( const CTlsContext & );        ///< Not copyable.
            CTlsContext        &operator =( const CTlsContext & );        ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_TLS__
#endif // IASLIB_NETWORKING__
#endif // IASLIB_TLSCONTEXT_H__
