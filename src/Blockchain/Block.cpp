/**
 *  Block Class
 *
 *      One block of a blockchain. See Block.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/20/2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Blockchain/Block.h"
#include "Blockchain/MerkleTree.h"

#ifdef IASLIB_JSON_SUPPORT__
#include "JSON/ArrayNode.h"
#include "JSON/ObjectNode.h"
#endif

#include <chrono>
#include <cstring>
#include <limits>

namespace IASLib
{
    namespace
    {
        void putU32( unsigned char *p, uint32_t nValue )
        {
            for ( int n = 0; n < 4; n++ )
                p[ n ] = (unsigned char)( nValue >> ( 24 - 8 * n ) );
        }

        void putU64( unsigned char *p, uint64_t nValue )
        {
            for ( int n = 0; n < 8; n++ )
                p[ n ] = (unsigned char)( nValue >> ( 56 - 8 * n ) );
        }

        uint32_t getU32( const unsigned char *p )
        {
            uint32_t nValue = 0;
            for ( int n = 0; n < 4; n++ )
                nValue = ( nValue << 8 ) | p[ n ];
            return nValue;
        }

        uint64_t getU64( const unsigned char *p )
        {
            uint64_t nValue = 0;
            for ( int n = 0; n < 8; n++ )
                nValue = ( nValue << 8 ) | p[ n ];
            return nValue;
        }

        void fail( std::string *pError, const std::string &strWhy )
        {
            if ( pError )
                *pError = strWhy;
        }

        int64_t nowMilliseconds( void )
        {
            return (int64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch() ).count();
        }
    }

    CBlock::CBlock( void )
    {
        init();
    }

    CBlock::CBlock( const std::vector<CEntry> &aEntries )
    {
        init();
        m_aEntries = aEntries;
        m_merkleRoot = CMerkleTree::Root( m_aEntries );
    }

    CBlock::CBlock( const std::string &strData )
    {
        init();
        m_aEntries.push_back( CEntry( strData.begin(), strData.end() ) );
        m_merkleRoot = CMerkleTree::Root( m_aEntries );
    }

    CBlock::CBlock( uint32_t nIndex, const CString &strData )
    {
        (void)nIndex;
        init();
        const char *pszData = (const char *)strData;
        m_aEntries.push_back( CEntry( pszData, pszData + strData.GetLength() ) );
        m_merkleRoot = CMerkleTree::Root( m_aEntries );
    }

    void CBlock::init( void )
    {
        m_nVersion = FORMAT_VERSION;
        m_nIndex = 0;
        m_llTimestamp = nowMilliseconds();
        m_nDifficulty = 0;
        m_nNonce = 0;
        m_previousHash.fill( 0 );
        m_merkleRoot.fill( 0 );
        m_hash.fill( 0 );
        m_bMined = false;
    }

    void CBlock::place( uint64_t nIndex, const CSha256Digest &previousHash, int64_t llNotBefore )
    {
        m_nIndex = nIndex;
        m_previousHash = previousHash;
            // Never before the previous block, even if the clock went back.
        int64_t llNow = nowMilliseconds();
        m_llTimestamp = ( llNow < llNotBefore ) ? llNotBefore : llNow;
        m_bMined = false;
        m_hash.fill( 0 );
    }

    CString CBlock::getHashString( void ) const
    {
        return CString( CSHA256::ToHex( m_hash ).c_str() );
    }

    CString CBlock::getPrevHash( void ) const
    {
        return CString( CSHA256::ToHex( m_previousHash ).c_str() );
    }

    const CBlock::CEntry &CBlock::getEntry( size_t nIndex ) const
    {
        static const CEntry *pEmpty = new CEntry();
        return ( nIndex < m_aEntries.size() ) ? m_aEntries[ nIndex ] : *pEmpty;
    }

    std::string CBlock::getEntryText( size_t nIndex ) const
    {
        const CEntry &entry = getEntry( nIndex );
        return std::string( entry.begin(), entry.end() );
    }

    std::vector<CSha256Digest> CBlock::getInclusionProof( size_t nIndex ) const
    {
        return CMerkleTree::InclusionProof( m_aEntries, nIndex );
    }

    void CBlock::getHeader( unsigned char achHeader[ HEADER_SIZE ] ) const
    {
        putU32( achHeader, m_nVersion );
        putU64( achHeader + 4, m_nIndex );
        putU64( achHeader + 12, (uint64_t)m_llTimestamp );
        putU32( achHeader + 20, m_nDifficulty );
        memcpy( achHeader + 24, m_previousHash.data(), 32 );
        memcpy( achHeader + 56, m_merkleRoot.data(), 32 );
        putU32( achHeader + 88, (uint32_t)m_aEntries.size() );
        putU64( achHeader + 92, m_nNonce );
    }

    CSha256Digest CBlock::calculateHash( void ) const
    {
        unsigned char achHeader[ HEADER_SIZE ];
        getHeader( achHeader );
        return CSHA256::Digest( achHeader, HEADER_SIZE );
    }

    uint32_t CBlock::LeadingZeroBits( const CSha256Digest &hash )
    {
        uint32_t nBits = 0;
        for ( size_t n = 0; n < hash.size(); n++ )
        {
            if ( hash[ n ] == 0 )
            {
                nBits += 8;
                continue;
            }
            for ( unsigned char chMask = 0x80; chMask && ! ( hash[ n ] & chMask ); chMask >>= 1 )
                nBits++;
            break;
        }
        return nBits;
    }

    bool CBlock::MeetsDifficulty( const CSha256Digest &hash, uint32_t nDifficulty )
    {
        return LeadingZeroBits( hash ) >= nDifficulty;
    }

    bool CBlock::mine( uint32_t nDifficulty, uint64_t nMaxAttempts, const std::atomic<bool> *pCancel )
    {
        if ( nDifficulty > 256 )
            return false;
        m_nDifficulty = nDifficulty;
        m_bMined = false;

            // Everything but the nonce is fixed, and the first 64 bytes (one
            // SHA-256 block) do not include it: hash them once and carry on
            // from there for each nonce.
        unsigned char achHeader[ HEADER_SIZE ];
        m_nNonce = 0;
        getHeader( achHeader );
        CSHA256 prefix;
        prefix.init();
        prefix.updateData( achHeader, (size_t)64 );

        uint64_t nAttempts = 0;
        for ( uint64_t nNonce = 0; ; nNonce++ )
        {
            if ( nMaxAttempts != 0 && nAttempts >= nMaxAttempts )
                return false;
            if ( pCancel != NULL && ( nAttempts & 0xFF ) == 0 && pCancel->load() )
                return false;
            nAttempts++;

            putU64( achHeader + 92, nNonce );
            CSHA256 ctx = prefix;
            ctx.updateData( achHeader + 64, (size_t)( HEADER_SIZE - 64 ) );
            CSha256Digest hash;
            ctx.final( hash.data() );
            if ( MeetsDifficulty( hash, nDifficulty ) )
            {
                m_nNonce = nNonce;
                m_hash = hash;
                m_bMined = true;
                return true;
            }
            if ( nNonce == std::numeric_limits<uint64_t>::max() )
                return false;
        }
    }

    bool CBlock::validate( std::string *pReason ) const
    {
        if ( m_nVersion != FORMAT_VERSION )
        {
            fail( pReason, "unknown block format version " + std::to_string( (unsigned long long)m_nVersion ) );
            return false;
        }
        if ( CMerkleTree::Root( m_aEntries ) != m_merkleRoot )
        {
            fail( pReason, "the entries do not match the Merkle root" );
            return false;
        }
        if ( calculateHash() != m_hash )
        {
            fail( pReason, "the hash is not the hash of the block's header" );
            return false;
        }
        if ( ! MeetsDifficulty( m_hash, m_nDifficulty ) )
        {
            fail( pReason, "the hash does not meet the block's difficulty" );
            return false;
        }
        return true;
    }

    void CBlock::serialize( std::vector<unsigned char> &aOut ) const
    {
        unsigned char achHeader[ HEADER_SIZE ];
        getHeader( achHeader );
        aOut.insert( aOut.end(), achHeader, achHeader + HEADER_SIZE );
        aOut.insert( aOut.end(), m_hash.begin(), m_hash.end() );
        unsigned char achNumber[ 4 ];
        for ( size_t n = 0; n < m_aEntries.size(); n++ )
        {
            putU32( achNumber, (uint32_t)m_aEntries[ n ].size() );
            aOut.insert( aOut.end(), achNumber, achNumber + 4 );
            aOut.insert( aOut.end(), m_aEntries[ n ].begin(), m_aEntries[ n ].end() );
        }
    }

    std::unique_ptr<CBlock> CBlock::Deserialize( const unsigned char *pData, size_t nLength, size_t &nUsed, std::string *pError )
    {
        nUsed = 0;
        const size_t nFixed = HEADER_SIZE + 32;
        if ( pData == NULL || nLength < nFixed )
        {
            fail( pError, "the data ends inside a block header" );
            return std::unique_ptr<CBlock>();
        }
        std::unique_ptr<CBlock> pBlock( new CBlock() );
        pBlock->m_nVersion = getU32( pData );
        pBlock->m_nIndex = getU64( pData + 4 );
        pBlock->m_llTimestamp = (int64_t)getU64( pData + 12 );
        pBlock->m_nDifficulty = getU32( pData + 20 );
        memcpy( pBlock->m_previousHash.data(), pData + 24, 32 );
        memcpy( pBlock->m_merkleRoot.data(), pData + 56, 32 );
        uint32_t nEntries = getU32( pData + 88 );
        pBlock->m_nNonce = getU64( pData + 92 );
        memcpy( pBlock->m_hash.data(), pData + HEADER_SIZE, 32 );

        size_t nPos = nFixed;
        for ( uint32_t n = 0; n < nEntries; n++ )
        {
            if ( nLength - nPos < 4 )
            {
                fail( pError, "the data ends inside a block's entries" );
                return std::unique_ptr<CBlock>();
            }
            uint32_t nSize = getU32( pData + nPos );
            nPos += 4;
            if ( nLength - nPos < nSize )
            {
                fail( pError, "an entry is longer than the data left" );
                return std::unique_ptr<CBlock>();
            }
            pBlock->m_aEntries.push_back( CEntry( pData + nPos, pData + nPos + nSize ) );
            nPos += nSize;
        }
        pBlock->m_bMined = true;
        nUsed = nPos;
        return pBlock;
    }

#ifdef IASLIB_JSON_SUPPORT__
    namespace
    {
        bool readNumber( const CJsonNode &node, const char *pszField, unsigned long long nMax, unsigned long long &nValue, std::string *pError )
        {
            const CJsonNode &field = node.path( pszField );
            if ( ! field.isIntegralNumber() || ! field.canConvertToLong() || field.longValue() < 0 ||
                 (unsigned long long)field.longValue() > nMax )
            {
                fail( pError, std::string( "the block's \"" ) + pszField + "\" is missing or out of range" );
                return false;
            }
            nValue = (unsigned long long)field.longValue();
            return true;
        }

        bool readHash( const CJsonNode &node, const char *pszField, CSha256Digest &hash, std::string *pError )
        {
            const CJsonNode &field = node.path( pszField );
            if ( ! field.isTextual() || ! CSHA256::FromHex( field.textString(), hash ) )
            {
                fail( pError, std::string( "the block's \"" ) + pszField + "\" is not a SHA-256 hash in hexadecimal" );
                return false;
            }
            return true;
        }
    }

    CObjectNode *CBlock::toJson( void ) const
    {
        CObjectNode *pNode = new CObjectNode();
        pNode->put( "version", (unsigned int)m_nVersion );
        pNode->put( "index", (unsigned long long)m_nIndex );
        pNode->put( "timestamp", (long long)m_llTimestamp );
        pNode->put( "difficulty", (unsigned int)m_nDifficulty );
        pNode->put( "nonce", (unsigned long long)m_nNonce );
        pNode->put( "previousHash", CSHA256::ToHex( m_previousHash ) );
        pNode->put( "merkleRoot", CSHA256::ToHex( m_merkleRoot ) );
        pNode->put( "hash", CSHA256::ToHex( m_hash ) );
        CArrayNode *pEntries = pNode->putArray( "entries" );
        for ( size_t n = 0; n < m_aEntries.size(); n++ )
            pEntries->add( CJsonNode::encodeBase64( m_aEntries[ n ] ) );
        return pNode;
    }

    std::unique_ptr<CBlock> CBlock::FromJson( const CJsonNode &node, std::string *pError )
    {
        if ( ! node.isObject() )
        {
            fail( pError, "a block must be a JSON object" );
            return std::unique_ptr<CBlock>();
        }
        std::unique_ptr<CBlock> pBlock( new CBlock() );
        unsigned long long nValue = 0;
        const unsigned long long nMaxSigned = (unsigned long long)std::numeric_limits<int64_t>::max();
        if ( ! readNumber( node, "version", 0xFFFFFFFFULL, nValue, pError ) )
            return std::unique_ptr<CBlock>();
        pBlock->m_nVersion = (uint32_t)nValue;
        if ( ! readNumber( node, "index", nMaxSigned, nValue, pError ) )
            return std::unique_ptr<CBlock>();
        pBlock->m_nIndex = nValue;
        if ( ! readNumber( node, "timestamp", nMaxSigned, nValue, pError ) )
            return std::unique_ptr<CBlock>();
        pBlock->m_llTimestamp = (int64_t)nValue;
        if ( ! readNumber( node, "difficulty", 256, nValue, pError ) )
            return std::unique_ptr<CBlock>();
        pBlock->m_nDifficulty = (uint32_t)nValue;
        if ( ! readNumber( node, "nonce", nMaxSigned, nValue, pError ) )
            return std::unique_ptr<CBlock>();
        pBlock->m_nNonce = nValue;
        if ( ! readHash( node, "previousHash", pBlock->m_previousHash, pError ) ||
             ! readHash( node, "merkleRoot", pBlock->m_merkleRoot, pError ) ||
             ! readHash( node, "hash", pBlock->m_hash, pError ) )
            return std::unique_ptr<CBlock>();

        const CJsonNode &entries = node.path( "entries" );
        if ( ! entries.isArray() )
        {
            fail( pError, "the block's \"entries\" is not an array" );
            return std::unique_ptr<CBlock>();
        }
        for ( int n = 0; n < entries.size(); n++ )
        {
            CEntry entry;
            if ( ! entries.path( n ).isTextual() || ! CJsonNode::decodeBase64( entries.path( n ).textString(), entry ) )
            {
                fail( pError, "entry " + std::to_string( (long long)n ) + " is not base64 text" );
                return std::unique_ptr<CBlock>();
            }
            pBlock->m_aEntries.push_back( entry );
        }
        pBlock->m_bMined = true;
        return pBlock;
    }
#endif
} // namespace IASLib
