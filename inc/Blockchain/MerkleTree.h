/**
 * @file MerkleTree.h
 * @brief CMerkleTree, Merkle tree roots and inclusion proofs (RFC 6962/9162).
 *
 *  Merkle Tree
 *
 *      A Merkle tree (hash tree) over a list of entries, as defined for
 * Certificate Transparency (RFC 6962 section 2.1, RFC 9162 section 2.1).
 * Its root hash commits to every entry and their order, and an inclusion
 * proof of a few hashes shows that one entry is in the tree without the
 * others.
 *
 *      Leaves are hashed as SHA-256( 0x00 || entry ) and inner nodes as
 * SHA-256( 0x01 || left || right ), so a node can never pass for a leaf.
 * A list that is not a power of two long is split at the largest power of
 * two below its length; no hash is ever duplicated, so two different lists
 * cannot have the same root (unlike the duplicate-last-hash rule behind
 * Bitcoin's CVE-2012-2459). The root of no entries is SHA-256 of nothing.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/2020
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MERKLETREE_H__
#define IASLIB_MERKLETREE_H__

#include "Encryption/Sha256.h"

#include <stddef.h>
#include <vector>

namespace IASLib
{
        /// A Merkle tree (hash tree) over a list of entries, as defined for
        /// Certificate Transparency (RFC 6962 section 2.1, RFC 9162 section 2.1).
        ///
        /// The root hash commits to every entry and their order, and an
        /// inclusion proof of a few hashes shows that one entry is in the tree
        /// without the others. Leaves are hashed as SHA-256( 0x00 || entry )
        /// and inner nodes as SHA-256( 0x01 || left || right ), so a node can
        /// never pass for a leaf. A list that is not a power of two long is
        /// split at the largest power of two below its length; no hash is ever
        /// duplicated, so two different lists cannot have the same root
        /// (unlike the duplicate-last-hash rule behind Bitcoin's
        /// CVE-2012-2459). The root of no entries is SHA-256 of nothing.
        ///
        /// Every method is static and keeps no state, so they may be called
        /// from any thread.
        ///
        /// @code
        ///     CSha256Digest root = CMerkleTree::Root( aEntries );
        ///     std::vector<CSha256Digest> aProof = CMerkleTree::InclusionProof( aEntries, 3 );
        ///     bool bIn = CMerkleTree::VerifyInclusion( aEntries[ 3 ], 3, aEntries.size(), aProof, root );
        /// @endcode
    class CMerkleTree
    {
        public:
                /// One entry: any bytes.
            typedef std::vector<unsigned char> CEntry;

                /// The hash of one entry as a leaf: SHA-256( 0x00 || entry ).
            static CSha256Digest LeafHash( const CEntry &entry );
                /// The hash of two nodes as their parent:
                /// SHA-256( 0x01 || left || right ).
            static CSha256Digest NodeHash( const CSha256Digest &left, const CSha256Digest &right );

                /// The root of a list of entries (SHA-256 of nothing for none).
            static CSha256Digest Root( const std::vector<CEntry> &aEntries );
                /// The root of a list of leaf hashes (LeafHash() of each entry).
            static CSha256Digest RootOfLeaves( const std::vector<CSha256Digest> &aLeaves );

                /// The inclusion proof (audit path) of entry nIndex: the hashes
                /// needed, with the entry, to rebuild the root, from the bottom
                /// of the tree up.
                /// @return The proof; empty for an index out of range (or a tree
                ///         of one entry, which needs none).
            static std::vector<CSha256Digest> InclusionProof( const std::vector<CEntry> &aEntries, size_t nIndex );
                /// The same, from the leaf hashes.
            static std::vector<CSha256Digest> InclusionProofOfLeaves( const std::vector<CSha256Digest> &aLeaves, size_t nIndex );

                /// Checks an inclusion proof (RFC 9162 section 2.1.3.2).
                /// @param entry     The entry.
                /// @param nIndex    Its position in the tree, from 0.
                /// @param nTreeSize The number of entries in the tree.
                /// @param aProof    The proof, as InclusionProof() gives it.
                /// @param root      The tree's root.
                /// @return True if the proof shows that the entry is entry
                ///         nIndex of a tree of nTreeSize entries with this root.
            static bool VerifyInclusion( const CEntry &entry, size_t nIndex, size_t nTreeSize,
                                         const std::vector<CSha256Digest> &aProof, const CSha256Digest &root );
                /// The same, for a leaf hash (LeafHash() of the entry).
            static bool VerifyLeafInclusion( const CSha256Digest &leaf, size_t nIndex, size_t nTreeSize,
                                             const std::vector<CSha256Digest> &aProof, const CSha256Digest &root );
    };
} // namespace IASLib

#endif // IASLIB_MERKLETREE_H__
