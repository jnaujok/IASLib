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
        // The names carry a JSON_ prefix because the bare ones (BOOLEAN,
        // OBJECT, STRING and so on) collide with the typedefs and macros of
        // the Windows headers. (They used to be ARRAY, BINARY, BOOLEAN,
        // MISSING, NULLVAL, NUMBER, OBJECT and STRING.)
    enum JsonNodeType
    {
        JSON_ARRAY,
        JSON_BINARY,        // bytes, written as a base64 string
        JSON_BOOLEAN,
        JSON_MISSING,       // what path() and at() give for a value that is not there
        JSON_NULL,
        JSON_NUMBER,
        JSON_OBJECT,
        JSON_STRING

#if !defined( _WIN32 ) && !defined( IASLIB_JSON_NO_LEGACY_NAMES__ )
            // The old names, kept as aliases where they cannot collide with
            // the Windows headers, so existing code still compiles. They are
            // the same values, so a switch must not use both an old and a new
            // name for one type. Define IASLIB_JSON_NO_LEGACY_NAMES__ to drop
            // them; new code should use the JSON_ names.
        ,
        ARRAY = JSON_ARRAY,
        BINARY = JSON_BINARY,
        BOOLEAN = JSON_BOOLEAN,
        MISSING = JSON_MISSING,
        NULLVAL = JSON_NULL,
        NUMBER = JSON_NUMBER,
        OBJECT = JSON_OBJECT,
        STRING = JSON_STRING
#endif
    };
}

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_JSONNODETYPE_H__
