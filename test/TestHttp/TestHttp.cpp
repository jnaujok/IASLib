/**
 * HTTP server tests
 *
 * Tests request framing (CHttpUtils), the HTTP protocol core (CHttpServer)
 * and CBasicHttpServer over real TCP connections on the loopback interface.
 */

#include "IASLib.h"
#include "NetworkServices/HTTP/BasicHttpServer.h"
#include "NetworkServices/HTTP/HttpServer.h"
#include "NetworkServices/HTTP/HttpUtils.h"
#include "NetworkServices/HTTP/Handlers/BaseHttpHandlerFactory.h"
#include "NetworkServices/Entities/TextPlainEntity.h"
#include "Logging/LogSink.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

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

class CTestLogSink : public CLogSink
{
    public:
        std::atomic<int> lines;
        CTestLogSink( void ) : CLogSink( CLogSink::DEBUG ), lines( 0 ) {}
        virtual bool writeLine( const char *message ) { lines++; return true; }
};

/***********************************************************************
 * Test handlers
 ***********************************************************************/

    // GET /hello -> "hello"; /empty -> 204; /throw -> exception; /close ->
    // asks to close the connection; anything else -> 404.
class CTestGetHandler : public CHttpHandler
{
    IASLIB_DEFINE_HTTP_HANDLER( CTestGetHandler, "GET" );
    public:
        DEFINE_OBJECT( CTestGetHandler );
        CTestGetHandler( CString uri, CString version ) : CHttpHandler( uri, version ) {}
        virtual bool process( CHttpRequest *request, CHttpResponse *response )
        {
            CString path = request->getPath();
            if ( path == "/hello" )
            {
                response->setStatus( 200 );
                response->SetEntity( new CTextPlainEntity( CString( "hello" ) ) );
            }
            else if ( path == "/empty" )
            {
                response->setStatus( 204 );
            }
            else if ( path == "/throw" )
            {
                throw 42;
            }
            else if ( path == "/close" )
            {
                response->setStatus( 200 );
                response->addHeader( "Connection", "close" );
            }
            else
            {
                response->setStatus( 404 );
            }
            return true;
        }
};
IMPLEMENT_OBJECT( CTestGetHandler, CHttpHandler );

    // POST echoes the body back.
class CEchoPostHandler : public CHttpHandler
{
    IASLIB_DEFINE_HTTP_HANDLER( CEchoPostHandler, "POST" );
    public:
        DEFINE_OBJECT( CEchoPostHandler );
        CEchoPostHandler( CString uri, CString version ) : CHttpHandler( uri, version ) {}
        virtual bool process( CHttpRequest *request, CHttpResponse *response )
        {
            CString body = request->getEntity() ? request->getEntity()->toString() : CString( "" );
            response->setStatus( 200 );
            response->SetEntity( new CTextPlainEntity( body ) );
            return true;
        }
};
IMPLEMENT_OBJECT( CEchoPostHandler, CHttpHandler );

static CHttpHandlerFactory *testFactory( void )
{
    CHttpHandlerFactory *factory = new CHttpHandlerFactory();
    factory->registerHandler( new CTestGetHandler( "", "" ) );
    factory->registerHandler( new CEchoPostHandler( "", "" ) );
    return factory;
}

/***********************************************************************
 * Helpers
 ***********************************************************************/

static int statusOf( const std::string &response )
{
    if ( response.compare( 0, 9, "HTTP/1.1 " ) != 0 )
        return -1;
    return atoi( response.c_str() + 9 );
}

static std::string headerOf( const std::string &message, const std::string &name )
{
    std::string lowerName = name;
    for ( size_t n = 0; n < lowerName.size(); n++ ) lowerName[ n ] = (char)tolower( lowerName[ n ] );
    size_t pos = message.find( "\r\n" );
    while ( pos != std::string::npos )
    {
        size_t start = pos + 2;
        size_t end = message.find( "\r\n", start );
        if ( end == std::string::npos || end == start )
            break;
        std::string line = message.substr( start, end - start );
        size_t colon = line.find( ':' );
        if ( colon != std::string::npos )
        {
            std::string lineName = line.substr( 0, colon );
            for ( size_t n = 0; n < lineName.size(); n++ ) lineName[ n ] = (char)tolower( lineName[ n ] );
            if ( lineName == lowerName )
            {
                std::string value = line.substr( colon + 1 );
                while ( ! value.empty() && value[0] == ' ' ) value.erase( 0, 1 );
                return value;
            }
        }
        pos = end;
    }
    return std::string();
}

static bool hasHeader( const std::string &message, const std::string &name )
{
    size_t end = message.find( "\r\n\r\n" );
    std::string head = message.substr( 0, end );
    std::string lowerHead = head;
    for ( size_t n = 0; n < lowerHead.size(); n++ ) lowerHead[ n ] = (char)tolower( lowerHead[ n ] );
    std::string lowerName = name;
    for ( size_t n = 0; n < lowerName.size(); n++ ) lowerName[ n ] = (char)tolower( lowerName[ n ] );
    return lowerHead.find( "\r\n" + lowerName + ":" ) != std::string::npos;
}

static std::string bodyOf( const std::string &message )
{
    size_t pos = message.find( "\r\n\r\n" );
    return ( pos == std::string::npos ) ? std::string() : message.substr( pos + 4 );
}

static CInternetAddress loopback( void )
{
    return CInternetAddress( "127.0.0.1", 50000, SOCK_STREAM );
}

    // Frames and handles one request through the core.
static std::string exchange( CHttpServer &server, const std::string &request, bool &keepAlive )
{
    CHttpFrame frame = CHttpUtils::frameRequest( request.data(), request.size(), 65536, 1024 * 1024 );
    if ( frame.status != CHttpFrame::COMPLETE )
    {
        keepAlive = false;
        return std::string( (const char *)server.errorResponse( frame.status == CHttpFrame::FAILED ? frame.errorStatus : 400 ) );
    }
    CString response = server.handleRequest( CString( frame.message.data(), frame.message.size() ), loopback(), keepAlive );
    return std::string( (const char *)response, response.GetLength() );
}

/***********************************************************************
 * Framing
 ***********************************************************************/

static void testFraming( void )
{
    printf( "Framing\n" );

    auto frame = []( const std::string &text ) { return CHttpUtils::frameRequest( text.data(), text.size(), 1024, 4096 ); };

    std::string two = "\r\nPOST /a HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\n\r\nabcGET /b HTTP/1.1\r\nHost: x\r\n\r\n";
    CHttpFrame f = frame( two );
    CHECK( f.status == CHttpFrame::COMPLETE );
    CHECK( f.consumed == two.find( "GET" ) );
    CHECK( f.message == "POST /a HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\n\r\nabc" );
    std::string rest = two.substr( f.consumed );
    f = frame( rest );
    CHECK( f.status == CHttpFrame::COMPLETE && f.consumed == rest.size() );
    CHECK( f.message == "GET /b HTTP/1.1\r\nHost: x\r\nContent-Length: 0\r\n\r\n" );

        // Incomplete headers and bodies.
    CHECK( frame( "GET / HTTP/1.1\r\nHost: x\r\n" ).status == CHttpFrame::NEED_MORE );
    f = frame( "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 10\r\nExpect: 100-continue\r\n\r\nabc" );
    CHECK( f.status == CHttpFrame::NEED_MORE );
    CHECK( f.expectContinue && f.headersComplete );

        // Chunked, with an extension and a trailer.
    std::string chunked = "POST /c HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n"
                          "5;name=value\r\nhello\r\n6\r\n world\r\n0\r\nTrailer: yes\r\n\r\nNEXT";
    f = frame( chunked );
    CHECK( f.status == CHttpFrame::COMPLETE );
    CHECK( f.message == "POST /c HTTP/1.1\r\nHost: x\r\nContent-Length: 11\r\n\r\nhello world" );
    CHECK( chunked.substr( f.consumed ) == "NEXT" );
    CHECK( frame( chunked.substr( 0, chunked.find( " world" ) ) ).status == CHttpFrame::NEED_MORE );

        // Request smuggling vectors and limits.
    f = frame( "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n" );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 400 );
    f = frame( "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\nContent-Length: 4\r\n\r\nabcd" );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 400 );
    f = frame( "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 3, 3\r\n\r\nabc" );
    CHECK( f.status == CHttpFrame::COMPLETE );
    f = frame( "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: -1\r\n\r\n" );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 400 );
    f = frame( "POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: gzip, chunked\r\n\r\n" );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 501 );
    f = frame( "POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nzz\r\n" );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 400 );
    f = frame( "GET / HTTP/1.1\r\nHost : x\r\n\r\n" );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 400 );
    f = frame( "GET / HTTP/1.1\r\nX: " + std::string( 2000, 'a' ) + "\r\n\r\n" );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 431 );
    f = frame( "GET / HTTP/1.1\r\nX: " + std::string( 2000, 'a' ) );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 431 );
    f = frame( "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 5000\r\n\r\n" );
    CHECK( f.status == CHttpFrame::FAILED && f.errorStatus == 413 );
}

/***********************************************************************
 * Protocol core
 ***********************************************************************/

static void testCore( void )
{
    printf( "Protocol core\n" );
    CHttpServer server( 8080 );
    server.AddHandlerFactory( new CBaseHttpHandlerFactory() );
    bool keepAlive = false;

        // The base handlers.
    std::string r = exchange( server, "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 404 );
    CHECK( keepAlive );
    CHECK( headerOf( r, "Content-Length" ) == "0" );
    CHECK( headerOf( r, "Date" ).size() == 29 );
    CHECK( headerOf( r, "Date" ).find( " GMT" ) == 25 );
    CHECK( headerOf( r, "Server" ) == "IASLib/2.0" );
    CHECK( ! hasHeader( r, "Connection" ) );

    r = exchange( server, "OPTIONS * HTTP/1.1\r\nHost: example.com\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 200 );
    CHECK( headerOf( r, "Allow" ) == "GET, POST, OPTIONS, HEAD" );
    r = exchange( server, "OPTIONS /thing HTTP/1.1\r\nHost: example.com\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 200 );
    CHECK( headerOf( r, "Allow" ) == "GET, POST, OPTIONS, HEAD" );

    r = exchange( server, "PUT /x HTTP/1.1\r\nHost: example.com\r\nContent-Length: 0\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 405 );
    CHECK( ! headerOf( r, "Allow" ).empty() );
    r = exchange( server, "BREW /pot HTTP/1.1\r\nHost: example.com\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 501 );

        // Request checks.
    r = exchange( server, "GET / HTTP/1.1\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 400 );
    CHECK( ! keepAlive );
    CHECK( headerOf( r, "Connection" ) == "close" );
    r = exchange( server, "GET / HTTP/1.1\r\nHost: a\r\nHost: b\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 400 );
    r = exchange( server, "GET / HTTP/2.0\r\nHost: a\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 505 );
    r = exchange( server, "GET / HTTP/1.1x\r\nHost: a\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 400 );
    r = exchange( server, "GET / HTTP/1.1\r\nHost: a\r\nExpect: tea\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 417 );
    r = exchange( server, "garbage\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 400 );

        // Persistence.
    r = exchange( server, "GET / HTTP/1.1\r\nHost: a\r\nConnection: close\r\n\r\n", keepAlive );
    CHECK( ! keepAlive && headerOf( r, "Connection" ) == "close" );
    r = exchange( server, "GET / HTTP/1.0\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 404 );
    CHECK( ! keepAlive );
    r = exchange( server, "GET / HTTP/1.0\r\nConnection: Keep-Alive\r\n\r\n", keepAlive );
    CHECK( keepAlive && headerOf( r, "Connection" ) == "keep-alive" );

        // Application handlers in front of the base ones.
    server.AddHandlerFactory( testFactory() );
    r = exchange( server, "GET /hello?x=1 HTTP/1.1\r\nHost: a\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 200 );
    CHECK( headerOf( r, "Content-Length" ) == "5" );
    CHECK( headerOf( r, "Content-Type" ) == "text/plain" );
    CHECK( bodyOf( r ) == "hello" );

        // HEAD uses the GET handler and leaves out the body.
    r = exchange( server, "HEAD /hello HTTP/1.1\r\nHost: a\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 200 );
    CHECK( headerOf( r, "Content-Length" ) == "5" );
    CHECK( bodyOf( r ).empty() );

    r = exchange( server, "GET /empty HTTP/1.1\r\nHost: a\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 204 );
    CHECK( ! hasHeader( r, "Content-Length" ) );

    r = exchange( server, "GET /throw HTTP/1.1\r\nHost: a\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 500 );

    r = exchange( server, "GET /close HTTP/1.1\r\nHost: a\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 200 && ! keepAlive && headerOf( r, "Connection" ) == "close" );

    r = exchange( server, "POST /echo HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nabc\r\n0\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 200 && bodyOf( r ) == "abc" );

        // Format directives and very long values are harmless.
    r = exchange( server, "GET /%n%n%s%s" + std::string( 20000, 'a' ) + " HTTP/1.1\r\nHost: %n%s\r\n\r\n", keepAlive );
    CHECK( statusOf( r ) == 404 );

        // Request target parts.
    CHttpRequest request( "GET", "http://example.com:8080/a/b?q=1#frag" );
    CHECK( request.getPath() == "/a/b" );
    CHECK( request.getQuery() == "q=1" );
    CHttpRequest origin( "GET", "/x?redirect=http://y/z" );
    CHECK( origin.getPath() == "/x" );
    CHECK( origin.getQuery() == "redirect=http://y/z" );
}

/***********************************************************************
 * CBasicHttpServer over real sockets
 ***********************************************************************/

static int connectTo( int port )
{
    int fd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons( (uint16_t)port );
    addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    if ( connect( fd, (struct sockaddr *)&addr, sizeof( addr ) ) != 0 )
    {
        close( fd );
        return -1;
    }
    return fd;
}

static void sendAll( int fd, const std::string &data )
{
    size_t sent = 0;
    while ( sent < data.size() )
    {
        ssize_t n = send( fd, data.data() + sent, data.size() - sent, MSG_NOSIGNAL );
        if ( n <= 0 )
            return;
        sent += (size_t)n;
    }
}

    // Reads until nResponses complete responses have arrived (or the
    // connection closes or the time runs out). Sets bClosed if the server
    // closed the connection.
static std::string readResponses( int fd, int nResponses, int timeoutMs, bool &bClosed )
{
    std::string data;
    bClosed = false;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( timeoutMs );
    while ( std::chrono::steady_clock::now() < deadline )
    {
            // Count complete responses (they all carry Content-Length here,
            // except 1xx and 204).
        int nComplete = 0;
        size_t pos = 0;
        while ( true )
        {
            size_t end = data.find( "\r\n\r\n", pos );
            if ( end == std::string::npos )
                break;
            std::string head = data.substr( pos, end - pos );
            size_t length = 0;
            size_t cl = head.find( "Content-Length: " );
            if ( cl != std::string::npos )
                length = (size_t)atoi( head.c_str() + cl + 16 );
            if ( data.size() < end + 4 + length )
                break;
            pos = end + 4 + length;
            nComplete++;
        }
        if ( nComplete >= nResponses )
            break;

        struct pollfd pfd = { fd, POLLIN, 0 };
        if ( poll( &pfd, 1, 50 ) > 0 )
        {
            char buffer[ 8192 ];
            ssize_t n = recv( fd, buffer, sizeof( buffer ), 0 );
            if ( n <= 0 )
            {
                bClosed = true;
                break;
            }
            data.append( buffer, (size_t)n );
        }
    }
    return data;
}

static bool waitForClose( int fd, int timeoutMs )
{
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( timeoutMs );
    while ( std::chrono::steady_clock::now() < deadline )
    {
        struct pollfd pfd = { fd, POLLIN, 0 };
        if ( poll( &pfd, 1, 50 ) > 0 )
        {
            char buffer[ 4096 ];
            if ( recv( fd, buffer, sizeof( buffer ), 0 ) <= 0 )
                return true;
        }
    }
    return false;
}

static void testBasicServer( void )
{
    printf( "CBasicHttpServer\n" );
    CBasicHttpServer server( 0, 2 );
    server.AddHandlerFactory( new CBaseHttpHandlerFactory() );
    server.AddHandlerFactory( testFactory() );
    server.setRequestTimeout( 500 );
    server.setKeepAliveTimeout( 800 );
    server.Start();
    CHECK( server.waitUntilReady( 5000 ) );
    int port = server.getListeningPort();
    CHECK( port > 0 );
    bool bClosed = false;

        // Two pipelined requests on one connection, then the connection idles
        // out.
    int fd = connectTo( port );
    CHECK( fd >= 0 );
    sendAll( fd, "GET /hello HTTP/1.1\r\nHost: a\r\n\r\nGET /missing HTTP/1.1\r\nHost: a\r\n\r\n" );
    std::string data = readResponses( fd, 2, 3000, bClosed );
    CHECK( statusOf( data ) == 200 );
    CHECK( data.find( "HTTP/1.1 404 Not Found" ) != std::string::npos );
    CHECK( ! bClosed );
    CHECK( waitForClose( fd, 3000 ) );       // keep-alive timeout
    close( fd );

        // Expect: 100-continue with a chunked body sent afterwards.
    fd = connectTo( port );
    sendAll( fd, "POST /echo HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\nExpect: 100-continue\r\n\r\n" );
    data = readResponses( fd, 1, 2000, bClosed );
    CHECK( data == "HTTP/1.1 100 Continue\r\n\r\n" );
    sendAll( fd, "4\r\nping\r\n0\r\n\r\n" );
    data = readResponses( fd, 1, 2000, bClosed );
    CHECK( statusOf( data ) == 200 && bodyOf( data ) == "ping" );

        // Connection: close is honoured.
    sendAll( fd, "GET /hello HTTP/1.1\r\nHost: a\r\nConnection: close\r\n\r\n" );
    data = readResponses( fd, 1, 2000, bClosed );
    CHECK( statusOf( data ) == 200 && headerOf( data, "Connection" ) == "close" );
    CHECK( waitForClose( fd, 2000 ) );
    close( fd );

        // A request that never completes gets 408.
    fd = connectTo( port );
    sendAll( fd, "GET /hello HTTP/1.1\r\nHost: a\r\n" );
    data = readResponses( fd, 1, 3000, bClosed );
    CHECK( statusOf( data ) == 408 );
    close( fd );

        // A smuggling attempt gets 400 and the connection is closed.
    fd = connectTo( port );
    sendAll( fd, "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 4\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n" );
    data = readResponses( fd, 1, 2000, bClosed );
    CHECK( statusOf( data ) == 400 );
    CHECK( waitForClose( fd, 2000 ) );
    close( fd );

        // Over the connection limit (2): the third connection gets 503.
    int a = connectTo( port );
    int b = connectTo( port );
    std::this_thread::sleep_for( std::chrono::milliseconds( 400 ) );
    int c = connectTo( port );
    data = readResponses( c, 1, 2000, bClosed );
    CHECK( statusOf( data ) == 503 );
    close( c );

        // Stopping with idle connections open returns promptly.
    auto start = std::chrono::steady_clock::now();
    server.Stop();
    server.Wait();
    long elapsed = (long)std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - start ).count();
    CHECK( elapsed < 2000 );
    CHECK( waitForClose( a, 1000 ) );
    close( a );
    close( b );

        // A port in use is reported.
    CBasicHttpServer first( 0 );
    first.Start();
    CHECK( first.waitUntilReady( 5000 ) );
    CBasicHttpServer clash( first.getListeningPort() );
    clash.Start();
    CHECK( ! clash.waitUntilReady( 5000 ) );
    CHECK( clash.getStartupError().GetLength() > 0 );

    {
        CBasicHttpServer unused( 0 );
    }
    {
        CHttpServer core;
    }
}

int main( int argc, char *argv[] )
{
    CTestLogSink *log = new CTestLogSink();

    testFraming();
    testCore();
    testBasicServer();

    printf( "\n%d checks, %d failures, %d log lines\n", g_nChecks, g_nFailures, (int)log->lines );
    delete log;
    return ( g_nFailures == 0 ) ? 0 : 1;
}
