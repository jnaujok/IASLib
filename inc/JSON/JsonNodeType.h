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

#ifdef IASLIB_JSONSUPPORT__

namespace IASLib
{
    enum JsonNodeType
    {
        ARRAY,
        BINARY,
        BOOLEAN,
        MISSING,
        NULLVAL,
        NUMBER,
        OBJECT,
        POJO,
        STRING
    };
}

#endif // IASLIB_JSONSUPPORT__
#endif // IASLIB_JSONNODETYPE_H__

