/**
 *  Resource Node
 *
 *      The base of the nodes in a view of a resource archive's contents:
 * a folder (CResourceDirectoryNode) or a resource in it
 * (CResourceFileNode).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RESOURCENODE_H__
#define IASLIB_RESOURCENODE_H__

#include "../BaseTypes/Object.h"

namespace IASLib
{
    class CResourceNode : public CObject
    {
        public:
            virtual            ~CResourceNode( void ) {}
            virtual bool        IsDirectory( void ) const = 0;
    };
}

#endif // IASLIB_RESOURCENODE_H__
