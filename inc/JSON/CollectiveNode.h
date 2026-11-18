/**
 * Collective Node Class
 *
 *  This abstract class is the base of the JSON nodes that hold other nodes:
 * objects and arrays. A collective node owns its children.
 *
 *  It also holds the tree walks that arrays and objects share: deleting,
 * copying, comparing and writing. Each keeps its own list of the nodes still
 * to visit instead of recursing, so the depth of a tree is limited only by
 * memory, not by the stack. (They used to recurse, and a deeply nested tree
 * overflowed the stack.)
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_COLLECTIVENODE_H__
#define IASLIB_COLLECTIVENODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "JsonNode.h"

#include <string>
#include <vector>

namespace IASLib
{
    class CCollectiveNode : public CJsonNode
    {
        public:
            DEFINE_OBJECT( CCollectiveNode )

                // Deletes every child.
            virtual void            removeAll( void ) = 0;
            bool                    isEmpty( void ) const { return size() == 0; }

        protected:
                // Moves every child into aChildren, leaving this node empty
                // without deleting them.
            virtual void            detachChildren( std::vector<CJsonNode *> &aChildren ) = 0;

                // Deletes the nodes in aNodes and everything under them,
                // emptying the list.
            static void             deleteNodes( std::vector<CJsonNode *> &aNodes );
                // A deep copy of a tree, owned by the caller.
            static CJsonNode       *cloneTree( const CJsonNode *pRoot );
                // Deep equality of two trees (see CJsonNode::equals()).
            static bool             equalTrees( const CJsonNode *pFirst, const CJsonNode *pSecond );
                // Appends a tree's JSON, as CJsonNode::writeJson() does.
            static void             writeTree( const CJsonNode *pRoot, std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly );
                // What a container stores for a child it is given: the node
                // itself, or a new null node for NULL or the missing node
                // (which is deleted unless it is the shared one).
            static CJsonNode       *storable( CJsonNode *pValue );
                // Deletes a node a container was given but will not keep
                // (never the shared missing node).
            static void             discard( CJsonNode *pValue );
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_COLLECTIVENODE_H__
