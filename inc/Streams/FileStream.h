/**
 * @file
 * @brief File Stream Class.
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
	/// A stream over a CFile.
	///
	/// The stream owns its CFile: the destructor and Close() delete it.
	///
	/// @code
	/// CFileStream oStream( "data.txt", true, false );
	/// while ( ! oStream.IsEOS() )
	///     ProcessLine( oStream.GetLine() );
	/// @endcode
	class CFileStream : public CStream
	{
		protected:
			CFile              *m_pFile;    ///< The file (owned), or NULL once closed.

		public:
								/// Makes a closed stream with no file.
								CFileStream( void );
								/// Wraps an open file. The stream takes ownership of @p pFile.
								CFileStream( CFile *pFile );
								/// Opens a file.
								/// @param strFileName Path of the file.
								/// @param bRead Open for reading.
								/// @param bWrite Open for writing. With neither flag set, the file
								///        is opened for writing.
								/// @note IsOpen() returns true even if the file could not be opened;
								///       check GetFile()->IsOpen().
								CFileStream( const char *strFileName, bool bRead, bool bWrite );
			/// Deletes the file (which closes it).
			virtual            ~CFileStream( void );

								DEFINE_OBJECT( CFileStream );

			/// Reads up to @p nLength bytes.
			/// @return Bytes read; 0 at the end, with no file, or if @p achBuffer
			///         is NULL or @p nLength <= 0.
			virtual int         GetBuffer( char *achBuffer, int nLength );
            /// Writes @p nLength bytes, stopping at the first failed write.
            /// (It used to try again for ever.)
            /// @return Bytes written; 0 with no file.
            virtual int         PutBuffer( const char *achBuffer, int nLength );
            /// True with no file, or at the end of the file.
            virtual bool        IsEOS( void ) { return m_pFile == NULL || m_pFile->IsEOF(); }
            /// Reads the next byte and seeks back.
            /// (It used to seek with SEEK_SET, which is not one of CFile's seek modes.)
            /// @throws CStreamException at the end of the file.
            virtual char        PeekChar( void );
			/// File size minus position; 0 with no file.
			virtual size_t		bytesRemaining( void );
			/// Deletes the file (which closes it).
			virtual void 		Close( void );

            /// The file, still owned by the stream; NULL once closed.
            CFile              *GetFile( void ) { return m_pFile; }

		private:
								CFileStream( const CFileStream & );     ///< Not copyable.
			CFileStream        &operator =( const CFileStream & );      ///< Not copyable.
	};
} // namespace IASLib

#endif // IASLIB_FILESTREAM_H__
