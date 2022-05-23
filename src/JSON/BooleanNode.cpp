/**
 * Object Node Class Implementation
 *
 *  This Class implements a JSON Object node which contains named Json nodes
 * in a non-order-dependent manner.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/24/2020
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_JSON_SUPPORT__

#include "BooleanNode.h"

namespace IASLib 
{
    IMPLEMENT_OBJECT(CBooleanNode,CValueNode)
    
    CBooleanNode::CBooleanNode( CJsonNode *parent, CString name, CString value) : CValueNode( parent, name) 
    {
        if ( value == "null" )
        {
            setNull();
        }
        else if ( value == "true" )
        {
            setValue( true );
        }
        else if ( value == "false" )
        {
            setValue( false );
        }
    }



} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
