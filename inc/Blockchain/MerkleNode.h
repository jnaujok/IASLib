/**
 *  Merkle Node
 *
 *      This template lays out the basis for a Merkle Tree that can be used
 * to store transactions within a blockchain block with cryptographic 
 * certainty of whether a transaction belongs to the tree or not.
 *      Merkle Trees are based on the idea that each node contains a hash
 * key of the parent node, and 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/2020
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MERKLENODE_H__
#define IASLIB_MERKLENODE_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"
#include <memory>

namespace IASLib
{
    template <typename T, const char *(hash_func)(const T&), size_t hash_len>
    class MerkleNode
    {
        protected:
            CString                             hash;
            std::unique_ptr<const MerkleNode>   left;
            std::unique_ptr<const MerkleNode>   right;
            const std::shared_ptr<T>            value;

             /**
             * Computes the hash of the children nodes' respective hashes.
             * In other words, given a node N, with children (N1, N2), whose hash values are,
             * respectively, H1 and H2, computes:
             *
             *     H = hash(H1 || H2)
             *
             * where `||` represents the concatenation operation.
             * If the `right_` descendant is missing, it will simply duplicate the `left_` node's hash.
             *
             * For a "leaf" node (both descendants missing), it will use the `hash_func()` to compute the
             * hash of the stored `value_`.
             */
            virtual const char *computeHash() const = 0;
        public:
            MerkleNode( const MerkleNode *_left, MerkleNode *_right ) : left(_left), right(_right), value(nullptr), hash( nullptr ) {}
            MerkleNode( const T &_value ) : value( new T(value) ), left(nullptr), right(nullptr), hash( hash_func(value)) {}

            virtual ~MerkleNode() {}

            virtual bool validate( void ) const;

            size_t len( void ) const { return hash_len; }

            CString getHash( void ) const { return hash; }

            bool hasChildren( ) const { return (left || right); }

            const MerkleNode *getLeft( void ) const { return left.get(); }
            const MerkleNode *getRight( void ) const { return right.get(); }
    };
} // namespace IASLib

#endif // IASLIB_MERKLENODE_H__