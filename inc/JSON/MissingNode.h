/**
 * @file MissingNode.h
 * @brief CMissingNode, what a path through a JSON tree gives where there is no value.
 *
 * Missing Node Class
 *
 *  What path(), at() and findPath() give where there is no value. It
 * answers every question with nothing (and its own path() is itself), so a
 * chain of calls through a document never fails. getInstance() is the shared
 * one those methods return; it must not be deleted.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MISSINGNODE_H__
#define IASLIB_MISSINGNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "JsonNode.h"

namespace IASLib
{
        /// What path(), at() and findPath() give where there is no value.
        ///
        /// It answers every question with nothing (NULL, 0, false, "" or
        /// itself), so a chain of calls through a document never fails.
        /// Containers do not hold missing nodes: one given to a container is
        /// stored as a JSON null.
        ///
        /// @code
        ///     if ( pRoot->path( "settings" ).path( "colour" ).isMissingNode() )
        ///         ...
        /// @endcode
    class CMissingNode : public CJsonNode
    {
        public:
            DEFINE_OBJECT( CMissingNode )

                /// The shared missing node, which path(), at() and findPath()
                /// give. It is read-only: do not delete it or add it to a tree.
                /// It is created on first use and is safe to use from several
                /// threads.
                /// @note This used to give a non-const reference, which let
                /// callers change or delete it.
            static const CMissingNode &getInstance( void );

                /// A new missing node. Rarely needed: use getInstance().
                                    CMissingNode( void ) {}

                /// #JSON_MISSING.
            virtual JsonNodeType    getNodeType( void ) const { return JSON_MISSING; }
                /// True for any missing node.
            virtual bool            equals( const CJsonNode *pOther ) const { return pOther != NULL && pOther->isMissingNode(); }
                /// A new missing node, owned by the caller like any clone.
            virtual CJsonNode      *clone( void ) const { return new CMissingNode(); }
                /// Appends nothing: a missing node has no JSON form. (Inside a
                /// container it is written as null.)
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_MISSINGNODE_H__
