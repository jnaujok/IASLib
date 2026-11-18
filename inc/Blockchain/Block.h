/**
 * @file Block.h
 * @brief CBlock, one block of a CBlockchain.
 *
 *  Block Class
 *
 *      One block of a CBlockchain: a list of entries (any bytes), committed
 * to by their Merkle root, and a header linking the block to the one before
 * it. The block's hash is SHA-256 of its 100-byte header:
 *
 *          version      4 bytes  (big-endian; FORMAT_VERSION)
 *          index        8 bytes  (position in the chain, from 0)
 *          timestamp    8 bytes  (milliseconds since 1970 UTC)
 *          difficulty   4 bytes  (leading zero bits the hash must have)
 *          previous    32 bytes  (the previous block's hash; zeros for the first)
 *          merkle root 32 bytes  (CMerkleTree root of the entries)
 *          entries      4 bytes  (how many: the tree size for inclusion proofs)
 *          nonce        8 bytes
 *
 * Every field has a fixed size, so no two different blocks hash the same
 * bytes, and the difficulty is part of what the work proves. Mining searches
 * for a nonce that gives a hash with at least `difficulty` leading zero bits
 * (each bit doubles the expected work: 16 bits is about 65,000 attempts).
 *
 *      A block is filled in and mined before it joins a chain; a chain only
 * hands out const blocks, so blocks in a chain do not change.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/20/2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BLOCK_H__
#define IASLIB_BLOCK_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"
#include "Encryption/Sha256.h"

#include <atomic>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace IASLib
{
    class CJsonNode;
    class CObjectNode;

        /// One block of a CBlockchain: a list of entries and a header linking
        /// it to the block before.
        ///
        /// The entries (any bytes) are committed to by their Merkle root
        /// (CMerkleTree). The block's hash is SHA-256 of its 100-byte header,
        /// all big-endian:
        ///
        /// | Field       | Bytes | Meaning                                           |
        /// |-------------|-------|---------------------------------------------------|
        /// | version     | 4     | #FORMAT_VERSION                                   |
        /// | index       | 8     | position in the chain, from 0                     |
        /// | timestamp   | 8     | milliseconds since 1970 UTC                       |
        /// | difficulty  | 4     | leading zero bits the hash must have              |
        /// | previous    | 32    | the previous block's hash; zeros for the first    |
        /// | merkle root | 32    | CMerkleTree root of the entries                   |
        /// | entries     | 4     | how many: the tree size for inclusion proofs      |
        /// | nonce       | 8     |                                                   |
        ///
        /// Every field has a fixed size, so no two different blocks hash the
        /// same bytes, and the difficulty is part of what the work proves.
        /// Mining searches for a nonce that gives a hash with at least that
        /// many leading zero bits (each bit doubles the expected work: 16 bits
        /// is about 65,000 attempts).
        ///
        /// A block is filled in and mined before it joins a chain; a chain only
        /// hands out const blocks, so blocks in a chain do not change. A block
        /// not in a chain may be mined on any thread, but by one at a time.
        /// Blocks cannot be copied.
        ///
        /// @code
        ///     CBlockchain chain;
        ///     std::unique_ptr<CBlock> pBlock = chain.createNextBlock( aEntries );
        ///     if ( pBlock->mine( chain.getDifficulty() ) )
        ///         chain.appendBlock( pBlock );
        /// @endcode
    class CBlock : public CObject
    {
        public:
                /// One entry: any bytes.
            typedef std::vector<unsigned char> CEntry;

                /// The header format version this class writes and accepts.
            static const uint32_t FORMAT_VERSION = 1;
                /// The size of the header in bytes.
            static const size_t HEADER_SIZE = 100;
                /// The highest difficulty mine() accepts: the nonce has 64 bits,
                /// so past 64 leading zero bits a block can be expected to have
                /// no nonce that works at all.
                /// @note Up to 256 used to be accepted, and mine() searched
                /// forever.
            static const uint32_t MAX_DIFFICULTY = 64;

            DECLARE_OBJECT( CBlock, CObject );

                /// A block holding these entries (copied), not yet mined. A
                /// chain sets its index, previous hash and time when it is
                /// added (or created with CBlockchain::createNextBlock()).
            explicit            CBlock( const std::vector<CEntry> &aEntries );
                /// A block with one entry: the text's bytes.
            explicit            CBlock( const std::string &strData );
                /// The same, as the earlier interface had it (the chain sets the
                /// index, so nIndex is ignored).
                                CBlock( uint32_t nIndex, const CString &strData );
                /// Destroys the block.
            virtual            ~CBlock( void ) {}

                /// @name Header fields
                ///@{

                /// The header format version.
            uint32_t            getVersion( void ) const { return m_nVersion; }
                /// The position in the chain, from 0.
            uint64_t            getIndex( void ) const { return m_nIndex; }
                /// The time, in milliseconds since 1970 UTC: when the block was
                /// made, or placed in a chain (never before the block before).
            int64_t             getTimestamp( void ) const { return m_llTimestamp; }
                /// The leading zero bits the hash must have (0 until mined).
            uint32_t            getDifficulty( void ) const { return m_nDifficulty; }
                /// The nonce mining found.
            uint64_t            getNonce( void ) const { return m_nNonce; }
                /// The previous block's hash; all zeros for the first block.
            const CSha256Digest &getPreviousHash( void ) const { return m_previousHash; }
                /// The CMerkleTree root of the entries.
            const CSha256Digest &getMerkleRoot( void ) const { return m_merkleRoot; }
                /// The block's hash (all zeros until it is mined).
            const CSha256Digest &getHash( void ) const { return m_hash; }
                /// The block's hash as lower-case hexadecimal.
            CString             getHashString( void ) const;
                /// The previous block's hash as lower-case hexadecimal.
            CString             getPrevHash( void ) const;
                ///@}

                /// @name Entries
                ///@{

                /// The number of entries.
            size_t              getEntryCount( void ) const { return m_aEntries.size(); }
                /// Entry nIndex; an empty entry if there is no such entry.
            const CEntry       &getEntry( size_t nIndex ) const;
                /// Entry nIndex's bytes as a string; "" if there is no such entry.
            std::string         getEntryText( size_t nIndex ) const;
                /// All the entries, in order.
            const std::vector<CEntry> &getEntries( void ) const { return m_aEntries; }
                /// Proof that entry nIndex is in the block: check it with
                /// CMerkleTree::VerifyInclusion(), getEntryCount() and
                /// getMerkleRoot().
                /// @return The proof; empty for an index out of range (or a
                ///         block of one entry, which needs none).
            std::vector<CSha256Digest> getInclusionProof( size_t nIndex ) const;
                ///@}

                /// Mines the block: searches nonces, from 0, for a hash with
                /// nDifficulty leading zero bits, and records the difficulty and
                /// hash. Runs on the calling thread.
                /// @param nDifficulty  Leading zero bits wanted, at most
                ///                     #MAX_DIFFICULTY. Each extra bit doubles
                ///                     the expected work, so give nMaxAttempts or
                ///                     pCancel for high difficulties.
                /// @param nMaxAttempts The most nonces to try (0: no limit).
                /// @param pCancel      If given, mining stops once it becomes
                ///                     true (checked every 256 attempts).
                /// @return True once mined. False, leaving the block unmined, if
                ///         nDifficulty is above #MAX_DIFFICULTY, the attempts
                ///         run out, mining is cancelled, or every nonce has been
                ///         tried.
            bool                mine( uint32_t nDifficulty, uint64_t nMaxAttempts = 0, const std::atomic<bool> *pCancel = NULL );
                /// The earlier interface: mine() with no limit, ignoring the result.
            void                mineBlock( uint32_t nDifficulty ) { mine( nDifficulty ); }
                /// True once the block has been mined (or read back from a saved
                /// form).
            bool                isMined( void ) const { return m_bMined; }

                /// Checks the block by itself: its format version is known, the
                /// Merkle root matches the entries, its hash is its header's
                /// hash, and the hash meets its difficulty.
                /// @param pReason If given, receives the reason on failure.
                /// @return True if the block is sound.
            bool                validate( std::string *pReason = NULL ) const;

                /// SHA-256 of the header as it stands now.
            CSha256Digest       calculateHash( void ) const;
                /// Writes the header's #HEADER_SIZE bytes, laid out as the class
                /// description gives.
            void                getHeader( unsigned char achHeader[ HEADER_SIZE ] ) const;

                /// The number of leading zero bits of a hash (256 for all zeros).
            static uint32_t     LeadingZeroBits( const CSha256Digest &hash );
                /// True if a hash has at least nDifficulty leading zero bits.
            static bool         MeetsDifficulty( const CSha256Digest &hash, uint32_t nDifficulty );

                /// Appends the block's binary form to aOut: the header, the
                /// hash, then each entry as a 4-byte big-endian length and its
                /// bytes (the header gives how many).
            void                serialize( std::vector<unsigned char> &aOut ) const;
                /// Reads one block, as serialize() writes it, from the start of
                /// the data. A block read back is marked mined but is not
                /// checked: use validate().
                /// @param pData   The data.
                /// @param nLength Its length in bytes.
                /// @param nUsed   Receives the length of the block read (0 on
                ///                failure).
                /// @param pError  If given, receives the reason on failure.
                /// @return The block, or NULL for data that is not a whole block.
            static std::unique_ptr<CBlock> Deserialize( const unsigned char *pData, size_t nLength, size_t &nUsed, std::string *pError = NULL );

#ifdef IASLIB_JSON_SUPPORT__
                /// The block as a JSON object: version, index, timestamp,
                /// difficulty and nonce as numbers, previousHash, merkleRoot and
                /// hash in hexadecimal, and entries as an array of base64 text.
                /// @return A new object, owned by the caller.
            CObjectNode        *toJson( void ) const;
                /// Reads a block from a JSON object as toJson() writes it. A
                /// block read back is marked mined but is not checked: use
                /// validate().
                /// @param node   The object.
                /// @param pError If given, receives the reason on failure.
                /// @return The block, or NULL if the JSON is not a block (a
                ///         field missing, of the wrong type or out of range).
            static std::unique_ptr<CBlock> FromJson( const CJsonNode &node, std::string *pError = NULL );
#endif

        private:
            uint32_t            m_nVersion;         ///< Header format version.
            uint64_t            m_nIndex;           ///< Position in the chain.
            int64_t             m_llTimestamp;      ///< Milliseconds since 1970 UTC.
            uint32_t            m_nDifficulty;      ///< Leading zero bits the hash must have.
            uint64_t            m_nNonce;           ///< The nonce mining found.
            CSha256Digest       m_previousHash;     ///< The previous block's hash.
            CSha256Digest       m_merkleRoot;       ///< The Merkle root of m_aEntries.
            CSha256Digest       m_hash;             ///< The block's hash (zeros until mined).
            bool                m_bMined;           ///< True once mined (or read back).
            std::vector<CEntry> m_aEntries;         ///< The entries.

                /// An empty block, for Deserialize() and FromJson().
            CBlock( void );
                /// Sets every field to its starting value (the time to now).
            void                init( void );
                /// Called by the chain: places the block at nIndex after the
                /// block with previousHash (zeros for the first block), with a
                /// timestamp no earlier than llNotBefore. Leaves it unmined.
            void                place( uint64_t nIndex, const CSha256Digest &previousHash, int64_t llNotBefore );

                /// Not copyable.
            CBlock( const CBlock & );
                /// Not assignable.
            CBlock &operator =( const CBlock & );

            friend class CBlockchain;
    };
} // namespace IASLib

#endif // IASLIB_BLOCK_H__
