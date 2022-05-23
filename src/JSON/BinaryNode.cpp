/**
 * Binary Node Class Implementation
 *
 *  This Class implements a JSON value node that stores Binary data in Base64
 *  encoded format.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/25/2020
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_JSON_SUPPORT__

#include "BinaryNode.h"

namespace IASLib {

    IMPLEMENT_OBJECT(CBinaryNode,CValueNode)

    CBinaryNode::CBinaryNode( CString name, CString value ) : CValueNode( name )
    {
        dataBlock = new CDataBlock( value.GetLength() );


    }

}

#endif // IASLIB_JSON_SUPPORT__

