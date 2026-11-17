/**
 * Value Node Class
 *
 *  The base of the JSON nodes that hold a single value: text, numbers,
 * booleans, null and binary data. Value nodes have no children.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_VALUENODE_H__
#define IASLIB_VALUENODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "JsonNode.h"

namespace IASLib
{
    class CValueNode : public CJsonNode
    {
        public:
            DEFINE_OBJECT( CValueNode )

            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_VALUENODE_H__
