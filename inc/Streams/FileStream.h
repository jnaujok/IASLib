/*
 * File Stream Class
 *
 *      This class provides the basis for streaming data to a file. This
 * allows a file and other data source/sinks to be treated as a stream
 * without differentiating what we're actually writing to.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/18/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_FILESTREAM_H__
#define IASLIB_FILESTREAM_H__

#include "Stream.h"
#include "../Files/File.h"

namespace IASLib
{
	class CFileStream : public CStream
	{
		protected:
			CFile              *m_pFile;
		public:
								CFileStream( void );
								CFileStream( CFile *pFile );
								CFileStream( const char *strFileName, bool bRead, bool bWrite );
			virtual            ~CFileStream( void );

								DEFINE_OBJECT( CFileStream );

			virtual CString     GetLine( void );
			virtual char        GetChar( void );
			virtual void        PutChar( const char chPut );
			virtual void        PutLine( const CString &strOutput );
			virtual unsigned char        GetUChar( void );
			virtual void        PutChar( const unsigned char chPut );
			virtual int         PutBuffer( const char *achBuffer, int nLength );
			virtual int         GetBuffer( char *achBuffer, int nLength );
			virtual size_t		bytesRemaining( void );

            virtual bool        IsEOS( void ) { return m_pFile->IsEOF(); }

			virtual void 		Close( void );
	};
} // namespace IASLib

#endif // __FILESTREAM_H__
