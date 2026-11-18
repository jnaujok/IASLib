/***********************************************************************
**  TestSjson
**
**  Tests for SJSON (CSjson): bit I/O and NybbleFields, Python-compatible
**  float text, byte-for-byte agreement with packets written by the
**  reference Python library (sjson_vectors.json, from make_vectors.py),
**  decoding Python's tagged and stream-mode packets, round trips, the NAK
**  exchange, stream sequencing, gaps and resets, and a fuzz pass over
**  damaged packets.
***********************************************************************/

#include "IASLib.h"
#include "SJSON/SjsonSchemaNumber.h"

#include <clocale>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
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

#define CHECK_EQ_STR( what, got, expected ) \
    do { \
        g_nChecks++; \
        std::string _got = ( got ); \
        std::string _exp = ( expected ); \
        if ( _got != _exp ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s gave [%s], expected [%s]\n", __FILE__, __LINE__, std::string( what ).c_str(), _got.substr( 0, 300 ).c_str(), _exp.substr( 0, 300 ).c_str() ); \
        } \
    } while ( 0 )

static std::string toHex( const std::vector<unsigned char> &aBytes )
{
    static const char s_szDigits[] = "0123456789abcdef";
    std::string strOut;
    for ( size_t n = 0; n < aBytes.size(); n++ )
    {
        strOut += s_szDigits[ aBytes[ n ] >> 4 ];
        strOut += s_szDigits[ aBytes[ n ] & 15 ];
    }
    return strOut;
}

static std::vector<unsigned char> fromHex( const std::string &strHex )
{
    std::vector<unsigned char> aOut;
    for ( size_t n = 0; n + 1 < strHex.size(); n += 2 )
        aOut.push_back( (unsigned char)strtoul( strHex.substr( n, 2 ).c_str(), NULL, 16 ) );
    return aOut;
}

static std::string json( const CJsonNode *pNode )
{
    return pNode ? pNode->toJson() : std::string( "(null)" );
}

    // Decodes, answering one NAK if needed.
static CJsonNode *decodeWithNak( CSjson &receiver, CSjson &sender, const std::vector<unsigned char> &packet )
{
    unsigned nSender = sender.senderId();    // for packets that leave it out
    CJsonNode *pValue = receiver.decode( packet.data(), packet.size(), nSender );
    if ( pValue == NULL && receiver.lastError() == SJSON_ERROR_UNKNOWN_TAG )
    {
        std::vector<unsigned char> update = sender.nakDictionary( nSender, receiver.lastKnownTag( nSender ), receiver.lastKnownValue( nSender ) );
        receiver.loadDictionary( update.data(), update.size() );
        pValue = receiver.decode( packet.data(), packet.size(), nSender );
    }
    return pValue;
}

static void testBits( void )
{
    printf( "Bits and NybbleFields\n" );
    CSjsonBitWriter out;
    out.writeBinary( "101" );
    out.writeBits( 0x3, 2 );
    out.writeNybble( 300 );
    out.writeBytes( std::string( "ab" ) );
    CHECK_EQ_STR( "bits", out.binString(), "10111" "110" "000100101100" "0110000101100010" );
    CSjsonBitReader in( out.bytes().data(), out.bitLength() );
    CHECK( in.readBits( 3 ) == 5 && in.readBits( 2 ) == 3 && in.readNybble() == 300 && in.readBytes( 2 ) == "ab" && in.remaining() == 0 );
    const unsigned long long aValues[] = { 0, 15, 16, 255, 256, 4095, 4096, 65535, 65536, 1ULL << 20, 1ULL << 24, 1ULL << 28, 0xFFFFFFFFULL };
    for ( size_t n = 0; n < sizeof( aValues ) / sizeof( aValues[ 0 ] ); n++ )
    {
        CSjsonBitWriter w;
        w.writeNybble( aValues[ n ] );
        CHECK( w.bitLength() == CSjsonBitWriter::nybbleBits( aValues[ n ] ) );
        CSjsonBitReader r( w.bytes().data(), w.bitLength() );
        CHECK( r.readNybble() == aValues[ n ] );
    }
    bool bThrew = false;
    try { CSjsonBitWriter w; w.writeNybble( 1ULL << 32 ); } catch ( const CSjsonException & ) { bThrew = true; }
    CHECK( bThrew );
    bThrew = false;
    try { CSjsonBitReader r( out.bytes().data(), 4 ); r.readBits( 5 ); } catch ( const CSjsonException & ) { bThrew = true; }
    CHECK( bThrew );
}

static void testFloatRepr( const CJsonNode &vectors )
{
    printf( "Python float text\n" );
    const CJsonNode &list = vectors.path( "float_reprs" );
    int nBad = 0;
    for ( int n = 0; n < list.size(); n++ )
    {
        std::vector<unsigned char> aBits = fromHex( list.path( n ).path( 0 ).asText().c_str() );
        unsigned long long u = 0;
        for ( size_t k = 0; k < 8; k++ )
            u = ( u << 8 ) | aBits[ k ];
        double d;
        memcpy( &d, &u, sizeof( d ) );
        std::string strExpected = list.path( n ).path( 1 ).textString();
        std::string strGot = SjsonUtil::floatRepr( d );
        if ( strGot != strExpected && nBad++ < 5 )
            printf( "    repr mismatch: %s vs Python %s\n", strGot.c_str(), strExpected.c_str() );
    }
    g_nChecks++;
    if ( nBad )
        g_nFailures++;
    printf( "    %d values, %d mismatches\n", list.size(), nBad );
    CHECK_EQ_STR( "-0.0", SjsonUtil::floatRepr( -0.0 ), "-0.0" );
    CHECK_EQ_STR( "inf", SjsonUtil::floatRepr( HUGE_VAL ), "inf" );
}

static void testPythonVectors( const CJsonNode &vectors, const char *pszSet, bool bExact )
{
    printf( "Python vectors: %s\n", pszSet );
    const CJsonNode &sets = vectors.path( pszSet );
    unsigned nSender = (unsigned)vectors.path( "sender_id" ).asInt();
    for ( int s = 0; s < sets.size(); s++ )
    {
        const CJsonNode &set = sets.path( s );
        std::string strName = set.path( "name" ).textString();
        const CJsonNode &documents = set.path( "documents" );
        const CJsonNode &packets = set.path( "packets" );
        int nSame = 0, nDecoded = 0, nRoundTrip = 0;

            // 1. C++ writes the same bytes as Python.
        if ( bExact )
        {
            CSjson sender( nSender );
            sender.setLz4( false );
            for ( int n = 0; n < documents.size(); n++ )
            {
                std::vector<unsigned char> packet;
                CHECK( sender.encode( documents.path( n ), packet ) );
                std::string strExpected = packets.path( n ).textString();
                if ( toHex( packet ) == strExpected )
                    nSame++;
                else if ( nSame + 1 >= n )
                    CHECK_EQ_STR( strName + " #" + std::to_string( n ) + " " + json( &documents.path( n ) ).substr( 0, 80 ), toHex( packet ), strExpected );
            }
            std::vector<unsigned char> nak = sender.nakDictionary( nSender, 0, 0 );
            CHECK_EQ_STR( strName + " NAK", toHex( nak ), set.path( "nak" ).textString() );
        }

            // 2. C++ reads Python's packets.
        CSjson receiver( 2 );
        if ( set.path( "nak" ).isTextual() )
        {
            std::vector<unsigned char> nak = fromHex( set.path( "nak" ).textString() );
            CHECK( receiver.loadDictionary( nak.data(), nak.size() ) );
        }
        for ( int n = 0; n < documents.size(); n++ )
        {
            std::vector<unsigned char> packet = fromHex( packets.path( n ).textString() );
            std::unique_ptr<CJsonNode> pValue( receiver.decode( packet.data(), packet.size() ) );
            if ( pValue && SjsonCodec::sameValue( pValue.get(), &documents.path( n ) ) )
                nDecoded++;
            else
            {
                CHECK_EQ_STR( strName + " decode #" + std::to_string( n ) + " (" + receiver.lastErrorText() + ")", json( pValue.get() ), json( &documents.path( n ) ) );
                break;
            }
        }

            // 3. C++ round trip, with LZ4 if available.
        CSjson sender( 5, std::string( pszSet ) == "stream" );
        CSjson other( 6 );
        for ( int n = 0; n < documents.size(); n++ )
        {
            std::vector<unsigned char> packet;
            CHECK( sender.encode( documents.path( n ), packet ) );
            std::unique_ptr<CJsonNode> pValue( decodeWithNak( other, sender, packet ) );
            if ( pValue && SjsonCodec::sameValue( pValue.get(), &documents.path( n ) ) )
                nRoundTrip++;
            else
            {
                CHECK_EQ_STR( strName + " round trip #" + std::to_string( n ) + " (" + other.lastErrorText() + ")", json( pValue.get() ), json( &documents.path( n ) ) );
                break;
            }
        }
        g_nChecks += 2;
        if ( nDecoded != documents.size() ) g_nFailures++;
        if ( nRoundTrip != documents.size() ) g_nFailures++;
        printf( "    %-22s %3d documents: %s%d decoded, %d round trips\n", strName.c_str(), documents.size(),
                bExact ? ( std::to_string( nSame ) + " identical bytes, " ).c_str() : "", nDecoded, nRoundTrip );
    }
}

    // Decodes with a NAK answer given as hex (loaded only when needed).
static CJsonNode *decodeWithHexNak( CSjson &receiver, const std::vector<unsigned char> &packet, const std::string &strNak )
{
    CJsonNode *pValue = receiver.decode( packet.data(), packet.size() );
    if ( pValue == NULL && receiver.lastError() == SJSON_ERROR_UNKNOWN_TAG )
    {
        std::vector<unsigned char> nak = fromHex( strNak );
        receiver.loadDictionary( nak.data(), nak.size() );
        pValue = receiver.decode( packet.data(), packet.size() );
    }
    return pValue;
}

static std::vector<std::string> strings( const CJsonNode &list )
{
    std::vector<std::string> aOut;
    for ( int n = 0; n < list.size(); n++ )
        aOut.push_back( list.path( n ).textString() );
    return aOut;
}

static void testSchemas( const CJsonNode &vectors )
{
    printf( "Schemas (loaded here, compared with Python)\n" );
    const CJsonNode &entries = vectors.path( "schemas" );
    for ( int e = 0; e < entries.size(); e++ )
    {
        const CJsonNode &entry = entries.path( e );
        std::string strFile = entry.path( "file" ).textString();
        std::shared_ptr<const CSjsonSchemaModel> pModel;
        try
        {
            pModel.reset( SjsonSchema::loadSchema( "schemas/" + strFile ) );
        }
        catch ( const CSjsonException &error )
        {
            CHECK_EQ_STR( strFile + " load", error.what(), "" );
            continue;
        }
        CHECK( pModel->fingerprint() == (unsigned)entry.path( "fingerprint" ).longValue() );
        CHECK( pModel->fieldNames() == strings( entry.path( "names" ) ) );
        CHECK( pModel->enumStrings() == strings( entry.path( "enums" ) ) );
        std::unique_ptr<CSjsonDictionary> pDictionary( CSjsonDictionary::fromSchema( pModel ) );
        CHECK( pDictionary->fingerprint() == pModel->fingerprint() );

        const CJsonNode &documents = entry.path( "documents" );
        const CJsonNode &packets = entry.path( "packets" );
        const CJsonNode &streamPackets = entry.path( "stream_packets" );
        int nSame = 0, nDecoded = 0, nStreamDecoded = 0, nRoundTrip = 0;
        CSjson sender( 1 );
        sender.setDictionary( *pDictionary );
        sender.setLz4( false );
        CSjson receiver( 2 ), streamReceiver( 3 ), cppReceiver( 4 );
        receiver.registerDictionary( *pDictionary );
        streamReceiver.registerDictionary( *pDictionary );
        cppReceiver.registerDictionary( *pDictionary );
        CSjson streamSender( 6, true );
        streamSender.setDictionary( *pDictionary );
        for ( int n = 0; n < documents.size(); n++ )
        {
            const CJsonNode &doc = documents.path( n );
            std::vector<unsigned char> packet;
            CHECK( sender.encode( doc, packet ) );
            if ( toHex( packet ) == packets.path( n ).textString() )
                nSame++;
            else
                CHECK_EQ_STR( strFile + " #" + std::to_string( n ), toHex( packet ), packets.path( n ).textString() );

            std::vector<unsigned char> python = fromHex( packets.path( n ).textString() );
            std::unique_ptr<CJsonNode> pValue( decodeWithHexNak( receiver, python, entry.path( "nak" ).textString() ) );
                // An enumerated object comes back in the schema's member order
                // (as in Python), so compare with sorted keys.
            if ( pValue && SjsonSchema::pythonJson( pValue.get() ) == SjsonSchema::pythonJson( &doc ) )
                nDecoded++;
            else
                CHECK_EQ_STR( strFile + " decode #" + std::to_string( n ) + " (" + receiver.lastErrorText() + ")", json( pValue.get() ), json( &doc ) );

            std::vector<unsigned char> stream = fromHex( streamPackets.path( n ).textString() );
            pValue.reset( streamReceiver.decode( stream.data(), stream.size() ) );
                // Schema order: compare member sets loosely through JSON with
                // sorted keys. (Python's stream packets carry zstd text.)
            if ( pValue && SjsonSchema::pythonJson( pValue.get() ) == SjsonSchema::pythonJson( &doc ) )
                nStreamDecoded++;
            else if ( CSjson::zstdAvailable() )
                CHECK_EQ_STR( strFile + " stream decode #" + std::to_string( n ) + " (" + streamReceiver.lastErrorText() + ")", json( pValue.get() ), json( &doc ) );

            CHECK( streamSender.encode( doc, packet ) );
            pValue.reset( cppReceiver.decode( packet.data(), packet.size() ) );
            if ( pValue && SjsonSchema::pythonJson( pValue.get() ) == SjsonSchema::pythonJson( &doc ) )
                nRoundTrip++;
            else
                CHECK_EQ_STR( strFile + " stream round trip #" + std::to_string( n ) + " (" + cppReceiver.lastErrorText() + ")", json( pValue.get() ), json( &doc ) );
        }
        printf( "    %-20s %2d documents: %d identical bytes, %d decoded, %d stream decoded, %d stream round trips\n", strFile.c_str(),
                documents.size(), nSame, nDecoded, nStreamDecoded, nRoundTrip );
    }
}

static void testCompiledDictionaries( const CJsonNode &vectors )
{
    printf( "Compiled dictionaries (written by Python)\n" );
    const CJsonNode &entries = vectors.path( "compiled" );
    for ( int e = 0; e < entries.size(); e++ )
    {
        const CJsonNode &entry = entries.path( e );
        std::string strLabel = entry.path( "label" ).textString();
        std::unique_ptr<CSjsonDictionary> pDictionary;
        try
        {
            pDictionary.reset( CSjsonDictionary::fromJson( entry.path( "dictionary" ) ) );
        }
        catch ( const CSjsonException &error )
        {
            CHECK_EQ_STR( strLabel + " load", error.what(), "" );
            continue;
        }
        CHECK( pDictionary->fingerprint() == (unsigned)entry.path( "fingerprint" ).longValue() );
        std::unique_ptr<CJsonNode> pSaved( pDictionary->toJson() );
        std::unique_ptr<CSjsonDictionary> pReloaded( CSjsonDictionary::fromJson( *pSaved ) );
        CHECK( pReloaded->fingerprint() == pDictionary->fingerprint() );

        const CJsonNode &documents = entry.path( "documents" );
        const CJsonNode &packets = entry.path( "packets" );
        int nSame = 0, nDecoded = 0;
        CSjson sender( 1 ), receiver( 2 );
        sender.setDictionary( *pDictionary );
        sender.setLz4( false );
        receiver.registerDictionary( *pReloaded );
        bool bText = ! pDictionary->textDictionary().empty();
        for ( int n = 0; n < documents.size(); n++ )
        {
            std::vector<unsigned char> packet;
            CHECK( sender.encode( documents.path( n ), packet ) );
                // A packet with a text block has zstd output inside it, which
                // may differ between zstd versions; it is checked by decoding.
            if ( toHex( packet ) == packets.path( n ).textString() )
                nSame++;
            else if ( ! bText )
                CHECK_EQ_STR( strLabel + " #" + std::to_string( n ), toHex( packet ), packets.path( n ).textString() );
            std::vector<unsigned char> python = fromHex( packets.path( n ).textString() );
            std::unique_ptr<CJsonNode> pValue( decodeWithHexNak( receiver, python, entry.path( "nak" ).textString() ) );
            if ( pValue && SjsonSchema::pythonJson( pValue.get() ) == SjsonSchema::pythonJson( &documents.path( n ) ) )
                nDecoded++;
            else if ( ! bText || CSjson::zstdAvailable() )     // a text block needs zstd
                CHECK_EQ_STR( strLabel + " decode #" + std::to_string( n ) + " (" + receiver.lastErrorText() + ")", json( pValue.get() ), json( &documents.path( n ) ) );
        }
        printf( "    %-26s fingerprint ok, %d documents: %d identical bytes, %d decoded\n", strLabel.c_str(), documents.size(), nSame, nDecoded );
    }
}

static CJsonNode *document( int n )
{
    CObjectNode *pObject = new CObjectNode();
    pObject->put( "id", "6f9619ff-8b86-4011-b42d-00c04fc964ff" );
    pObject->put( "seq", n );
    pObject->put( "lat", 0.6 + n * 1e-5 );
    pObject->put( "label", n % 2 ? "ALPHA ONE" : "BRAVO TWO" );
    pObject->put( "note", std::string( "a fairly long free-text note number " ) + std::to_string( n % 3 ) );
    pObject->put( "url", std::string( "https://example.com/items/" ) + std::to_string( n % 4 ) );
    pObject->putArray( "flags" )->add( true )->add( n % 2 == 0 )->addNull();
    return pObject;
}

static void testStreamMode( void )
{
    printf( "Stream mode\n" );
    for ( unsigned nWidth = 0; nWidth <= 5; nWidth += 5 )
    {
        CSjson sender( 7, true, nWidth ), receiver( 8 ), plain( 9 );
        size_t nStream = 0, nPlain = 0;
        bool bOk = true;
        for ( int n = 0; n < 60; n++ )
        {
            std::unique_ptr<CJsonNode> pDoc( document( n ) );
            std::vector<unsigned char> packet, other;
            sender.encode( *pDoc, packet );
            plain.encode( *pDoc, other );
            nStream += packet.size();
            nPlain += other.size();
            std::unique_ptr<CJsonNode> pBack( receiver.decode( packet.data(), packet.size() ) );
            bOk = bOk && pBack && SjsonCodec::sameValue( pBack.get(), pDoc.get() );
        }
        CHECK( bOk );
        CHECK( nStream * 2 < nPlain );
        printf( "    width %u: %zu bytes in stream mode, %zu without\n", nWidth, nStream, nPlain );
    }

        // A lost packet is detected; a reset recovers (even after a new name).
    CSjson sender( 7, true ), receiver( 8 );
    std::vector<unsigned char> packet;
    std::unique_ptr<CJsonNode> p0( document( 0 ) ), p1( document( 1 ) ), p2( document( 2 ) );
    sender.encode( *p0, packet );
    std::unique_ptr<CJsonNode> pBack( receiver.decode( packet.data(), packet.size() ) );
    CHECK( pBack != NULL );
    static_cast<CObjectNode *>( p1.get() )->put( "brand new", 1 );
    sender.encode( *p1, packet );   // lost
    sender.encode( *p2, packet );
    pBack.reset( receiver.decode( packet.data(), packet.size() ) );
    CHECK( pBack == NULL && receiver.lastError() == SJSON_ERROR_STREAM_GAP );
    sender.resetStream();
    static_cast<CObjectNode *>( p2.get() )->put( "brand new", 2 );
    sender.encode( *p2, packet );
    pBack.reset( receiver.decode( packet.data(), packet.size() ) );
    CHECK( pBack && SjsonCodec::sameValue( pBack.get(), p2.get() ) );

        // The sequence number wraps.
    CSjson wrapSender( 7, true ), wrapReceiver( 8 );
    bool bOk = true;
    for ( int n = 0; n < 600 && bOk; n++ )
    {
        CObjectNode doc;
        doc.put( "i", n );
        wrapSender.encode( doc, packet );
        pBack.reset( wrapReceiver.decode( packet.data(), packet.size() ) );
        bOk = pBack && SjsonCodec::sameValue( pBack.get(), &doc );
    }
    CHECK( bOk );
}

static void testOptionalSender( void )
{
    printf( "Optional sender id\n" );
    std::unique_ptr<CJsonNode> pDoc( document( 1 ) );
    std::vector<unsigned char> packet, other;
    CSjson::Header header;

        // Stream mode leaves the id out; stateless packets keep it.
    CSjson stream( 7, true ), plain( 7 );
    CHECK( ! stream.includeSenderId() && plain.includeSenderId() );
    CHECK( stream.encode( *pDoc, packet ) );
    CHECK( CSjson::readHeader( packet.data(), packet.size(), header, 42 ) );
    CHECK( ! header.bSender && header.nSenderId == 42 && ! ( header.nFlags & CSjson::FLAG_SENDER ) );
    CHECK( plain.encode( *pDoc, packet ) );
    CHECK( CSjson::readHeader( packet.data(), packet.size(), header, 42 ) );
    CHECK( header.bSender && header.nSenderId == 7 );

        // Leaving it out saves exactly 16 bits.
    CSjson with( 9 ), without( 9 );
    without.setIncludeSenderId( false );
    size_t nWith = 0, nWithout = 0;
    CHECK( with.encode( *pDoc, packet, &nWith ) && without.encode( *pDoc, other, &nWithout ) );
    CHECK( nWith - nWithout == 16 );

        // One receiver, two links, each attributed to its own id.
    CSjson a( 1, true ), b( 2, true ), receiver( 99 );
    for ( int n = 0; n < 10; n++ )
    {
        std::unique_ptr<CJsonNode> pIn( document( n ) );
        CHECK( a.encode( *pIn, packet ) );
        std::unique_ptr<CJsonNode> pA( receiver.decode( packet.data(), packet.size(), 101 ) );
        CHECK( b.encode( *pIn, packet ) );
        std::unique_ptr<CJsonNode> pB( receiver.decode( packet.data(), packet.size(), 102 ) );
        CHECK( pA && pB && json( pA.get() ) == json( pIn.get() ) && json( pB.get() ) == json( pIn.get() ) );
    }
    CHECK( receiver.lastKnownTag( 101 ) > 0 && receiver.lastKnownTag( 101 ) == receiver.lastKnownTag( 102 ) );

        // A NAK under the link's id is answered with the sender's own names.
    CSjson quiet( 7 ), listener( 8 );
    quiet.setIncludeSenderId( false );
    CHECK( quiet.encode( *pDoc, packet ) );
    CHECK( listener.decode( packet.data(), packet.size(), 55 ) == NULL && listener.lastError() == SJSON_ERROR_UNKNOWN_TAG );
    std::vector<unsigned char> update = quiet.nakDictionary( 55, 0, 0 );
    CHECK( listener.loadDictionary( update.data(), update.size() ) );
    std::unique_ptr<CJsonNode> pBack( listener.decode( packet.data(), packet.size(), 55 ) );
    CHECK( pBack && json( pBack.get() ) == json( pDoc.get() ) );

        // Both peers omit ids and call each other 0: each NAK still gets the
        // answering side's own names, not the names it learnt from the peer.
    CSjson left( 1 ), right( 2 );
    left.setIncludeSenderId( false );
    right.setIncludeSenderId( false );
    CObjectNode theirs, mine;
    theirs.put( "theirs", 1 );
    mine.put( "mine", 2 );
    CHECK( right.encode( theirs, packet ) );
    update = right.nakDictionary( 0, 0, 0 );
    left.loadDictionary( update.data(), update.size() );
    pBack.reset( left.decode( packet.data(), packet.size(), 0 ) );
    CHECK( pBack && json( pBack.get() ) == json( &theirs ) );
    CHECK( left.encode( mine, packet ) );
    update = left.nakDictionary( 0, 0, 0 );
    right.loadDictionary( update.data(), update.size() );
    pBack.reset( right.decode( packet.data(), packet.size(), 0 ) );
    CHECK( pBack && json( pBack.get() ) == json( &mine ) );
}

    // One direction of an in-memory pipe.
struct PipeChannel
{
    std::mutex              mutex;
    std::condition_variable ready;
    std::deque<char>        data;
    bool                    bClosed = false;
};

    // One end of a duplex pipe, as a CStream. Writes listed in aCorrupt or
    // aDrop (counted from 1; each CSjsonLink frame is one write) are damaged
    // or lost.
class CPipeEnd : public CStream
{
    public:
        CPipeEnd( PipeChannel &in, PipeChannel &out ) : m_in( in ), m_out( out ), m_nWrites( 0 ) {}
        std::vector<int>    aCorrupt;
        virtual int GetBuffer( char *achBuffer, int nLength )
        {
            std::unique_lock<std::mutex> lock( m_in.mutex );
            m_in.ready.wait( lock, [this] { return ! m_in.data.empty() || m_in.bClosed; } );
            int nRead = 0;
            while ( nRead < nLength && ! m_in.data.empty() )
            {
                achBuffer[ nRead++ ] = m_in.data.front();
                m_in.data.pop_front();
            }
            return nRead;
        }
        virtual int PutBuffer( const char *achBuffer, int nLength )
        {
            std::string strData( achBuffer, (size_t)nLength );
            m_nWrites++;
            for ( size_t n = 0; n < aCorrupt.size(); n++ )
                if ( aCorrupt[ n ] == m_nWrites && nLength > 12 )
                    strData[ (size_t)nLength - 6 ] ^= 0x10;
            std::lock_guard<std::mutex> lock( m_out.mutex );
            m_out.data.insert( m_out.data.end(), strData.begin(), strData.end() );
            m_out.ready.notify_all();
            return nLength;
        }
        virtual bool IsEOS( void )
        {
            std::lock_guard<std::mutex> lock( m_in.mutex );
            return m_in.bClosed && m_in.data.empty();
        }
        virtual char PeekChar( void )
        {
            std::unique_lock<std::mutex> lock( m_in.mutex );
            m_in.ready.wait( lock, [this] { return ! m_in.data.empty() || m_in.bClosed; } );
            return m_in.data.empty() ? '\0' : m_in.data.front();
        }
        virtual void Close( void )
        {
            std::lock_guard<std::mutex> lock( m_out.mutex );
            m_out.bClosed = true;
            m_out.ready.notify_all();
        }
    private:
        PipeChannel        &m_in;
        PipeChannel        &m_out;
        int                 m_nWrites;
};

static CJsonNode *report( int n )
{
    CObjectNode *pObject = new CObjectNode();
    pObject->put( "platform", "UAV-7" );
    pObject->put( "sequence", (long long)n );
    pObject->put( "lat", 35.0 + n * 1e-4 );
    pObject->put( "status", n % 5 ? "NOMINAL" : "DEGRADED" );
    return pObject;
}

    // Sends count reports from sender to a receiver thread; nRestartAt
    // swaps in a fresh receiving codec (a restarted process).
static CSjsonLink::Stats linkRun( CSjson &sender, int nCount, int nRestartAt = -1, std::vector<int> aCorrupt = std::vector<int>() )
{
    PipeChannel forward, back;
    CPipeEnd senderEnd( back, forward ), receiverEnd( forward, back );
    senderEnd.aCorrupt = aCorrupt;
    CSjsonLink senderLink( senderEnd, sender );
    CSjson first( 2 ), second( 2 );
    CSjsonLink receiverLink( receiverEnd, first, 1 );
    std::vector<std::string> aGot;
    std::thread receiver( [&]() {
        for ( int n = 0; n < nCount; n++ )
        {
            if ( n == nRestartAt )
                receiverLink.setCodec( second );
            std::unique_ptr<CJsonNode> pDoc( receiverLink.receive() );
            aGot.push_back( json( pDoc.get() ) );
        }
    } );
    bool bAllSent = true;
    for ( int n = 0; n < nCount; n++ )
    {
        std::unique_ptr<CJsonNode> pDoc( report( n ) );
        bAllSent = senderLink.send( *pDoc ) && bAllSent;
    }
    receiver.join();
    CHECK( bAllSent );
    CHECK( (int)aGot.size() == nCount );
    for ( int n = 0; n < nCount && n < (int)aGot.size(); n++ )
    {
        std::unique_ptr<CJsonNode> pDoc( report( n ) );
        CHECK( aGot[ (size_t)n ] == json( pDoc.get() ) );
    }
    return senderLink.stats();
}

static void testLink( void )
{
    printf( "ACK/NAK link\n" );
    CSjson stateless( 1 );
    CSjsonLink::Stats stats = linkRun( stateless, 20 );
    CHECK( stats.nNaks == 1 && stats.nDictionaryBytes > 0 && stats.nResent == 0 );
    printf( "    stateless: %.1f bytes/msg, %zu NAK, %zu dictionary bytes\n", stats.nSent / 20.0, stats.nNaks, stats.nDictionaryBytes );

    CSjson stream( 1, true );
    stats = linkRun( stream, 20 );
    CHECK( stats.nNaks == 0 && stats.nResent == 0 );
    printf( "    stream mode: %.1f bytes/msg, no NAKs\n", stats.nSent / 20.0 );

    CSjson restarted( 1, true );
    stats = linkRun( restarted, 20, 10 );
    CHECK( stats.nNaks == 1 && stats.nResent > 0 );

    CSjson damaged( 1, true );
    std::vector<int> aCorrupt;
    aCorrupt.push_back( 3 );
    aCorrupt.push_back( 9 );
    stats = linkRun( damaged, 20, -1, aCorrupt );
    CHECK( stats.nNaks == 2 && stats.nResent > 0 );

        // A dictionary the receiver never registered: refused, not retried.
    {
        PipeChannel forward, back;
        CPipeEnd senderEnd( back, forward ), receiverEnd( forward, back );
        std::vector<std::string> aNames( 1, "alpha" );
        std::unique_ptr<CSjsonDictionary> pDictionary( CSjsonDictionary::fromNames( aNames, std::vector<std::string>() ) );
        CSjson sender( 1 ), receiver( 2 );
        sender.setDictionary( *pDictionary );
        CSjsonLink senderLink( senderEnd, sender ), receiverLink( receiverEnd, receiver );
        std::thread peer( [&]() { std::unique_ptr<CJsonNode> pDoc( receiverLink.receive() ); } );
        CObjectNode doc;
        doc.put( "alpha", 1LL );
        CHECK( ! senderLink.send( doc ) && senderLink.lastError() == CSjsonLink::LINK_REFUSED );
        senderEnd.Close();
        peer.join();
        CHECK( receiverLink.lastError() == CSjsonLink::LINK_CLOSED );
    }

        // Both ends send.
    {
        PipeChannel forward, back;
        CPipeEnd leftEnd( back, forward ), rightEnd( forward, back );
        CSjson left( 1 ), right( 2 );
        CSjsonLink leftLink( leftEnd, left, 2 ), rightLink( rightEnd, right, 1 );
        std::string strGot;
        std::thread peer( [&]() {
            std::unique_ptr<CJsonNode> pDoc( rightLink.receive() );
            strGot = json( pDoc.get() );
            CObjectNode pong;
            pong.put( "pong", 1LL );
            rightLink.send( pong );
        } );
        CObjectNode ping;
        ping.put( "ping", 1LL );
        CHECK( leftLink.send( ping ) );
        std::unique_ptr<CJsonNode> pPong( leftLink.receive() );
        peer.join();
        CHECK( strGot == json( &ping ) );
        CHECK( pPong && json( pPong.get() ) == "{\"pong\":1}" );
    }
}

static void testReviewFixes( void )
{
    printf( "Review fixes\n" );
    std::vector<unsigned char> packet;
    std::shared_ptr<const CSjsonSchemaModel> pModel( SjsonSchema::loadJsonSchema( "schemas/order.schema.json" ) );
    std::unique_ptr<CSjsonDictionary> pDictionary( CSjsonDictionary::fromSchema( pModel ) );

        // A stream gap reports the gap and leaves the dictionary alone.
    {
        CSjson sender( 1, true ), receiver( 2 );
        sender.setDictionary( *pDictionary );
        receiver.registerDictionary( *pDictionary );
        std::unique_ptr<CJsonNode> pDoc( document( 1 ) );
        CHECK( sender.encode( *pDoc, packet ) );
        std::unique_ptr<CJsonNode> pBack( receiver.decode( packet.data(), packet.size(), 1 ) );
        CHECK( pBack != NULL );
        unsigned long long nKnown = receiver.lastKnownTag( 1 );
        CHECK( nKnown >= pDictionary->baseSize() );
        CHECK( sender.encode( *pDoc, packet ) );       // lost
        CHECK( sender.encode( *pDoc, packet ) );
        pBack.reset( receiver.decode( packet.data(), packet.size(), 1 ) );
        CHECK( pBack == NULL && receiver.lastError() == SJSON_ERROR_STREAM_GAP );
        CHECK( receiver.lastKnownTag( 1 ) == nKnown );
    }

        // setDictionary with the endpoint's own dictionary, mid-stream: no
        // use-after-free, and the stream restarts naming the dictionary.
    {
        CSjson sender( 1, true );
        sender.setDictionary( *pDictionary );
        std::unique_ptr<CJsonNode> pDoc( document( 2 ) );
        CHECK( sender.encode( *pDoc, packet ) && sender.encode( *pDoc, packet ) );
        sender.setDictionary( sender.dictionary( 1 ) );
        CHECK( sender.encode( *pDoc, packet ) );
        CSjson::Header header;
        CHECK( CSjson::readHeader( packet.data(), packet.size(), header ) );
        CHECK( header.nSequence == 0 && header.bFingerprint && header.nFingerprint == pDictionary->fingerprint() );
    }

        // A far-off dictionary id from a peer does not allocate up to it.
    {
        CSjsonBitWriter out;
        out.writeBits( 5, 16 );
        out.writeNybble( 1 );
        out.writeNybble( 0xFFFFFFF0ULL );
        out.writeNybble( 4 );
        out.writeBytes( std::string( "far!" ) );
        out.writeNybble( 0 );
        CSjson receiver( 2 );
        CHECK( receiver.loadDictionary( out.bytes().data(), out.bytes().size() ) );
        CHECK( receiver.dictionary( 5 ).lookup( 0xFFFFFFF0ULL ) == "far!" );
        CHECK( receiver.lastKnownTag( 5 ) == 0xFFFFFFF0ULL );
        CHECK( receiver.dictionary( 5 ).entriesAfter( 0 ).size() == 1 );
        receiver.dictionary( 5 ).set( "near", 1 );
        CHECK( receiver.dictionary( 5 ).entriesAfter( 0 ).size() == 2 );
        receiver.dictionary( 5 ).resetToBase();
        CHECK( ! receiver.dictionary( 5 ).hasId( 0xFFFFFFF0ULL ) && ! receiver.dictionary( 5 ).hasTag( "far!" ) );
    }

        // Integers and doubles compare exactly, as in Python.
    CHECK( SjsonNumber::compare( SjsonNumber::integer( "9007199254740993" ), SjsonNumber::real( 9007199254740992.0 ) ) == 1 );
    CHECK( SjsonNumber::compare( SjsonNumber::real( 9007199254740992.0 ), SjsonNumber::integer( "9007199254740993" ) ) == -1 );
    CHECK( SjsonNumber::compare( SjsonNumber::integer( "9007199254740992" ), SjsonNumber::real( 9007199254740992.0 ) ) == 0 );
    CHECK( SjsonNumber::compare( SjsonNumber::integer( "2" ), SjsonNumber::real( 2.5 ) ) == -1 );
    CHECK( SjsonNumber::compare( SjsonNumber::integer( "3" ), SjsonNumber::real( 2.5 ) ) == 1 );
    CHECK( SjsonNumber::compare( SjsonNumber::integer( "-3" ), SjsonNumber::real( -2.5 ) ) == -1 );
    CHECK( SjsonNumber::compare( SjsonNumber::integer( "5" ), SjsonNumber::real( -HUGE_VAL ) ) == 1 );
    CHECK( SjsonNumber::max( SjsonNumber::real( 9007199254740992.0 ), SjsonNumber::integer( "9007199254740993" ) ).bInt );

        // Arrays of zero-bit items (here nulls) decode.
    {
        std::unique_ptr<CJsonNode> pSchema( CJsonParser::parse( "{\"type\":\"array\",\"items\":{\"type\":\"null\"}}" ) );
        std::shared_ptr<const CSjsonSchemaModel> pNulls( SjsonSchema::loadJsonSchemaDocument( *pSchema ) );
        std::unique_ptr<CSjsonDictionary> pNullDictionary( CSjsonDictionary::fromSchema( pNulls ) );
        CSjson sender( 1 ), receiver( 2 );
        sender.setDictionary( *pNullDictionary );
        receiver.registerDictionary( *pNullDictionary );
        std::unique_ptr<CJsonNode> pDoc( CJsonParser::parse( "[null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null]" ) );
        CHECK( sender.encode( *pDoc, packet ) );
        CSjson::Header header;
        CHECK( CSjson::readHeader( packet.data(), packet.size(), header ) && ( header.nFlags & CSjson::FLAG_SCHEMA_ENCODED ) );
        std::unique_ptr<CJsonNode> pBack( receiver.decode( packet.data(), packet.size() ) );
        CHECK( pBack && json( pBack.get() ) == json( pDoc.get() ) );
    }

        // A comma-decimal locale does not change floats or their text.
    const char *apszLocales[] = { "de_DE.UTF-8", "de_DE.utf8", "fr_FR.UTF-8", "German_Germany.1252", NULL };
    std::string strSaved = setlocale( LC_NUMERIC, NULL );
    bool bComma = false;
    for ( int n = 0; apszLocales[ n ] && ! bComma; n++ )
        bComma = setlocale( LC_NUMERIC, apszLocales[ n ] ) != NULL && localeconv()->decimal_point[ 0 ] == ',';
    if ( bComma )
    {
        CHECK( SjsonUtil::floatRepr( 1.5 ) == "1.5" && SjsonUtil::floatRepr( 0.1 ) == "0.1" );
        CHECK( SjsonUtil::parseDouble( "2.25" ) == 2.25 );
        CSjson sender( 1 ), receiver( 2 );
        CObjectNode doc;
        doc.put( "x", 1.5 );
        doc.put( "y", 0.1 );
        CHECK( sender.encode( doc, packet ) );
        std::unique_ptr<CJsonNode> pBack( decodeWithNak( receiver, sender, packet ) );
        CHECK( pBack && pBack->path( "x" ).doubleValue() == 1.5 && pBack->path( "y" ).doubleValue() == 0.1 );
    }
    setlocale( LC_NUMERIC, strSaved.c_str() );
    printf( "    comma-decimal locale %s\n", bComma ? "checked" : "not installed (skipped)" );

        // A packet without a sender id must not be filed under our own id.
    {
        CSjson sender( 7, true ), receiver( 0 );
        CObjectNode doc;
        doc.put( "a", 1 );
        CHECK( sender.encode( doc, packet ) );
        CHECK( receiver.decode( packet.data(), packet.size() ) == NULL && receiver.lastError() == SJSON_ERROR_UNSUPPORTED );
        std::unique_ptr<CJsonNode> pBack( receiver.decode( packet.data(), packet.size(), 1 ) );
        CHECK( pBack && json( pBack.get() ) == json( &doc ) );
    }

        // A DICT that cannot be loaded makes the receiver forget what it
        // learnt from that sender and NAK again from the base.
    {
        PipeChannel forward, back;
        CPipeEnd senderEnd( back, forward ), receiverEnd( forward, back );
        CSjson sender( 1 ), receiver( 2 );
        CSjsonLink link( receiverEnd, receiver, 1 );
        CObjectNode doc;
        doc.put( "alpha", 1 );
        CHECK( sender.encode( doc, packet ) );
        std::vector<unsigned char> frame;
        frame.push_back( CSjsonLink::FRAME_DATA ); frame.push_back( 0 ); frame.push_back( 0 );
        frame.push_back( 0 ); frame.push_back( 0 ); frame.push_back( 0 ); frame.push_back( (unsigned char)packet.size() );
        frame.insert( frame.end(), packet.begin(), packet.end() );
        const unsigned char achDict[ 8 ] = { CSjsonLink::FRAME_DICT, 0, 0, 0, 0, 0, 1, 0xFF };
        frame.insert( frame.end(), achDict, achDict + 8 );
        senderEnd.PutBuffer( (const char *)frame.data(), (int)frame.size() );
        senderEnd.Close();
        CHECK( link.receive() == NULL );        // the stream ends after the DICT
        char achAnswers[ 64 ];
        int nGot = senderEnd.GetBuffer( achAnswers, sizeof( achAnswers ) );
        CHECK( nGot >= 24 && achAnswers[ 0 ] == CSjsonLink::FRAME_NAK_DICT && achAnswers[ 17 ] == CSjsonLink::FRAME_NAK_DICT );
    }

        // A stream packet that carries our own id (our broadcast heard back)
        // is refused rather than resetting our sending dictionary.
    {
        CSjson echo( 1, true );
        echo.setIncludeSenderId( true );
        CObjectNode doc;
        doc.put( "alpha", 1 );
        CHECK( echo.encode( doc, packet ) );
        CHECK( echo.decode( packet.data(), packet.size() ) == NULL && echo.lastError() == SJSON_ERROR_UNSUPPORTED );
    }

        // Unknown frame types end the link; links start numbering at random.
    {
        PipeChannel forward, back;
        CPipeEnd senderEnd( back, forward ), receiverEnd( forward, back );
        CSjson codec( 2 );
        CSjsonLink link( receiverEnd, codec, 1 );
        const char achBad[ 7 ] = { 9, 0, 0, 0, 0, 0, 0 };
        senderEnd.PutBuffer( achBad, 7 );
        CHECK( link.receive() == NULL && link.lastError() == CSjsonLink::LINK_PROTOCOL );
    }

        // A frame longer than the link allows ends it before any allocation.
    {
        PipeChannel forward, back;
        CPipeEnd senderEnd( back, forward ), receiverEnd( forward, back );
        CSjson codec( 2 );
        CSjsonLink link( receiverEnd, codec, 1, 4, 1024 );
        const char achHeader[ 7 ] = { 1, 0, 0, 0x7F, (char)0xFF, (char)0xFF, (char)0xFF };
        senderEnd.PutBuffer( achHeader, 7 );
        CHECK( link.receive() == NULL && link.lastError() == CSjsonLink::LINK_PROTOCOL );
        const char achAck[ 8 ] = { 2, 0, 0, 0, 0, 0, 1, 0 };     // an ACK with a payload
        CSjsonLink other( receiverEnd, codec );
        senderEnd.PutBuffer( achAck, 8 );
        CHECK( other.receive() == NULL && other.lastError() == CSjsonLink::LINK_PROTOCOL );
    }
}

static void testFuzz( void )
{
    printf( "Fuzz\n" );
    std::mt19937 rng( 7 );
    int nDecoded = 0, nRejected = 0;
        // (Stream senders are fewer: each new one sets up a zstd context.)
    for ( int round = 0; round < 3000; round++ )
    {
        bool bStream = round % 10 == 1;
        CSjson sender( 1, bStream ), receiver( 2 );
        std::unique_ptr<CJsonNode> pDoc( document( round ) );
        std::vector<unsigned char> packet;
        sender.encode( *pDoc, packet );
        size_t nFlips = 1 + rng() % 4;
        for ( size_t k = 0; k < nFlips; k++ )
            packet[ rng() % packet.size() ] ^= (unsigned char)( 1 << ( rng() % 8 ) );
        if ( rng() % 4 == 0 )
            packet.resize( rng() % packet.size() );
        std::unique_ptr<CJsonNode> pBack( receiver.decode( packet.data(), packet.size() ) );
        if ( pBack ) nDecoded++; else nRejected++;
    }
    CHECK( nRejected > 2500 );     // the CRC catches nearly all damage
    printf( "    3000 damaged packets: %d rejected, %d decoded (CRC collisions)\n", nRejected, nDecoded );

        // Damage behind a valid CRC: the decoder itself must reject or cope,
        // never crash or hang.
    int nHandled = 0;
    for ( int round = 0; round < 20000; round++ )
    {
        bool bStream = round % 40 == 1;
        CSjson sender( 1, bStream ), receiver( 2 );
        std::unique_ptr<CJsonNode> pDoc( document( round ) );
        std::vector<unsigned char> packet;
        size_t nBits = 0;
        sender.encode( *pDoc, packet, &nBits );
        CSjson::Header header;
        CHECK( CSjson::readHeader( packet.data(), packet.size(), header ) );
        size_t nMessage = header.messageLength();
        size_t nFlips = 1 + rng() % 6;
        for ( size_t k = 0; k < nFlips; k++ )
        {
            size_t nBit = header.nHeaderLength + rng() % ( nMessage - header.nHeaderLength );
            packet[ nBit / 8 ] ^= (unsigned char)( 0x80 >> ( nBit % 8 ) );
        }
            // Re-sign: CRC of the zero-padded message bytes.
        std::vector<unsigned char> aMessage( packet.begin(), packet.begin() + (long)( ( nMessage + 7 ) / 8 ) );
        if ( nMessage & 7 )
            aMessage.back() &= (unsigned char)( 0xFF << ( 8 - ( nMessage & 7 ) ) );
        unsigned nCrc = SjsonUtil::crc32( aMessage.data(), aMessage.size() );
        for ( int k = 0; k < 32; k++ )
        {
            size_t nBit = nMessage + (size_t)k;
            unsigned char byMask = (unsigned char)( 0x80 >> ( nBit % 8 ) );
            if ( ( nCrc >> ( 31 - k ) ) & 1 ) packet[ nBit / 8 ] |= byMask; else packet[ nBit / 8 ] &= (unsigned char)~byMask;
        }
        std::unique_ptr<CJsonNode> pBack( receiver.decode( packet.data(), packet.size() ) );
        nHandled++;
    }
    CHECK( nHandled == 20000 );
    printf( "    20000 damaged packets behind a valid CRC handled\n" );
}

int main( void )
{
    printf( "SJSON (LZ4 %s, zstd %s)\n", SjsonUtil::lz4Available() ? "on" : "off", CSjson::zstdAvailable() ? "on" : "off" );
    std::ifstream file( "sjson_vectors.json", std::ios::binary );
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::unique_ptr<CJsonNode> pVectors( CJsonParser::parse( buffer.str() ) );
    CHECK( pVectors != NULL );

    testBits();
    if ( pVectors )
    {
        testFloatRepr( *pVectors );
        testPythonVectors( *pVectors, "exact", true );
        testPythonVectors( *pVectors, "lz4", false );
        if ( CSjson::zstdAvailable() )
            testPythonVectors( *pVectors, "stream", false );
        testSchemas( *pVectors );
        testCompiledDictionaries( *pVectors );
    }
    testStreamMode();
    testOptionalSender();
    testLink();
    testReviewFixes();
    testFuzz();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return g_nFailures == 0 ? 0 : 1;
}
