/**
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
    class CMissingNode : public CJsonNode
    {
        public:
            DEFINE_OBJECT( CMissingNode )

            static CMissingNode    &getInstance( void );

                                    CMissingNode( void ) {}

            virtual JsonNodeType    getNodeType( void ) const { return MISSING; }
            virtual bool            equals( const CJsonNode *pOther ) const { return pOther != NULL && pOther->isMissingNode(); }
                // A new missing node, owned by the caller like any clone.
            virtual CJsonNode      *clone( void ) const { return new CMissingNode(); }
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_MISSINGNODE_H__
