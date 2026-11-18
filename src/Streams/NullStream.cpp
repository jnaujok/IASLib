/*
 * Null Stream Implementor Class
 *
 *      This class extends the basic stream interface to support streaming
 * data to and from a null data sink. This, in effect, throws data away
 * while still supporting the same interface as an any other stream.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "NullStream.h"

#ifdef IASLIB_WIN32__
    // Hide warnings for unused parameters on Windows
#pragma warning(disable:4100)
#else
        // This stops the complaint about memset on non-windows systems (linux)
    #include <memory.h>
#endif

#include "StreamException.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CNullStream, CStream );

    char CNullStream::PeekChar( void )
    {
        IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        return '\0';
    }
} // namespace IASLib
