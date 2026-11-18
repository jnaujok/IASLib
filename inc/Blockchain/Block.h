/**
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

    class CBlock : public CObject
    {
        public:
            typedef std::vector<unsigned char> CEntry;

            static const uint32_t FORMAT_VERSION = 1;
            static const size_t HEADER_SIZE = 100;
                // The highest difficulty mine() accepts: the nonce has 64 bits,
                // so past 64 leading zero bits a block can be expected to have
                // no nonce that works at all. (Up to 256 used to be accepted,
                // and mine() searched forever.)
            static const uint32_t MAX_DIFFICULTY = 64;

            DECLARE_OBJECT( CBlock, CObject );

                // A block holding these entries. A chain sets its index,
                // previous hash and time when it is added (or created with
                // CBlockchain::createNextBlock()).
            explicit            CBlock( const std::vector<CEntry> &aEntries );
                // A block with one entry: the text's bytes.
            explicit            CBlock( const std::string &strData );
                // The same, as the earlier interface had it (the chain sets the
                // index, so nIndex is ignored).
                                CBlock( uint32_t nIndex, const CString &strData );
            virtual            ~CBlock( void ) {}

                // The header fields.
            uint32_t            getVersion( void ) const { return m_nVersion; }
            uint64_t            getIndex( void ) const { return m_nIndex; }
            int64_t             getTimestamp( void ) const { return m_llTimestamp; }
            uint32_t            getDifficulty( void ) const { return m_nDifficulty; }
            uint64_t            getNonce( void ) const { return m_nNonce; }
            const CSha256Digest &getPreviousHash( void ) const { return m_previousHash; }
            const CSha256Digest &getMerkleRoot( void ) const { return m_merkleRoot; }
                // The block's hash (all zeros until it is mined).
            const CSha256Digest &getHash( void ) const { return m_hash; }
                // The same as lower-case hexadecimal.
            CString             getHashString( void ) const;
            CString             getPrevHash( void ) const;

                // The entries.
            size_t              getEntryCount( void ) const { return m_aEntries.size(); }
            const CEntry       &getEntry( size_t nIndex ) const;
            std::string         getEntryText( size_t nIndex ) const;
            const std::vector<CEntry> &getEntries( void ) const { return m_aEntries; }
                // Proof that entry nIndex is in the block: see
                // CMerkleTree::VerifyInclusion(), with getMerkleRoot().
            std::vector<CSha256Digest> getInclusionProof( size_t nIndex ) const;

                // Searches nonces (from 0) for a hash with nDifficulty leading
                // zero bits, and records the difficulty and hash. False, leaving
                // the block unmined, if nDifficulty is above MAX_DIFFICULTY, if
                // nMaxAttempts (0: no limit) run out, if *pCancel becomes true,
                // or if every nonce has been tried. Each extra bit doubles the
                // expected work, so give nMaxAttempts or pCancel for high
                // difficulties. Runs on the calling thread.
            bool                mine( uint32_t nDifficulty, uint64_t nMaxAttempts = 0, const std::atomic<bool> *pCancel = NULL );
            void                mineBlock( uint32_t nDifficulty ) { mine( nDifficulty ); }
            bool                isMined( void ) const { return m_bMined; }

                // Checks the block by itself: its hash is its header's hash, the
                // hash meets its difficulty, and the Merkle root matches the
                // entries. On failure, the reason (if pReason is given).
            bool                validate( std::string *pReason = NULL ) const;

            CSha256Digest       calculateHash( void ) const;
            void                getHeader( unsigned char achHeader[ HEADER_SIZE ] ) const;

                // The number of leading zero bits of a hash, and whether it has
                // at least nDifficulty.
            static uint32_t     LeadingZeroBits( const CSha256Digest &hash );
            static bool         MeetsDifficulty( const CSha256Digest &hash, uint32_t nDifficulty );

                // A binary form (header, hash, then each entry with its length;
                // the header gives how many),
                // and back. Deserialize reads one block from the start of the
                // data, sets nUsed to its length, and gives NULL (with the
                // reason) for data that is not a whole block. A block read back
                // is not checked: use validate().
            void                serialize( std::vector<unsigned char> &aOut ) const;
            static std::unique_ptr<CBlock> Deserialize( const unsigned char *pData, size_t nLength, size_t &nUsed, std::string *pError = NULL );

#ifdef IASLIB_JSON_SUPPORT__
                // A JSON object (hashes in hexadecimal, entries in base64), and
                // back (NULL, with the reason, if it is not a block). Not checked
                // either: use validate().
            CObjectNode        *toJson( void ) const;
            static std::unique_ptr<CBlock> FromJson( const CJsonNode &node, std::string *pError = NULL );
#endif

        private:
            uint32_t            m_nVersion;
            uint64_t            m_nIndex;
            int64_t             m_llTimestamp;
            uint32_t            m_nDifficulty;
            uint64_t            m_nNonce;
            CSha256Digest       m_previousHash;
            CSha256Digest       m_merkleRoot;
            CSha256Digest       m_hash;
            bool                m_bMined;
            std::vector<CEntry> m_aEntries;

            CBlock( void );
            void                init( void );
                // Called by the chain: places the block after pPrevious (NULL
                // for the first block).
            void                place( uint64_t nIndex, const CSha256Digest &previousHash, int64_t llNotBefore );

            CBlock( const CBlock & );
            CBlock &operator =( const CBlock & );

            friend class CBlockchain;
    };
} // namespace IASLib

#endif // IASLIB_BLOCK_H__
