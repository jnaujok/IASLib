/**
 * SIP server tests
 *
 * Tests the SIP protocol core (CSipServer) through a recording transport
 * and a controllable clock, and the CBasicSipServer over real UDP and TCP
 * sockets on the loopback interface.
 */

#include "IASLib.h"
#include "NetworkServices/HeaderList.h"
#include "NetworkServices/Entities/RawEntity.h"
#include "NetworkServices/Entities/SdpEntity.h"
#include "NetworkServices/SIP/SipRequest.h"
#include "NetworkServices/SIP/SipServer.h"
#include "NetworkServices/SIP/SipUtils.h"
#include "NetworkServices/SIP/BasicSipServer.h"
#include "NetworkServices/SIP/Handlers/BaseSipHandlerFactory.h"
#include "Logging/LogSink.h"
#include "Streams/StringStream.h"

#include "../TestSockets.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
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

/***********************************************************************
 * Helpers
 ***********************************************************************/

    // Captures log output so that every logging path is exercised.
class CTestLogSink : public CLogSink
{
    public:
        std::atomic<int> lines;
        CTestLogSink( void ) : CLogSink( CLogSink::DEBUG ), lines( 0 ) {}
        virtual bool writeLine( const char *message ) { lines++; return true; }
};

    // Records everything the server sends.
class CTestTransport : public CSipTransport
{
    public:
        bool                        reliable;
        std::mutex                  mutex;
        std::vector<std::string>    sent;

        CTestTransport( bool bReliable = false ) : reliable( bReliable ) {}

        virtual bool send( const CString &data, const CInternetAddress &target )
        {
            std::lock_guard<std::mutex> lock( mutex );
            sent.push_back( std::string( (const char *)data, data.GetLength() ) );
            return true;
        }
        virtual bool isReliable( void ) { return reliable; }
        virtual const char *getProtocol( void ) { return reliable ? "TCP" : "UDP"; }

        size_t count( void ) { std::lock_guard<std::mutex> lock( mutex ); return sent.size(); }
        std::string at( size_t n ) { std::lock_guard<std::mutex> lock( mutex ); return ( n < sent.size() ) ? sent[ n ] : std::string(); }
        std::string last( void ) { std::lock_guard<std::mutex> lock( mutex ); return sent.empty() ? std::string() : sent.back(); }
        void clear( void ) { std::lock_guard<std::mutex> lock( mutex ); sent.clear(); }
};

    // A server whose clock the test controls.
class CTestSipServer : public CSipServer
{
    public:
        std::atomic<unsigned long long> now;

        CTestSipServer( void ) : CSipServer( 5060 ), now( 1000000 )
        {
            AddHandlerFactory( new CBaseSipHandlerFactory() );
        }
        virtual ~CTestSipServer( void ) {}

        void advance( unsigned long long ms ) { now += ms; processTimers( now ); }

    protected:
        virtual unsigned long long currentTimeMs( void ) { return now; }
};

static int statusOf( const std::string &response )
{
    if ( response.compare( 0, 8, "SIP/2.0 " ) != 0 )
        return -1;
    return atoi( response.c_str() + 8 );
}

    // Returns the value of the first header with this name ("" if none).
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

static int headerCount( const std::string &message, const std::string &name )
{
    int count = 0;
    std::string needle = "\r\n" + name + ":";
    size_t pos = message.find( needle );
    while ( pos != std::string::npos )
    {
        count++;
        pos = message.find( needle, pos + 1 );
    }
    return count;
}

static std::string bodyOf( const std::string &message )
{
    size_t pos = message.find( "\r\n\r\n" );
    return ( pos == std::string::npos ) ? std::string() : message.substr( pos + 4 );
}

static std::string toTagOf( const std::string &message )
{
    std::string to = headerOf( message, "To" );
    size_t pos = to.find( ";tag=" );
    return ( pos == std::string::npos ) ? std::string() : to.substr( pos + 5 );
}

struct RequestOptions
{
    std::string method = "OPTIONS";
    std::string uri = "sip:15551234567@127.0.0.1";
    std::string version = "SIP/2.0";
    std::string branch = "z9hG4bK776asdhds";
    std::string viaHost = "127.0.0.1:5099";
    std::string viaParams = "";
    std::string toTag = "";
    std::string callId = "a84b4c76e66710@pc33";
    unsigned long cseq = 314159;
    std::string cseqMethod = "";
    std::string extra = "";
    std::string body = "";
    std::string contentType = "";
    bool includeCallId = true;
};

static std::string buildRequest( const RequestOptions &o )
{
    std::string m = o.method + " " + o.uri + " " + o.version + "\r\n";
    m += "Via: SIP/2.0/UDP " + o.viaHost + ";branch=" + o.branch + o.viaParams + "\r\n";
    m += "Max-Forwards: 70\r\n";
    m += "From: \"Alice\" <sip:+15550001111@example.com>;tag=1928301774\r\n";
    m += "To: <sip:15551234567@127.0.0.1>" + ( o.toTag.empty() ? std::string() : ";tag=" + o.toTag ) + "\r\n";
    if ( o.includeCallId )
        m += "Call-ID: " + o.callId + "\r\n";
    m += "CSeq: " + std::to_string( o.cseq ) + " " + ( o.cseqMethod.empty() ? o.method : o.cseqMethod ) + "\r\n";
    m += o.extra;
    if ( ! o.contentType.empty() )
        m += "Content-Type: " + o.contentType + "\r\n";
    m += "Content-Length: " + std::to_string( o.body.size() ) + "\r\n\r\n";
    m += o.body;
    return m;
}

static const std::string SDP =
    "v=0\r\n"
    "o=alice 2890844526 2890844526 IN IP4 127.0.0.1\r\n"
    "s=-\r\n"
    "c=IN IP4 127.0.0.1\r\n"
    "t=0 0\r\n"
    "m=audio 49170 RTP/AVP 0 101\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-15;mode=20\r\n";

static CInternetAddress loopback( int port )
{
    return CInternetAddress( "127.0.0.1", port, SOCK_DGRAM );
}

static void deliver( CSipServer &server, CTestTransport &transport, const std::string &message, int sourcePort = 5099 )
{
    server.handleMessage( CString( message.data(), message.size() ), &transport, loopback( sourcePort ) );
}

/***********************************************************************
 * Parsing
 ***********************************************************************/

static void testParsing( void )
{
    printf( "Parsing\n" );

        // Compact forms, folded lines, whitespace before the colon, LF-only
        // line endings and a Content-Type with parameters.
    std::string message =
        "INVITE sip:bob@biloxi.com SIP/2.0\r\n"
        "v: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bKnashds8\r\n"
        "f: Alice <sip:alice@atlanta.com>;tag=1928301774\r\n"
        "t: Bob <sip:bob@biloxi.com>\r\n"
        "i: a84b4c76e66710\r\n"
        "CSeq : 314159 INVITE\r\n"
        "Subject: first line\r\n"
        "  continued\n"
        "c: application/sdp; charset=utf-8\r\n"
        "l: " + std::to_string( SDP.size() ) + "\r\n"
        "\r\n" + SDP;

    CSipRequest request( loopback( 5060 ) );
    CHECK( request.parse( CString( message.data(), message.size() ) ) );
    CHECK( request.getRequestType() == "INVITE" );
    CHECK( request.getUri() == "sip:bob@biloxi.com" );
    CHECK( request.getHeaderValue( "Via" ) == "SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bKnashds8" );
    CHECK( request.getHeaderValue( "Call-ID" ) == "a84b4c76e66710" );
    CHECK( request.getHeaderValue( "call-id" ) == "a84b4c76e66710" );
    CHECK( request.getHeaderValue( "CSeq" ) == "314159 INVITE" );
    CHECK( request.getHeaderValue( "Subject" ) == "first line continued" );
    CHECK( request.getFromTag() == "1928301774" );
    CHECK( request.getEntity() != NULL );
    if ( request.getEntity() )
    {
        CHECK( strcmp( request.getEntity()->getMimeType(), "application/sdp" ) == 0 );
            // An '=' inside a value must not break the SDP line.
        CHECK( request.getEntity()->toString() == CString( SDP.c_str() ) );
        CSDPEntity *sdp = (CSDPEntity *)request.getEntity();
        CHECK( sdp->getAttribute( "m" ) == "audio 49170 RTP/AVP 0 101" );
    }

        // A body of an unregistered type is kept as it is.
    std::string multipart = "--b\r\nContent-Type: application/sdp\r\n\r\n" + SDP + "--b--\r\n";
    RequestOptions o;
    o.method = "INVITE";
    o.body = multipart;
    o.contentType = "multipart/mixed;boundary=b";
    std::string mp = buildRequest( o );
    CSipRequest mpRequest( loopback( 5060 ) );
    CHECK( mpRequest.parse( CString( mp.data(), mp.size() ) ) );
    CHECK( mpRequest.getEntity() != NULL );
    if ( mpRequest.getEntity() )
    {
        CHECK( mpRequest.getEntity()->toString() == CString( multipart.c_str() ) );
        CHECK( strcmp( mpRequest.getEntity()->getMimeType(), "multipart/mixed;boundary=b" ) == 0 );
    }
    CHECK( mpRequest.toString().IndexOf( "--b--" ) != NOT_FOUND );

        // Content-Length larger than the body is an error.
    std::string shortBody = buildRequest( RequestOptions() );
    shortBody.replace( shortBody.find( "Content-Length: 0" ), 17, "Content-Length: 50" );
    CSipRequest shortRequest( loopback( 5060 ) );
    CHECK( ! shortRequest.parse( CString( shortBody.data(), shortBody.size() ) ) );

        // Keep-alives and garbage parse as invalid without crashing.
    CSipRequest keepAlive( loopback( 5060 ) );
    CHECK( ! keepAlive.parse( CString( "\r\n\r\n" ) ) );
    CHECK( keepAlive.getHeaderValue( "Via" ) == "" );
    CSipRequest garbage( loopback( 5060 ) );
    CHECK( ! garbage.parse( CString( "garbage" ) ) );

        // Header list behaviour.
    CHeaderList headers;
    headers.appendHeader( "Via", "a" );
    headers.appendHeader( "via", "b" );
    headers.addHeader( "From", "x" );
    CHECK( headers.allValues( "VIA" ).Count() == 2 );
    CHECK( headers.toString() == "Via: a\r\nVia: b\r\nFrom: x\r\n" );
    CHeaderList copy( headers );
    headers.removeHeader( "via" );
    CHECK( copy.allValues( "Via" ).Count() == 2 );
    CHECK( ! headers.hasHeader( "Via" ) );
    CHECK( headers.firstValue( "Missing" ) == "" );

        // Utilities.
    CHECK( CSipUtils::getParameter( "SIP/2.0/UDP h;branch=z9hG4bK1;rport", "branch" ) == "z9hG4bK1" );
    bool bFound = false;
    CSipUtils::getParameter( "SIP/2.0/UDP h;branch=z9hG4bK1;rport", "rport", &bFound );
    CHECK( bFound );
    CHECK( CSipUtils::getTag( "\"A;B\" <sip:a@b;tag=wrong>;tag=right" ) == "right" );
    CHECK( CSipUtils::splitList( "SIP/2.0/UDP a, SIP/2.0/UDP b" ).Count() == 2 );
    CHECK( CSipUtils::splitList( "\"Smith, J\" <sip:j@x>, <sip:k@y,z>" ).Count() == 2 );
    CString transport, host;
    int port = 0;
    CHECK( CSipUtils::parseVia( "SIP/2.0/TCP [2001:db8::1]:5070;branch=x", transport, host, port ) );
    CHECK( transport == "TCP" && host == "2001:db8::1" && port == 5070 );
    unsigned long cseq = 0;
    CString cseqMethod;
    CHECK( CSipUtils::parseCSeq( "  42   INVITE ", cseq, cseqMethod ) && cseq == 42 && cseqMethod == "INVITE" );
    CHECK( ! CSipUtils::parseCSeq( "99999999999 INVITE", cseq, cseqMethod ) );
    CHECK( CSipUtils::randomToken( 16 ).GetLength() == 32 );
    CHECK( CSipUtils::randomToken( 16 ) != CSipUtils::randomToken( 16 ) );

    size_t leading = 0;
    std::string twoMessages = "\r\n\r\n" + buildRequest( RequestOptions() ) + "OPTIONS sip:x SIP/2.0\r\nl: 3\r\n\r\nab";
    long first = CSipUtils::frameMessage( twoMessages.data(), twoMessages.size(), leading, 65535 );
    CHECK( leading == 4 );
    CHECK( first == (long)( 4 + buildRequest( RequestOptions() ).size() ) );
    std::string rest = twoMessages.substr( (size_t)first );
    CHECK( CSipUtils::frameMessage( rest.data(), rest.size(), leading, 65535 ) == 0 );     // body incomplete
    rest += "c";
    CHECK( CSipUtils::frameMessage( rest.data(), rest.size(), leading, 65535 ) == (long)rest.size() );
}

/***********************************************************************
 * UAS core behaviour
 ***********************************************************************/

static void testOptions( void )
{
    printf( "OPTIONS\n" );
    CTestSipServer server;
    CTestTransport transport;

    deliver( server, transport, buildRequest( RequestOptions() ) );
    CHECK( transport.count() == 1 );
    std::string r = transport.last();
    CHECK( statusOf( r ) == 200 );
    CHECK( headerOf( r, "Via" ) == "SIP/2.0/UDP 127.0.0.1:5099;branch=z9hG4bK776asdhds" );
    CHECK( headerOf( r, "From" ) == "\"Alice\" <sip:+15550001111@example.com>;tag=1928301774" );
    CHECK( headerOf( r, "Call-ID" ) == "a84b4c76e66710@pc33" );
    CHECK( headerOf( r, "CSeq" ) == "314159 OPTIONS" );
    CHECK( toTagOf( r ).size() == 16 );
    CHECK( headerOf( r, "Allow" ) == "INVITE, ACK, BYE, CANCEL, OPTIONS, REGISTER" );
    CHECK( headerOf( r, "Content-Length" ) == "0" );
    CHECK( bodyOf( r ).empty() );
    CHECK( headerOf( r, "Supported" ).empty() );

    server.addSupportedExtension( "timer" );
    RequestOptions o;
    o.branch = "z9hG4bKsecond";
    o.cseq = 314160;
    deliver( server, transport, buildRequest( o ) );
    CHECK( headerOf( transport.last(), "Supported" ) == "timer" );
}

static void testInviteNon2xxTransaction( void )
{
    printf( "INVITE with a non-2xx response over UDP\n" );
    CTestSipServer server;
    CTestTransport transport;

    RequestOptions invite;
    invite.method = "INVITE";
    invite.body = SDP;
    invite.contentType = "application/sdp";
    std::string inviteText = buildRequest( invite );

    deliver( server, transport, inviteText );
    CHECK( transport.count() == 2 );
    CHECK( statusOf( transport.at( 0 ) ) == 100 );
    CHECK( toTagOf( transport.at( 0 ) ).empty() );                   // 100 carries no tag
    CHECK( statusOf( transport.at( 1 ) ) == 404 );
    std::string tag = toTagOf( transport.at( 1 ) );
    CHECK( ! tag.empty() );
    CHECK( server.getTransactionCount() == 1 );

        // A retransmitted INVITE gets the same final response.
    deliver( server, transport, inviteText );
    CHECK( transport.count() == 3 );
    CHECK( transport.last() == transport.at( 1 ) );

        // Timer G: retransmissions at T1, 2*T1, 4*T1 ...
    server.advance( 499 );
    CHECK( transport.count() == 3 );
    server.advance( 1 );
    CHECK( transport.count() == 4 );
    server.advance( 1000 );
    CHECK( transport.count() == 5 );
    CHECK( statusOf( transport.last() ) == 404 );

        // The ACK (same branch) is absorbed and never answered.
    RequestOptions ack;
    ack.method = "ACK";
    ack.toTag = tag;
    deliver( server, transport, buildRequest( ack ) );
    CHECK( transport.count() == 5 );
    server.advance( 10000 );
    CHECK( transport.count() == 5 );                                 // no more retransmissions
    CHECK( server.getTransactionCount() == 0 );                      // Timer I has run

        // Without an ACK, Timer H ends the transaction after 64*T1.
    RequestOptions invite2 = invite;
    invite2.branch = "z9hG4bKnoack";
    invite2.cseq = 2;
    deliver( server, transport, buildRequest( invite2 ) );
    CHECK( server.getTransactionCount() == 1 );
    server.advance( 31000 );
    CHECK( server.getTransactionCount() == 1 );
    server.advance( 1100 );
    CHECK( server.getTransactionCount() == 0 );
}

static void testInviteOverReliableTransport( void )
{
    printf( "INVITE over a reliable transport\n" );
    CTestSipServer server;
    CTestTransport transport( true );

    RequestOptions invite;
    invite.method = "INVITE";
    deliver( server, transport, buildRequest( invite ) );
    CHECK( transport.count() == 2 );
    server.advance( 10000 );
    CHECK( transport.count() == 2 );                                 // no Timer G on TCP

        // A double-CRLF keep-alive gets a single CRLF back on a connection...
    deliver( server, transport, "\r\n\r\n" );
    CHECK( transport.count() == 3 );
    CHECK( transport.last() == "\r\n" );

        // ...but nothing on UDP.
    CTestTransport udp;
    deliver( server, udp, "\r\n\r\n" );
    CHECK( udp.count() == 0 );
}

    // An INVITE handler that answers 200 OK.
class CAnswerInviteHandler : public CSipHandler
{
    IASLIB_SET_SIP_HANDLER( CAnswerInviteHandler, "INVITE" );
    public:
        DEFINE_OBJECT( CAnswerInviteHandler );
        CAnswerInviteHandler( CString uri, CString version ) : CSipHandler( uri, version ) {}
        virtual bool process( CSipRequest *request, CSipResponse *response )
        {
            response->setStatus( 200 );
            response->addHeader( "Contact", "<sip:uas@127.0.0.1:5060>" );
            return true;
        }
};
IMPLEMENT_OBJECT( CAnswerInviteHandler, CSipHandler );

static void testInvite2xxRetransmission( void )
{
    printf( "INVITE with a 2xx response\n" );
    CTestSipServer server;
    CSipHandlerFactory *factory = new CSipHandlerFactory();
    factory->registerHandler( new CAnswerInviteHandler( "", "" ) );
    server.AddHandlerFactory( factory );
    CTestTransport transport;

    RequestOptions invite;
    invite.method = "INVITE";
    invite.extra = "Record-Route: <sip:p1.example.com;lr>\r\nRecord-Route: <sip:p2.example.com;lr>\r\n";
    std::string inviteText = buildRequest( invite );
    deliver( server, transport, inviteText );
    CHECK( transport.count() == 2 );
    std::string ok = transport.at( 1 );
    CHECK( statusOf( ok ) == 200 );
    CHECK( headerCount( ok, "Record-Route" ) == 2 );
    std::string tag = toTagOf( ok );

        // A retransmitted INVITE is absorbed by the RFC 6026 Accepted state,
        // not handled again, and gets the same 2xx.
    deliver( server, transport, inviteText );
    CHECK( transport.count() == 3 );
    CHECK( transport.last() == ok );

        // The 2xx is retransmitted until the ACK arrives.
    server.advance( 500 );
    CHECK( transport.count() == 4 );
    server.advance( 1000 );
    CHECK( transport.count() == 5 );

        // The ACK for a 2xx is a new transaction (new branch).
    RequestOptions ack;
    ack.method = "ACK";
    ack.branch = "z9hG4bKackfor2xx";
    ack.toTag = tag;
    deliver( server, transport, buildRequest( ack ) );
    CHECK( transport.count() == 5 );
    server.advance( 4000 );
    CHECK( transport.count() == 5 );
}

    // An INVITE handler that waits until the test lets it finish.
class CSlowInviteHandler : public CSipHandler
{
    IASLIB_SET_SIP_HANDLER( CSlowInviteHandler, "INVITE" );
    public:
        static std::mutex mutex;
        static std::condition_variable condition;
        static bool entered;
        static bool release;

        DEFINE_OBJECT( CSlowInviteHandler );
        CSlowInviteHandler( CString uri, CString version ) : CSipHandler( uri, version ) {}
        virtual bool process( CSipRequest *request, CSipResponse *response )
        {
            std::unique_lock<std::mutex> lock( mutex );
            entered = true;
            condition.notify_all();
            condition.wait( lock, [] { return release; } );
            response->setStatus( 486 );
            return true;
        }
};
IMPLEMENT_OBJECT( CSlowInviteHandler, CSipHandler );
std::mutex CSlowInviteHandler::mutex;
std::condition_variable CSlowInviteHandler::condition;
bool CSlowInviteHandler::entered = false;
bool CSlowInviteHandler::release = false;

static void testCancel( void )
{
    printf( "CANCEL\n" );
    CTestSipServer server;
    CTestTransport transport;

        // A CANCEL that matches nothing gets 481.
    RequestOptions cancel;
    cancel.method = "CANCEL";
    cancel.branch = "z9hG4bKnothing";
    deliver( server, transport, buildRequest( cancel ) );
    CHECK( transport.count() == 1 );
    CHECK( statusOf( transport.last() ) == 481 );

        // A CANCEL for an INVITE that is still being handled.
    CSipHandlerFactory *factory = new CSipHandlerFactory();
    factory->registerHandler( new CSlowInviteHandler( "", "" ) );
    server.AddHandlerFactory( factory );
    transport.clear();

    RequestOptions invite;
    invite.method = "INVITE";
    invite.branch = "z9hG4bKpending";
    std::string inviteText = buildRequest( invite );
    std::thread inviteThread( [&] { deliver( server, transport, inviteText ); } );
    {
        std::unique_lock<std::mutex> lock( CSlowInviteHandler::mutex );
        CSlowInviteHandler::condition.wait( lock, [] { return CSlowInviteHandler::entered; } );
    }
    CHECK( transport.count() == 1 );                                 // 100 Trying
    CHECK( statusOf( transport.at( 0 ) ) == 100 );

    cancel.branch = "z9hG4bKpending";                                 // same branch as the INVITE
    cancel.cseq = invite.cseq;
    deliver( server, transport, buildRequest( cancel ) );
    CHECK( transport.count() == 3 );
    CHECK( statusOf( transport.at( 1 ) ) == 200 );
    CHECK( headerOf( transport.at( 1 ), "CSeq" ) == "314159 CANCEL" );
    CHECK( statusOf( transport.at( 2 ) ) == 487 );
    CHECK( headerOf( transport.at( 2 ), "CSeq" ) == "314159 INVITE" );
    CHECK( toTagOf( transport.at( 1 ) ) == toTagOf( transport.at( 2 ) ) );

    {
        std::lock_guard<std::mutex> lock( CSlowInviteHandler::mutex );
        CSlowInviteHandler::release = true;
    }
    CSlowInviteHandler::condition.notify_all();
    inviteThread.join();
    CHECK( transport.count() == 3 );                                 // the handler's 486 is discarded
}

static void testMethodsAndValidation( void )
{
    printf( "Methods and request validation\n" );
    CTestSipServer server;
    CTestTransport transport;
    int nBranch = 0;

    auto send = [&]( RequestOptions o ) -> std::string
    {
        o.branch = "z9hG4bKv" + std::to_string( ++nBranch );
            // A new CSeq each time, or the server rightly sees merged requests.
        o.cseq = 1000 + nBranch;
        size_t before = transport.count();
        deliver( server, transport, buildRequest( o ) );
        return ( transport.count() > before ) ? transport.last() : std::string();
    };

    RequestOptions o;

    o.method = "BYE";
    CHECK( statusOf( send( o ) ) == 481 );

    o.method = "FOO";
    std::string r = send( o );
    CHECK( statusOf( r ) == 501 );
    CHECK( headerOf( r, "Allow" ) == "INVITE, ACK, BYE, CANCEL, OPTIONS, REGISTER" );

    o.method = "SUBSCRIBE";
    r = send( o );
    CHECK( statusOf( r ) == 405 );
    CHECK( ! headerOf( r, "Allow" ).empty() );

    o = RequestOptions();
    o.version = "SIP/3.0";
    CHECK( statusOf( send( o ) ) == 505 );

    o = RequestOptions();
    o.includeCallId = false;
    r = send( o );
    CHECK( statusOf( r ) == 400 );
    CHECK( r.find( "Missing Call-ID" ) != std::string::npos );

    o = RequestOptions();
    o.cseqMethod = "INVITE";
    CHECK( statusOf( send( o ) ) == 400 );

    o = RequestOptions();
    o.extra = "Require: foo, 100rel\r\n";
    r = send( o );
    CHECK( statusOf( r ) == 420 );
    CHECK( headerOf( r, "Unsupported" ) == "foo, 100rel" );

    server.addSupportedExtension( "foo" );
    server.addSupportedExtension( "100rel" );
    CHECK( statusOf( send( o ) ) == 200 );

    o = RequestOptions();
    o.uri = "http://example.com/";
    CHECK( statusOf( send( o ) ) == 416 );

    o = RequestOptions();
    o.uri = "tel:+15551234567";
    CHECK( statusOf( send( o ) ) == 200 );

        // A stray ACK is never answered.
    o = RequestOptions();
    o.method = "ACK";
    CHECK( send( o ).empty() );
    o.version = "SIP/3.0";
    CHECK( send( o ).empty() );

        // Garbage and SIP responses are ignored.
    size_t before = transport.count();
    deliver( server, transport, "Not a SIP message at all" );
    deliver( server, transport, "SIP/2.0 200 OK\r\nContent-Length: 0\r\n\r\n" );
    CHECK( transport.count() == before );

        // A request whose body is shorter than its Content-Length gets 400.
    o = RequestOptions();
    o.branch = "z9hG4bKshort";
    std::string shortBody = buildRequest( o );
    shortBody.replace( shortBody.find( "Content-Length: 0" ), 17, "Content-Length: 50" );
    deliver( server, transport, shortBody );
    CHECK( statusOf( transport.last() ) == 400 );
}

static void testRegister( void )
{
    printf( "REGISTER\n" );
    CTestSipServer server;
    server.setRealm( "example.com" );
    CTestTransport transport;

    RequestOptions o;
    o.method = "REGISTER";
    o.uri = "sip:127.0.0.1";
    o.branch = "z9hG4bKr1";
    deliver( server, transport, buildRequest( o ) );
    std::string first = transport.last();
    CHECK( statusOf( first ) == 401 );
    std::string challenge = headerOf( first, "WWW-Authenticate" );
    CHECK( challenge.find( "Digest realm=\"example.com\"" ) == 0 );
    CHECK( challenge.find( "qop=\"auth\"" ) != std::string::npos );

    o.branch = "z9hG4bKr2";
    o.cseq++;
    deliver( server, transport, buildRequest( o ) );
    CHECK( headerOf( transport.last(), "WWW-Authenticate" ) != challenge );     // a fresh nonce

    o.branch = "z9hG4bKr3";
    o.cseq++;
    o.extra = "Authorization: Digest username=\"alice\", realm=\"example.com\", nonce=\"x\", uri=\"sip:127.0.0.1\", response=\"y\"\r\n";
    deliver( server, transport, buildRequest( o ) );
    CHECK( statusOf( transport.last() ) == 403 );
}

static void testViaHandling( void )
{
    printf( "Via received/rport and retransmissions\n" );
    CTestSipServer server;
    CTestTransport transport;

    RequestOptions o;
    o.viaHost = "client.example.com:5070";
    o.viaParams = ";rport";
    deliver( server, transport, buildRequest( o ), 40000 );
    std::string r = transport.last();
    CHECK( headerOf( r, "Via" ) == "SIP/2.0/UDP client.example.com:5070;branch=z9hG4bK776asdhds;rport=40000;received=127.0.0.1" );

        // A retransmitted OPTIONS gets the same response; the handler is not
        // run again (the To tag would differ if it were).
    deliver( server, transport, buildRequest( o ), 40000 );
    CHECK( transport.count() == 2 );
    CHECK( transport.last() == r );

        // Timer J removes the transaction after 64*T1 on UDP.
    CHECK( server.getTransactionCount() == 1 );
    server.advance( 32001 );
    CHECK( server.getTransactionCount() == 0 );

        // Merged request: same From tag, Call-ID and CSeq, different branch.
    RequestOptions a;
    a.branch = "z9hG4bKpath1";
    a.cseq = 77;
    RequestOptions b = a;
    b.branch = "z9hG4bKpath2";
    deliver( server, transport, buildRequest( a ) );
    CHECK( statusOf( transport.last() ) == 200 );
    deliver( server, transport, buildRequest( b ) );
    CHECK( statusOf( transport.last() ) == 482 );

        // Several Via headers are all returned, in order.
    RequestOptions multi;
    multi.branch = "z9hG4bKmulti";
    multi.extra = "Via: SIP/2.0/UDP proxy1.example.com;branch=z9hG4bKp1, SIP/2.0/UDP proxy2.example.com;branch=z9hG4bKp2\r\n";
    std::string multiText = buildRequest( multi );
    deliver( server, transport, multiText );
    r = transport.last();
    CHECK( headerCount( r, "Via" ) == 2 );
    CHECK( r.find( "proxy1.example.com;branch=z9hG4bKp1, SIP/2.0/UDP proxy2.example.com" ) != std::string::npos );
}

static void testHostileInput( void )
{
    printf( "Hostile input\n" );
    CTestSipServer server;
    CTestTransport transport;

        // Format directives in headers must not reach a printf format string.
    RequestOptions o;
    o.method = "INVITE";
    o.uri = "sip:%n%n%n%s%s%s@127.0.0.1";
    o.extra = "Subject: %n%n%n%s%s%s%s%s\r\n";
    deliver( server, transport, buildRequest( o ) );
    CHECK( statusOf( transport.last() ) == 404 );

        // Very long values (bigger than the logging buffers).
    RequestOptions big;
    big.branch = "z9hG4bKbig";
    big.uri = "sip:" + std::string( 20000, 'a' ) + "@127.0.0.1";
    big.extra = "Subject: " + std::string( 30000, 'x' ) + "\r\n";
    deliver( server, transport, buildRequest( big ) );
    CHECK( statusOf( transport.last() ) == 200 );

        // CString stops at a NUL byte, so a body containing one arrives
        // shorter than its Content-Length. The server must answer 400, not
        // crash or read past the message.
    RequestOptions binary;
    binary.branch = "z9hG4bKbin";
    binary.cseq = 99;
    binary.body = std::string( "ab\0cd", 5 );
    binary.contentType = "application/octet-stream";
    std::string text = buildRequest( binary );
    deliver( server, transport, text );
    CHECK( statusOf( transport.last() ) == 400 );
}

/***********************************************************************
 * CBasicSipServer over real sockets
 ***********************************************************************/

static std::string udpExchange( int port, const std::string &message, int timeoutMs, int expected )
{
    int fd = socket( AF_INET, SOCK_DGRAM, 0 );
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons( (uint16_t)port );
    addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    sendto( fd, message.data(), message.size(), 0, (struct sockaddr *)&addr, sizeof( addr ) );

    std::string all;
    int received = 0;
    while ( received < expected )
    {
        struct pollfd pfd = { fd, POLLIN, 0 };
        if ( poll( &pfd, 1, timeoutMs ) <= 0 )
            break;
        char buffer[ 65536 ];
        ssize_t n = recv( fd, buffer, sizeof( buffer ), 0 );
        if ( n <= 0 )
            break;
        all.append( buffer, (size_t)n );
        all += "\n#####\n";
        received++;
    }
    closeSocket( fd );
    return all;
}

    // A SIP exchange with the server over IPv6, where the machine has it:
    // the default sockets take both families.
static void testBasicServerIPv6( void )
{
    printf( "CBasicSipServer over IPv6\n" );
    if ( ! CSocket::IsIPv6Available() )
    {
        printf( "  (no IPv6 here; skipped)\n" );
        return;
    }
    CBasicSipServer server( 0, true, true, 2 );
    server.AddHandlerFactory( new CBaseSipHandlerFactory() );
    server.Start();
    CHECK( server.waitUntilReady( 5000 ) );

    CUDPSocket client( 0, "::1" );
    RequestOptions o;
    o.viaHost = "[::1]:" + std::to_string( client.GetPort() );
    o.viaParams = ";rport";
    o.branch = "z9hG4bKipv6udp";
    std::string request = buildRequest( o );
    CInternetAddress target( "::1", server.getUdpPort(), SOCK_DGRAM );
    CHECK( client.Send( request.data(), (int)request.size(), target ) == (int)request.size() );
    char buffer[ 65536 ];
    CInternetAddress from;
    int n = client.Read( buffer, sizeof( buffer ) - 1, from, 2000 );
    CHECK( n > 0 );
    std::string reply = ( n > 0 ) ? std::string( buffer, (size_t)n ) : std::string();
    CHECK( statusOf( reply ) == 200 );
    CHECK( headerOf( reply, "Via" ).find( "[::1]" ) != std::string::npos );
    CHECK( from.GetFamily() == AF_INET6 );

        // TCP over IPv6 too.
    CClientSocket tcp( "::1", server.getTcpPort() );
    CHECK( tcp.IsConnected() );
    RequestOptions t;
    t.viaHost = "[::1]:5099";
    t.branch = "z9hG4bKipv6tcp";
        // A new call: the same Call-ID, From tag and CSeq with another branch
        // is a merged request, which gets 482 (RFC 3261 8.2.2.2).
    t.callId = "ipv6-tcp-call";
    request = buildRequest( t );
    tcp.Send( request.data(), (int)request.size() );
    n = tcp.ReadWithTimeout( buffer, sizeof( buffer ) - 1, 2000 );
    CHECK( n > 0 && statusOf( std::string( buffer, (size_t)n ) ) == 200 );

    server.Stop();
}

static void testBasicServer( void )
{
    printf( "CBasicSipServer over UDP and TCP\n" );
    CBasicSipServer server( 0, true, true, 4 );
    server.AddHandlerFactory( new CBaseSipHandlerFactory() );
    server.Start();
    CHECK( server.waitUntilReady( 5000 ) );
    int udpPort = server.getUdpPort();
    int tcpPort = server.getTcpPort();
    CHECK( udpPort > 0 );
    CHECK( tcpPort > 0 );

        // UDP: the response goes back to the sender's port (rport).
    RequestOptions o;
    o.viaParams = ";rport";
    std::string reply = udpExchange( udpPort, buildRequest( o ), 2000, 1 );
    CHECK( statusOf( reply ) == 200 );
    CHECK( headerOf( reply, "Via" ).find( ";rport=" ) != std::string::npos );

    RequestOptions invite;
    invite.method = "INVITE";
    invite.branch = "z9hG4bKudpinvite";
    invite.viaParams = ";rport";
    reply = udpExchange( udpPort, buildRequest( invite ), 2000, 2 );
    CHECK( reply.find( "SIP/2.0 100 Trying" ) == 0 );
    CHECK( reply.find( "SIP/2.0 404 Not Found" ) != std::string::npos );

        // TCP: two pipelined requests and a keep-alive in one write.
    int fd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons( (uint16_t)tcpPort );
    addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    CHECK( connect( fd, (struct sockaddr *)&addr, sizeof( addr ) ) == 0 );

    RequestOptions t1;
    t1.branch = "z9hG4bKtcp1";
    t1.callId = "tcp-call-1";
    t1.body = "hello";
    t1.contentType = "text/plain";
    RequestOptions t2;
    t2.branch = "z9hG4bKtcp2";
    t2.callId = "tcp-call-1";
    t2.cseq = 314160;
    std::string stream = buildRequest( t1 ) + buildRequest( t2 ) + "\r\n\r\n";
        // Send in two pieces, split inside the first message's body.
    size_t split = buildRequest( t1 ).size() - 2;
    send( fd, stream.data(), split, MSG_NOSIGNAL );
    std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    send( fd, stream.data() + split, stream.size() - split, MSG_NOSIGNAL );

    std::string received;
    std::vector<std::string> responses;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 3 );
    while ( std::chrono::steady_clock::now() < deadline && ! ( responses.size() == 2 && received == "\r\n" ) )
    {
        struct pollfd pfd = { fd, POLLIN, 0 };
        if ( poll( &pfd, 1, 100 ) > 0 )
        {
            char buffer[ 8192 ];
            ssize_t n = recv( fd, buffer, sizeof( buffer ), 0 );
            if ( n <= 0 )
                break;
            received.append( buffer, (size_t)n );
        }
            // Split off complete responses as they arrive.
        size_t leading = 0;
        long length = 0;
        while ( responses.size() < 2 &&
                ( length = CSipUtils::frameMessage( received.data(), received.size(), leading, 65535 ) ) > 0 )
        {
            responses.push_back( received.substr( leading, (size_t)length - leading ) );
            received.erase( 0, (size_t)length );
        }
    }
    CHECK( responses.size() == 2 );
    if ( responses.size() == 2 )
    {
        CHECK( statusOf( responses[0] ) == 200 );
        CHECK( headerOf( responses[0], "CSeq" ) == "314159 OPTIONS" );
        CHECK( statusOf( responses[1] ) == 200 );
        CHECK( headerOf( responses[1], "CSeq" ) == "314160 OPTIONS" );
        CHECK( headerOf( responses[0], "Via" ).find( "received" ) == std::string::npos );
    }
    CHECK( received == "\r\n" );                                    // the keep-alive pong
    closeSocket( fd );

        // A client that disconnects while the server writes must not kill the
        // process (SIGPIPE).
    fd = socket( AF_INET, SOCK_STREAM, 0 );
    CHECK( connect( fd, (struct sockaddr *)&addr, sizeof( addr ) ) == 0 );
    RequestOptions dropped;
    dropped.callId = "tcp-call-2";
    std::string again = buildRequest( dropped );
    send( fd, again.data(), again.size(), MSG_NOSIGNAL );
    closeSocket( fd );
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

    server.Stop();
    server.Wait();
    CHECK( server.getUdpPort() == 0 );

        // A second server on a port that is in use reports the failure.
    CBasicSipServer first( 0, true, false, 1 );
    first.Start();
    CHECK( first.waitUntilReady( 5000 ) );
    CBasicSipServer clash( first.getUdpPort(), true, false, 1 );
    clash.Start();
    CHECK( ! clash.waitUntilReady( 5000 ) );
    CHECK( clash.getStartupError().GetLength() > 0 );

        // A server that is destroyed without being started must not hang.
    {
        CBasicSipServer unused( 0 );
    }
    {
        CSipServer core;
    }
}

int main( int argc, char *argv[] )
{
    CTestLogSink *log = new CTestLogSink();

    testParsing();
    testOptions();
    testInviteNon2xxTransaction();
    testInviteOverReliableTransport();
    testInvite2xxRetransmission();
    testCancel();
    testMethodsAndValidation();
    testRegister();
    testViaHandling();
    testHostileInput();
    testBasicServer();
    testBasicServerIPv6();

    printf( "\n%d checks, %d failures, %d log lines\n", g_nChecks, g_nFailures, (int)log->lines );
    delete log;
    return ( g_nFailures == 0 ) ? 0 : 1;
}
