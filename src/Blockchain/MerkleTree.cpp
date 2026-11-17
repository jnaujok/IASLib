/**
 *  Merkle Tree
 *
 *      Merkle tree hashing, inclusion proofs and their verification, as in
 * RFC 6962 section 2.1 and RFC 9162 section 2.1. See MerkleTree.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/2020
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Blockchain/MerkleTree.h"

namespace IASLib
{
    namespace
    {
            // The largest power of two smaller than n (n > 1).
        size_t splitPoint( size_t n )
        {
            size_t k = 1;
            while ( ( k << 1 ) < n )
                k <<= 1;
            return k;
        }

            // MTH(D[nStart:nEnd]) over leaf hashes.
        CSha256Digest subtreeRoot( const std::vector<CSha256Digest> &aLeaves, size_t nStart, size_t nEnd )
        {
            size_t n = nEnd - nStart;
            if ( n == 1 )
                return aLeaves[ nStart ];
            size_t k = splitPoint( n );
            return CMerkleTree::NodeHash( subtreeRoot( aLeaves, nStart, nStart + k ), subtreeRoot( aLeaves, nStart + k, nEnd ) );
        }

            // PATH(m, D[nStart:nEnd]), appended bottom-up.
        void subtreePath( const std::vector<CSha256Digest> &aLeaves, size_t m, size_t nStart, size_t nEnd, std::vector<CSha256Digest> &aPath )
        {
            size_t n = nEnd - nStart;
            if ( n <= 1 )
                return;
            size_t k = splitPoint( n );
            if ( m < k )
            {
                subtreePath( aLeaves, m, nStart, nStart + k, aPath );
                aPath.push_back( subtreeRoot( aLeaves, nStart + k, nEnd ) );
            }
            else
            {
                subtreePath( aLeaves, m - k, nStart + k, nEnd, aPath );
                aPath.push_back( subtreeRoot( aLeaves, nStart, nStart + k ) );
            }
        }

        std::vector<CSha256Digest> leavesOf( const std::vector<CMerkleTree::CEntry> &aEntries )
        {
            std::vector<CSha256Digest> aLeaves;
            aLeaves.reserve( aEntries.size() );
            for ( size_t n = 0; n < aEntries.size(); n++ )
                aLeaves.push_back( CMerkleTree::LeafHash( aEntries[ n ] ) );
            return aLeaves;
        }
    }

    CSha256Digest CMerkleTree::LeafHash( const CEntry &entry )
    {
        CSHA256 ctx;
        ctx.init();
        unsigned char chPrefix = 0x00;
        ctx.updateData( &chPrefix, (size_t)1 );
        if ( ! entry.empty() )
            ctx.updateData( &entry[0], entry.size() );
        CSha256Digest digest;
        ctx.final( digest.data() );
        return digest;
    }

    CSha256Digest CMerkleTree::NodeHash( const CSha256Digest &left, const CSha256Digest &right )
    {
        CSHA256 ctx;
        ctx.init();
        unsigned char chPrefix = 0x01;
        ctx.updateData( &chPrefix, (size_t)1 );
        ctx.updateData( left.data(), left.size() );
        ctx.updateData( right.data(), right.size() );
        CSha256Digest digest;
        ctx.final( digest.data() );
        return digest;
    }

    CSha256Digest CMerkleTree::RootOfLeaves( const std::vector<CSha256Digest> &aLeaves )
    {
        if ( aLeaves.empty() )
            return CSHA256::Digest( NULL, 0 );
        return subtreeRoot( aLeaves, 0, aLeaves.size() );
    }

    CSha256Digest CMerkleTree::Root( const std::vector<CEntry> &aEntries )
    {
        return RootOfLeaves( leavesOf( aEntries ) );
    }

    std::vector<CSha256Digest> CMerkleTree::InclusionProofOfLeaves( const std::vector<CSha256Digest> &aLeaves, size_t nIndex )
    {
        std::vector<CSha256Digest> aPath;
        if ( nIndex < aLeaves.size() )
            subtreePath( aLeaves, nIndex, 0, aLeaves.size(), aPath );
        return aPath;
    }

    std::vector<CSha256Digest> CMerkleTree::InclusionProof( const std::vector<CEntry> &aEntries, size_t nIndex )
    {
        return InclusionProofOfLeaves( leavesOf( aEntries ), nIndex );
    }

        // RFC 9162 section 2.1.3.2.
    bool CMerkleTree::VerifyLeafInclusion( const CSha256Digest &leaf, size_t nIndex, size_t nTreeSize,
                                           const std::vector<CSha256Digest> &aProof, const CSha256Digest &root )
    {
        if ( nIndex >= nTreeSize )
            return false;
        size_t fn = nIndex;
        size_t sn = nTreeSize - 1;
        CSha256Digest r = leaf;
        for ( size_t n = 0; n < aProof.size(); n++ )
        {
            if ( sn == 0 )
                return false;
            if ( ( fn & 1 ) == 1 || fn == sn )
            {
                r = NodeHash( aProof[ n ], r );
                if ( ( fn & 1 ) == 0 )
                {
                    while ( ( fn & 1 ) == 0 && fn != 0 )
                    {
                        fn >>= 1;
                        sn >>= 1;
                    }
                }
            }
            else
            {
                r = NodeHash( r, aProof[ n ] );
            }
            fn >>= 1;
            sn >>= 1;
        }
        return sn == 0 && r == root;
    }

    bool CMerkleTree::VerifyInclusion( const CEntry &entry, size_t nIndex, size_t nTreeSize,
                                       const std::vector<CSha256Digest> &aProof, const CSha256Digest &root )
    {
        return VerifyLeafInclusion( LeafHash( entry ), nIndex, nTreeSize, aProof, root );
    }
} // namespace IASLib
