/**
 * Value Node Class Implementation
 *
 *  This Class implements an abstract JSON value node that provides the base class
 *  for all derived value node types.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/25/2020
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"

namespace IASLib {

    IMPLEMENT_OBJECT(CValueNode,CJsonNode)


    CValueNode::CValueNode( const CJsonNode *parent, const CString &name )
    {
        setParent( parent );
        setName( name );
    }

    CValueNode::CValueNode( const CString &name )
    {
        setParent( nullptr );
        setName( name );
    }    
    

}
#endif // IASLIB_JSON_SUPPORT__
