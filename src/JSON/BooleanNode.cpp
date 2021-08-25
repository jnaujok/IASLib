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

namespace IASLib {
    CBooleanNode::CBooleanNode(CString name, bool value) : CValueNode(name) {
        bValue = value;
    }

    void setValue(bool value) {
        bValue = value;
    }

} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
