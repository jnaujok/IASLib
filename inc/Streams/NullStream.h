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
	class CNullStream : public CStream
	{
		public:
								CNullStream( void );
			virtual            ~CNullStream( void );

								DEFINE_OBJECT( CNullStream );

			virtual CString     GetLine( void );
			virtual char        GetChar( void );
			virtual void        PutChar( const char chPut );
			virtual void        PutLine( const CString &strOutput );
			virtual unsigned char        GetUChar( void );
			virtual void        PutChar( const unsigned char chPut );
            virtual char        PeekChar( void );
            virtual int         PutBuffer( const char *achBuffer, int nLength );
			virtual int         GetBuffer( char *achBuffer, int nLength );
			virtual size_t		bytesRemaining( void ) { return 0; }

            virtual bool        IsEOS( void ) { return true; }

			virtual void 		Close() { m_bIsOpen = false; }
	};
} // namespace IASLib

#endif // IASLIB_NULLSTREAM_H__
