/*
 * File Stream Class
 *
 *      A stream over a CFile, which the stream owns (it deletes the file
 * when it is closed or destroyed). Files opened in text mode may read
 * back fewer bytes than their size on Windows (CRLF becomes LF); open in
 * binary mode for exact bytes.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997 (reworked 11/18/2026)
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

			virtual int         GetBuffer( char *achBuffer, int nLength );
            virtual int         PutBuffer( const char *achBuffer, int nLength );
            virtual bool        IsEOS( void ) { return m_pFile == NULL || m_pFile->IsEOF(); }
            virtual char        PeekChar( void );
			virtual size_t		bytesRemaining( void );
			virtual void 		Close( void );

            CFile              *GetFile( void ) { return m_pFile; }

		private:
								CFileStream( const CFileStream & );
			CFileStream        &operator =( const CFileStream & );
	};
} // namespace IASLib

#endif // IASLIB_FILESTREAM_H__
