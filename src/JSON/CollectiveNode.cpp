/**
 * Collective Node Class
 *
 *  The tree walks shared by arrays and objects: deleting, copying, comparing
 * and writing. See CollectiveNode.h.
 *
 *  Each walk keeps the nodes it still has to visit in a vector of its own
 * rather than recursing, so a tree nested a million deep needs a million
 * entries of heap, not a million stack frames. (These used to recurse, and a
 * deeply nested tree, which the parser could build with a raised depth limit
 * or a program could build by hand, overflowed the stack.)
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_JSON_SUPPORT__

#include "CollectiveNode.h"
#include "ArrayNode.h"
#include "ObjectNode.h"
#include "NullNode.h"
#include "MissingNode.h"

#include <utility>

namespace IASLib
{
    namespace
    {
            // Whether a node is the shared missing node, which belongs to no
            // tree and is never deleted.
        bool isSharedMissing( const CJsonNode *pNode )
        {
            return pNode == &CMissingNode::getInstance();
        }

            // A new, empty container of the same kind as pNode.
        CJsonNode *emptyLike( const CJsonNode *pNode )
        {
            if ( pNode->isObject() )
                return new CObjectNode();
            return new CArrayNode();
        }
    }

    CJsonNode *CCollectiveNode::storable( CJsonNode *pValue )
    {
        if ( pValue == NULL )
            return new CNullNode();
        if ( pValue->isMissingNode() )
        {
                // A missing node in a tree has no JSON form, so it is kept as a
                // null. (It used to be stored, and then written as nothing,
                // which made the output invalid.)
            discard( pValue );
            return new CNullNode();
        }
        return pValue;
    }

    void CCollectiveNode::discard( CJsonNode *pValue )
    {
        if ( pValue == NULL || isSharedMissing( pValue ) )
            return;
        std::vector<CJsonNode *> aNodes( 1, pValue );
        deleteNodes( aNodes );
    }

        // Each container's children are moved onto the list before it is
        // deleted, so its destructor finds it empty and does not descend.
    void CCollectiveNode::deleteNodes( std::vector<CJsonNode *> &aNodes )
    {
        while ( ! aNodes.empty() )
        {
            CJsonNode *pNode = aNodes.back();
            aNodes.pop_back();
            if ( pNode == NULL || isSharedMissing( pNode ) )
                continue;
            if ( pNode->isContainerNode() )
                static_cast<CCollectiveNode *>( pNode )->detachChildren( aNodes );
            delete pNode;
        }
    }

        // Containers are copied empty and filled in later from the list of
        // (original, copy) pairs; values are copied by their own clone().
    CJsonNode *CCollectiveNode::cloneTree( const CJsonNode *pRoot )
    {
        if ( pRoot == NULL )
            return NULL;
        if ( ! pRoot->isContainerNode() )
            return pRoot->clone();

        CJsonNode *pCopy = emptyLike( pRoot );
        std::vector< std::pair<const CJsonNode *, CJsonNode *> > aPending;
        aPending.push_back( std::make_pair( pRoot, pCopy ) );
        while ( ! aPending.empty() )
        {
            const CJsonNode *pFrom = aPending.back().first;
            CJsonNode *pTo = aPending.back().second;
            aPending.pop_back();

            int nSize = pFrom->size();
            for ( int n = 0; n < nSize; n++ )
            {
                const CJsonNode *pChild = pFrom->isObject() ? static_cast<const CObjectNode *>( pFrom )->fieldValue( n ) : pFrom->get( n );
                CJsonNode *pChildCopy = NULL;
                if ( pChild != NULL && pChild->isContainerNode() )
                {
                    pChildCopy = emptyLike( pChild );
                    aPending.push_back( std::make_pair( pChild, pChildCopy ) );
                }
                else if ( pChild != NULL )
                {
                    pChildCopy = pChild->clone();
                }
                    // NULL (and a missing node) become a null when stored.
                if ( pTo->isObject() )
                    static_cast<CObjectNode *>( pTo )->set( static_cast<const CObjectNode *>( pFrom )->fieldNameString( n ), pChildCopy );
                else
                    static_cast<CArrayNode *>( pTo )->add( pChildCopy );
            }
        }
        return pCopy;
    }

        // Pairs still to compare are kept on a list; containers add their
        // children's pairs, and values compare with their own equals().
    bool CCollectiveNode::equalTrees( const CJsonNode *pFirst, const CJsonNode *pSecond )
    {
        std::vector< std::pair<const CJsonNode *, const CJsonNode *> > aPending;
        aPending.push_back( std::make_pair( pFirst, pSecond ) );
        while ( ! aPending.empty() )
        {
            const CJsonNode *pA = aPending.back().first;
            const CJsonNode *pB = aPending.back().second;
            aPending.pop_back();

            if ( pA == pB )
                continue;
            if ( pA == NULL )
                std::swap( pA, pB );
            if ( pB == NULL )
                return false;
            if ( ! pA->isContainerNode() )
            {
                if ( ! pA->equals( pB ) )
                    return false;
                continue;
            }
            if ( pA->getNodeType() != pB->getNodeType() || pA->size() != pB->size() )
                return false;

            int nSize = pA->size();
            if ( pA->isArray() )
            {
                for ( int n = 0; n < nSize; n++ )
                    aPending.push_back( std::make_pair( pA->get( n ), pB->get( n ) ) );
            }
            else
            {
                    // Fields compare regardless of order.
                const CObjectNode *pObject = static_cast<const CObjectNode *>( pA );
                for ( int n = 0; n < nSize; n++ )
                {
                    const CJsonNode *pOther = pB->get( pObject->fieldNameString( n ) );
                    if ( pOther == NULL )
                        return false;
                    aPending.push_back( std::make_pair( pObject->fieldValue( n ), pOther ) );
                }
            }
        }
        return true;
    }

    namespace
    {
        struct CWriteFrame
        {
            const CJsonNode    *pNode;
            int                 nNext;      // the next child to write
            int                 nLevel;
        };
    }

        // A stack of the containers open in the output, and the next child of
        // each. A child that is NULL or missing is written as null, so the
        // output is always valid JSON. (It used to be written as nothing.)
    void CCollectiveNode::writeTree( const CJsonNode *pRoot, std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly )
    {
        std::vector<CWriteFrame> aStack;
        const CJsonNode *pValue = pRoot;
        int nValueLevel = nLevel;
        while ( true )
        {
                // Write the value waiting (if any): open a container, or write
                // a value whole.
            if ( pValue == NULL || pValue->isMissingNode() )
            {
                if ( pValue != pRoot || pValue == NULL )
                    strOut += "null";
            }
            else if ( pValue->isContainerNode() )
            {
                bool bObject = pValue->isObject();
                if ( pValue->size() == 0 )
                {
                    strOut += bObject ? "{}" : "[]";
                }
                else
                {
                    strOut += bObject ? '{' : '[';
                    CWriteFrame frame = { pValue, 0, nValueLevel };
                    aStack.push_back( frame );
                }
            }
            else
            {
                pValue->writeJson( strOut, nIndent, nValueLevel, bAsciiOnly );
            }

                // Then find the next child to write, closing the containers
                // that are finished.
            pValue = NULL;
            bool bFound = false;
            while ( ! aStack.empty() )
            {
                CWriteFrame &frame = aStack.back();
                if ( frame.nNext >= frame.pNode->size() )
                {
                    newLine( strOut, nIndent, frame.nLevel );
                    strOut += frame.pNode->isObject() ? '}' : ']';
                    aStack.pop_back();
                    continue;
                }
                int n = frame.nNext++;
                if ( n > 0 )
                    strOut += ',';
                newLine( strOut, nIndent, frame.nLevel + 1 );
                if ( frame.pNode->isObject() )
                {
                    const CObjectNode *pObject = static_cast<const CObjectNode *>( frame.pNode );
                    writeString( strOut, pObject->fieldNameString( n ), bAsciiOnly );
                    strOut += ( nIndent >= 0 ) ? ": " : ":";
                    pValue = pObject->fieldValue( n );
                }
                else
                {
                    pValue = frame.pNode->get( n );
                }
                nValueLevel = frame.nLevel + 1;
                bFound = true;
                break;
            }
            if ( ! bFound )
                return;
        }
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
