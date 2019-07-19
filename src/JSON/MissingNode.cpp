/**
 * Missing Node Class
 *
 *  This class defines a Node that is missing -- not null, not empty, but
 * a node that doesn't exist at all.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 5/06/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "MissingNode.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CMissingNode, CJsonNode );
    
    CMissingNode *CMissingNode::instance = NULL;

    CMissingNode *CMissingNode::getInstance( void )
    {
        if ( instance == NULL )
        {
            instance = new CMissingNode(); 
        }
        return instance;
    }

}; // namespace IASLib


