/***********************************************************************
**  TestBlockchain
**
**  SHA-256, the RFC 6962 Merkle tree (reference roots and inclusion
**  proofs), blocks (header layout, binary entries, mining limits) and
**  chains: validation, tampering of every kind, JSON and binary saving,
**  mining elsewhere, and several threads adding blocks at once.
***********************************************************************/

#include "IASLib.h"
#include "Blockchain/Block.h"
#include "Blockchain/Blockchain.h"
#include "Blockchain/MerkleTree.h"
#include "Encryption/Sha256.h"

#include <atomic>
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

static CBlock::CEntry bytes( const std::string &strText )
{
    return CBlock::CEntry( strText.begin(), strText.end() );
}

static CBlock::CEntry fromHex( const std::string &strHex )
{
    CBlock::CEntry entry;
    for ( size_t n = 0; n + 1 < strHex.size(); n += 2 )
        entry.push_back( (unsigned char)strtol( strHex.substr( n, 2 ).c_str(), NULL, 16 ) );
    return entry;
}

static bool contains( const std::string &strText, const char *pszPart )
{
    return strText.find( pszPart ) != std::string::npos;
}

/***********************************************************************
 * SHA-256
 ***********************************************************************/

static void testSha256( void )
{
    printf( "SHA-256\n" );
    CHECK( CSHA256::ToHex( CSHA256::Digest( std::string( "" ) ) ) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" );
    CHECK( CSHA256::ToHex( CSHA256::Digest( std::string( "abc" ) ) ) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
    CHECK( CSHA256::ToHex( CSHA256::Digest( std::string( "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" ) ) ) ==
           "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" );
    std::string strMillion( 1000000, 'a' );
    CHECK( CSHA256::ToHex( CSHA256::Digest( strMillion ) ) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0" );
    std::string strNul( "a\0b", 3 );
    CHECK( CSHA256::ToHex( CSHA256::Digest( strNul ) ) == "59b271ae1bbcb1d31d41929817f4b16fb439eb4f31520b5ad1d5ce98920a7138" );
    CString strAbc( "abc" );
    CHECK( CSHA256::StringDigest( strAbc ) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );

        // In pieces of every size, and from a copied prefix.
    std::string strText( 300, 'q' );
    for ( size_t n = 0; n < strText.size(); n++ )
        strText[ n ] = (char)( n * 7 );
    CSha256Digest whole = CSHA256::Digest( strText );
    bool bAllSame = true;
    for ( size_t nPiece = 1; nPiece <= 130; nPiece++ )
    {
        CSHA256 ctx;
        ctx.init();
        for ( size_t n = 0; n < strText.size(); n += nPiece )
            ctx.updateData( strText.data() + n, ( strText.size() - n < nPiece ) ? strText.size() - n : nPiece );
        CSha256Digest digest;
        ctx.final( digest.data() );
        bAllSame = bAllSame && ( digest == whole );
    }
    CHECK( bAllSame );
    CSHA256 prefix;
    prefix.init();
    prefix.updateData( strText.data(), 64 );
    CSHA256 copy = prefix;
    copy.updateData( strText.data() + 64, strText.size() - 64 );
    CSha256Digest fromCopy;
    copy.final( fromCopy.data() );
    CHECK( fromCopy == whole );

    CSha256Digest parsed;
    CHECK( CSHA256::FromHex( CSHA256::ToHex( whole ), parsed ) && parsed == whole );
    CHECK( CSHA256::FromHex( "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD", parsed ) &&
           CSHA256::ToHex( parsed ) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
    CHECK( ! CSHA256::FromHex( "abc", parsed ) );
    CHECK( ! CSHA256::FromHex( std::string( 63, '0' ) + "g", parsed ) );
}

/***********************************************************************
 * Merkle tree
 ***********************************************************************/

static void testMerkleTree( void )
{
    printf( "Merkle tree\n" );
        // RFC 6962 roots for the Certificate Transparency reference leaves.
    const char *leaves[] = { "", "00", "10", "2021", "3031", "40414243", "5051525354555657", "606162636465666768696a6b6c6d6e6f" };
    const char *roots[] = {
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        "6e340b9cffb37a989ca544e6bb780a2c78901d3fb33738768511a30617afa01d",
        "fac54203e7cc696cf0dfcb42c92a1d9dbaf70ad9e621f4bd8d98662f00e3c125",
        "aeb6bcfe274b70a14fb067a5e5578264db0fa9b51af5e0ba159158f329e06e77",
        "d37ee418976dd95753c1c73862b9398fa2a2cf9b4ff0fdfe8b30cd95209614b7",
        "4e3bbb1f7b478dcfe71fb631631519a3bca12c9aefca1612bfce4c13a86264d4",
        "76e67dadbcdf1e10e1b74ddc608abd2f98dfb16fbce75277b5232a127f2087ef",
        "ddb89be403809e325750d3d263cd78929c2942b7942a34b77e122c9594a74c8c",
        "5dc9da79a70659a9ad559cb701ded9a2ab9d823aad2f4960cfe370eff4604328",
    };
    std::vector<CBlock::CEntry> entries;
    CHECK( CSHA256::ToHex( CMerkleTree::Root( entries ) ) == roots[0] );
    for ( size_t n = 0; n < 8; n++ )
    {
        entries.push_back( fromHex( leaves[ n ] ) );
        CHECK( CSHA256::ToHex( CMerkleTree::Root( entries ) ) == roots[ n + 1 ] );
    }

        // Every inclusion proof in trees of 1 to 20 entries verifies, and a
        // changed entry, index or proof does not. (The tree size is the
        // verifier's to know: a block's header records it.)
    bool bAllVerify = true;
    bool bNoForgery = true;
    for ( size_t nSize = 1; nSize <= 20; nSize++ )
    {
        std::vector<CBlock::CEntry> tree;
        for ( size_t n = 0; n < nSize; n++ )
            tree.push_back( bytes( "entry " + std::to_string( (unsigned long long)n ) ) );
        CSha256Digest root = CMerkleTree::Root( tree );
        for ( size_t nIndex = 0; nIndex < nSize; nIndex++ )
        {
            std::vector<CSha256Digest> proof = CMerkleTree::InclusionProof( tree, nIndex );
            bAllVerify = bAllVerify && CMerkleTree::VerifyInclusion( tree[ nIndex ], nIndex, nSize, proof, root );
            bNoForgery = bNoForgery && ! CMerkleTree::VerifyInclusion( bytes( "forged" ), nIndex, nSize, proof, root );
            if ( nSize > 1 )
            {
                bNoForgery = bNoForgery && ! CMerkleTree::VerifyInclusion( tree[ nIndex ], ( nIndex + 1 ) % nSize, nSize, proof, root );
                std::vector<CSha256Digest> altered = proof;
                altered[ 0 ][ 5 ] ^= 1;
                bNoForgery = bNoForgery && ! CMerkleTree::VerifyInclusion( tree[ nIndex ], nIndex, nSize, altered, root );
                std::vector<CSha256Digest> shorter( proof.begin(), proof.end() - 1 );
                bNoForgery = bNoForgery && ! CMerkleTree::VerifyInclusion( tree[ nIndex ], nIndex, nSize, shorter, root );
            }
            std::vector<CSha256Digest> longer = proof;
            longer.push_back( root );
            bNoForgery = bNoForgery && ! CMerkleTree::VerifyInclusion( tree[ nIndex ], nIndex, nSize, longer, root );
        }
        CHECK( CMerkleTree::InclusionProof( tree, nSize ).empty() );
        CHECK( ! CMerkleTree::VerifyInclusion( tree[0], nSize, nSize, CMerkleTree::InclusionProof( tree, 0 ), root ) );
    }
    CHECK( bAllVerify );
    CHECK( bNoForgery );

        // No duplicated last hash (CVE-2012-2459): [a,b,c] and [a,b,c,c] differ.
    std::vector<CBlock::CEntry> three;
    three.push_back( bytes( "a" ) );
    three.push_back( bytes( "b" ) );
    three.push_back( bytes( "c" ) );
    std::vector<CBlock::CEntry> four = three;
    four.push_back( bytes( "c" ) );
    CHECK( CMerkleTree::Root( three ) != CMerkleTree::Root( four ) );

        // A node cannot pass for a leaf: the entry made of two leaf hashes
        // does not hash to their parent.
    CSha256Digest left = CMerkleTree::LeafHash( bytes( "a" ) );
    CSha256Digest right = CMerkleTree::LeafHash( bytes( "b" ) );
    CBlock::CEntry joined( left.begin(), left.end() );
    joined.insert( joined.end(), right.begin(), right.end() );
    CHECK( CMerkleTree::LeafHash( joined ) != CMerkleTree::NodeHash( left, right ) );
    std::vector<CBlock::CEntry> pair( three.begin(), three.begin() + 2 );
    CHECK( CMerkleTree::Root( pair ) == CMerkleTree::NodeHash( left, right ) );
}

/***********************************************************************
 * Blocks
 ***********************************************************************/

static void testBlocks( void )
{
    printf( "Blocks\n" );
    CHECK( CBlock::LeadingZeroBits( CSHA256::Digest( std::string( "abc" ) ) ) == 0 );     // ba...
    CSha256Digest zeros;
    zeros.fill( 0 );
    CHECK( CBlock::LeadingZeroBits( zeros ) == 256 );
    zeros[2] = 0x10;
    CHECK( CBlock::LeadingZeroBits( zeros ) == 19 && CBlock::MeetsDifficulty( zeros, 19 ) && ! CBlock::MeetsDifficulty( zeros, 20 ) );

    std::vector<CBlock::CEntry> entries;
    entries.push_back( bytes( std::string( "binary\0data\xff", 12 ) ) );
    entries.push_back( bytes( "second" ) );
    CBlock block( entries );
    CHECK( block.getEntryCount() == 2 && block.getEntry( 0 ).size() == 12 && block.getEntry( 0 )[6] == 0 );
    CHECK( block.getEntryText( 1 ) == "second" && block.getEntry( 9 ).empty() );
    CHECK( block.getMerkleRoot() == CMerkleTree::Root( entries ) );
    CHECK( ! block.isMined() );
    std::string strReason;
    CHECK( ! block.validate( &strReason ) );                                     // not mined

    CHECK( block.mine( 12 ) && block.isMined() );
    CHECK( CBlock::LeadingZeroBits( block.getHash() ) >= 12 && block.getDifficulty() == 12 );
    CHECK( block.calculateHash() == block.getHash() && block.validate( &strReason ) );
    CHECK( block.getHashString() == CSHA256::ToHex( block.getHash() ).c_str() );

        // The header layout: fixed fields, the nonce last.
    unsigned char achHeader[ CBlock::HEADER_SIZE ];
    block.getHeader( achHeader );
    CHECK( achHeader[0] == 0 && achHeader[3] == CBlock::FORMAT_VERSION );
    CHECK( achHeader[23] == 12 && memcmp( achHeader + 56, block.getMerkleRoot().data(), 32 ) == 0 );
    CHECK( achHeader[88] == 0 && achHeader[91] == 2 );                          // the entry count
    uint64_t nNonce = 0;
    for ( int n = 0; n < 8; n++ )
        nNonce = ( nNonce << 8 ) | achHeader[ 92 + n ];
    CHECK( nNonce == block.getNonce() );
    CHECK( CSHA256::Digest( achHeader, sizeof( achHeader ) ) == block.getHash() );

        // Entries are bytes with their lengths: "ab" + "12" is not "ab1" + "2",
        // and a NUL does not end an entry.
    std::vector<CBlock::CEntry> a;
    a.push_back( bytes( "ab" ) );
    a.push_back( bytes( "12" ) );
    std::vector<CBlock::CEntry> b;
    b.push_back( bytes( "ab1" ) );
    b.push_back( bytes( "2" ) );
    CHECK( CMerkleTree::Root( a ) != CMerkleTree::Root( b ) );
    CHECK( CBlock( std::string( "a\0b", 3 ) ).getMerkleRoot() != CBlock( std::string( "a" ) ).getMerkleRoot() );

        // Inclusion proofs for a block's entries.
    std::vector<CSha256Digest> proof = block.getInclusionProof( 0 );
    CHECK( CMerkleTree::VerifyInclusion( block.getEntry( 0 ), 0, block.getEntryCount(), proof, block.getMerkleRoot() ) );

        // Limits: attempts, cancellation.
    CBlock hard( std::string( "hard" ) );
    CHECK( ! hard.mine( 64, 1000 ) && ! hard.isMined() );
    std::atomic<bool> cancel( true );
    auto start = std::chrono::steady_clock::now();
    CHECK( ! hard.mine( 64, 0, &cancel ) );
    CHECK( std::chrono::steady_clock::now() - start < std::chrono::seconds( 1 ) );
    CHECK( ! hard.mine( 257 ) );

        // Difficulties past the nonce's 64 bits are refused at once rather
        // than searched forever.
    CHECK( CBlock::MAX_DIFFICULTY == 64 );
    start = std::chrono::steady_clock::now();
    CHECK( ! hard.mine( 256 ) && ! hard.mine( 65 ) && ! hard.isMined() );
    CHECK( std::chrono::steady_clock::now() - start < std::chrono::seconds( 1 ) );

#ifdef IASLIB_JSON_SUPPORT__
        // Nonces use all 64 bits, and come back from JSON exactly.
    const unsigned long long nonces[] = { 0ULL, 9223372036854775807ULL, 9223372036854775808ULL, 18446744073709551615ULL };
    for ( size_t n = 0; n < sizeof( nonces ) / sizeof( nonces[0] ); n++ )
    {
        std::unique_ptr<CObjectNode> pJson( block.toJson() );
        pJson->put( "nonce", nonces[ n ] );
        std::string strText = pJson->toJson();
        std::unique_ptr<CJsonNode> pParsed( CJsonParser::parse( strText ) );
        std::string strError;
        std::unique_ptr<CBlock> pBack = pParsed ? CBlock::FromJson( *pParsed, &strError ) : std::unique_ptr<CBlock>();
        CHECK( pBack && pBack->getNonce() == nonces[ n ] );
        if ( ! pBack )
            printf( "  (nonce %llu: %s)\n", nonces[ n ], strError.c_str() );
        std::unique_ptr<CObjectNode> pAgain( pBack ? pBack->toJson() : NULL );
        CHECK( pAgain && pAgain->toJson() == strText );
    }
    {
            // One past the 64-bit range is refused.
        std::unique_ptr<CObjectNode> pJson( block.toJson() );
        std::string strText = pJson->toJson();
        size_t nAt = strText.find( "\"nonce\":" );
        size_t nEnd = strText.find( ',', nAt );
        strText.replace( nAt, nEnd - nAt, "\"nonce\":18446744073709551616" );
        std::unique_ptr<CJsonNode> pParsed( CJsonParser::parse( strText ) );
        std::string strError;
        CHECK( pParsed && ! CBlock::FromJson( *pParsed, &strError ) && contains( strError, "nonce" ) );
    }
        // A mined block keeps its nonce through JSON.
    {
        std::unique_ptr<CObjectNode> pJson( block.toJson() );
        std::unique_ptr<CJsonNode> pParsed( CJsonParser::parse( pJson->toJson() ) );
        std::unique_ptr<CBlock> pBack = pParsed ? CBlock::FromJson( *pParsed ) : std::unique_ptr<CBlock>();
        CHECK( pBack && pBack->getNonce() == block.getNonce() && pBack->validate() );
    }
#endif
}

/***********************************************************************
 * Chains
 ***********************************************************************/

    // A tampered copy of a chain: the JSON changed by fChange, read back
    // without validation.
template <typename Change>
static std::unique_ptr<CBlockchain> tampered( const CBlockchain &chain, Change fChange )
{
    std::unique_ptr<CObjectNode> pJson( chain.toJson() );
    CArrayNode *pBlocks = static_cast<CArrayNode *>( pJson->get( "blocks" ) );
    fChange( *pBlocks );
    std::string strError;
    std::unique_ptr<CBlockchain> pChain = CBlockchain::FromJson( *pJson, &strError, false );
    if ( ! pChain )
        printf( "  (could not read the tampered chain: %s)\n", strError.c_str() );
    return pChain;
}

static CObjectNode *blockAt( CArrayNode &blocks, int n )
{
    return static_cast<CObjectNode *>( blocks.get( n ) );
}

    // Re-mines a block given as JSON (after it was changed), as an attacker
    // would, and puts it back.
static void remine( CArrayNode &blocks, int n, uint32_t nDifficulty )
{
    CObjectNode *pBlock = blockAt( blocks, n );
    std::unique_ptr<CBlock> pParsed = CBlock::FromJson( *pBlock );
    std::vector<CBlock::CEntry> aEntries = pParsed->getEntries();
    pBlock->put( "merkleRoot", CSHA256::ToHex( CMerkleTree::Root( aEntries ) ) );
    pParsed = CBlock::FromJson( *pBlock );
    pParsed->mine( nDifficulty );
    blocks.set( n, pParsed->toJson() );
}

static bool failsAt( const CBlockchain *pChain, size_t nBlock, const char *pszReason )
{
    if ( pChain == NULL )
        return false;
    std::string strReason;
    size_t nBad = 999;
    bool bValid = pChain->validate( &strReason, &nBad );
    if ( bValid || nBad != nBlock || ! contains( strReason, pszReason ) )
    {
        printf( "  (validation gave %s at %lu: %s)\n", bValid ? "valid" : "invalid", (unsigned long)nBad, strReason.c_str() );
        return false;
    }
    return true;
}

static void testChains( void )
{
    printf( "Chains\n" );
    CBlockchain chain( 8 );
    CHECK( chain.size() == 1 && chain.getDifficulty() == 8 );
    const CBlock *pGenesis = chain.getBlock( 0 );
    CSha256Digest zeros;
    zeros.fill( 0 );
    CHECK( pGenesis && pGenesis->isMined() && pGenesis->getPreviousHash() == zeros && pGenesis->getEntryText( 0 ) == "Genesis Block" );
    CHECK( chain.validate() );

    CHECK( chain.addBlock( "first" ) != NULL );
    CHECK( chain.addBlock( std::string( "second\0with a NUL", 17 ) ) != NULL );
    std::vector<CBlock::CEntry> entries;
    for ( int n = 0; n < 5; n++ )
        entries.push_back( bytes( "transaction " + std::to_string( (long long)n ) ) );
    const CBlock *pFourth = chain.addBlock( entries );
    CHECK( pFourth != NULL && pFourth->getEntryCount() == 5 );
    CHECK( chain.addBlock( new CBlock( 7, CString( "legacy interface" ) ) ) );       // the index given is ignored
    CHECK( chain.size() == 5 && chain.getBlock( 5 ) == NULL && chain.getLastBlock() == chain.getBlock( 4 ) );
    for ( size_t n = 0; n < chain.size(); n++ )
    {
        CHECK( chain.getBlock( n )->getIndex() == n && chain.getBlock( n )->getDifficulty() == 8 );
        if ( n > 0 )
        {
            CHECK( chain.getBlock( n )->getPreviousHash() == chain.getBlock( n - 1 )->getHash() );
            CHECK( chain.getBlock( n )->getPrevHash() == chain.getBlock( n - 1 )->getHashString() );
            CHECK( chain.getBlock( n )->getTimestamp() >= chain.getBlock( n - 1 )->getTimestamp() );
        }
    }
    CHECK( chain.getBlock( 2 )->getEntry( 0 ).size() == 17 );
    std::string strReason;
    CHECK( chain.validate( &strReason ) );
    std::vector<CSha256Digest> proof = pFourth->getInclusionProof( 3 );
    CHECK( CMerkleTree::VerifyInclusion( bytes( "transaction 3" ), 3, 5, proof, pFourth->getMerkleRoot() ) );

        // The required difficulty is the reader's to choose.
    size_t nBad = 0;
    CHECK( ! chain.validate( 9, &strReason, &nBad ) && nBad == 0 && contains( strReason, "below the required 9" ) );
    chain.setDifficulty( 999 );
    CHECK( chain.getDifficulty() == CBlockchain::MAX_DIFFICULTY && CBlockchain::MAX_DIFFICULTY == 64 );
    chain.setDifficulty( 8 );

        // Tampering, each caught at the block it was done to (or the next).
    std::unique_ptr<CBlockchain> pBad = tampered( chain, []( CArrayNode &blocks ) {
        CArrayNode *pEntries = static_cast<CArrayNode *>( blockAt( blocks, 2 )->get( "entries" ) );
        pEntries->set( 0, new CTextNode( CJsonNode::encodeBase64( bytes( "changed" ) ) ) );
    } );
    CHECK( failsAt( pBad.get(), 2, "Merkle root" ) );

    pBad = tampered( chain, []( CArrayNode &blocks ) {          // and the root updated to match
        CArrayNode *pEntries = static_cast<CArrayNode *>( blockAt( blocks, 2 )->get( "entries" ) );
        pEntries->set( 0, new CTextNode( CJsonNode::encodeBase64( bytes( "changed" ) ) ) );
        std::unique_ptr<CBlock> pBlock = CBlock::FromJson( *blockAt( blocks, 2 ) );
        std::vector<CBlock::CEntry> aEntries = pBlock->getEntries();
        blockAt( blocks, 2 )->put( "merkleRoot", CSHA256::ToHex( CMerkleTree::Root( aEntries ) ) );
    } );
    CHECK( failsAt( pBad.get(), 2, "not the hash of the block's header" ) );

    pBad = tampered( chain, []( CArrayNode &blocks ) {          // and re-mined: the next link breaks
        CArrayNode *pEntries = static_cast<CArrayNode *>( blockAt( blocks, 2 )->get( "entries" ) );
        pEntries->set( 0, new CTextNode( CJsonNode::encodeBase64( bytes( "changed" ) ) ) );
        remine( blocks, 2, 8 );
    } );
    CHECK( failsAt( pBad.get(), 3, "does not link" ) );

    pBad = tampered( chain, []( CArrayNode &blocks ) {
        blockAt( blocks, 3 )->put( "nonce", (unsigned long long)( blockAt( blocks, 3 )->path( "nonce" ).longValue() + 1 ) );
    } );
    CHECK( failsAt( pBad.get(), 3, "not the hash" ) );

    pBad = tampered( chain, []( CArrayNode &blocks ) {          // swap blocks 1 and 2
        CJsonNode *pOne = blocks.release( 1 );
        blocks.insert( 2, pOne );
    } );
    CHECK( failsAt( pBad.get(), 1, "index" ) );

    pBad = tampered( chain, []( CArrayNode &blocks ) { blocks.remove( 2 ); } );
    CHECK( failsAt( pBad.get(), 2, "index" ) );

    pBad = tampered( chain, []( CArrayNode &blocks ) {          // claim less work: the hash changes
        blockAt( blocks, 3 )->put( "difficulty", 0u );
    } );
    CHECK( failsAt( pBad.get(), 3, "not the hash" ) );

    pBad = tampered( chain, []( CArrayNode &blocks ) {          // a block earlier than the one before
        blockAt( blocks, 1 )->put( "timestamp", (long long)( blockAt( blocks, 0 )->path( "timestamp" ).longValue() - 1000 ) );
        remine( blocks, 1, 8 );
        while ( blocks.size() > 2 )
            blocks.remove( 2 );
    } );
    CHECK( failsAt( pBad.get(), 1, "earlier" ) );

    pBad = tampered( chain, []( CArrayNode &blocks ) {          // a genesis block that links somewhere
        blockAt( blocks, 0 )->put( "previousHash", std::string( 63, '0' ) + "1" );
        remine( blocks, 0, 8 );
    } );
    CHECK( failsAt( pBad.get(), 0, "genesis" ) );

        // A whole chain redone with no work passes its own (saved) difficulty
        // of 0, but not the difficulty the reader requires.
    std::unique_ptr<CObjectNode> pJson( chain.toJson() );
    pJson->put( "difficulty", 0u );
    CArrayNode *pBlocks = static_cast<CArrayNode *>( pJson->get( "blocks" ) );
    std::string strPrevious = CSHA256::ToHex( zeros );
    for ( int n = 0; n < pBlocks->size(); n++ )
    {
        blockAt( *pBlocks, n )->put( "previousHash", strPrevious );
        std::unique_ptr<CBlock> pBlock = CBlock::FromJson( *blockAt( *pBlocks, n ) );
        pBlock->mine( 0 );
        strPrevious = CSHA256::ToHex( pBlock->getHash() );
        pBlocks->set( n, pBlock->toJson() );
    }
    std::unique_ptr<CBlockchain> pCheap = CBlockchain::FromJson( *pJson, NULL, false );
    CHECK( pCheap && pCheap->validate() );
    CHECK( pCheap && ! pCheap->validate( 8, &strReason ) && contains( strReason, "below the required" ) );
}

static void testSaving( void )
{
    printf( "Saving and loading\n" );
    CBlockchain chain( 8, "origin" );
    chain.addBlock( "one" );
    chain.addBlock( std::string( "two\0bytes\x01\x02", 12 ) );

        // JSON, both ways.
    std::string strJson = chain.toJsonText();
    CHECK( contains( strJson, "\"format\": \"IASLib blockchain\"" ) && contains( strJson, "\"merkleRoot\"" ) );
    std::string strError;
    std::unique_ptr<CBlockchain> pCopy = CBlockchain::FromJsonText( strJson, &strError );
    CHECK( pCopy && pCopy->size() == 3 && pCopy->validate() );
    CHECK( pCopy && pCopy->toJsonText() == strJson && pCopy->getDifficulty() == 8 );
    CHECK( pCopy && pCopy->getBlock( 2 )->getEntry( 0 ) == chain.getBlock( 2 )->getEntry( 0 ) );
    CHECK( pCopy && pCopy->getBlock( 0 )->getEntryText( 0 ) == "origin" );
    CHECK( chain.toJsonText( false ).find( '\n' ) == std::string::npos );

        // A loaded chain can be added to.
    CHECK( pCopy && pCopy->addBlock( "three" ) && pCopy->size() == 4 && pCopy->validate() );

        // Bad JSON is refused with a reason.
    const char *bad[] = {
        "not json", "[]", "{\"format\":\"something else\"}",
        "{\"format\":\"IASLib blockchain\",\"version\":2,\"difficulty\":8,\"blocks\":[]}",
        "{\"format\":\"IASLib blockchain\",\"version\":1,\"difficulty\":-1,\"blocks\":[]}",
        "{\"format\":\"IASLib blockchain\",\"version\":1,\"difficulty\":8,\"blocks\":{}}",
        "{\"format\":\"IASLib blockchain\",\"version\":1,\"difficulty\":8,\"blocks\":[{\"index\":0}]}",
        "{\"format\":\"IASLib blockchain\",\"version\":1,\"difficulty\":8,\"blocks\":[]}",     // no blocks: not valid
    };
    for ( size_t n = 0; n < sizeof( bad ) / sizeof( bad[0] ); n++ )
    {
        strError.clear();
        CHECK( ! CBlockchain::FromJsonText( bad[ n ], &strError ) && ! strError.empty() );
    }
        // A difficulty no block could be mined at is refused.
    CHECK( ! CBlockchain::FromJsonText( "{\"format\":\"IASLib blockchain\",\"version\":1,\"difficulty\":65,\"blocks\":[]}", &strError ) &&
           contains( strError, "difficulty" ) );
    std::string strBadEntry = strJson;
    size_t nEntries = strBadEntry.find( "\"entries\": [" );
    strBadEntry.insert( strBadEntry.find( '"', nEntries + 12 ) + 1, "!!" );
    CHECK( ! CBlockchain::FromJsonText( strBadEntry, &strError ) && contains( strError, "base64" ) );

        // Binary, both ways.
    std::vector<unsigned char> aData = chain.serialize();
    std::unique_ptr<CBlockchain> pBinary = CBlockchain::Deserialize( aData, &strError );
    CHECK( pBinary && pBinary->validate() && pBinary->toJsonText() == strJson );

        // Cut short anywhere: refused. Any single byte changed (except the
        // saved difficulty, which is not part of any hash): refused.
    bool bAllRefused = true;
    for ( size_t nLength = 0; nLength < aData.size(); nLength++ )
    {
        std::vector<unsigned char> aShort( aData.begin(), aData.begin() + nLength );
        bAllRefused = bAllRefused && ! CBlockchain::Deserialize( aShort );
    }
    CHECK( bAllRefused );
    bool bAllCaught = true;
    for ( size_t nByte = 0; nByte < aData.size(); nByte++ )
    {
        if ( nByte >= 8 && nByte < 12 )
            continue;
        std::vector<unsigned char> aFlipped = aData;
        aFlipped[ nByte ] ^= 0x20;
        if ( CBlockchain::Deserialize( aFlipped ) )
        {
            bAllCaught = false;
            printf( "  (a change at byte %lu was not caught)\n", (unsigned long)nByte );
        }
    }
    CHECK( bAllCaught );
    std::vector<unsigned char> aLonger = aData;
    aLonger.push_back( 0 );
    CHECK( ! CBlockchain::Deserialize( aLonger, &strError ) && contains( strError, "after the last block" ) );
    std::vector<unsigned char> aHuge = aData;
    aHuge[ 20 + 88 ] = 0xFF;                  // the genesis block's entry count
    CHECK( ! CBlockchain::Deserialize( aHuge, &strError ) );
}

static void testMiningControl( void )
{
    printf( "Mining control and threads\n" );
    CBlockchain chain( 8 );

        // Limits leave the chain as it was.
    chain.setDifficulty( 64 );
    CHECK( chain.addBlock( "too hard", 2000 ) == NULL && chain.size() == 1 );
    std::atomic<bool> cancel( false );
    std::chrono::steady_clock::time_point cancelledAt;
    std::thread canceller( [&cancel, &cancelledAt] {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
        cancelledAt = std::chrono::steady_clock::now();
        cancel = true;
    } );
    CHECK( chain.addBlock( "cancelled", 0, &cancel ) == NULL );
    auto stoppedAt = std::chrono::steady_clock::now();
    canceller.join();
        // Measured from when the flag was set (a thread may start late).
    CHECK( stoppedAt - cancelledAt < std::chrono::seconds( 2 ) );
    CHECK( chain.size() == 1 );
    chain.setDifficulty( 8 );

        // Mining on another thread, then appending.
    std::unique_ptr<CBlock> pNext = chain.createNextBlock( std::vector<CBlock::CEntry>( 1, bytes( "mined elsewhere" ) ) );
    CHECK( pNext && pNext->getIndex() == 1 && pNext->getPreviousHash() == chain.getLastBlock()->getHash() );
    std::string strReason;
    CHECK( ! chain.appendBlock( pNext, &strReason ) && contains( strReason, "not been mined" ) && pNext );
    std::thread miner( [&pNext] { pNext->mine( 8 ); } );
    miner.join();
    CHECK( chain.appendBlock( pNext, &strReason ) && ! pNext && chain.size() == 2 );

        // A block made before another was appended no longer follows; one
        // mined below the chain's difficulty is refused.
    std::unique_ptr<CBlock> pStale = chain.createNextBlock( std::vector<CBlock::CEntry>( 1, bytes( "stale" ) ) );
    pStale->mine( 8 );
    chain.addBlock( "meanwhile" );
    CHECK( ! chain.appendBlock( pStale, &strReason ) && contains( strReason, "does not follow" ) && pStale );
    std::unique_ptr<CBlock> pWeak = chain.createNextBlock( std::vector<CBlock::CEntry>( 1, bytes( "weak" ) ) );
    pWeak->mine( 2 );
    CHECK( ( CBlock::LeadingZeroBits( pWeak->getHash() ) >= 8 ) ||
           ( ! chain.appendBlock( pWeak, &strReason ) && contains( strReason, "below the chain's" ) ) );
    std::unique_ptr<CBlock> pNone;
    CHECK( ! chain.appendBlock( pNone, &strReason ) );
    CHECK( chain.validate() );

        // Several threads adding at once.
    size_t nBefore = chain.size();
    std::vector<std::thread> threads;
    for ( int t = 0; t < 4; t++ )
    {
        threads.push_back( std::thread( [&chain, t] {
            for ( int n = 0; n < 5; n++ )
                chain.addBlock( "thread " + std::to_string( (long long)t ) + " block " + std::to_string( (long long)n ) );
        } ) );
    }
    for ( size_t t = 0; t < threads.size(); t++ )
        threads[ t ].join();
    CHECK( chain.size() == nBefore + 20 );
    CHECK( chain.validate( &strReason ) );
}

int main( void )
{
    setvbuf( stdout, NULL, _IONBF, 0 );
    testSha256();
    testMerkleTree();
    testBlocks();
    testChains();
    testSaving();
    testMiningControl();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
