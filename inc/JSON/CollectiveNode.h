/**
 * Collective Node Class
 *
 *  This abstract class is the base of the JSON nodes that hold other nodes:
 * objects and arrays. A collective node owns its children.
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

namespace IASLib
{
    class CCollectiveNode : public CJsonNode
    {
        public:
            DEFINE_OBJECT( CCollectiveNode )

                // Deletes every child.
            virtual void            removeAll( void ) = 0;
            bool                    isEmpty( void ) const { return size() == 0; }
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_COLLECTIVENODE_H__
