/***********************************************************************
**  TestSocketClasses
**
**  Tests for the socket classes over loopback: CInternetAddress and
**  CSafeAddressInfo, the CSocketConfigBuilder chain, CServerSocket and
**  CClientSocket (ports and addresses of both ends, data both ways, the
**  end of the stream, errors, timeouts, non-blocking reads), CUDPSocket,
**  and IPv6 where the machine has it.
***********************************************************************/

#include "IASLib.h"

#include <errno.h>
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

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
}

    // The error number a socket call throws, or 0 if it does not throw.
template <typename Function>
static int thrownError( Function fn )
{
    try
    {
        fn();
    }
    catch ( CSocketException &e )
    {
        return ( e.GetErrorNumber() == 0 ) ? -1 : e.GetErrorNumber();
    }
    return 0;
}

static bool isTimeoutError( int nError )
{
    return nError == EWOULDBLOCK || nError == EAGAIN || nError == ETIMEDOUT;
}

static void testInternetAddress( void )
{
    section( "CInternetAddress" );

    CInternetAddress invalid;
    CHECK( ! invalid.isValid() );
    CHECK( invalid.toString() == "0.0.0.0" );
    CHECK( invalid.toStringWithPort() == "0.0.0.0:0" );
    CHECK( invalid.GetAddress() == NULL );
    CHECK( invalid.GetPort() == -1 );
    CHECK( invalid.GetFamilyString() == "INVALID" );

    CInternetAddress stream( "127.0.0.1", 5060, SOCK_STREAM );
    CHECK( stream.isValid() );
    CHECK( stream.toString() == "127.0.0.1" );
    CHECK( stream.toStringWithPort() == "127.0.0.1:5060" );
    CHECK( stream.GetPort() == 5060 );
    CHECK( stream.GetFamily() == AF_INET );
    CHECK( stream.GetIPv4() != NULL );
    CHECK( stream.GetAddressLength() == sizeof( struct sockaddr_in ) );

        // A datagram lookup, and a numeric service, used to drop the port.
    CInternetAddress datagram( "127.0.0.1", 5070, SOCK_DGRAM );
    CHECK( datagram.isValid() && datagram.GetPort() == 5070 );
    CInternetAddress service( "127.0.0.1", "5080", SOCK_STREAM );
    CHECK( service.isValid() && service.GetPort() == 5080 );

        // The sockaddr constructor copies the address (it went through
        // text and a SOCK_RAW lookup).
    CInternetAddress copied( stream.GetAddress() );
    CHECK( copied.isValid() );
    CHECK( copied.toStringWithPort() == "127.0.0.1:5060" );
    CHECK( copied.GetHostname() == "127.0.0.1" );

    CInternetAddress copy( stream );
    CHECK( copy.toStringWithPort() == "127.0.0.1:5060" );
    CInternetAddress assigned;
    assigned = datagram;
    CHECK( assigned.isValid() && assigned.GetPort() == 5070 );
    assigned = assigned;
    CHECK( assigned.isValid() && assigned.GetPort() == 5070 );

        // A new host keeps the port, and the validity follows it.
    assigned.SetAddress( "127.0.0.2" );
    CHECK( assigned.isValid() );
    CHECK( assigned.toStringWithPort() == "127.0.0.2:5070" );
    assigned.SetAddress( "no-such-host.invalid" );
    CHECK( ! assigned.isValid() );
    assigned.SetAddress( (const struct sockaddr *)NULL );
    CHECK( ! assigned.isValid() );

    CInternetAddress v6( "::1", 80, SOCK_STREAM );
    if ( v6.isValid() )
    {
        CHECK( v6.GetFamily() == AF_INET6 );
        CHECK( v6.toStringWithPort() == "[::1]:80" );
        CHECK( v6.GetIPv6() != NULL );
        CHECK( v6.GetIPv4() == NULL );
        CInternetAddress v6Copy( v6.GetIPv6() );
        CHECK( v6Copy.toStringWithPort() == "[::1]:80" );
    }

    CInternetAddress wildcard( (const char *)NULL, 0, SOCK_STREAM );
    CHECK( wildcard.isValid() );

    CHECK( CInternetAddress::addressToString( NULL ) == "" );
    CHECK( CInternetAddress::lookupHost( NULL ) == "" );
    CHECK( CInternetAddress::getPort( NULL ) == -1 );
    CHECK( CInternetAddress::lookupHost( stream.GetAddress() ).GetLength() > 0 );

        // A list of addresses copies as a list.
    struct sockaddr_in a1, a2;
    memset( &a1, 0, sizeof( a1 ) );
    a1.sin_family = AF_INET;
    a1.sin_port = htons( 1 );
    a2 = a1;
    a2.sin_port = htons( 2 );
    struct addrinfo i2;
    memset( &i2, 0, sizeof( i2 ) );
    i2.ai_family = AF_INET;
    i2.ai_addrlen = sizeof( a2 );
    i2.ai_addr = (struct sockaddr *)&a2;
    struct addrinfo i1 = i2;
    i1.ai_addr = (struct sockaddr *)&a1;
    i1.ai_next = &i2;
    CSafeAddressInfo list( &i1 );
    CHECK( list.getPort() == 1 && list.getNext() && list.getNext()->getPort() == 2 );
    CSafeAddressInfo listCopy( list );
    CHECK( listCopy.getPort() == 1 && listCopy.getNext() && listCopy.getNext()->getPort() == 2 );
    CHECK( listCopy.getNext()->getAddr() != list.getNext()->getAddr() );
    CSafeAddressInfo single( (struct sockaddr *)&a2, sizeof( a2 ) );
    single = list;
    CHECK( single.getPort() == 1 && single.getNext() && single.getNext()->getNext() == NULL );
        // An oversized length is clipped to the storage.
    unsigned char abyLarge[ 512 ];
    memset( abyLarge, 0, sizeof( abyLarge ) );
    memcpy( abyLarge, &a1, sizeof( a1 ) );
    CSafeAddressInfo clipped( (struct sockaddr *)abyLarge, (socklen_t)sizeof( abyLarge ) );
    CHECK( clipped.getPort() == 1 );
    CHECK( clipped.getAddrLen() <= sizeof( struct sockaddr_storage ) );
}

static void testConfig( void )
{
    section( "CSocketConfig" );

    CSocketConfig defaults;
    CHECK( defaults.getBacklogSize() == 32 );
    CHECK( defaults.getTimeout() == 0 );
    CHECK( defaults.isBlocking() );
    CHECK( ! defaults.isTcpNoDelay() );

        // Every setting in a chain sticks (they returned copies).
    CSocketConfigBuilder builder;
    builder.setBacklogSize( 7 ).setTimeout( 250 ).setTCPNoDelay( true ).setKeepalive( true ).setLinger( true, 3 ).setBlocking( false );
    CSocketConfig config = builder.Build();
    CHECK( config.getBacklogSize() == 7 );
    CHECK( config.getTimeout() == 250 );
    CHECK( config.isTcpNoDelay() );
    CHECK( config.isKeepAlive() );
    CHECK( config.isLinger() && config.getLingerSeconds() == 3 );
    CHECK( ! config.isBlocking() );

    CSocketConfig *pConfig = builder.build();
    CHECK( pConfig->getTimeout() == 250 );
    delete pConfig;

    CSocketConfigBuilder copy( builder );
    CHECK( copy.Build().getBacklogSize() == 7 );
}

    // Reads until nWanted bytes or the end.
static std::string readFully( CSocket &oSocket, size_t nWanted )
{
    std::string strAll;
    char achBuffer[ 4096 ];
    while ( strAll.size() < nWanted )
    {
        int nRead = oSocket.Read( achBuffer, (int)sizeof( achBuffer ) );
        if ( nRead <= 0 )
            break;
        strAll.append( achBuffer, (size_t)nRead );
    }
    return strAll;
}

static void testTcp( void )
{
    section( "CServerSocket and CClientSocket" );

    CHECK( NULL_SOCKET != 0 );

    CServerSocket server( 0 );
    CHECK( server.IsConnected() );
    int nPort = server.GetPort();
    CHECK( nPort > 0 );
    CHECK( server.GetLocalPort() == nPort );

    CClientSocket client( "127.0.0.1", nPort );
    CHECK( client.IsConnected() );
    CSocket *pAccepted = server.Accept( 2000 );
    CHECK( pAccepted != NULL );
    if ( pAccepted == NULL )
        return;

        // The far end is the far end (the client reported its own port).
    CHECK( client.GetPort() == nPort );
    CHECK( client.GetAddress() == 0x7F000001UL );
    CHECK( client.GetLocalPort() > 0 && client.GetLocalPort() != nPort );
    CHECK( pAccepted->GetPort() == client.GetLocalPort() );
    CHECK( pAccepted->GetLocalPort() == nPort );
    CHECK( pAccepted->getInternetAddress().isValid() );
    CHECK( pAccepted->getInternetAddress().GetPort() == client.GetLocalPort() );
    CHECK( client.getInternetAddress().GetPort() == nPort );

    CString strExpected;
    strExpected.Format( "127.0.0.1:%d", nPort );
    CHECK( strcmp( client.GetAddressString( true, true ), (const char *)strExpected ) == 0 );
    CHECK( strcmp( client.GetAddressString( true, false ), "127.0.0.1" ) == 0 );
        // (It overwrote the remote address, so this came back wrong.)
    CHECK( strcmp( client.GetAddressString( true, true ), (const char *)strExpected ) == 0 );
    CHECK( strcmp( client.GetAddressString( false, false ), "127.0.0.1" ) == 0 );

        // Nothing waiting yet.
    CHECK( ! pAccepted->HasData() );
    char achBuffer[ 16 ];
    CHECK( pAccepted->ReadWithTimeout( achBuffer, sizeof( achBuffer ), 50 ) == CSocket::READ_TIMEOUT );

        // A large transfer, both ways at once.
    std::string strBig;
    for ( int i = 0; i < 300000; i++ )
        strBig += (char)( 'a' + i % 26 );
    std::string strFromClient;
    std::thread reader( [&]() { strFromClient = readFully( *pAccepted, strBig.size() ); } );
    CHECK( client.Send( strBig.data(), (int)strBig.size() ) == (int)strBig.size() );
    reader.join();
    CHECK( strFromClient == strBig );

    CHECK( pAccepted->Send( 'Z' ) == 1 );
    CHECK( client.Read() == 'Z' );

    CHECK( pAccepted->Send( "xy", 2 ) == 2 );
    CHECK( client.ReadWithTimeout( achBuffer, 1, 2000 ) == 1 && achBuffer[0] == 'x' );
    CHECK( client.HasData() );
    CHECK( client.Read( achBuffer, sizeof( achBuffer ) ) == 1 && achBuffer[0] == 'y' );

        // The end of the stream: 0, and Read( void ) throws.
    pAccepted->Close();
    CHECK( ! pAccepted->IsConnected() );
    CHECK( client.Read( achBuffer, sizeof( achBuffer ) ) == 0 );
    CHECK( thrownError( [&]() { client.Read(); } ) == EPIPE );
    pAccepted->Close();         // twice is harmless
    delete pAccepted;

        // Closed sockets.
    client.Close();
    CHECK( client.Read( achBuffer, sizeof( achBuffer ) ) == 0 );
    CHECK( client.Send( "x", 1 ) == 0 );
    CHECK( client.ReadWithTimeout( achBuffer, 1, 0 ) == CSocket::READ_ERROR );
    CHECK( thrownError( [&]() { client.Read(); } ) == ENOTCONN );

    CHECK( server.Accept( 50 ) == NULL );
}

static void testErrors( void )
{
    section( "Socket errors" );

        // A port nobody listens on.
    int nPort;
    {
        CServerSocket closed( 0 );
        nPort = closed.GetPort();
    }
    CClientSocket refused( "127.0.0.1", nPort );
    CHECK( ! refused.IsConnected() );
    CHECK( ! refused.getInternetAddress().isValid() );

    CClientSocket badHost( "no-such-host.invalid", 80 );
    CHECK( ! badHost.IsConnected() );
    CClientSocket badPort( "127.0.0.1", 70000 );
    CHECK( ! badPort.IsConnected() );
    CClientSocket noHost( NULL, 80 );
    CHECK( ! noHost.IsConnected() );

        // A port in use, with the errno value on every platform.
    CServerSocket first( 0 );
    int nInUse = first.GetPort();
    int nError = thrownError( [&]() { CServerSocket second( nInUse ); } );
#ifdef IASLIB_WIN32__
    CHECK( nError == EADDRINUSE || nError == EACCES );     // (exclusive use)
#else
    CHECK( nError == EADDRINUSE );
#endif
    CHECK( thrownError( [&]() { CServerSocket outOfRange( 70000 ); } ) == EINVAL );
    CSocketConfig config;
    CHECK( thrownError( [&]() { CServerSocket badBind( config, 0, "no-such-host.invalid" ); } ) == EADDRNOTAVAIL );

    CSocketException unknown( 123456 );
    CHECK( unknown.GetErrorNumber() == 123456 );
    CHECK( strstr( (const char *)unknown.GetMessage(), "123456" ) != NULL );
}

static void testOptions( void )
{
    section( "Configured sockets" );

    CSocketConfigBuilder builder;
    builder.setTimeout( 200 ).setTCPNoDelay( true ).setBacklogSize( 4 ).setKeepalive( true ).setLinger( true, 1 );
    CSocketConfig config = builder.Build();

    CServerSocket server( config, 0, "127.0.0.1" );
    CHECK( server.IsConnected() );
    CHECK( strcmp( server.GetAddressString( false, false ), "127.0.0.1" ) == 0 );

    CClientSocket client( config, "localhost", server.GetPort() );
    CHECK( client.IsConnected() );
    CSocket *pAccepted = server.Accept();
    CHECK( pAccepted != NULL );

        // The receive timeout applies (it was passed as an int, which POSIX
        // systems refuse).
    char achBuffer[ 8 ];
    int nError = thrownError( [&]() { client.Read( achBuffer, sizeof( achBuffer ) ); } );
    CHECK( isTimeoutError( nError ) );

        // Non-blocking reads, on a fresh connection (Windows leaves a
        // socket in an undefined state after a receive timeout).
    CClientSocket plain( "127.0.0.1", server.GetPort() );
    CHECK( plain.IsConnected() );
    CSocket *pPlainPeer = server.Accept();
    CHECK( pPlainPeer != NULL );
    CHECK( plain.IsBlocking() );
    plain.SetNonBlocking( true );
    CHECK( ! plain.IsBlocking() );
    nError = thrownError( [&]() { plain.Read( achBuffer, sizeof( achBuffer ) ); } );
    CHECK( isTimeoutError( nError ) );
    if ( pPlainPeer )
    {
        pPlainPeer->Send( "ok", 2 );
        CHECK( plain.ReadWithTimeout( achBuffer, sizeof( achBuffer ), 2000 ) == 2 );
    }
    plain.SetNonBlocking( false );
    CHECK( plain.IsBlocking() );
    delete pPlainPeer;

        // A non-blocking listener returns NULL when nobody is waiting.
    CSocketConfigBuilder nonBlocking;
    nonBlocking.setBlocking( false );
    CServerSocket idle( nonBlocking.Build(), 0 );
    CHECK( ! idle.IsBlocking() );
    CSocket *pNone = (CSocket *)1;
    CHECK( thrownError( [&]() { pNone = idle.Accept(); } ) == 0 );
    CHECK( pNone == NULL );

    delete pAccepted;
}

static void testUdp( void )
{
    section( "CUDPSocket" );

    CUDPSocket server( 0 );
    CHECK( server.IsConnected() );
    CHECK( server.GetPort() > 0 );
    CHECK( server.GetAddress().isValid() );
    CHECK( server.GetAddress().GetPort() == server.GetPort() );

        // The client knows its port at once (it read a random address).
    CUDPSocket client;
    CHECK( client.IsConnected() );
    CHECK( client.GetPort() > 0 );

    CInternetAddress target( "127.0.0.1", server.GetPort(), SOCK_DGRAM );
    CHECK( client.Send( "hello", 5, target ) == 5 );
    char achBuffer[ 64 ];
    CInternetAddress from;
    int nRead = server.Read( achBuffer, sizeof( achBuffer ), from, 2000 );
    CHECK( nRead == 5 );
    CHECK( nRead == 5 && strcmp( achBuffer, "hello" ) == 0 );
    CHECK( from.isValid() && from.GetPort() == client.GetPort() );

        // Replying to where it came from.
    CHECK( server.Send( "back", 4, from ) == 4 );
    CHECK( client.Read( achBuffer, sizeof( achBuffer ), from, 2000 ) == 4 );

        // "localhost" may resolve to ::1 first; the IPv4 address is used.
    CInternetAddress named( "localhost", server.GetPort(), SOCK_DGRAM );
    if ( named.GetIPv4() )
    {
        CHECK( client.Send( "named", 5, named ) == 5 );
        CHECK( server.Read( achBuffer, sizeof( achBuffer ), from, 2000 ) == 5 );
    }

        // A datagram longer than the buffer is cut short.
    CHECK( client.Send( "0123456789", 10, target ) == 10 );
    CHECK( server.Read( achBuffer, 4, from, 2000 ) == 4 );
    CHECK( memcmp( achBuffer, "0123", 4 ) == 0 );

    CHECK( server.Read( achBuffer, sizeof( achBuffer ), from, 50 ) == CSocket::READ_TIMEOUT );
    CHECK( client.Send( "x", 1, CInternetAddress() ) == -1 );

        // A second socket cannot take the port.
    int nPort = server.GetPort();
    CHECK( thrownError( [&]() { CUDPSocket second( nPort ); } ) == EADDRINUSE );

    server.Close();
    server.Close();
    CHECK( ! server.IsConnected() );
    CHECK( server.Read( achBuffer, sizeof( achBuffer ), from ) == -1 );
    CHECK( server.Send( "x", 1, target ) == -1 );
}

static void testIPv6( void )
{
    section( "IPv6" );

    CSocketConfig config;
    CServerSocket *pServer = NULL;
    try
    {
        pServer = new CServerSocket( config, 0, "::1" );
    }
    catch ( CSocketException & )
    {
        printf( "  (no IPv6 loopback here; skipped)\n" );
        return;
    }

    CClientSocket client( "::1", pServer->GetPort() );
    CHECK( client.IsConnected() );
    CSocket *pAccepted = pServer->Accept( 2000 );
    CHECK( pAccepted != NULL );
    CHECK( client.GetAddress() == 0 );
    CHECK( client.getInternetAddress().GetFamily() == AF_INET6 );
    CString strExpected;
    strExpected.Format( "[::1]:%d", pServer->GetPort() );
    CHECK( strcmp( client.GetAddressString( true, true ), (const char *)strExpected ) == 0 );
    if ( pAccepted )
    {
        CHECK( client.Send( "six", 3 ) == 3 );
        char achBuffer[ 8 ];
        CHECK( pAccepted->ReadWithTimeout( achBuffer, sizeof( achBuffer ), 2000 ) == 3 );
        delete pAccepted;
    }
    delete pServer;
}

int main( void )
{
    testInternetAddress();
    testConfig();
    testTcp();
    testErrors();
    testOptions();
    testUdp();
    testIPv6();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
