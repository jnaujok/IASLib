/**
 *  TLS Context Class
 *
 *      Settings shared by secure connections. See TlsContext.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: November 17, 2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_TLS__

#include "TlsContext.h"

#ifdef IASLIB_WIN32__
    // Before OpenSSL, whose headers undo wincrypt.h's clashing macros
    // (X509_NAME and others).
#include "Platform.h"
#include <wincrypt.h>
#ifdef _MSC_VER
#pragma comment( lib, "crypt32.lib" )
#endif
#endif

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <cctype>
#include <string>

namespace IASLib
{
    IMPLEMENT_OBJECT( CTlsContext, CObject );

    namespace
    {
            // OpenSSL's queued errors, oldest first, as one line.
        std::string lowerCase( const char *pszText )
        {
            std::string strText( pszText ? pszText : "" );
            for ( size_t n = 0; n < strText.size(); n++ )
                strText[ n ] = (char)tolower( (unsigned char)strText[ n ] );
            return strText;
        }

            // A host name against a name or "*.domain" pattern (lower case).
        bool hostMatches( const std::string &strPattern, const std::string &strHost )
        {
            if ( strPattern.size() > 2 && strPattern[0] == '*' && strPattern[1] == '.' )
            {
                std::string strSuffix = strPattern.substr( 1 );       // ".domain"
                if ( strHost.size() <= strSuffix.size() ||
                     strHost.compare( strHost.size() - strSuffix.size(), strSuffix.size(), strSuffix ) != 0 )
                    return false;
                std::string strLabel = strHost.substr( 0, strHost.size() - strSuffix.size() );
                return strLabel.find( '.' ) == std::string::npos;
            }
            return strPattern == strHost;
        }

#ifdef IASLIB_WIN32__
            // Windows keeps its trusted roots in a certificate store, which
            // OpenSSL does not read; copy them into the context's store.
        int loadWindowsRoots( X509_STORE *pStore )
        {
            int nAdded = 0;
            HCERTSTORE hStore = CertOpenSystemStoreW( 0, L"ROOT" );
            if ( hStore == NULL )
                return 0;
            PCCERT_CONTEXT pCertificate = NULL;
            while ( ( pCertificate = CertEnumCertificatesInStore( hStore, pCertificate ) ) != NULL )
            {
                const unsigned char *pData = pCertificate->pbCertEncoded;
                X509 *pX509 = d2i_X509( NULL, &pData, (long)pCertificate->cbCertEncoded );
                if ( pX509 )
                {
                    if ( X509_STORE_add_cert( pStore, pX509 ) == 1 )
                        nAdded++;
                    X509_free( pX509 );
                }
            }
            CertCloseStore( hStore, 0 );
            ERR_clear_error();      // duplicates are not errors
            return nAdded;
        }
#endif

            // True if OpenSSL refused a key because it is not the
            // certificate's.
        bool keyMismatch( void )
        {
            unsigned long nError = ERR_peek_last_error();
            return ERR_GET_LIB( nError ) == ERR_LIB_X509 && ERR_GET_REASON( nError ) == X509_R_KEY_VALUES_MISMATCH;
        }

        std::string opensslErrors( void )
        {
            std::string strErrors;
            unsigned long nError;
            while ( ( nError = ERR_get_error() ) != 0 )
            {
                char achBuffer[ 256 ];
                ERR_error_string_n( nError, achBuffer, sizeof( achBuffer ) );
                if ( ! strErrors.empty() )
                    strErrors += "; ";
                strErrors += achBuffer;
            }
            return strErrors;
        }
    }

    CTlsContext::CTlsContext( Role eRole )
        : m_eRole( eRole ), m_pContext( NULL ), m_bHaveCertificate( false ), m_bHaveKey( false ),
          m_bVerifyPeer( eRole == CLIENT ), m_bRequireKnownHost( false )
    {
        ERR_clear_error();
        m_pContext = SSL_CTX_new( eRole == SERVER ? TLS_server_method() : TLS_client_method() );
        if ( m_pContext == NULL )
        {
            fail( "could not create a TLS context" );
            return;
        }

        SSL_CTX_set_min_proto_version( m_pContext, TLS1_2_VERSION );
        long lOptions = SSL_OP_NO_COMPRESSION | SSL_OP_CIPHER_SERVER_PREFERENCE;
#ifdef SSL_OP_NO_RENEGOTIATION
        lOptions |= SSL_OP_NO_RENEGOTIATION;
#endif
#ifdef SSL_OP_IGNORE_UNEXPECTED_EOF
            // Peers that close without a TLS close_notify (common for HTTP
            // clients) end the connection normally rather than as an error.
        lOptions |= SSL_OP_IGNORE_UNEXPECTED_EOF;
#endif
        SSL_CTX_set_options( m_pContext, lOptions );
            // Writes are retried with the same data after a would-block.
        SSL_CTX_set_mode( m_pContext, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER );

        if ( eRole == CLIENT )
        {
            SSL_CTX_set_default_verify_paths( m_pContext );
#ifdef IASLIB_WIN32__
            loadWindowsRoots( SSL_CTX_get_cert_store( m_pContext ) );
#endif
            ERR_clear_error();      // a missing system store is not an error yet
        }
        else
        {
            SSL_CTX_set_tlsext_servername_callback( m_pContext, serverNameCallback );
            SSL_CTX_set_tlsext_servername_arg( m_pContext, this );
        }
        setVerifyPeer( m_bVerifyPeer );
    }

    CTlsContext::~CTlsContext( void )
    {
        if ( m_pContext )
            SSL_CTX_free( m_pContext );
    }

    bool CTlsContext::fail( const char *pszWhat )
    {
        std::string strDetail = opensslErrors();
        m_strError = pszWhat;
        if ( ! strDetail.empty() )
        {
            m_strError += ": ";
            m_strError += strDetail.c_str();
        }
        return false;
    }

    bool CTlsContext::checkKey( void )
    {
        if ( m_bHaveCertificate && m_bHaveKey && SSL_CTX_check_private_key( m_pContext ) != 1 )
        {
            m_bHaveKey = false;
            return fail( "the private key does not match the certificate" );
        }
        m_strError = "";
        return true;
    }

    bool CTlsContext::setCertificateFile( const char *pszPath )
    {
        ERR_clear_error();
        if ( m_pContext == NULL || pszPath == NULL )
            return fail( "no certificate file" );
        if ( SSL_CTX_use_certificate_chain_file( m_pContext, pszPath ) != 1 )
            return fail( ( std::string( "could not read the certificate file " ) + pszPath ).c_str() );
        m_bHaveCertificate = true;
        return checkKey();
    }

    bool CTlsContext::setCertificatePem( const char *pszPem )
    {
        ERR_clear_error();
        if ( m_pContext == NULL || pszPem == NULL )
            return fail( "no certificate" );
        BIO *pBio = BIO_new_mem_buf( pszPem, -1 );
        X509 *pCertificate = PEM_read_bio_X509( pBio, NULL, NULL, NULL );
        bool bOk = ( pCertificate != NULL && SSL_CTX_use_certificate( m_pContext, pCertificate ) == 1 );
        if ( pCertificate )
            X509_free( pCertificate );
        if ( bOk )
        {
                // The rest of the text is the chain.
            SSL_CTX_clear_chain_certs( m_pContext );
            X509 *pChain;
            while ( bOk && ( pChain = PEM_read_bio_X509( pBio, NULL, NULL, NULL ) ) != NULL )
            {
                bOk = ( SSL_CTX_add0_chain_cert( m_pContext, pChain ) == 1 );
                if ( ! bOk )
                    X509_free( pChain );
            }
            ERR_clear_error();      // the end of the text
        }
        BIO_free( pBio );
        if ( ! bOk )
            return fail( "could not read the certificate" );
        m_bHaveCertificate = true;
        return checkKey();
    }

    bool CTlsContext::setPrivateKeyFile( const char *pszPath )
    {
        ERR_clear_error();
        if ( m_pContext == NULL || pszPath == NULL )
            return fail( "no private key file" );
        if ( SSL_CTX_use_PrivateKey_file( m_pContext, pszPath, SSL_FILETYPE_PEM ) != 1 )
        {
            if ( keyMismatch() )
                return fail( "the private key does not match the certificate" );
            return fail( ( std::string( "could not read the private key file " ) + pszPath ).c_str() );
        }
        m_bHaveKey = true;
        return checkKey();
    }

    bool CTlsContext::setPrivateKeyPem( const char *pszPem )
    {
        ERR_clear_error();
        if ( m_pContext == NULL || pszPem == NULL )
            return fail( "no private key" );
        BIO *pBio = BIO_new_mem_buf( pszPem, -1 );
        EVP_PKEY *pKey = PEM_read_bio_PrivateKey( pBio, NULL, NULL, NULL );
        bool bOk = ( pKey != NULL && SSL_CTX_use_PrivateKey( m_pContext, pKey ) == 1 );
        if ( pKey )
            EVP_PKEY_free( pKey );
        BIO_free( pBio );
        if ( ! bOk )
            return fail( keyMismatch() ? "the private key does not match the certificate" : "could not read the private key" );
        m_bHaveKey = true;
        return checkKey();
    }

    bool CTlsContext::setTrustedCertificatesFile( const char *pszPath )
    {
        ERR_clear_error();
        if ( m_pContext == NULL || pszPath == NULL || SSL_CTX_load_verify_locations( m_pContext, pszPath, NULL ) != 1 )
            return fail( ( std::string( "could not read the trusted certificates in " ) + ( pszPath ? pszPath : "(none)" ) ).c_str() );
        m_strError = "";
        return true;
    }

    bool CTlsContext::addTrustedCertificatePem( const char *pszPem )
    {
        ERR_clear_error();
        if ( m_pContext == NULL || pszPem == NULL )
            return fail( "no certificate" );
        BIO *pBio = BIO_new_mem_buf( pszPem, -1 );
        X509_STORE *pStore = SSL_CTX_get_cert_store( m_pContext );
        int nAdded = 0;
        X509 *pCertificate;
        while ( ( pCertificate = PEM_read_bio_X509( pBio, NULL, NULL, NULL ) ) != NULL )
        {
            if ( X509_STORE_add_cert( pStore, pCertificate ) == 1 )
                nAdded++;
            X509_free( pCertificate );
        }
        BIO_free( pBio );
        ERR_clear_error();
        if ( nAdded == 0 )
            return fail( "no certificates could be read" );
        m_strError = "";
        return true;
    }

    bool CTlsContext::useSystemTrustStore( void )
    {
        ERR_clear_error();
        if ( m_pContext == NULL )
            return fail( "could not load the system's trusted certificates" );
        bool bOk = ( SSL_CTX_set_default_verify_paths( m_pContext ) == 1 );
#ifdef IASLIB_WIN32__
        bOk = loadWindowsRoots( SSL_CTX_get_cert_store( m_pContext ) ) > 0 || getTrustedCertificateCount() > 0;
#endif
        if ( ! bOk )
            return fail( "could not load the system's trusted certificates" );
        ERR_clear_error();
        m_strError = "";
        return true;
    }

    int CTlsContext::getTrustedCertificateCount( void ) const
    {
        if ( m_pContext == NULL )
            return 0;
        X509_STORE *pStore = SSL_CTX_get_cert_store( m_pContext );
        int nCount = 0;
        X509_STORE_lock( pStore );
        STACK_OF( X509_OBJECT ) *pObjects = X509_STORE_get0_objects( pStore );
        for ( int n = 0; n < sk_X509_OBJECT_num( pObjects ); n++ )
        {
            if ( X509_OBJECT_get_type( sk_X509_OBJECT_value( pObjects, n ) ) == X509_LU_X509 )
                nCount++;
        }
        X509_STORE_unlock( pStore );
        return nCount;
    }

    bool CTlsContext::addHostContext( const char *pszHostName, const std::shared_ptr<CTlsContext> &pContext )
    {
        if ( m_eRole != SERVER || pszHostName == NULL || *pszHostName == '\0' )
        {
            m_strError = "host contexts are for servers, and need a host name";
            return false;
        }
        if ( ! pContext || pContext.get() == this || pContext->getRole() != SERVER || ! pContext->isReady() )
        {
            m_strError = "a host context must be another server context with a certificate and its key";
            return false;
        }
        std::string strName = lowerCase( pszHostName );
        std::lock_guard<std::mutex> lock( m_hostMutex );
        for ( size_t n = 0; n < m_aHosts.size(); n++ )
        {
            if ( m_aHosts[ n ].first == strName )
            {
                m_aHosts[ n ].second = pContext;
                m_strError = "";
                return true;
            }
        }
        m_aHosts.push_back( std::make_pair( strName, pContext ) );
        m_strError = "";
        return true;
    }

    std::shared_ptr<CTlsContext> CTlsContext::findHost( const char *pszHostName ) const
    {
        std::string strHost = lowerCase( pszHostName );
        std::lock_guard<std::mutex> lock( m_hostMutex );
            // Exact names first, then patterns.
        for ( size_t n = 0; n < m_aHosts.size(); n++ )
            if ( m_aHosts[ n ].first == strHost )
                return m_aHosts[ n ].second;
        for ( size_t n = 0; n < m_aHosts.size(); n++ )
            if ( hostMatches( m_aHosts[ n ].first, strHost ) )
                return m_aHosts[ n ].second;
        return std::shared_ptr<CTlsContext>();
    }

        // Picks the certificate for the host name the client asked for.
    int CTlsContext::serverNameCallback( SSL *pSsl, int *pnAlert, void *pArgument )
    {
        CTlsContext *pThis = static_cast<CTlsContext *>( pArgument );
        const char *pszName = SSL_get_servername( pSsl, TLSEXT_NAMETYPE_host_name );
        if ( pszName == NULL || *pszName == '\0' )
            return SSL_TLSEXT_ERR_NOACK;            // no name asked for: the default
        std::shared_ptr<CTlsContext> pHost = pThis->findHost( pszName );
        if ( pHost )
        {
            SSL_set_SSL_CTX( pSsl, pHost->getNativeContext() );
            return SSL_TLSEXT_ERR_OK;
        }
        if ( pThis->m_bRequireKnownHost )
        {
            *pnAlert = SSL_AD_UNRECOGNIZED_NAME;
            return SSL_TLSEXT_ERR_ALERT_FATAL;
        }
        return SSL_TLSEXT_ERR_NOACK;
    }

    void CTlsContext::setVerifyPeer( bool bVerify )
    {
        m_bVerifyPeer = bVerify;
        if ( m_pContext == NULL )
            return;
        int nMode = SSL_VERIFY_NONE;
        if ( bVerify )
            nMode = ( m_eRole == SERVER ) ? ( SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT ) : SSL_VERIFY_PEER;
        SSL_CTX_set_verify( m_pContext, nMode, NULL );
    }

    bool CTlsContext::isReady( void ) const
    {
        if ( m_pContext == NULL )
            return false;
        return ( m_eRole == CLIENT ) || ( m_bHaveCertificate && m_bHaveKey );
    }
} // namespace IASLib

#endif // IASLIB_TLS__
#endif // IASLIB_NETWORKING__
