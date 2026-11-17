/**
 *  Blockchain Class
 *
 *      A chain of mined blocks. See Blockchain.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/20/2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Blockchain/Blockchain.h"

#ifdef IASLIB_JSON_SUPPORT__
#include "JSON/ArrayNode.h"
#include "JSON/JsonParser.h"
#include "JSON/ObjectNode.h"
#endif

#include <cstring>

namespace IASLib
{
    namespace
    {
        const char FORMAT_MAGIC[ 4 ] = { 'I', 'A', 'S', 'C' };
        const uint32_t CHAIN_FORMAT_VERSION = 1;
        const char *JSON_FORMAT_NAME = "IASLib blockchain";

        void fail( std::string *pError, const std::string &strWhy )
        {
            if ( pError )
                *pError = strWhy;
        }

        void putU32( std::vector<unsigned char> &aOut, uint32_t nValue )
        {
            for ( int n = 0; n < 4; n++ )
                aOut.push_back( (unsigned char)( nValue >> ( 24 - 8 * n ) ) );
        }

        void putU64( std::vector<unsigned char> &aOut, uint64_t nValue )
        {
            for ( int n = 0; n < 8; n++ )
                aOut.push_back( (unsigned char)( nValue >> ( 56 - 8 * n ) ) );
        }

        uint64_t getU( const unsigned char *p, int nBytes )
        {
            uint64_t nValue = 0;
            for ( int n = 0; n < nBytes; n++ )
                nValue = ( nValue << 8 ) | p[ n ];
            return nValue;
        }
    }

    CBlockchain::CBlockchain( uint32_t nDifficulty, const std::string &strGenesisData ) : m_nDifficulty( 0 )
    {
        setDifficulty( nDifficulty );
        std::unique_ptr<CBlock> pGenesis( new CBlock( strGenesisData ) );
        CSha256Digest zeros;
        zeros.fill( 0 );
        pGenesis->place( 0, zeros, 0 );
        pGenesis->mine( m_nDifficulty );
        m_aBlocks.push_back( std::move( pGenesis ) );
    }

    CBlockchain::CBlockchain( CEmpty ) : m_nDifficulty( DEFAULT_DIFFICULTY )
    {
    }

    CBlockchain::~CBlockchain( void )
    {
    }

    size_t CBlockchain::size( void ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return m_aBlocks.size();
    }

    const CBlock *CBlockchain::getBlock( size_t nIndex ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return ( nIndex < m_aBlocks.size() ) ? m_aBlocks[ nIndex ].get() : NULL;
    }

    const CBlock *CBlockchain::getLastBlock( void ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return m_aBlocks.empty() ? NULL : m_aBlocks.back().get();
    }

    void CBlockchain::placeNext( CBlock &block ) const
    {
        if ( m_aBlocks.empty() )
        {
            CSha256Digest zeros;
            zeros.fill( 0 );
            block.place( 0, zeros, 0 );
        }
        else
        {
            const CBlock &last = *m_aBlocks.back();
            block.place( last.getIndex() + 1, last.getHash(), last.getTimestamp() );
        }
    }

    std::unique_ptr<CBlock> CBlockchain::createNextBlock( const std::vector<CBlock::CEntry> &aEntries ) const
    {
        std::unique_ptr<CBlock> pBlock( new CBlock( aEntries ) );
        std::lock_guard<std::mutex> lock( m_mutex );
        placeNext( *pBlock );
        return pBlock;
    }

    bool CBlockchain::appendBlock( std::unique_ptr<CBlock> &pBlock, std::string *pReason )
    {
        if ( ! pBlock )
        {
            fail( pReason, "no block" );
            return false;
        }
        std::string strReason;
        if ( ! pBlock->isMined() || ! pBlock->validate( &strReason ) )
        {
            fail( pReason, pBlock->isMined() ? strReason : std::string( "the block has not been mined" ) );
            return false;
        }
        if ( pBlock->getDifficulty() < m_nDifficulty )
        {
            fail( pReason, "the block's difficulty is below the chain's" );
            return false;
        }

        std::lock_guard<std::mutex> lock( m_mutex );
        uint64_t nExpected = m_aBlocks.size();
        if ( pBlock->getIndex() != nExpected ||
             ( ! m_aBlocks.empty() && pBlock->getPreviousHash() != m_aBlocks.back()->getHash() ) )
        {
            fail( pReason, "the block does not follow the chain's last block" );
            return false;
        }
        if ( ! m_aBlocks.empty() && pBlock->getTimestamp() < m_aBlocks.back()->getTimestamp() )
        {
            fail( pReason, "the block is earlier than the chain's last block" );
            return false;
        }
        m_aBlocks.push_back( std::move( pBlock ) );
        return true;
    }

    const CBlock *CBlockchain::addBlock( const std::vector<CBlock::CEntry> &aEntries, uint64_t nMaxAttempts, const std::atomic<bool> *pCancel )
    {
        std::unique_ptr<CBlock> pBlock( new CBlock( aEntries ) );
        while ( true )
        {
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                placeNext( *pBlock );
            }
                // Mine without the lock, so other threads can read (or add).
            if ( ! pBlock->mine( m_nDifficulty, nMaxAttempts, pCancel ) )
                return NULL;
            CBlock *pResult = pBlock.get();
            if ( appendBlock( pBlock ) )
                return pResult;
                // Another block was added meanwhile: place after it, mine again.
        }
    }

    const CBlock *CBlockchain::addBlock( const std::string &strData, uint64_t nMaxAttempts, const std::atomic<bool> *pCancel )
    {
        std::vector<CBlock::CEntry> aEntries( 1, CBlock::CEntry( strData.begin(), strData.end() ) );
        return addBlock( aEntries, nMaxAttempts, pCancel );
    }

    bool CBlockchain::addBlock( CBlock *pNew )
    {
        std::unique_ptr<CBlock> pBlock( pNew );
        if ( ! pBlock )
            return false;
        while ( true )
        {
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                placeNext( *pBlock );
            }
            pBlock->mine( m_nDifficulty );
            if ( appendBlock( pBlock ) )
                return true;
        }
    }

    bool CBlockchain::validate( std::string *pReason, size_t *pBadBlock ) const
    {
        return validate( m_nDifficulty, pReason, pBadBlock );
    }

    bool CBlockchain::validate( uint32_t nRequiredDifficulty, std::string *pReason, size_t *pBadBlock ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        if ( m_aBlocks.empty() )
        {
            fail( pReason, "the chain has no blocks" );
            if ( pBadBlock )
                *pBadBlock = 0;
            return false;
        }
        CSha256Digest zeros;
        zeros.fill( 0 );
        for ( size_t n = 0; n < m_aBlocks.size(); n++ )
        {
            const CBlock &block = *m_aBlocks[ n ];
            std::string strReason;
            bool bOk = block.validate( &strReason );
            if ( bOk && block.getIndex() != n )
            {
                strReason = "the block's index is " + std::to_string( (unsigned long long)block.getIndex() );
                bOk = false;
            }
            if ( bOk && block.getDifficulty() < nRequiredDifficulty )
            {
                strReason = "the block's difficulty (" + std::to_string( (unsigned long long)block.getDifficulty() ) +
                            ") is below the required " + std::to_string( (unsigned long long)nRequiredDifficulty );
                bOk = false;
            }
            if ( bOk && n == 0 && block.getPreviousHash() != zeros )
            {
                strReason = "the genesis block links to a previous block";
                bOk = false;
            }
            if ( bOk && n > 0 && block.getPreviousHash() != m_aBlocks[ n - 1 ]->getHash() )
            {
                strReason = "the block does not link to the hash of the block before it";
                bOk = false;
            }
            if ( bOk && n > 0 && block.getTimestamp() < m_aBlocks[ n - 1 ]->getTimestamp() )
            {
                strReason = "the block is earlier than the block before it";
                bOk = false;
            }
            if ( ! bOk )
            {
                fail( pReason, "block " + std::to_string( (unsigned long long)n ) + ": " + strReason );
                if ( pBadBlock )
                    *pBadBlock = n;
                return false;
            }
        }
        return true;
    }

    std::vector<unsigned char> CBlockchain::serialize( void ) const
    {
        std::vector<unsigned char> aOut( FORMAT_MAGIC, FORMAT_MAGIC + 4 );
        putU32( aOut, CHAIN_FORMAT_VERSION );
        putU32( aOut, m_nDifficulty );
        std::lock_guard<std::mutex> lock( m_mutex );
        putU64( aOut, m_aBlocks.size() );
        for ( size_t n = 0; n < m_aBlocks.size(); n++ )
            m_aBlocks[ n ]->serialize( aOut );
        return aOut;
    }

    std::unique_ptr<CBlockchain> CBlockchain::Deserialize( const std::vector<unsigned char> &aData, std::string *pError, bool bValidate )
    {
        const size_t nHeader = 4 + 4 + 4 + 8;
        if ( aData.size() < nHeader || memcmp( &aData[0], FORMAT_MAGIC, 4 ) != 0 )
        {
            fail( pError, "the data is not a saved blockchain" );
            return std::unique_ptr<CBlockchain>();
        }
        const unsigned char *pData = &aData[0];
        if ( getU( pData + 4, 4 ) != CHAIN_FORMAT_VERSION )
        {
            fail( pError, "unknown blockchain format version" );
            return std::unique_ptr<CBlockchain>();
        }
        std::unique_ptr<CBlockchain> pChain( new CBlockchain( CEmpty() ) );
        pChain->setDifficulty( (uint32_t)getU( pData + 8, 4 ) );
        uint64_t nBlocks = getU( pData + 12, 8 );

        size_t nPos = nHeader;
        for ( uint64_t n = 0; n < nBlocks; n++ )
        {
            size_t nUsed = 0;
            std::string strError;
            std::unique_ptr<CBlock> pBlock = CBlock::Deserialize( pData + nPos, aData.size() - nPos, nUsed, &strError );
            if ( ! pBlock )
            {
                fail( pError, "block " + std::to_string( (unsigned long long)n ) + ": " + strError );
                return std::unique_ptr<CBlockchain>();
            }
            pChain->m_aBlocks.push_back( std::move( pBlock ) );
            nPos += nUsed;
        }
        if ( nPos != aData.size() )
        {
            fail( pError, "there is data after the last block" );
            return std::unique_ptr<CBlockchain>();
        }
        if ( bValidate && ! pChain->validate( pError ) )
            return std::unique_ptr<CBlockchain>();
        return pChain;
    }

#ifdef IASLIB_JSON_SUPPORT__
    CObjectNode *CBlockchain::toJson( void ) const
    {
        CObjectNode *pNode = new CObjectNode();
        pNode->put( "format", JSON_FORMAT_NAME );
        pNode->put( "version", (unsigned int)CHAIN_FORMAT_VERSION );
        pNode->put( "difficulty", (unsigned int)m_nDifficulty );
        CArrayNode *pBlocks = pNode->putArray( "blocks" );
        std::lock_guard<std::mutex> lock( m_mutex );
        for ( size_t n = 0; n < m_aBlocks.size(); n++ )
            pBlocks->add( m_aBlocks[ n ]->toJson() );
        return pNode;
    }

    std::string CBlockchain::toJsonText( bool bPretty ) const
    {
        std::unique_ptr<CObjectNode> pNode( toJson() );
        return pNode->toJson( bPretty ? 2 : -1 );
    }

    std::unique_ptr<CBlockchain> CBlockchain::FromJson( const CJsonNode &node, std::string *pError, bool bValidate )
    {
        if ( ! node.isObject() || node.path( "format" ).textString() != JSON_FORMAT_NAME )
        {
            fail( pError, "the JSON is not a saved blockchain" );
            return std::unique_ptr<CBlockchain>();
        }
        if ( ! node.path( "version" ).isIntegralNumber() || node.path( "version" ).longValue() != (long long)CHAIN_FORMAT_VERSION )
        {
            fail( pError, "unknown blockchain format version" );
            return std::unique_ptr<CBlockchain>();
        }
        const CJsonNode &difficulty = node.path( "difficulty" );
        if ( ! difficulty.isIntegralNumber() || difficulty.longValue() < 0 || difficulty.longValue() > 256 )
        {
            fail( pError, "the chain's \"difficulty\" is missing or out of range" );
            return std::unique_ptr<CBlockchain>();
        }
        const CJsonNode &blocks = node.path( "blocks" );
        if ( ! blocks.isArray() )
        {
            fail( pError, "the chain's \"blocks\" is not an array" );
            return std::unique_ptr<CBlockchain>();
        }

        std::unique_ptr<CBlockchain> pChain( new CBlockchain( CEmpty() ) );
        pChain->setDifficulty( (uint32_t)difficulty.longValue() );
        for ( int n = 0; n < blocks.size(); n++ )
        {
            std::string strError;
            std::unique_ptr<CBlock> pBlock = CBlock::FromJson( blocks.path( n ), &strError );
            if ( ! pBlock )
            {
                fail( pError, "block " + std::to_string( (long long)n ) + ": " + strError );
                return std::unique_ptr<CBlockchain>();
            }
            pChain->m_aBlocks.push_back( std::move( pBlock ) );
        }
        if ( bValidate && ! pChain->validate( pError ) )
            return std::unique_ptr<CBlockchain>();
        return pChain;
    }

    std::unique_ptr<CBlockchain> CBlockchain::FromJsonText( const std::string &strText, std::string *pError, bool bValidate )
    {
        CJsonParser parser;
        std::unique_ptr<CJsonNode> pRoot( parser.readTree( strText ) );
        if ( ! pRoot )
        {
            fail( pError, std::string( "the JSON could not be read: " ) + (const char *)parser.getError() );
            return std::unique_ptr<CBlockchain>();
        }
        return FromJson( *pRoot, pError, bValidate );
    }
#endif
} // namespace IASLib
