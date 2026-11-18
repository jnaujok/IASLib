/**
 * @file
 * @brief Null Stream Class.
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
    /// A stream that throws away what is written and has nothing to read
    /// (reading from it, like reading past the end of any stream, throws).
	class CNullStream : public CStream
	{
		public:
								/// Makes an open null stream.
								CNullStream( void ) {}
			/// Destructor.
			virtual            ~CNullStream( void ) {}

								DEFINE_OBJECT( CNullStream );

			/// Always 0: there is nothing to read.
			virtual int         GetBuffer( char *, int ) { return 0; }
            /// Discards the bytes. @return @p nLength (0 if it is negative).
            virtual int         PutBuffer( const char *, int nLength ) { return ( nLength > 0 ) ? nLength : 0; }
            /// Always true.
            virtual bool        IsEOS( void ) { return true; }
            /// Always throws.
            /// @throws CStreamException always.
            virtual char        PeekChar( void );
            /// Always "".
            virtual CString     GetLine( void ) { return CString(); }
			/// Always 0.
			virtual size_t		bytesRemaining( void ) { return 0; }
			/// Marks the stream closed.
			virtual void 		Close( void ) { m_bIsOpen = false; }
	};
} // namespace IASLib

#endif // IASLIB_NULLSTREAM_H__
