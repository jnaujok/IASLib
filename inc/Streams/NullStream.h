/*
 * Null Stream Class
 *
 *      This class provides the basis for streaming data to a null. In
 * other words, everything you send to this file goes to the great bit
 * bucket in the sky.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/19/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_NULLSTREAM_H__
#define IASLIB_NULLSTREAM_H__

#include "Stream.h"

namespace IASLib
{
        // A stream that throws away what is written and has nothing to read
        // (reading from it, like reading past the end of any stream, throws).
	class CNullStream : public CStream
	{
		public:
								CNullStream( void ) {}
			virtual            ~CNullStream( void ) {}

								DEFINE_OBJECT( CNullStream );

			virtual int         GetBuffer( char *, int ) { return 0; }
            virtual int         PutBuffer( const char *, int nLength ) { return ( nLength > 0 ) ? nLength : 0; }
            virtual bool        IsEOS( void ) { return true; }
            virtual char        PeekChar( void );
            virtual CString     GetLine( void ) { return CString(); }
			virtual size_t		bytesRemaining( void ) { return 0; }
			virtual void 		Close( void ) { m_bIsOpen = false; }
	};
} // namespace IASLib

#endif // IASLIB_NULLSTREAM_H__
