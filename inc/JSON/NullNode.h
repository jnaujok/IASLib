/**
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
    class CNullNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CNullNode )

            virtual JsonNodeType    getNodeType( void ) const { return NULLVAL; }
            virtual CString         asText( void ) const { return CString( "null" ); }

            virtual bool            equals( const CJsonNode *pOther ) const { return pOther != NULL && pOther->isNull(); }
            virtual CJsonNode      *clone( void ) const { return new CNullNode(); }
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_NULLNODE_H__
