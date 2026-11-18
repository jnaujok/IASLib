/**
 * HTTP client tests
 *
 * Tests response framing (CHttpUtils::frameResponse), CHttpResponse::parse,
 * the refusal of CR, LF and NUL in header names, values and request URIs,
 * and CHttpClient over real TCP connections on the loopback interface:
 * against the library's own CBasicHttpServer, and against small scripted
 * servers in this program for the cases CBasicHttpServer never produces
 * (bodies delimited by the connection closing, servers that do not answer).
 * Every server listens on a port the system picks, so the test can run
 * beside others.
 */

#include "IASLib.h"
#include "NetworkServices/HTTP/BasicHttpServer.h"
#include "NetworkServices/HTTP/HttpClient.h"
#include "NetworkServices/HTTP/HttpUtils.h"
#include "NetworkServices/HTTP/Handlers/BaseHttpHandlerFactory.h"
#include "NetworkServices/Entities/TextPlainEntity.h"
#include "NetworkServices/Entities/RawEntity.h"
#include "Sockets/ServerSocket.h"
#include "Streams/StringStream.h"
#include "Logging/LogSink.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
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

class CQuietLogSink : public CLogSink
{
    public:
        CQuietLogSink( void ) : CLogSink( CLogSink::DEBUG ) {}
        virtual bool writeLine( const char *message ) { (void)message; return true; }
};

static std::string text( const CString &str )
{
    return std::string( (const char *)str, str.GetLength() );
}

static std::string bodyOf( CHttpResponse *response )
{
    if ( response == NULL || response->getEntity() == NULL )
        return std::string();
    return text( response->getEntity()->toString() );
}

static long elapsedMs( const std::chrono::steady_clock::time_point &start )
{
    return (long)std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - start ).count();
}

/***********************************************************************
 * Handlers for CBasicHttpServer
 ***********************************************************************/

    // GET /hello -> "hello"; /chunked -> a chunked body; /close -> asks to
    // close; /binary -> bytes with zeros in them; anything else -> 404.
class CClientTestGetHandler : public CHttpHandler
{
    IASLIB_DEFINE_HTTP_HANDLER( CClientTestGetHandler, "GET" );
    public:
        DEFINE_OBJECT( CClientTestGetHandler );
        CClientTestGetHandler( CString uri, CString version ) : CHttpHandler( uri, version ) {}
        virtual bool process( CHttpRequest *request, CHttpResponse *response )
        {
            CString path = request->getPath();
            if ( path == "/hello" )
            {
                response->setStatus( 200 );
                response->SetEntity( new CTextPlainEntity( CString( "hello" ) ) );
            }
            else if ( path == "/host" )
            {
                response->setStatus( 200 );
                response->SetEntity( new CTextPlainEntity( request->getHeaderValue( "Host" ) ) );
            }
            else if ( path == "/chunked" )
            {
                    // The handler encodes the body itself; the server must not
                    // add a Content-Length beside Transfer-Encoding.
                response->setStatus( 200 );
                response->addHeader( "Transfer-Encoding", "chunked" );
                response->SetEntity( new CRawEntity( CString( "text/plain" ),
                                                     CString( "5\r\nhello\r\n7;ext=1\r\n, world\r\n0\r\nX-Trailer: t\r\n\r\n" ) ) );
            }
            else if ( path == "/close" )
            {
                response->setStatus( 200 );
                response->addHeader( "Connection", "close" );
                response->SetEntity( new CTextPlainEntity( CString( "bye" ) ) );
            }
            else if ( path == "/binary" )
            {
                response->setStatus( 200 );
                response->SetEntity( new CRawEntity( CString( "application/octet-stream" ), CString::FromBytes( "a\0b\0c", 5 ) ) );
            }
            else
            {
                response->setStatus( 404 );
            }
            return true;
        }
};
IMPLEMENT_OBJECT( CClientTestGetHandler, CHttpHandler );

    // POST echoes the body back with the request's Content-Type.
class CClientEchoHandler : public CHttpHandler
{
    IASLIB_DEFINE_HTTP_HANDLER( CClientEchoHandler, "POST" );
    public:
        DEFINE_OBJECT( CClientEchoHandler );
        CClientEchoHandler( CString uri, CString version ) : CHttpHandler( uri, version ) {}
        virtual bool process( CHttpRequest *request, CHttpResponse *response )
        {
            CString body = request->getEntity() ? request->getEntity()->toString() : CString( "" );
            response->setStatus( 200 );
            response->SetEntity( new CRawEntity( request->getHeaderValue( "Content-Type" ), body ) );
            return true;
        }
};
IMPLEMENT_OBJECT( CClientEchoHandler, CHttpHandler );

static CHttpHandlerFactory *clientTestFactory( void )
{
    CHttpHandlerFactory *factory = new CHttpHandlerFactory();
    factory->registerHandler( new CClientTestGetHandler( "", "" ) );
    factory->registerHandler( new CClientEchoHandler( "", "" ) );
    return factory;
}

/***********************************************************************
 * A scripted server: accepts one connection at a time, reads a request
 * (up to the blank line) and answers with the next canned reply. An empty
 * reply means "say nothing and hold the connection until told to stop".
 ***********************************************************************/

class CScriptedServer
{
    public:
        CScriptedServer( const std::vector<std::string> &replies, bool bCloseAfterReply )
            : m_replies( replies ), m_bClose( bCloseAfterReply ), m_stop( false ), m_nAccepted( 0 )
        {
            m_pListen = new CServerSocket( 0, 8 );
            m_nPort = m_pListen->GetPort();
            m_thread = std::thread( [this] { run(); } );
        }
        ~CScriptedServer( void )
        {
            m_stop = true;
            m_thread.join();
            delete m_pListen;
        }
        int port( void ) const { return m_nPort; }
        int accepted( void ) const { return m_nAccepted; }
        std::string lastRequest( void ) { std::lock_guard<std::mutex> lock( m_mutex ); return m_strLastRequest; }

    private:
        void run( void )
        {
            size_t nReply = 0;
            while ( ! m_stop )
            {
                CSocket *pSocket = m_pListen->Accept( 100 );
                if ( pSocket == NULL )
                    continue;
                m_nAccepted++;
                bool bOpen = true;
                std::string buffer;
                while ( bOpen && ! m_stop )
                {
                    char achBuffer[ 4096 ];
                    int nRead = pSocket->ReadWithTimeout( achBuffer, (int)sizeof( achBuffer ), 100 );
                    if ( nRead == CSocket::READ_TIMEOUT )
                        continue;
                    if ( nRead <= 0 )
                        break;
                    buffer.append( achBuffer, (size_t)nRead );
                    size_t nEnd = buffer.find( "\r\n\r\n" );
                    if ( nEnd == std::string::npos )
                        continue;
                    {
                        std::lock_guard<std::mutex> lock( m_mutex );
                        m_strLastRequest = buffer.substr( 0, nEnd + 4 );
                    }
                    buffer.erase( 0, nEnd + 4 );
                    std::string reply = ( nReply < m_replies.size() ) ? m_replies[ nReply++ ] : std::string( "HTTP/1.1 500 No More\r\nContent-Length: 0\r\n\r\n" );
                    if ( reply.empty() )
                    {
                        while ( ! m_stop )
                            std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
                        break;
                    }
                    try { pSocket->Send( reply.data(), (int)reply.size() ); } catch ( ... ) {}
                    if ( m_bClose )
                        bOpen = false;
                }
                pSocket->Close();
                delete pSocket;
            }
        }

        std::vector<std::string>    m_replies;
        bool                        m_bClose;
        std::atomic<bool>           m_stop;
        std::atomic<int>            m_nAccepted;
        CServerSocket              *m_pListen;
        int                         m_nPort;
        std::thread                 m_thread;
        std::mutex                  m_mutex;
        std::string                 m_strLastRequest;
};

/***********************************************************************
 * Response framing
 ***********************************************************************/

static CHttpFrame frame( const std::string &data, bool bHead = false, bool bClosed = false )
{
    return CHttpUtils::frameResponse( data.data(), data.size(), bHead, bClosed, 65536, 1024 * 1024 );
}

static void testResponseFraming( void )
{
    printf( "Response framing\n" );

        // Content-Length, with the next response's bytes left over.
    std::string data = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhelloHTTP/1.1";
    CHttpFrame result = frame( data );
    CHECK( result.status == CHttpFrame::COMPLETE && result.statusCode == 200 && result.keepAlive && ! result.closeDelimited );
    CHECK( result.consumed == data.size() - 8 );
    CHECK( result.message.size() >= 5 && result.message.compare( result.message.size() - 9, 9, "\r\n\r\nhello" ) == 0 );

        // Arriving in pieces.
    CHECK( frame( "HTTP/1.1 200 OK\r\nContent-Len" ).status == CHttpFrame::NEED_MORE );
    CHECK( frame( "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhel" ).status == CHttpFrame::NEED_MORE );
    CHECK( frame( "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhel", false, true ).status == CHttpFrame::FAILED );

        // Chunked, with an extension and a trailer: decoded, and given a
        // Content-Length in place of Transfer-Encoding.
    data = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5;x=y\r\nhello\r\n6\r\n world\r\n0\r\nT: 1\r\n\r\n";
    result = frame( data );
    CHECK( result.status == CHttpFrame::COMPLETE && result.consumed == data.size() );
    CHECK( result.message.find( "Transfer-Encoding" ) == std::string::npos );
    CHECK( result.message.find( "Content-Length: 11\r\n" ) != std::string::npos );
    CHECK( result.message.find( "\r\n\r\nhello world" ) != std::string::npos );
    CHECK( frame( data.substr( 0, data.size() - 2 ) ).status == CHttpFrame::NEED_MORE );
    CHECK( frame( "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\nzz\r\n" ).status == CHttpFrame::FAILED );
        // Transfer-Encoding wins over Content-Length.
    result = frame( "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nabc\r\n0\r\n\r\n" );
    CHECK( result.status == CHttpFrame::COMPLETE && result.message.find( "\r\n\r\nabc" ) != std::string::npos );

        // No length: the body runs to the close.
    data = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nuntil the end";
    CHECK( frame( data ).status == CHttpFrame::NEED_MORE );
    result = frame( data, false, true );
    CHECK( result.status == CHttpFrame::COMPLETE && result.closeDelimited && ! result.keepAlive );
    CHECK( result.message.find( "\r\n\r\nuntil the end" ) != std::string::npos );

        // No body for HEAD, 204, 304 and 1xx, whatever the headers say.
    result = frame( "HTTP/1.1 200 OK\r\nContent-Length: 100\r\n\r\n", true );
    CHECK( result.status == CHttpFrame::COMPLETE && result.message.find( "Content-Length: 100" ) != std::string::npos );
    CHECK( frame( "HTTP/1.1 204 No Content\r\n\r\n" ).status == CHttpFrame::COMPLETE );
    CHECK( frame( "HTTP/1.1 304 Not Modified\r\nContent-Length: 9\r\n\r\n" ).status == CHttpFrame::COMPLETE );
    result = frame( "HTTP/1.1 100 Continue\r\n\r\nHTTP/1.1 200 OK\r\n" );
    CHECK( result.status == CHttpFrame::COMPLETE && result.statusCode == 100 && result.consumed == 25 );

        // Connection persistence.
    CHECK( ! frame( "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 0\r\n\r\n" ).keepAlive );
    CHECK( ! frame( "HTTP/1.0 200 OK\r\nContent-Length: 0\r\n\r\n" ).keepAlive );
    CHECK( frame( "HTTP/1.0 200 OK\r\nConnection: Keep-Alive\r\nContent-Length: 0\r\n\r\n" ).keepAlive );

        // Malformed responses.
    CHECK( frame( "HTTP/1.1 2000 OK\r\n\r\n" ).status == CHttpFrame::FAILED );
    CHECK( frame( "HTTP/1.1 OK\r\n\r\n" ).status == CHttpFrame::FAILED );
    CHECK( frame( "SIP/2.0 200 OK\r\n\r\n" ).status == CHttpFrame::FAILED );
    CHECK( frame( "HTTP/1.1 200 OK\r\nContent-Length: 1\r\nContent-Length: 2\r\n\r\nab" ).status == CHttpFrame::FAILED );
    CHECK( frame( std::string( "HTTP/1.1 200 OK\r\nX: a\0b\r\nContent-Length: 0\r\n\r\n", 46 ) ).status == CHttpFrame::FAILED );
    CHECK( frame( "HTTP/1.1 200 OK\r\nX: a\rY: b\r\nContent-Length: 0\r\n\r\n" ).status == CHttpFrame::FAILED );
    result = CHttpUtils::frameResponse( "HTTP/1.1 200 OK\r\nContent-Length: 50\r\n\r\n", 40, false, false, 65536, 10 );
    CHECK( result.status == CHttpFrame::FAILED && result.errorStatus == 413 );

        // Parsing a framed response.
    CHttpResponse response;
    CHECK( response.parse( CString::FromBytes( "HTTP/1.1 201 Created\r\nX-A: 1\r\nX-A: 2\r\nContent-Type: application/x\r\n\r\nab\0c", 73 ) ) );
    CHECK( response.getStatusCode() == 201 && response.getStatusDescription() == "Created" && response.getVersion() == "HTTP/1.1" );
    CHECK( response.getHeaderValues( "x-a" ).Count() == 2 );
    CHECK( response.getEntity() && response.getEntity()->toString().GetLength() == 4 );
    CHECK( response.parse( CString( "HTTP/1.1 404\r\n\r\n" ) ) && response.getStatusCode() == 404 && response.getEntity() == NULL );
    CHECK( ! response.parse( CString( "garbage\r\n\r\n" ) ) );
    CHECK( ! response.parse( CString( "HTTP/1.1 200 OK\r\nNo-End: yes\r\n" ) ) );
}

/***********************************************************************
 * Header and request-line injection
 ***********************************************************************/

    // A stream that cannot say how much is left (bytesRemaining() is the
    // base class's NOT_FOUND) and gives at most 3 bytes per read.
class CTrickleStream : public CStringStream
{
    public:
        CTrickleStream( const std::string &data ) : CStringStream( data ) {}
        virtual size_t bytesRemaining( void ) { return NOT_FOUND; }
        virtual int GetBuffer( char *achBuffer, int nLength ) { return CStringStream::GetBuffer( achBuffer, nLength > 3 ? 3 : nLength ); }
};

static void testInjection( void )
{
    printf( "Injection\n" );

    CHECK( CHeader::isValidName( "Content-Type" ) && CHeader::isValidName( "X_a.b~!" ) );
    CHECK( ! CHeader::isValidName( "" ) && ! CHeader::isValidName( "Bad Name" ) && ! CHeader::isValidName( "X:Y" ) );
    CHECK( ! CHeader::isValidName( "X\r\nY" ) );
    CHECK( CHeader::isValidValue( "text/plain; charset=utf-8" ) && CHeader::isValidValue( "a\tb" ) );
    CHECK( ! CHeader::isValidValue( "a\r\nSet-Cookie: x" ) && ! CHeader::isValidValue( "a\nb" ) && ! CHeader::isValidValue( "a\rb" ) );
    CHECK( ! CHeader::isValidValue( CString::FromBytes( "a\0b", 3 ) ) );

    CHeaderList list;
    CHECK( list.addHeader( "X-Ok", "fine" ) );
    CHECK( ! list.addHeader( "X-Bad", "a\r\nInjected: yes" ) );
    CHECK( ! list.addHeader( "X-Bad\r\nInjected", "v" ) );
    CHECK( ! list.appendHeader( "X-Ok", "b\nc" ) );
    CStringArray values;
    values.Append( "one" );
    values.Append( "two\r\nInjected: 1" );
    CHECK( ! list.addHeader( "X-List", values ) && ! list.hasHeader( "X-List" ) );
    CHECK( list.firstValue( "X-Ok" ) == "fine" && list.allValues( "X-Ok" ).Count() == 1 );
    CHECK( text( list.toString() ) == "X-Ok: fine\r\n" );
    CHeader header( "X" );
    CHECK( header.addValue( "ok" ) && ! header.addValue( "a\r\nb" ) && ! header.setValue( "\n" ) && header.getValueCount() == 1 );

        // Parsed headers: a lone CR or a name that is not a token is refused.
    CStringStream lone( std::string( "X-A: 1\rX-B: 2\r\nGood: yes\r\n\r\n" ) );
    CHeaderList parsedLone( lone );
    CHECK( parsedLone.isMalformed() && ! parsedLone.hasHeader( "X-A" ) && parsedLone.hasHeader( "Good" ) );
    CStringStream badName( std::string( "Bad Name: 1\r\n\r\n" ) );
    CHeaderList parsedBadName( badName );
    CHECK( parsedBadName.isMalformed() && parsedBadName.getHeaderCount() == 0 );

        // Responses.
    CHttpResponse response;
    CHECK( ! response.addHeader( "Location", "/x\r\nSet-Cookie: evil=1" ) && ! response.hasHeader( "Location" ) );
    response.setStatus( 200, "OK\r\nSet-Cookie: evil=1" );
    CHECK( response.getStatusDescription() == "" );
    CHECK( text( response.toString() ).find( "evil" ) == std::string::npos );

        // serialize() leaves Content-Length out when Transfer-Encoding is set.
    CHttpResponse chunked;
    chunked.setStatus( 200 );
    chunked.addHeader( "Content-Length", "99" );
    chunked.addHeader( "Transfer-Encoding", "chunked" );
    chunked.SetEntity( new CRawEntity( CString( "text/plain" ), CString( "2\r\nhi\r\n0\r\n\r\n" ) ) );
    std::string strChunked = text( chunked.toString() );
    CHECK( strChunked.find( "Content-Length" ) == std::string::npos );
    CHECK( strChunked.find( "\r\n\r\n2\r\nhi\r\n0\r\n\r\n" ) != std::string::npos );
    CHttpResponse plain;
    plain.setStatus( 200 );
    plain.SetEntity( new CTextPlainEntity( CString( "abc" ) ) );
    CHECK( text( plain.toString() ).find( "Content-Length: 3\r\n" ) != std::string::npos );

        // Request lines.
    CHttpRequest good( "GET", "/path?q=1" );
    CHECK( good.isValid() );
    CHttpRequest badUri( "GET", "/x HTTP/1.1\r\nHost: evil\r\n\r\nGET /y" );
    CHECK( ! badUri.isValid() );
    CHttpRequest badMethod( "G T", "/" );
    CHECK( ! badMethod.isValid() );
    CHECK( ! good.setUri( "/a\nb" ) && good.getUri() == "/path?q=1" );
    CHECK( ! good.setUri( "" ) && ! good.setUri( "/a\x7f" ) && good.setUri( "/new" ) && good.getUri() == "/new" );
    CHECK( ! CGenericRequest::isValidUri( CString::FromBytes( "/a\0b", 4 ) ) );
    CHECK( ! good.setRequestType( "GET\r\n" ) && ! good.setVersion( "HTTP/1.1\r\nX: y" ) );
    CHECK( ! good.setHeaderValue( "X", "1\r\n2" ) && good.setHeaderValue( "X", "1" ) );
    CStringArray badValues;
    badValues.Append( "ok" );
    badValues.Append( "bad\n" );
    CHECK( ! good.setHeaderValues( "X", badValues ) && good.getHeaderValue( "X" ) == "1" );

        // Incoming requests: framing refuses NUL and lone CRs in the
        // headers, parsing refuses control characters in the URI.
    std::string strNul( "GET / HTTP/1.1\r\nHost: a\r\nX: a\0b\r\n\r\n", 35 );
    CHttpFrame result = CHttpUtils::frameRequest( strNul.data(), strNul.size(), 65536, 65536 );
    CHECK( result.status == CHttpFrame::FAILED && result.errorStatus == 400 );
    std::string strCr = "GET / HTTP/1.1\r\nHost: a\r\nX: a\rTransfer-Encoding: chunked\r\n\r\n";
    result = CHttpUtils::frameRequest( strCr.data(), strCr.size(), 65536, 65536 );
    CHECK( result.status == CHttpFrame::FAILED && result.errorStatus == 400 );
    CInternetAddress from( "127.0.0.1", 50000, SOCK_STREAM );
    CHttpRequest incoming( from );
    CHECK( ! incoming.parse( CString( "GET /a\x01z HTTP/1.1\r\nHost: a\r\n\r\n" ) ) );

        // A request whose header section never ends is not complete.
    CHttpRequest truncated( from );
    CHECK( ! truncated.parse( CString( "GET / HTTP/1.1\r\nHost: a\r\n" ) ) );
    CHECK( truncated.getParseError() == "Incomplete header section" );

        // The body is read in full from a stream that does not know its
        // size and gives a few bytes at a time, zero bytes included.
    std::string strBinary( "POST / HTTP/1.1\r\nHost: a\r\nContent-Type: application/x\r\nContent-Length: 6\r\n\r\nab\0cd\0", 82 );
    CTrickleStream trickle( strBinary );
    CHttpRequest streamed( from );
    CHECK( streamed.parse( trickle ) );
    CHECK( streamed.getEntity() && streamed.getEntity()->toString().GetLength() == 6 );
    std::string strNoLength( "POST / HTTP/1.1\r\nHost: a\r\nContent-Type: application/x\r\n\r\nto the end" );
    CTrickleStream trickleToEnd( strNoLength );
    CHttpRequest toEnd( from );
    CHECK( toEnd.parse( trickleToEnd ) && toEnd.getEntity() && text( toEnd.getEntity()->toString() ) == "to the end" );
    CTrickleStream shortBody( std::string( "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 50\r\n\r\nabc" ) );
    CHttpRequest shortRequest( from );
    CHECK( ! shortRequest.parse( shortBody ) && shortRequest.getParseError() == "Message body is shorter than Content-Length" );
}

/***********************************************************************
 * CHttpClient against CBasicHttpServer
 ***********************************************************************/

static void testAgainstServer( void )
{
    printf( "CHttpClient and CBasicHttpServer\n" );
    CBasicHttpServer server( 0, 8 );
    server.AddHandlerFactory( new CBaseHttpHandlerFactory() );
    server.AddHandlerFactory( clientTestFactory() );
    server.setKeepAliveTimeout( 600 );
    server.Start();
    CHECK( server.waitUntilReady( 5000 ) );
    int nPort = server.getListeningPort();

    CHttpClient client( "127.0.0.1", nPort );
    client.setTimeout( 5000 );
    CHECK( ! client.IsConnected() );
    CHECK( text( client.hostHeader() ) == "127.0.0.1:" + std::to_string( nPort ) );

        // A request, then another on the same connection.
    CHttpResponse *response = client.get( "/hello" );
    CHECK( response && response->getStatusCode() == 200 && bodyOf( response ) == "hello" );
    CHECK( response && response->getHeaderValue( "Content-Length" ) == "5" );
    delete response;
    CHECK( client.IsConnected() );
    response = client.get( "/missing" );
    CHECK( response && response->getStatusCode() == 404 );
    delete response;
    CHECK( client.getConnectionsOpened() == 1 );

        // The Host header the server saw.
    response = client.get( "/host" );
    CHECK( response && bodyOf( response ) == "127.0.0.1:" + std::to_string( nPort ) );
    delete response;

        // A chunked response, decoded.
    response = client.get( "/chunked" );
    CHECK( response && response->getStatusCode() == 200 && bodyOf( response ) == "hello, world" );
    CHECK( response && ! response->hasHeader( "Transfer-Encoding" ) && response->getHeaderValue( "Content-Length" ) == "12" );
    delete response;

        // HEAD: headers only, and the connection is still good afterwards.
    response = client.head( "/hello" );
    CHECK( response && response->getStatusCode() == 200 && response->getHeaderValue( "Content-Length" ) == "5" && bodyOf( response ).empty() );
    delete response;

        // Binary bodies both ways.
    response = client.get( "/binary" );
    CHECK( response && bodyOf( response ) == std::string( "a\0b\0c", 5 ) );
    delete response;
    response = client.post( "/echo", "application/octet-stream", CString::FromBytes( "x\0y\r\n.z", 7 ) );
    CHECK( response && response->getStatusCode() == 200 && bodyOf( response ) == std::string( "x\0y\r\n.z", 7 ) );
    delete response;
    response = client.post( "/echo", "text/plain", CString( "" ) );
    CHECK( response && response->getStatusCode() == 200 && bodyOf( response ).empty() );
    delete response;
    CHECK( client.getConnectionsOpened() == 1 );

        // The server asks to close: the client does, and reconnects next time.
    response = client.get( "/close" );
    CHECK( response && bodyOf( response ) == "bye" );
    delete response;
    CHECK( ! client.IsConnected() );
    response = client.get( "/hello" );
    CHECK( response && response->getStatusCode() == 200 );
    delete response;
    CHECK( client.getConnectionsOpened() == 2 );

        // The server closes an idle connection; the next request still works.
    std::this_thread::sleep_for( std::chrono::milliseconds( 1200 ) );
    response = client.get( "/hello" );
    CHECK( response && response->getStatusCode() == 200 && bodyOf( response ) == "hello" );
    delete response;
    CHECK( client.getConnectionsOpened() == 3 );

        // A request that would inject headers is not sent.
    CHttpRequest injected( "GET", "/hello\r\nX-Injected: 1" );
    response = client.executeRequest( &injected );
    CHECK( response == NULL && client.getLastError().GetLength() > 0 );
    CHECK( client.getConnectionsOpened() == 3 );
    CHECK( client.post( "/echo", "text/plain\r\nX: y", CString( "a" ) ) == NULL );

        // Without keep-alive every request has its own connection.
    CHttpClient once( "localhost", nPort );
    once.setKeepAlive( false );
    response = once.get( "/hello" );
    CHECK( response && response->getStatusCode() == 200 );
    delete response;
    CHECK( ! once.IsConnected() );
    response = once.get( "/hello" );
    CHECK( response && response->getStatusCode() == 200 );
    delete response;
    CHECK( once.getConnectionsOpened() == 2 );

        // HTTP/1.0 requests.
    CHttpClient old( "127.0.0.1", nPort );
    old.setUseHttp11( false );
    response = old.get( "/hello" );
    CHECK( response && response->getStatusCode() == 200 && bodyOf( response ) == "hello" );
    delete response;

    server.Stop();
    server.Wait();
}

/***********************************************************************
 * CHttpClient against scripted servers
 ***********************************************************************/

static void testScripted( void )
{
    printf( "CHttpClient and scripted servers\n" );

        // A body delimited by the server closing the connection, after an
        // interim 100 response.
    {
        std::vector<std::string> replies;
        replies.push_back( "HTTP/1.1 100 Continue\r\n\r\nHTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nall of this" );
        CScriptedServer scripted( replies, true );
        CHttpClient client( "127.0.0.1", scripted.port() );
        client.setTimeout( 3000 );
        CHttpResponse *response = client.get( "/x" );
        CHECK( response && response->getStatusCode() == 200 && bodyOf( response ) == "all of this" );
        delete response;
        CHECK( ! client.IsConnected() );
        std::string request = scripted.lastRequest();
        CHECK( request.compare( 0, 17, "GET /x HTTP/1.1\r\n" ) == 0 );
        CHECK( request.find( "\r\nHost: 127.0.0.1:" + std::to_string( scripted.port() ) + "\r\n" ) != std::string::npos );
    }

        // A server that never answers: the client gives up after its timeout.
    {
        std::vector<std::string> replies;
        replies.push_back( "" );
        CScriptedServer silent( replies, false );
        CHttpClient client( "127.0.0.1", silent.port() );
        client.setTimeout( 400 );
        auto start = std::chrono::steady_clock::now();
        CHttpResponse *response = client.get( "/x" );
        long nElapsed = elapsedMs( start );
        CHECK( response == NULL );
        CHECK( client.getLastError().IndexOf( "Timed out" ) != NOT_FOUND );
        CHECK( nElapsed >= 300 && nElapsed < 3000 );
        CHECK( ! client.IsConnected() );
    }

        // A response cut short, and a malformed one.
    {
        std::vector<std::string> replies;
        replies.push_back( "HTTP/1.1 200 OK\r\nContent-Length: 100\r\n\r\nshort" );
        CScriptedServer cut( replies, true );
        CHttpClient client( "127.0.0.1", cut.port() );
        client.setTimeout( 3000 );
        CHECK( client.get( "/x" ) == NULL );
        CHECK( client.getLastError().IndexOf( "before the response was complete" ) != NOT_FOUND );
    }
    {
        std::vector<std::string> replies;
        replies.push_back( "NOT HTTP AT ALL\r\n\r\n" );
        CScriptedServer bad( replies, true );
        CHttpClient client( "127.0.0.1", bad.port() );
        client.setTimeout( 3000 );
        CHECK( client.post( "/x", "text/plain", CString( "body" ) ) == NULL );
        CHECK( client.getLastError().IndexOf( "malformed" ) != NOT_FOUND );
    }

        // Keep-alive with a server that says so, and bytes after a response
        // (which mean the connection cannot be trusted any more).
    {
        std::vector<std::string> replies;
        replies.push_back( "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\na" );
        replies.push_back( "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\nbEXTRA" );
        replies.push_back( "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\nc" );
        CScriptedServer keep( replies, false );
        CHttpClient client( "127.0.0.1", keep.port() );
        client.setTimeout( 3000 );
        CHttpResponse *response = client.get( "/1" );
        CHECK( response && bodyOf( response ) == "a" && client.IsConnected() );
        delete response;
        response = client.get( "/2" );
        CHECK( response && bodyOf( response ) == "b" );
        delete response;
        CHECK( ! client.IsConnected() );
        CHECK( client.getConnectionsOpened() == 1 );
        response = client.get( "/3" );
        CHECK( response && bodyOf( response ) == "c" );
        delete response;
        CHECK( client.getConnectionsOpened() == 2 );
    }

        // Nothing listening.
    {
        int nFreePort = 0;
        {
            CServerSocket probe( 0, 1 );
            nFreePort = probe.GetPort();
        }
        CHttpClient client( "127.0.0.1", nFreePort );
        client.setTimeout( 2000 );
        CHECK( client.get( "/" ) == NULL && client.getLastError().IndexOf( "Could not connect" ) != NOT_FOUND );
    }

#ifndef IASLIB_TLS__
    {
        CHttpClient secure( "127.0.0.1", 443, true );
        CHECK( secure.get( "/" ) == NULL && secure.getLastError().IndexOf( "OpenSSL" ) != NOT_FOUND );
    }
#endif
}

int main( void )
{
    setvbuf( stdout, NULL, _IONBF, 0 );
    CQuietLogSink *log = new CQuietLogSink();

    testResponseFraming();
    testInjection();
    testAgainstServer();
    testScripted();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    delete log;
    return ( g_nFailures == 0 ) ? 0 : 1;
}
