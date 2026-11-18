/***********************************************************************
**  TestTls
**
**  Secure sockets: CTlsContext (certificates and keys from PEM text and
**  files), CSecureSocket (client and server sides), HTTPS with
**  CBasicHttpServer and SIP over TLS with CBasicSipServer. The certificates
**  are made when the test runs, so none can expire in the repository.
***********************************************************************/

#include "IASLib.h"
#include "NetworkServices/HTTP/BasicHttpServer.h"
#include "NetworkServices/HTTP/HttpClient.h"
#include "NetworkServices/HTTP/Handlers/BaseHttpHandlerFactory.h"
#include "NetworkServices/Entities/TextPlainEntity.h"
#include "NetworkServices/SIP/BasicSipServer.h"
#include "NetworkServices/SIP/SipUtils.h"
#include "NetworkServices/SIP/Handlers/BaseSipHandlerFactory.h"
#include "../TestSockets.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace IASLib;

static int g_nChecks = 0;
static int g_nFailures = 0;

#define CHECK( condition ) \
    do { \
        g_nChecks++; \
        if ( ! ( condition ) ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition ); \
        } \
    } while ( 0 )

/***********************************************************************
 * Certificates
 ***********************************************************************/

struct CTestCertificate
{
    std::string certificate;
    std::string key;
};

static std::string bioText( BIO *pBio )
{
    char *pData = NULL;
    long nLength = BIO_get_mem_data( pBio, &pData );
    return std::string( pData, (size_t)nLength );
}

    // A self-signed P-256 certificate for the given names (as a subjectAltName
    // value, such as "DNS:localhost,IP:127.0.0.1").
static CTestCertificate makeCertificate( const char *pszCommonName, const char *pszAltNames )
{
    CTestCertificate result;

    EVP_PKEY_CTX *pKeyContext = EVP_PKEY_CTX_new_id( EVP_PKEY_EC, NULL );
    EVP_PKEY *pKey = NULL;
    EVP_PKEY_keygen_init( pKeyContext );
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid( pKeyContext, NID_X9_62_prime256v1 );
    EVP_PKEY_keygen( pKeyContext, &pKey );
    EVP_PKEY_CTX_free( pKeyContext );

    X509 *pCertificate = X509_new();
    X509_set_version( pCertificate, 2 );
    static long lSerial = 1000;
    ASN1_INTEGER_set( X509_get_serialNumber( pCertificate ), lSerial++ );
    X509_gmtime_adj( X509_getm_notBefore( pCertificate ), -3600 );
    X509_gmtime_adj( X509_getm_notAfter( pCertificate ), 30L * 86400L );
    X509_set_pubkey( pCertificate, pKey );
    X509_NAME *pName = X509_get_subject_name( pCertificate );
    X509_NAME_add_entry_by_txt( pName, "CN", MBSTRING_ASC, (const unsigned char *)pszCommonName, -1, -1, 0 );
    X509_set_issuer_name( pCertificate, pName );

    X509V3_CTX extensionContext;
    X509V3_set_ctx_nodb( &extensionContext );
    X509V3_set_ctx( &extensionContext, pCertificate, pCertificate, NULL, NULL, 0 );
    X509_EXTENSION *pExtension = X509V3_EXT_conf_nid( NULL, &extensionContext, NID_subject_alt_name, (char *)pszAltNames );
    X509_add_ext( pCertificate, pExtension, -1 );
    X509_EXTENSION_free( pExtension );
    X509_sign( pCertificate, pKey, EVP_sha256() );

    BIO *pBio = BIO_new( BIO_s_mem() );
    PEM_write_bio_X509( pBio, pCertificate );
    result.certificate = bioText( pBio );
    BIO_free( pBio );
    pBio = BIO_new( BIO_s_mem() );
    PEM_write_bio_PrivateKey( pBio, pKey, NULL, NULL, 0, NULL, NULL );
    result.key = bioText( pBio );
    BIO_free( pBio );

    X509_free( pCertificate );
    EVP_PKEY_free( pKey );
    return result;
}

static std::shared_ptr<CTlsContext> serverContext( const CTestCertificate &certificate )
{
    std::shared_ptr<CTlsContext> context = std::make_shared<CTlsContext>( CTlsContext::SERVER );
    context->setCertificatePem( certificate.certificate.c_str() );
    context->setPrivateKeyPem( certificate.key.c_str() );
    return context;
}

static std::shared_ptr<CTlsContext> clientTrusting( const CTestCertificate &certificate )
{
    std::shared_ptr<CTlsContext> context = std::make_shared<CTlsContext>( CTlsContext::CLIENT );
    context->addTrustedCertificatePem( certificate.certificate.c_str() );
    return context;
}

/***********************************************************************
 * Helpers
 ***********************************************************************/

    // GET answers with the name of its site.
class CSiteHandler : public CHttpHandler
{
    public:
        DEFINE_OBJECT( CSiteHandler );
        CSiteHandler( CString site, CString uri = "", CString version = "" ) : CHttpHandler( uri, version ), m_site( site ) {}
        virtual CString getMethod( void ) { return CString( "GET" ); }
        virtual CHttpHandler *generateHandler( CString uri, CString version ) { return new CSiteHandler( m_site, uri, version ); }
        virtual bool process( CHttpRequest *request, CHttpResponse *response )
        {
            (void)request;
            response->setStatus( 200 );
            response->SetEntity( new CTextPlainEntity( m_site ) );
            return true;
        }
    private:
        CString m_site;
};
IMPLEMENT_OBJECT( CSiteHandler, CHttpHandler );

static CHttpHandlerFactory *siteFactory( const char *pszSite )
{
    CHttpHandlerFactory *factory = new CHttpHandlerFactory();
    factory->registerHandler( new CSiteHandler( pszSite ) );
    return factory;
}

    // Reads until the text contains nMarkers copies of strMarker followed by
    // a complete header block, the peer closes, or the time runs out.
static std::string readUntil( CSocket &socket, const std::string &strMarker, int nMarkers, int nTimeoutMs, bool &bClosed )
{
    std::string strData;
    bClosed = false;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( nTimeoutMs );
    while ( std::chrono::steady_clock::now() < deadline )
    {
        int nFound = 0;
        size_t nPos = 0;
        while ( ( nPos = strData.find( strMarker, nPos ) ) != std::string::npos )
        {
            nFound++;
            nPos += strMarker.size();
        }
        if ( nFound >= nMarkers && strData.rfind( "\r\n\r\n" ) != std::string::npos &&
             strData.rfind( "\r\n\r\n" ) > strData.rfind( strMarker ) )
            break;

        char achBuffer[ 4096 ];
        int nRead = socket.ReadWithTimeout( achBuffer, sizeof( achBuffer ), 100 );
        if ( nRead == CSocket::READ_TIMEOUT )
            continue;
        if ( nRead <= 0 )
        {
            bClosed = true;
            break;
        }
        strData.append( achBuffer, (size_t)nRead );
    }
    return strData;
}

static int rawConnect( int nPort )
{
    int fd = (int)socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons( (uint16_t)nPort );
    addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    if ( connect( fd, (struct sockaddr *)&addr, sizeof( addr ) ) != 0 )
    {
        closeSocket( fd );
        return -1;
    }
    return fd;
}

    // True if the peer closes the raw connection within the time (any data
    // received before that is returned in strData).
static bool rawWaitForClose( int fd, int nTimeoutMs, std::string &strData )
{
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( nTimeoutMs );
    while ( std::chrono::steady_clock::now() < deadline )
    {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        if ( poll( &pfd, 1, 50 ) > 0 )
        {
            char achBuffer[ 4096 ];
            int n = (int)recv( fd, achBuffer, sizeof( achBuffer ), 0 );
            if ( n <= 0 )
                return true;
            strData.append( achBuffer, (size_t)n );
        }
    }
    return false;
}

static long elapsedMs( const std::chrono::steady_clock::time_point &start )
{
    return (long)std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - start ).count();
}

/***********************************************************************
 * Tests
 ***********************************************************************/

static void testContexts( const CTestCertificate &good, const CTestCertificate &other )
{
    printf( "TLS contexts\n" );
    CTlsContext server( CTlsContext::SERVER );
    CHECK( server.getRole() == CTlsContext::SERVER && ! server.isReady() && ! server.getVerifyPeer() );
    CHECK( server.setCertificatePem( good.certificate.c_str() ) && ! server.isReady() );
    CHECK( ! server.setPrivateKeyPem( other.key.c_str() ) );                       // not this certificate's key
    CHECK( server.getError().IndexOf( "does not match" ) != NOT_FOUND );
    CHECK( ! server.isReady() );
    CHECK( server.setPrivateKeyPem( good.key.c_str() ) && server.isReady() && server.getError().GetLength() == 0 );

    CTlsContext garbage( CTlsContext::SERVER );
    CHECK( ! garbage.setCertificatePem( "not a certificate" ) && garbage.getError().GetLength() > 0 );
    CHECK( ! garbage.setPrivateKeyPem( "-----BEGIN PRIVATE KEY-----\nAAAA\n-----END PRIVATE KEY-----\n" ) );
    CHECK( ! garbage.setCertificateFile( "/no/such/file.pem" ) && garbage.getError().IndexOf( "/no/such/file.pem" ) != NOT_FOUND );
    CHECK( ! garbage.setTrustedCertificatesFile( "/no/such/ca.pem" ) );
    CHECK( ! garbage.addTrustedCertificatePem( "nothing here" ) );
    CHECK( ! garbage.isReady() );

        // From files, with the key first.
    const char *pszCertFile = "iaslib_test_cert.pem";
    const char *pszKeyFile = "iaslib_test_key.pem";
    FILE *pFile = fopen( pszCertFile, "wb" );
    fwrite( good.certificate.data(), 1, good.certificate.size(), pFile );
    fclose( pFile );
    pFile = fopen( pszKeyFile, "wb" );
    fwrite( good.key.data(), 1, good.key.size(), pFile );
    fclose( pFile );
    CTlsContext fromFiles( CTlsContext::SERVER );
    CHECK( fromFiles.setPrivateKeyFile( pszKeyFile ) && fromFiles.setCertificateFile( pszCertFile ) && fromFiles.isReady() );
    CTlsContext trustFile( CTlsContext::CLIENT );
    CHECK( trustFile.setTrustedCertificatesFile( pszCertFile ) && trustFile.isReady() && trustFile.getVerifyPeer() );
    remove( pszCertFile );
    remove( pszKeyFile );
}

static void testHttps( const CTestCertificate &good, const CTestCertificate &other )
{
    printf( "HTTPS\n" );
    std::shared_ptr<CTlsContext> tls = serverContext( good );

    CBasicHttpServer server( 0, 8 );
    server.AddHandlerFactory( new CBaseHttpHandlerFactory() );
    CHECK( ! server.setTlsContext( std::make_shared<CTlsContext>( CTlsContext::CLIENT ) ) );
    CHECK( ! server.setTlsContext( std::make_shared<CTlsContext>( CTlsContext::SERVER ) ) );    // no certificate
    CHECK( ! server.isSecure() );
    CHECK( server.setTlsContext( tls ) && server.isSecure() );
    server.setHandshakeTimeout( 1000 );
    server.setKeepAliveTimeout( 2000 );
    server.Start();
    CHECK( server.waitUntilReady( 5000 ) );
    int nPort = server.getListeningPort();
    CHECK( nPort > 0 );

        // A request, verified against the certificate (by IP address).
    {
        CSecureSocket client( "127.0.0.1", nPort, clientTrusting( good ) );
        CHECK( client.Handshake( 5000 ) );
        CHECK( client.IsHandshakeComplete() );
        CString strVersion = client.GetProtocolVersion();
        CHECK( strVersion == "TLSv1.3" || strVersion == "TLSv1.2" );
        CHECK( client.GetCipher().GetLength() > 0 );
        CHECK( client.HasPeerCertificate() && client.GetPeerSubject().IndexOf( "CN=IASLib test" ) != NOT_FOUND );
        std::string strRequest = "GET /missing HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
        CHECK( client.Send( strRequest.data(), (int)strRequest.size() ) == (int)strRequest.size() );
        bool bClosed = false;
        std::string strData = readUntil( client, "HTTP/1.1 ", 1, 3000, bClosed );
        CHECK( strData.compare( 0, 22, "HTTP/1.1 404 Not Found" ) == 0 );
        char achByte;
        auto start = std::chrono::steady_clock::now();
        CHECK( client.ReadWithTimeout( &achByte, 1, 3000 ) == 0 );                 // Connection: close
        CHECK( elapsedMs( start ) < 2500 );
    }

        // Keep-alive: two requests on one connection, sent in pieces, by name.
    {
        CSecureSocket client( "localhost", nPort, clientTrusting( good ) );
        std::string strFirst = "GET /missing HTTP/1.1\r\nHost: localhost\r\n\r\n";
        std::string strSecond = "GET /other HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
        CHECK( client.Send( strFirst.data(), 10 ) == 10 );                        // handshakes first
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
        CHECK( client.Send( strFirst.data() + 10, (int)strFirst.size() - 10 ) == (int)strFirst.size() - 10 );
        bool bClosed = false;
        std::string strData = readUntil( client, "HTTP/1.1 404", 1, 3000, bClosed );
        CHECK( strData.find( "HTTP/1.1 404" ) == 0 && ! bClosed );
        CHECK( client.Send( strSecond.data(), (int)strSecond.size() ) == (int)strSecond.size() );
        strData = readUntil( client, "HTTP/1.1 404", 1, 3000, bClosed );
        CHECK( strData.find( "HTTP/1.1 404" ) != std::string::npos );
    }

        // CHttpClient over https: verified, with its connection reused.
    {
        CHttpClient client( "localhost", nPort, true );
        client.setTlsContext( clientTrusting( good ) );
        client.setTimeout( 5000 );
        CHttpResponse *pResponse = client.get( "/missing" );
        CHECK( pResponse && pResponse->getStatusCode() == 404 );
        delete pResponse;
        pResponse = client.head( "/other" );
        CHECK( pResponse && pResponse->getStatusCode() == 404 );
        delete pResponse;
        CHECK( client.IsConnected() && client.getConnectionsOpened() == 1 );

        CHttpClient untrusting( "127.0.0.1", nPort, true );
        untrusting.setTlsContext( clientTrusting( other ) );
        untrusting.setTimeout( 5000 );
        CHECK( untrusting.get( "/" ) == NULL && untrusting.getLastError().IndexOf( "TLS handshake failed" ) != NOT_FOUND );
    }

        // An untrusted certificate, and one for another name, are refused.
    {
        CSecureSocket client( "127.0.0.1", nPort, clientTrusting( other ) );
        CHECK( ! client.Handshake( 5000 ) );
        CHECK( client.GetTlsError().IndexOf( "certificate verification failed" ) != NOT_FOUND );
        CHECK( client.Send( "x", 1 ) == -1 );
    }
    {
        std::shared_ptr<CTlsContext> trustsBoth = clientTrusting( good );
        trustsBoth->addTrustedCertificatePem( other.certificate.c_str() );
        CBasicHttpServer wrongName( 0, 2 );
        wrongName.AddHandlerFactory( new CBaseHttpHandlerFactory() );
        wrongName.setTlsContext( serverContext( other ) );                           // "other.example" only
        wrongName.Start();
        CHECK( wrongName.waitUntilReady( 5000 ) );
        CSecureSocket client( "localhost", wrongName.getListeningPort(), trustsBoth );
        CHECK( ! client.Handshake( 5000 ) );
        CHECK( client.GetTlsError().IndexOf( "verification failed" ) != NOT_FOUND );
        wrongName.Stop();
        wrongName.Wait();
    }
    {
            // Without verification the same handshake succeeds.
        std::shared_ptr<CTlsContext> trustsAnyone = std::make_shared<CTlsContext>( CTlsContext::CLIENT );
        trustsAnyone->setVerifyPeer( false );
        CSecureSocket client( "127.0.0.1", nPort, trustsAnyone );
        CHECK( client.Handshake( 5000 ) );
    }

        // Plain HTTP to the HTTPS port gets no HTTP answer, and is closed.
    {
        int fd = rawConnect( nPort );
        CHECK( fd >= 0 );
        std::string strRequest = "GET /missing HTTP/1.1\r\nHost: a\r\n\r\n";
        send( fd, strRequest.data(), (int)strRequest.size(), MSG_NOSIGNAL );
        std::string strData;
        CHECK( rawWaitForClose( fd, 3000, strData ) );
        CHECK( strData.find( "HTTP/" ) == std::string::npos );
        closeSocket( fd );
    }

        // A client that never handshakes does not hold others up, and is
        // dropped after the handshake timeout.
    {
        int fdSilent = rawConnect( nPort );
        CHECK( fdSilent >= 0 );
        auto start = std::chrono::steady_clock::now();
        CSecureSocket client( "127.0.0.1", nPort, clientTrusting( good ) );
        CHECK( client.Handshake( 5000 ) );
        CHECK( elapsedMs( start ) < 1000 );                 // served before the silent one times out
        std::string strData;
        CHECK( rawWaitForClose( fdSilent, 6000, strData ) );
        CHECK( elapsedMs( start ) >= 900 );
        closeSocket( fdSilent );
    }

        // One thread writes while another reads on the same connection: 40
        // pipelined requests, answered while still being sent.
    {
        CSecureSocket client( "127.0.0.1", nPort, clientTrusting( good ) );
        CHECK( client.Handshake( 5000 ) );
        std::thread writer( [&client]
        {
            std::string strRequest = "GET /missing HTTP/1.1\r\nHost: localhost\r\n\r\n";
            for ( int n = 0; n < 40; n++ )
                client.Send( strRequest.data(), (int)strRequest.size() );
        } );
        bool bClosed = false;
        std::string strData = readUntil( client, "HTTP/1.1 404", 40, 10000, bClosed );
        writer.join();
        size_t nCount = 0;
        for ( size_t nPos = 0; ( nPos = strData.find( "HTTP/1.1 404", nPos ) ) != std::string::npos; nPos++ )
            nCount++;
        CHECK( nCount == 40 && ! bClosed );
    }

        // A client that vanishes mid-request does no harm.
    {
        CSecureSocket *pClient = new CSecureSocket( "127.0.0.1", nPort, clientTrusting( good ) );
        CHECK( pClient->Handshake( 5000 ) );
        pClient->Send( "GET /missing HTTP/1.1\r\nHost: a\r\n\r\n", 33 );
        delete pClient;
        std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
    }

        // Stopping with an idle TLS connection open returns promptly.
    CSecureSocket idle( "127.0.0.1", nPort, clientTrusting( good ) );
    CHECK( idle.Handshake( 5000 ) );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    CHECK( server.getConnectionCount() >= 1 );
    auto start = std::chrono::steady_clock::now();
    server.Stop();
    server.Wait();
    CHECK( elapsedMs( start ) < 2000 );
    char achByte;
    CHECK( idle.ReadWithTimeout( &achByte, 1, 2000 ) <= 0 && idle.ReadWithTimeout( &achByte, 1, 100 ) != CSocket::READ_TIMEOUT );
}

static std::string sipOptions( const char *pszBranch, int nCSeq, const char *pszTransport = "TLS", const char *pszScheme = "sips" )
{
    char achRequest[ 1024 ];
    snprintf( achRequest, sizeof( achRequest ),
              "OPTIONS %s:server@localhost SIP/2.0\r\n"
              "Via: SIP/2.0/%s client.example;branch=%s\r\n"
              "Max-Forwards: 70\r\n"
              "From: <%s:alice@client.example>;tag=tls1\r\n"
              "To: <%s:server@localhost>\r\n"
              "Call-ID: tls-call@client.example\r\n"
              "CSeq: %d OPTIONS\r\n"
              "Content-Length: 0\r\n\r\n", pszScheme, pszTransport, pszBranch, pszScheme, pszScheme, nCSeq );
    return achRequest;
}

    // Sends one SIP request over plain TCP and returns the first response.
static std::string tcpSipExchange( int nPort, const std::string &strRequest )
{
    int fd = rawConnect( nPort );
    if ( fd < 0 )
        return "";
    send( fd, strRequest.data(), (int)strRequest.size(), MSG_NOSIGNAL );
    std::string strReceived;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 3 );
    while ( std::chrono::steady_clock::now() < deadline )
    {
        size_t nLeading = 0;
        long nLength = CSipUtils::frameMessage( strReceived.data(), strReceived.size(), nLeading, 65535 );
        if ( nLength > 0 )
        {
            closeSocket( fd );
            return strReceived.substr( nLeading, (size_t)nLength - nLeading );
        }
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        if ( poll( &pfd, 1, 50 ) > 0 )
        {
            char achBuffer[ 4096 ];
            int n = (int)recv( fd, achBuffer, sizeof( achBuffer ), 0 );
            if ( n <= 0 )
                break;
            strReceived.append( achBuffer, (size_t)n );
        }
    }
    closeSocket( fd );
    return strReceived;
}

static void testSipOverTls( const CTestCertificate &good )
{
    printf( "SIP over TLS\n" );
    CBasicSipServer server( 0, false, false, 2 );                // TLS only
    server.AddHandlerFactory( new CBaseSipHandlerFactory() );
    CHECK( ! server.setTlsContext( std::make_shared<CTlsContext>( CTlsContext::SERVER ), 0 ) );
    CHECK( server.setTlsContext( serverContext( good ), 0 ) );
    server.setHandshakeTimeout( 2000 );
    server.Start();
    CHECK( server.waitUntilReady( 5000 ) );
    int nPort = server.getTlsPort();
    CHECK( nPort > 0 && server.getUdpPort() == 0 && server.getTcpPort() == 0 );

    CSecureSocket client( "localhost", nPort, clientTrusting( good ) );
    CHECK( client.Handshake( 5000 ) );
    std::string strStream = sipOptions( "z9hG4bKtls1", 1 ) + sipOptions( "z9hG4bKtls2", 2 ) + "\r\n\r\n";
    CHECK( client.Send( strStream.data(), (int)strStream.size() ) == (int)strStream.size() );

    std::string strReceived;
    std::vector<std::string> responses;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 3 );
    while ( std::chrono::steady_clock::now() < deadline && ! ( responses.size() == 2 && strReceived == "\r\n" ) )
    {
        char achBuffer[ 4096 ];
        int nRead = client.ReadWithTimeout( achBuffer, sizeof( achBuffer ), 100 );
        if ( nRead == CSocket::READ_TIMEOUT )
            continue;
        if ( nRead <= 0 )
            break;
        strReceived.append( achBuffer, (size_t)nRead );
        size_t nLeading = 0;
        long nLength = 0;
        while ( responses.size() < 2 && ( nLength = CSipUtils::frameMessage( strReceived.data(), strReceived.size(), nLeading, 65535 ) ) > 0 )
        {
            responses.push_back( strReceived.substr( nLeading, (size_t)nLength - nLeading ) );
            strReceived.erase( 0, (size_t)nLength );
        }
    }
    CHECK( responses.size() == 2 );
    if ( responses.size() == 2 )
    {
        CHECK( responses[0].compare( 0, 14, "SIP/2.0 200 OK" ) == 0 );
        CHECK( responses[0].find( "Via: SIP/2.0/TLS client.example;branch=z9hG4bKtls1" ) != std::string::npos );
        CHECK( responses[0].find( "CSeq: 1 OPTIONS" ) != std::string::npos );
        CHECK( responses[1].find( "CSeq: 2 OPTIONS" ) != std::string::npos );
    }
    CHECK( strReceived == "\r\n" );                             // the keep-alive pong

        // A server with all three transports uses separate ports.
    CBasicSipServer all( 0, true, true, 2 );
    all.AddHandlerFactory( new CBaseSipHandlerFactory() );
    CHECK( all.setTlsContext( serverContext( good ), 0 ) );
    all.Start();
    CHECK( all.waitUntilReady( 5000 ) );
    CHECK( all.getUdpPort() > 0 && all.getTcpPort() > 0 && all.getTlsPort() > 0 && all.getTlsPort() != all.getTcpPort() );

        // sips: needs TLS: refused over TCP (416), sip: is fine over TCP.
    CHECK( all.getRequireTlsForSips() );
    std::string strReply = tcpSipExchange( all.getTcpPort(), sipOptions( "z9hG4bKtcpsips", 1, "TCP", "sips" ) );
    CHECK( strReply.compare( 0, 34, "SIP/2.0 416 Unsupported URI Scheme" ) == 0 );
    strReply = tcpSipExchange( all.getTcpPort(), sipOptions( "z9hG4bKtcpsip", 2, "TCP", "sip" ) );     // a new CSeq: not a merged request
    CHECK( strReply.compare( 0, 14, "SIP/2.0 200 OK" ) == 0 );
    all.Stop();
    all.Wait();
    CHECK( all.getTlsPort() == 0 );

        // Behind a proxy that ends TLS for it, the check can be turned off.
    CBasicSipServer behindProxy( 0, false, true, 1 );
    behindProxy.AddHandlerFactory( new CBaseSipHandlerFactory() );
    behindProxy.setRequireTlsForSips( false );
    behindProxy.Start();
    CHECK( behindProxy.waitUntilReady( 5000 ) );
    strReply = tcpSipExchange( behindProxy.getTcpPort(), sipOptions( "z9hG4bKproxied", 1, "TCP", "sips" ) );
    CHECK( strReply.compare( 0, 14, "SIP/2.0 200 OK" ) == 0 );
    behindProxy.Stop();
    behindProxy.Wait();

    auto start = std::chrono::steady_clock::now();
    server.Stop();
    server.Wait();
    CHECK( elapsedMs( start ) < 2000 );
}

    // Connects to the server's address asking for pszName, and returns the
    // certificate subject it got ("" if the handshake failed).
static std::string subjectFor( int nPort, const char *pszName, const std::shared_ptr<CTlsContext> &client )
{
    CSecureSocket socket( "127.0.0.1", nPort, client );
    if ( pszName )
        socket.SetServerName( pszName );
    if ( ! socket.Handshake( 5000 ) )
        return "";
    return (const char *)socket.GetPeerSubject();
}

static void testServerNames( const CTestCertificate &good, const CTestCertificate &other )
{
    printf( "Server name indication\n" );
    CTestCertificate wild = makeCertificate( "Wild", "DNS:*.wild.example" );
    CTestCertificate late = makeCertificate( "Late", "DNS:late.example" );

    std::shared_ptr<CTlsContext> tls = serverContext( good );
    CHECK( ! tls->addHostContext( "x.example", std::make_shared<CTlsContext>( CTlsContext::CLIENT ) ) );
    CHECK( ! tls->addHostContext( "x.example", std::make_shared<CTlsContext>( CTlsContext::SERVER ) ) );   // no certificate
    CHECK( ! tls->addHostContext( "", serverContext( other ) ) );
    CHECK( ! std::make_shared<CTlsContext>( CTlsContext::CLIENT )->addHostContext( "x.example", serverContext( other ) ) );
    CHECK( tls->addHostContext( "Other.Example", serverContext( other ) ) );
    CHECK( tls->addHostContext( "*.wild.example", serverContext( wild ) ) );

    CBasicHttpServer server( 0, 8 );
    server.AddHandlerFactory( new CBaseHttpHandlerFactory() );
    CHECK( server.setTlsContext( tls ) );
    server.Start();
    CHECK( server.waitUntilReady( 5000 ) );
    int nPort = server.getListeningPort();

    std::shared_ptr<CTlsContext> anyCertificate = std::make_shared<CTlsContext>( CTlsContext::CLIENT );
    anyCertificate->setVerifyPeer( false );

        // The certificate follows the name asked for, and is verified for it.
    CHECK( subjectFor( nPort, "other.example", clientTrusting( other ) ).find( "CN=Other" ) != std::string::npos );
    CHECK( subjectFor( nPort, "OTHER.example", clientTrusting( other ) ).find( "CN=Other" ) != std::string::npos );
    CHECK( subjectFor( nPort, "a.wild.example", clientTrusting( wild ) ).find( "CN=Wild" ) != std::string::npos );
    CHECK( subjectFor( nPort, "a.b.wild.example", anyCertificate ).find( "CN=IASLib test" ) != std::string::npos );   // one label only
    CHECK( subjectFor( nPort, "nobody.example", anyCertificate ).find( "CN=IASLib test" ) != std::string::npos );
    CHECK( subjectFor( nPort, NULL, clientTrusting( good ) ).find( "CN=IASLib test" ) != std::string::npos );          // an address: no name
    CHECK( subjectFor( nPort, "localhost", clientTrusting( good ) ).find( "CN=IASLib test" ) != std::string::npos );
        // Verification uses the name asked for: the default certificate is
        // not valid for other.example.
    CHECK( subjectFor( nPort, "nobody.example", clientTrusting( good ) ).empty() );

        // The client side reports the name it asked for.
    {
        CSecureSocket socket( "127.0.0.1", nPort, clientTrusting( other ) );
        socket.SetServerName( "other.example" );
        CHECK( socket.Handshake( 5000 ) && socket.GetServerName() == "other.example" );
    }

        // Hosts can be added while serving.
    CHECK( subjectFor( nPort, "late.example", anyCertificate ).find( "CN=IASLib test" ) != std::string::npos );
    CHECK( tls->addHostContext( "late.example", serverContext( late ) ) );
    CHECK( subjectFor( nPort, "late.example", clientTrusting( late ) ).find( "CN=Late" ) != std::string::npos );
    server.Stop();
    server.Wait();

        // Strict: unknown names are refused; no name still gets the default.
    std::shared_ptr<CTlsContext> strict = serverContext( good );
    strict->addHostContext( "other.example", serverContext( other ) );
    strict->setRequireKnownHostName( true );
    CBasicHttpServer strictServer( 0, 4 );
    strictServer.AddHandlerFactory( new CBaseHttpHandlerFactory() );
    strictServer.setTlsContext( strict );
    strictServer.Start();
    CHECK( strictServer.waitUntilReady( 5000 ) );
    nPort = strictServer.getListeningPort();
    CHECK( subjectFor( nPort, "nobody.example", anyCertificate ).empty() );
    CHECK( subjectFor( nPort, "other.example", clientTrusting( other ) ).find( "CN=Other" ) != std::string::npos );
    CHECK( subjectFor( nPort, NULL, clientTrusting( good ) ).find( "CN=IASLib test" ) != std::string::npos );
    strictServer.Stop();
    strictServer.Wait();
}

    // One request on a TLS connection that asked for pszSni (NULL: none),
    // with the given Host; returns the response, and whether the server then
    // closed the connection.
static std::string httpsRequest( int nPort, const char *pszSni, const std::shared_ptr<CTlsContext> &client,
                                 const char *pszHost, bool &bClosed )
{
    CSecureSocket socket( "127.0.0.1", nPort, client );
    if ( pszSni )
        socket.SetServerName( pszSni );
    bClosed = false;
    if ( ! socket.Handshake( 5000 ) )
        return "no handshake";
    std::string strRequest = std::string( "GET / HTTP/1.1\r\nHost: " ) + pszHost + "\r\n\r\n";
    socket.Send( strRequest.data(), (int)strRequest.size() );
    std::string strData = readUntil( socket, "HTTP/1.1 ", 1, 3000, bClosed );
    if ( ! bClosed )
    {
        char achByte;
        int nRead = socket.ReadWithTimeout( &achByte, 1, 500 );
        bClosed = ( nRead == 0 || nRead == CSocket::READ_ERROR );
    }
    return strData;
}

static void testHttpsVirtualHosts( const CTestCertificate &good, const CTestCertificate &other )
{
    printf( "HTTPS virtual hosts\n" );
    std::shared_ptr<CTlsContext> tls = serverContext( good );          // localhost
    tls->addHostContext( "other.example", serverContext( other ) );

    CBasicHttpServer server( 0, 8 );
    server.AddVirtualHost( "localhost", siteFactory( "local site" ) );
    server.AddVirtualHost( "other.example", siteFactory( "other site" ) );
    CHECK( server.setTlsContext( tls ) );
    server.Start();
    CHECK( server.waitUntilReady( 5000 ) );
    int nPort = server.getListeningPort();
    std::shared_ptr<CTlsContext> anyCertificate = std::make_shared<CTlsContext>( CTlsContext::CLIENT );
    anyCertificate->setVerifyPeer( false );

        // Each name gets its own certificate and its own site.
    bool bClosed = false;
    std::string strData = httpsRequest( nPort, "other.example", clientTrusting( other ), "other.example", bClosed );
    CHECK( strData.find( "HTTP/1.1 200" ) == 0 && strData.find( "other site" ) != std::string::npos && ! bClosed );
    strData = httpsRequest( nPort, "localhost", clientTrusting( good ), "localhost:443", bClosed );
    CHECK( strData.find( "HTTP/1.1 200" ) == 0 && strData.find( "local site" ) != std::string::npos );

        // Another site's Host on a connection made for other.example: 421,
        // and the connection is closed.
    strData = httpsRequest( nPort, "other.example", clientTrusting( other ), "localhost", bClosed );
    CHECK( strData.find( "HTTP/1.1 421 Misdirected Request" ) == 0 );
    CHECK( strData.find( "local site" ) == std::string::npos && bClosed );

        // A client that asked for no name (an address) is routed by Host.
    strData = httpsRequest( nPort, NULL, anyCertificate, "other.example", bClosed );
    CHECK( strData.find( "HTTP/1.1 200" ) == 0 && strData.find( "other site" ) != std::string::npos );

        // An unknown name: the default certificate, and no virtual host.
    strData = httpsRequest( nPort, "nobody.example", anyCertificate, "nobody.example", bClosed );
    CHECK( strData.find( "HTTP/1.1 405" ) == 0 );                  // no default handlers at all

    server.Stop();
    server.Wait();
}

static void testTrustStore( const CTestCertificate &good )
{
    printf( "Trusted certificates\n" );
    CHECK( clientTrusting( good )->getTrustedCertificateCount() >= 1 );
    CTlsContext system( CTlsContext::CLIENT );
    int nSystem = system.getTrustedCertificateCount();
#ifdef IASLIB_WIN32__
        // Read from the Windows Trusted Root store.
    CHECK( nSystem > 0 );
    CHECK( system.useSystemTrustStore() );
#endif
    printf( "  %d system certificates loaded\n", nSystem );
    CTlsContext server( CTlsContext::SERVER );
    CHECK( server.getTrustedCertificateCount() == 0 );
}

int main( void )
{
    setvbuf( stdout, NULL, _IONBF, 0 );
    CTestCertificate good = makeCertificate( "IASLib test", "DNS:localhost,IP:127.0.0.1" );
    CTestCertificate other = makeCertificate( "Other", "DNS:other.example" );

    testContexts( good, other );
    testHttps( good, other );
    testSipOverTls( good );
    testServerNames( good, other );
    testHttpsVirtualHosts( good, other );
    testTrustStore( good );

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
