/**
 * @file Blockchain.h
 * @brief CBlockchain, a chain of mined blocks.
 *
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

        /// A chain of mined blocks, each holding the hash of the one before,
        /// starting from a genesis block.
        ///
        /// Changing anything in a block changes its hash and breaks the link
        /// from the next block; redoing the work for every block after it is
        /// what the proof of work makes expensive. validate() checks all of it.
        ///
        /// Mining runs on the thread that calls addBlock(); it can be limited
        /// or cancelled. To mine elsewhere, take createNextBlock(), mine() it
        /// on any thread and hand it to appendBlock().
        ///
        /// Threading: the chain may be used from several threads (a mutex
        /// guards the list of blocks, and mining is done without it). The
        /// chain owns its blocks; the const pointers it hands out stay valid
        /// for the chain's life, and the blocks never change.
        ///
        /// Difficulty is in leading zero bits of the hash.
        /// @note Earlier versions of this class counted hexadecimal zeros: 6
        /// then is 24 now.
        ///
        /// @code
        ///     CBlockchain chain( 16 );                 // 16 leading zero bits
        ///     chain.addBlock( "first record" );        // mines, then appends
        ///     std::string strReason;
        ///     if ( ! chain.validate( &strReason ) )
        ///         printf( "%s\n", strReason.c_str() );
        ///     std::string strJson = chain.toJsonText();
        /// @endcode
    class CBlockchain : public CObject
    {
        public:
                /// The difficulty a chain is made with if none is given.
            static const uint32_t DEFAULT_DIFFICULTY = 16;
                /// The highest difficulty a chain takes (CBlock::MAX_DIFFICULTY).
            static const uint32_t MAX_DIFFICULTY = CBlock::MAX_DIFFICULTY;

            DECLARE_OBJECT( CBlockchain, CObject );

                /// A chain with a mined genesis block holding strGenesisData.
                /// Mining it runs here, on the calling thread, with no limit.
                /// @param nDifficulty    The difficulty, clamped as
                ///                       setDifficulty() does.
                /// @param strGenesisData The genesis block's one entry.
            explicit            CBlockchain( uint32_t nDifficulty = DEFAULT_DIFFICULTY, const std::string &strGenesisData = "Genesis Block" );
                /// Destroys the chain and its blocks.
            virtual            ~CBlockchain( void );

                /// The difficulty new blocks are mined at (and validate()
                /// requires by default).
            uint32_t            getDifficulty( void ) const { return m_nDifficulty; }
                /// Sets the difficulty for new blocks. Above #MAX_DIFFICULTY it
                /// is clamped to that. Blocks already in the chain are not
                /// changed.
                /// @note It used to be clamped to 256, which no block can be
                /// mined at.
            void                setDifficulty( uint32_t nDifficulty ) { m_nDifficulty = ( nDifficulty > MAX_DIFFICULTY ) ? (uint32_t)MAX_DIFFICULTY : nDifficulty; }

                /// The number of blocks, the genesis block included.
            size_t              size( void ) const;
                /// Block nIndex, from the genesis block (0); NULL out of range.
                /// The chain owns it.
            const CBlock       *getBlock( size_t nIndex ) const;
                /// The last block; NULL if there are none. The chain owns it.
            const CBlock       *getLastBlock( void ) const;

                /// Makes a block of the entries, mines it at the chain's
                /// difficulty and appends it. If another thread appends a block
                /// meanwhile, the block is placed after that one and mined again.
                /// @param aEntries     The entries.
                /// @param nMaxAttempts The most nonces to try for each mining
                ///                     (0: no limit).
                /// @param pCancel      If given, mining stops once it becomes true.
                /// @return The block (owned by the chain), or NULL if mining
                ///         stopped.
            const CBlock       *addBlock( const std::vector<CBlock::CEntry> &aEntries, uint64_t nMaxAttempts = 0,
                                          const std::atomic<bool> *pCancel = NULL );
                /// The same, for a block of one entry: the text's bytes.
            const CBlock       *addBlock( const std::string &strData, uint64_t nMaxAttempts = 0,
                                          const std::atomic<bool> *pCancel = NULL );
                /// The earlier interface: places, mines (with no limit) and
                /// appends a block, taking ownership of it.
                /// @return False only for a NULL block.
            bool                addBlock( CBlock *pBlock );

                /// The next block, placed after the current last block, for
                /// mining elsewhere: mine() it on any thread, then appendBlock() it.
                /// @return A new block, owned by the caller, not yet mined.
            std::unique_ptr<CBlock> createNextBlock( const std::vector<CBlock::CEntry> &aEntries ) const;
                /// Appends a mined block. The block is taken if it is sound
                /// (CBlock::validate()), meets the chain's difficulty, follows
                /// the last block (the next index and that block's hash), and is
                /// not earlier than it.
                /// @param pBlock  The block; emptied if it is taken, and
                ///                otherwise left with the caller.
                /// @param pReason If given, receives the reason it was not taken.
                /// @return True if the block was taken.
            bool                appendBlock( std::unique_ptr<CBlock> &pBlock, std::string *pReason = NULL );

                /// Checks every block (CBlock::validate()), that each is at its
                /// index, links to the hash of the one before (the genesis block
                /// to zeros), is not earlier than the one before, and meets the
                /// chain's difficulty. A chain with no blocks fails.
                /// @param pReason   If given, receives the reason on failure.
                /// @param pBadBlock If given, receives the failing block's index.
                /// @return True if the chain is sound.
            bool                validate( std::string *pReason = NULL, size_t *pBadBlock = NULL ) const;
                /// The same, requiring nRequiredDifficulty rather than the
                /// chain's own. A reader should give the difficulty it trusts,
                /// since a saved chain's is just a number in the file.
            bool                validate( uint32_t nRequiredDifficulty, std::string *pReason, size_t *pBadBlock = NULL ) const;

                /// The chain's binary form: a magic number, format version,
                /// difficulty and block count, then each block as
                /// CBlock::serialize() writes it.
            std::vector<unsigned char> serialize( void ) const;
                /// Reads a chain from its binary form.
                /// @param aData     The data, all of it one chain.
                /// @param pError    If given, receives the reason on failure.
                /// @param bValidate Also validate() the chain (at its own saved
                ///                  difficulty: call validate() with the
                ///                  difficulty you trust as well).
                /// @return The chain, or NULL for data that is not a chain (or
                ///         has a difficulty above #MAX_DIFFICULTY), or, with
                ///         bValidate, one that does not validate.
            static std::unique_ptr<CBlockchain> Deserialize( const std::vector<unsigned char> &aData, std::string *pError = NULL, bool bValidate = true );

#ifdef IASLIB_JSON_SUPPORT__
                /// The chain as a JSON object: format ("IASLib blockchain"),
                /// version, difficulty, and blocks (an array of
                /// CBlock::toJson()).
                /// @return A new object, owned by the caller.
            CObjectNode        *toJson( void ) const;
                /// The chain as JSON text, indented by two spaces if bPretty,
                /// otherwise compact.
            std::string         toJsonText( bool bPretty = true ) const;
                /// Reads a chain from a JSON object as toJson() writes it.
                /// @param node      The object.
                /// @param pError    If given, receives the reason on failure.
                /// @param bValidate As for Deserialize().
                /// @return The chain, or NULL if the JSON is not a chain (or
                ///         has a difficulty above #MAX_DIFFICULTY), or, with
                ///         bValidate, one that does not validate.
            static std::unique_ptr<CBlockchain> FromJson( const CJsonNode &node, std::string *pError = NULL, bool bValidate = true );
                /// Parses JSON text (strictly) and reads a chain from it, as
                /// FromJson() does. Text that cannot be parsed fails with the
                /// parser's error.
            static std::unique_ptr<CBlockchain> FromJsonText( const std::string &strText, std::string *pError = NULL, bool bValidate = true );
#endif

        private:
            mutable std::mutex  m_mutex;            ///< Guards m_aBlocks.
            std::atomic<uint32_t> m_nDifficulty;    ///< The difficulty for new blocks.
            std::vector< std::unique_ptr<CBlock> > m_aBlocks;  ///< The blocks, from the genesis block.

                /// A tag for the private constructor.
            struct CEmpty {};
                /// A chain with no blocks, to be filled by Deserialize() or
                /// FromJson().
                                CBlockchain( CEmpty );
                /// Places a block after the last one (or first). Called with
                /// m_mutex held.
            void                placeNext( CBlock &block ) const;   // with m_mutex held

                /// Not copyable.
                                CBlockchain( const CBlockchain & );
                /// Not assignable.
            CBlockchain        &operator =( const CBlockchain & );
    };
} // namespace IASLib

#endif // IASLIB_BLOCKCHAIN_H__
