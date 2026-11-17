/**
 *  Secure Socket Class
 *
 *      A TCP connection protected by TLS. See SecureSocket.h.
 *
 *      The socket is non-blocking underneath: every TLS call returns at once,
 * and the waiting (for the peer, within a time limit) is done with
 * poll()/WSAPoll() outside the lock, so a thread waiting to read does not
 * stop another from writing. The TLS layer talks to the socket through a
 * small BIO of its own, which sends with MSG_NOSIGNAL (and SO_NOSIGPIPE on
 * macOS) so a closed peer never raises SIGPIPE.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/22/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_TLS__

#include "SecureSocket.h"
#include "SocketWait.h"

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include <chrono>
#include <cstring>
#include <stdint.h>

#ifndef IASLIB_WIN32__
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CSecureSocket, CSocket );

    namespace
    {
#if defined( MSG_NOSIGNAL )
        const int SEND_FLAGS = MSG_NOSIGNAL;
#else
        const int SEND_FLAGS = 0;
#endif

        SOCKET bioSocket( BIO *pBio )
        {
            return (SOCKET)(intptr_t)BIO_get_data( pBio );
        }

        int bioWrite( BIO *pBio, const char *pData, int nLength )
        {
            BIO_clear_retry_flags( pBio );
            int nSent = (int)send( bioSocket( pBio ), pData, nLength, SEND_FLAGS );
            if ( nSent < 0 && SocketErrorIsTimeout() )
                BIO_set_retry_write( pBio );
            return nSent;
        }

        int bioRead( BIO *pBio, char *pData, int nLength )
        {
            BIO_clear_retry_flags( pBio );
            int nRead = (int)recv( bioSocket( pBio ), pData, nLength, 0 );
            if ( nRead < 0 && SocketErrorIsTimeout() )
                BIO_set_retry_read( pBio );
            return nRead;
        }

        int bioPuts( BIO *pBio, const char *pszText )
        {
            return bioWrite( pBio, pszText, (int)strlen( pszText ) );
        }

        long bioCtrl( BIO *pBio, int nCommand, long lValue, void *pValue )
        {
            (void)pBio; (void)lValue; (void)pValue;
            return ( nCommand == BIO_CTRL_FLUSH ) ? 1 : 0;
        }

        int bioCreate( BIO *pBio )
        {
            BIO_set_init( pBio, 1 );
            return 1;
        }

        BIO_METHOD *createBioMethod( void )
        {
            BIO_METHOD *pMethod = BIO_meth_new( BIO_get_new_index() | BIO_TYPE_SOURCE_SINK, "IASLib socket" );
            BIO_meth_set_write( pMethod, bioWrite );
            BIO_meth_set_read( pMethod, bioRead );
            BIO_meth_set_puts( pMethod, bioPuts );
            BIO_meth_set_ctrl( pMethod, bioCtrl );
            BIO_meth_set_create( pMethod, bioCreate );
            return pMethod;
        }

            // Created once (thread-safe static initialisation), never freed.
        BIO_METHOD *bioMethod( void )
        {
            static BIO_METHOD *pMethod = createBioMethod();
            return pMethod;
        }

        void setNonBlocking( SOCKET hSocket )
        {
#ifdef IASLIB_WIN32__
            u_long ulMode = 1;
            ioctlsocket( hSocket, FIONBIO, &ulMode );
#else
            int nFlags = fcntl( hSocket, F_GETFL, 0 );
            fcntl( hSocket, F_SETFL, nFlags | O_NONBLOCK );
#endif
#ifdef SO_NOSIGPIPE
            int nOn = 1;
            setsockopt( hSocket, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&nOn, sizeof( nOn ) );
#endif
        }

            // The accepted connection's handle and peer address, taken from it
            // before the base class is built. Thread-local, so concurrent
            // accepts do not mix them up.
        struct CAdopted
        {
            struct sockaddr_in address;
            bool bHaveAddress;
        };

        CAdopted &adopted( void )
        {
            static thread_local CAdopted value;
            return value;
        }

        SOCKET adoptHandle( CSocket *pAccepted )
        {
            CAdopted &state = adopted();
            memset( &state.address, 0, sizeof( state.address ) );
            state.bHaveAddress = false;
            if ( pAccepted == NULL )
                return NULL_SOCKET;
            SOCKET hSocket = pAccepted->ReleaseHandle();
            delete pAccepted;
            if ( hSocket != NULL_SOCKET )
            {
                socklen_t nLength = sizeof( state.address );
                state.bHaveAddress = ( getpeername( hSocket, (struct sockaddr *)&state.address, &nLength ) == 0 &&
                                       state.address.sin_family == AF_INET );
            }
            return hSocket;
        }

        void *adoptedAddress( void )
        {
                // Evaluated before adoptHandle() fills it in, but only the
                // pointer is taken here; the base constructor reads it after
                // every argument has been evaluated.
            return &adopted().address;
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

            // The peer's certificate, with a reference the caller frees.
        X509 *peerCertificate( SSL *pSsl )
        {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
            return SSL_get1_peer_certificate( pSsl );
#else
            return SSL_get_peer_certificate( pSsl );
#endif
        }

        bool isIpAddress( const std::string &strHost )
        {
            unsigned char achAddress[ 16 ];
            return inet_pton( AF_INET, strHost.c_str(), achAddress ) == 1 ||
                   inet_pton( AF_INET6, strHost.c_str(), achAddress ) == 1;
        }

            // Milliseconds left before a deadline (negative timeouts never run
            // out: -1).
        int remaining( int nTimeoutMs, const std::chrono::steady_clock::time_point &deadline )
        {
            if ( nTimeoutMs < 0 )
                return -1;
            long long llLeft = std::chrono::duration_cast<std::chrono::milliseconds>( deadline - std::chrono::steady_clock::now() ).count();
            return ( llLeft > 0 ) ? (int)llLeft : 0;
        }
    }

    CSecureSocket::CSecureSocket( CSocket *pAccepted, const std::shared_ptr<CTlsContext> &pContext )
        : CSocket( adoptHandle( pAccepted ), "TLS connection", adoptedAddress() ),
          m_pContext( pContext ), m_pSsl( NULL ), m_bServer( true ), m_bHandshakeDone( false ),
          m_nSendTimeoutMs( 30000 ), m_nHandshakeTimeoutMs( 30000 )
    {
        setUp();
    }

    CSecureSocket::CSecureSocket( const char *strHost, int nPort, const std::shared_ptr<CTlsContext> &pContext )
        : CSocket( strHost, nPort ),
          m_pContext( pContext ), m_pSsl( NULL ), m_bServer( false ), m_bHandshakeDone( false ),
          m_nSendTimeoutMs( 30000 ), m_nHandshakeTimeoutMs( 30000 ), m_strHost( strHost ? strHost : "" )
    {
        setUp();
    }

    CSecureSocket::~CSecureSocket()
    {
        Close();
    }

    void CSecureSocket::setUp( void )
    {
        if ( m_hSocket == NULL_SOCKET || ! m_pContext || m_pContext->getNativeContext() == NULL )
        {
            m_strTlsError = m_pContext ? "the TLS context could not be created" : "no TLS context";
            return;
        }
        if ( m_bServer && ! m_pContext->isReady() )
        {
            m_strTlsError = "the TLS context has no certificate and key";
            return;
        }

        ERR_clear_error();
        m_pSsl = SSL_new( m_pContext->getNativeContext() );
        if ( m_pSsl == NULL )
        {
            recordError( "could not start TLS", 0 );
            return;
        }
        BIO *pBio = BIO_new( bioMethod() );
        BIO_set_data( pBio, (void *)(intptr_t)m_hSocket );
        SSL_set_bio( m_pSsl, pBio, pBio );
        setNonBlocking( m_hSocket );

        if ( m_bServer )
        {
            SSL_set_accept_state( m_pSsl );
        }
        else
        {
            SSL_set_connect_state( m_pSsl );
            if ( ! m_strHost.empty() )
            {
                bool bIp = isIpAddress( m_strHost );
                if ( ! bIp )
                    SSL_set_tlsext_host_name( m_pSsl, m_strHost.c_str() );     // SNI
                if ( m_pContext->getVerifyPeer() )
                {
                    X509_VERIFY_PARAM *pParam = SSL_get0_param( m_pSsl );
                    if ( bIp )
                        X509_VERIFY_PARAM_set1_ip_asc( pParam, m_strHost.c_str() );
                    else
                        X509_VERIFY_PARAM_set1_host( pParam, m_strHost.c_str(), 0 );
                }
            }
        }
    }

    void CSecureSocket::recordError( const char *pszWhat, int nSslError )
    {
        std::string strError = pszWhat;
        std::string strDetail = opensslErrors();
        if ( m_pSsl && SSL_get_verify_result( m_pSsl ) != X509_V_OK )
        {
            strDetail = std::string( "certificate verification failed: " ) +
                        X509_verify_cert_error_string( SSL_get_verify_result( m_pSsl ) );
        }
        else if ( strDetail.empty() && nSslError == SSL_ERROR_SYSCALL )
        {
            strDetail = "the connection was closed";
        }
        if ( ! strDetail.empty() )
            strError += ": " + strDetail;
        m_strTlsError = strError;
    }

        // Waits for what the TLS layer asked for: 1 ready, 0 timed out, -1 error.
    int CSecureSocket::waitFor( int nSslError, int nTimeoutMs )
    {
        if ( nSslError == SSL_ERROR_WANT_WRITE )
            return WaitForWritable( m_hSocket, nTimeoutMs );
        return WaitForReadable( m_hSocket, nTimeoutMs );
    }

    bool CSecureSocket::Handshake( int nTimeoutMs )
    {
        std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( nTimeoutMs < 0 ? 0 : nTimeoutMs );
        while ( true )
        {
            int nResult;
            int nError;
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                if ( m_bHandshakeDone )
                    return true;
                if ( m_pSsl == NULL )
                {
                    if ( m_strTlsError.empty() )
                        m_strTlsError = "the connection is closed";
                    return false;
                }
                ERR_clear_error();
                nResult = SSL_do_handshake( m_pSsl );
                if ( nResult == 1 )
                {
                    m_bHandshakeDone = true;
                    return true;
                }
                nError = SSL_get_error( m_pSsl, nResult );
                if ( nError != SSL_ERROR_WANT_READ && nError != SSL_ERROR_WANT_WRITE )
                {
                    recordError( "the TLS handshake failed", nError );
                    return false;
                }
            }
            int nLeft = remaining( nTimeoutMs, deadline );
            int nReady = ( nTimeoutMs >= 0 && nLeft == 0 ) ? 0 : waitFor( nError, nLeft );
            if ( nReady <= 0 )
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                m_strTlsError = ( nReady == 0 ) ? "the TLS handshake timed out" : "the TLS handshake failed: the connection failed";
                return false;
            }
        }
    }

    bool CSecureSocket::IsHandshakeComplete( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return m_bHandshakeDone;
    }

    int CSecureSocket::ReadWithTimeout( char *pchBuffer, int nBufferSize, int nTimeoutMs )
    {
        if ( pchBuffer == NULL || nBufferSize <= 0 )
            return ( nBufferSize == 0 ) ? 0 : READ_ERROR;

        std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( nTimeoutMs < 0 ? 0 : nTimeoutMs );
        if ( ! IsHandshakeComplete() )
        {
            int nLimit = ( nTimeoutMs < 0 ) ? m_nHandshakeTimeoutMs : nTimeoutMs;
            if ( ! Handshake( nLimit ) )
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                return ( m_strTlsError.find( "timed out" ) != std::string::npos && nTimeoutMs >= 0 ) ? READ_TIMEOUT : READ_ERROR;
            }
        }

        while ( true )
        {
            int nResult;
            int nError;
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                if ( m_pSsl == NULL )
                    return READ_ERROR;
                ERR_clear_error();
                nResult = SSL_read( m_pSsl, pchBuffer, nBufferSize );
                if ( nResult > 0 )
                    return nResult;
                nError = SSL_get_error( m_pSsl, nResult );
                if ( nError == SSL_ERROR_ZERO_RETURN )
                    return 0;                                   // close_notify
                if ( nError == SSL_ERROR_SYSCALL && nResult == 0 )
                    return 0;                                   // closed without close_notify
                if ( nError != SSL_ERROR_WANT_READ && nError != SSL_ERROR_WANT_WRITE )
                {
                    recordError( "TLS read failed", nError );
                    return READ_ERROR;
                }
            }
            int nLeft = remaining( nTimeoutMs, deadline );
            if ( nTimeoutMs >= 0 && nLeft == 0 )
                return READ_TIMEOUT;
            int nReady = waitFor( nError, nLeft );
            if ( nReady < 0 )
                return READ_ERROR;
            if ( nReady == 0 )
                return READ_TIMEOUT;
        }
    }

    int CSecureSocket::Read( char *pchBuffer, int nBufferSize )
    {
        int nRead = ReadWithTimeout( pchBuffer, nBufferSize, -1 );
        return ( nRead >= 0 ) ? nRead : -1;
    }

    unsigned char CSecureSocket::Read( void )
    {
        char chValue = 0;
        return ( Read( &chValue, 1 ) == 1 ) ? (unsigned char)chValue : 0;
    }

    int CSecureSocket::Send( const char *pchBuffer, int nBufferSize )
    {
        if ( pchBuffer == NULL || nBufferSize < 0 )
            return -1;
        if ( ! IsHandshakeComplete() && ! Handshake( m_nHandshakeTimeoutMs ) )
            return -1;

        std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( m_nSendTimeoutMs < 0 ? 0 : m_nSendTimeoutMs );
        int nSent = 0;
        while ( nSent < nBufferSize )
        {
            int nResult;
            int nError;
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                if ( m_pSsl == NULL )
                    return -1;
                ERR_clear_error();
                nResult = SSL_write( m_pSsl, pchBuffer + nSent, nBufferSize - nSent );
                if ( nResult > 0 )
                {
                    nSent += nResult;
                    continue;
                }
                nError = SSL_get_error( m_pSsl, nResult );
                if ( nError != SSL_ERROR_WANT_READ && nError != SSL_ERROR_WANT_WRITE )
                {
                    recordError( "TLS write failed", nError );
                    return -1;
                }
            }
            int nLeft = remaining( m_nSendTimeoutMs, deadline );
            if ( ( m_nSendTimeoutMs >= 0 && nLeft == 0 ) || waitFor( nError, nLeft ) <= 0 )
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                m_strTlsError = "TLS write timed out";
                return -1;
            }
        }
        return nSent;
    }

    int CSecureSocket::Send( unsigned char chSend )
    {
        char chValue = (char)chSend;
        return Send( &chValue, 1 );
    }

    bool CSecureSocket::HasData( void )
    {
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            if ( m_pSsl == NULL )
                return false;
            if ( SSL_pending( m_pSsl ) > 0 )
                return true;
        }
        return WaitForReadable( m_hSocket, 0 ) > 0;
    }

    void CSecureSocket::Close( void )
    {
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            if ( m_pSsl )
            {
                    // One attempt at close_notify; the socket does not block.
                if ( m_bHandshakeDone )
                    SSL_shutdown( m_pSsl );
                ERR_clear_error();
                SSL_free( m_pSsl );
                m_pSsl = NULL;
            }
            m_bHandshakeDone = false;
        }
        CSocket::Close();
    }

    CString CSecureSocket::GetTlsError( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return CString( m_strTlsError.c_str() );
    }

    CString CSecureSocket::GetProtocolVersion( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return CString( ( m_pSsl && m_bHandshakeDone ) ? SSL_get_version( m_pSsl ) : "" );
    }

    CString CSecureSocket::GetCipher( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        const char *pszCipher = ( m_pSsl && m_bHandshakeDone ) ? SSL_get_cipher_name( m_pSsl ) : NULL;
        return CString( pszCipher ? pszCipher : "" );
    }

    bool CSecureSocket::HasPeerCertificate( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        if ( m_pSsl == NULL || ! m_bHandshakeDone )
            return false;
        X509 *pPeer = peerCertificate( m_pSsl );
        if ( pPeer )
            X509_free( pPeer );
        return pPeer != NULL;
    }

    CString CSecureSocket::GetPeerSubject( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        if ( m_pSsl == NULL || ! m_bHandshakeDone )
            return CString( "" );
        X509 *pPeer = peerCertificate( m_pSsl );
        if ( pPeer == NULL )
            return CString( "" );
        char achBuffer[ 512 ];
        X509_NAME_oneline( X509_get_subject_name( pPeer ), achBuffer, sizeof( achBuffer ) );
        X509_free( pPeer );
        return CString( achBuffer );
    }
} // namespace IASLib

#endif // IASLIB_TLS__
#endif // IASLIB_NETWORKING__
