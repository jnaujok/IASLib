/**
 *  Blockchain Class
 *
 *      A chain of mined blocks (see CBlock), each holding the hash of the
 * one before, starting from a genesis block. Changing anything in a block
 * changes its hash and breaks the link from the next block; redoing the work
 * for every block after it is what the proof of work makes expensive.
 * validate() checks all of it.
 *
 *          CBlockchain chain( 16 );                 // 16 leading zero bits
 *          chain.addBlock( "first record" );        // mines, then appends
 *          if ( ! chain.validate( &reason ) ) ...
 *          std::string json = chain.toJsonText();
 *
 *      Mining runs on the thread that calls addBlock(); it can be limited or
 * cancelled. To mine elsewhere, take createNextBlock(), mine() it on any
 * thread and hand it to appendBlock(). The chain may be used from several
 * threads; blocks it hands out never change.
 *
 *      Difficulty is in leading zero bits of the hash (earlier versions of
 * this class counted hexadecimal zeros: 6 then is 24 now).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/20/2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BLOCKCHAIN_H__
#define IASLIB_BLOCKCHAIN_H__

#include "BaseTypes/Object.h"
#include "Blockchain/Block.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>

namespace IASLib
{
    class CJsonNode;
    class CObjectNode;

    class CBlockchain : public CObject
    {
        public:
            static const uint32_t DEFAULT_DIFFICULTY = 16;

            DECLARE_OBJECT( CBlockchain, CObject );

                // A chain with a mined genesis block holding strGenesisData.
            explicit            CBlockchain( uint32_t nDifficulty = DEFAULT_DIFFICULTY, const std::string &strGenesisData = "Genesis Block" );
            virtual            ~CBlockchain( void );

                // The difficulty new blocks are mined at (and validate()
                // requires by default).
            uint32_t            getDifficulty( void ) const { return m_nDifficulty; }
            void                setDifficulty( uint32_t nDifficulty ) { m_nDifficulty = ( nDifficulty > 256 ) ? 256 : nDifficulty; }

                // The blocks, from the genesis block (0). NULL out of range.
            size_t              size( void ) const;
            const CBlock       *getBlock( size_t nIndex ) const;
            const CBlock       *getLastBlock( void ) const;

                // Makes a block of the entries, mines it and appends it; returns
                // it, or NULL if mining stopped (nMaxAttempts, 0 for no limit,
                // ran out, or *pCancel became true).
            const CBlock       *addBlock( const std::vector<CBlock::CEntry> &aEntries, uint64_t nMaxAttempts = 0,
                                          const std::atomic<bool> *pCancel = NULL );
            const CBlock       *addBlock( const std::string &strData, uint64_t nMaxAttempts = 0,
                                          const std::atomic<bool> *pCancel = NULL );
                // The earlier interface: places, mines and appends a block,
                // taking ownership of it.
            bool                addBlock( CBlock *pBlock );

                // Mining elsewhere: the next block, placed after the current
                // last block, to mine() on any thread; then appendBlock() it.
                // appendBlock() takes the block if it follows the last block
                // and meets the chain's difficulty, and otherwise leaves it with
                // the caller and gives the reason.
            std::unique_ptr<CBlock> createNextBlock( const std::vector<CBlock::CEntry> &aEntries ) const;
            bool                appendBlock( std::unique_ptr<CBlock> &pBlock, std::string *pReason = NULL );

                // Checks every block (CBlock::validate()), that each is at its
                // index, links to the hash of the one before (the genesis block
                // to zeros), is not earlier than the one before, and meets the
                // required difficulty (the chain's own unless given; a reader
                // should give the difficulty it trusts, since a saved chain's is
                // just a number in the file). On failure, the reason and the
                // block's index.
            bool                validate( std::string *pReason = NULL, size_t *pBadBlock = NULL ) const;
            bool                validate( uint32_t nRequiredDifficulty, std::string *pReason, size_t *pBadBlock = NULL ) const;

                // A binary form, and back (NULL, with the reason, for data that
                // is not a chain, or with bValidate one that does not validate).
            std::vector<unsigned char> serialize( void ) const;
            static std::unique_ptr<CBlockchain> Deserialize( const std::vector<unsigned char> &aData, std::string *pError = NULL, bool bValidate = true );

#ifdef IASLIB_JSON_SUPPORT__
                // A JSON object or text, and back.
            CObjectNode        *toJson( void ) const;
            std::string         toJsonText( bool bPretty = true ) const;
            static std::unique_ptr<CBlockchain> FromJson( const CJsonNode &node, std::string *pError = NULL, bool bValidate = true );
            static std::unique_ptr<CBlockchain> FromJsonText( const std::string &strText, std::string *pError = NULL, bool bValidate = true );
#endif

        private:
            mutable std::mutex  m_mutex;
            std::atomic<uint32_t> m_nDifficulty;
            std::vector< std::unique_ptr<CBlock> > m_aBlocks;

            struct CEmpty {};
                                CBlockchain( CEmpty );
            void                placeNext( CBlock &block ) const;   // with m_mutex held

                                CBlockchain( const CBlockchain & );
            CBlockchain        &operator =( const CBlockchain & );
    };
} // namespace IASLib

#endif // IASLIB_BLOCKCHAIN_H__
