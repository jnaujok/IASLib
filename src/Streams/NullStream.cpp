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

namespace IASLib
{
    IMPLEMENT_OBJECT( CNullStream, CStream );

    CNullStream::CNullStream( void )
    {
    }

    CNullStream::~CNullStream( void )
    {
    }

    CString CNullStream::GetLine( void )
    {
        CString strRetVal = "";
        return strRetVal;
    }

    char CNullStream::GetChar( void )
    {
        return '\0';
    }

    unsigned char CNullStream::GetUChar( void )
    {
        return '\0';
    }

    void CNullStream::PutChar( const char )
    {
    }

    void CNullStream::PutChar( const unsigned char  )
    {
    }

    void CNullStream::PutLine( const CString & )
    {
    }

    char CNullStream::PeekChar()
    {
        return '\0';
    }

    int CNullStream::PutBuffer( const char *, int nLength )
    {
        return nLength;
    }

    int CNullStream::GetBuffer( char *achBuffer, int nLength )
    {
        memset( achBuffer, 0, nLength );
        return 0;
    }
} // namespace IASLib
