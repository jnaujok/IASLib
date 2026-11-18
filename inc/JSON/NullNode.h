/**
 * @file NullNode.h
 * @brief CNullNode, a JSON null.
 *
 * Null Node Class
 *
 *  A JSON null.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_NULLNODE_H__
#define IASLIB_NULLNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"

namespace IASLib
{
        /// A JSON null.
        ///
        /// Containers also store a null node in place of a NULL pointer or a
        /// missing node they are given.
    class CNullNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CNullNode )

                /// #JSON_NULL.
            virtual JsonNodeType    getNodeType( void ) const { return JSON_NULL; }
                /// "null".
            virtual CString         asText( void ) const { return CString( "null" ); }

                /// True for any null node.
            virtual bool            equals( const CJsonNode *pOther ) const { return pOther != NULL && pOther->isNull(); }
                /// A new null node, owned by the caller.
            virtual CJsonNode      *clone( void ) const { return new CNullNode(); }
                /// Appends null.
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_NULLNODE_H__
