/**
 * JSON Node Type Enumeration
 *
 *  This enumeration represents the type of the JSON Node.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_JSONNODETYPE_H__
#define IASLIB_JSONNODETYPE_H__

#ifdef IASLIB_JSON_SUPPORT__

namespace IASLib
{
    enum JsonNodeType
    {
        ARRAY,
        BINARY,         // bytes, written as a base64 string
        BOOLEAN,
        MISSING,        // what path() and at() give for a value that is not there
        NULLVAL,
        NUMBER,
        OBJECT,
        STRING
    };
}

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_JSONNODETYPE_H__
