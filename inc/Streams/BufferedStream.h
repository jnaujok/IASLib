/**
 * @file
 * @brief Buffered Stream Class.
 *
 *      Reads and writes another stream in blocks of BUFFER_SIZE bytes, for
 * streams where each call is costly (sockets, unbuffered files). Writes
 * are held until the buffer fills, Flush() is called, or the buffered
 * stream is closed or destroyed. Destroying it flushes but leaves the
 * stream underneath open; Close() flushes and closes that stream too.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997 (reworked 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BUFFEREDSTREAM_H__
#define IASLIB_BUFFEREDSTREAM_H__

#include "Stream.h"

namespace IASLib
{
	/// Adds read and write buffers in front of another stream.
	///
	/// Reads fetch up to BUFFER_SIZE bytes at a time from the stream below;
	/// writes are held until BUFFER_SIZE bytes collect, Flush() is called,
	/// or the stream is closed or destroyed. Reads and writes of a full
	/// buffer or more go straight through.
	///
	/// The stream below is not owned: it must outlive this object, and the
	/// destructor only flushes it. Close() does close it.
	///
	/// @code
	/// CSocketStream oRaw( pSocket );
	/// CBufferedStream oStream( oRaw );
	/// oStream.PutLine( "HELLO" );
	/// oStream.Flush();
	/// CString strReply = oStream.GetLine();
	/// @endcode
	class CBufferedStream : public CStream
	{
        public:
            static const int    BUFFER_SIZE = 4096;     ///< Size of each buffer in bytes.

		protected:
			CStream            &m_oStream;                          ///< The stream below (not owned).
            char                m_chReadBuffer[ BUFFER_SIZE ];      ///< Bytes read ahead.
            char                m_chWriteBuffer[ BUFFER_SIZE ];     ///< Bytes waiting to be written.
            int                 m_nReadPos;         ///< next to hand out
            int                 m_nReadEnd;         ///< end of what was read
            int                 m_nWriteLength;     ///< held back

		public:
								/// Buffers @p oStream, which must outlive this object.
								CBufferedStream( CStream &oStream );
			/// Flushes held writes (ignoring any exception). The stream below is
			/// not ours to close, so it is left open.
			virtual            ~CBufferedStream( void );

								DEFINE_OBJECT( CBufferedStream );

			/// Reads up to @p nLength bytes, from the buffer first.
			/// @return Bytes read; 0 at the end, or if @p achBuffer is NULL or @p nLength <= 0.
			virtual int         GetBuffer( char *achBuffer, int nLength );
            /// Buffers @p nLength bytes for writing (flushing as the buffer fills).
            /// @return @p nLength, or what the stream below accepted for a large write.
            virtual int         PutBuffer( const char *achBuffer, int nLength );
            /// True when the buffer is empty and the stream below is at its end.
            virtual bool        IsEOS( void );
            /// The next byte, without taking it.
            /// @throws CStreamException at the end of the stream.
            virtual char        PeekChar( void );
			/// Reads one byte.
			/// @throws CStreamException at the end of the stream.
			virtual char        GetChar( void );
			/// Buffers one byte for writing.
			virtual void        PutChar( const char chPut );
			/// Bytes buffered plus what the stream below reports, or NOT_FOUND if it cannot tell.
			virtual size_t		bytesRemaining( void );
            /// Writes held bytes to the stream below, then flushes it.
            virtual void        Flush( void );
			/// Flushes, then closes the stream below.
			virtual void		Close( void );

        private:
            /// Reads the next block, if the buffer is empty.
            /// @return True if there are bytes in the buffer.
            bool                Fill( void );
                                CBufferedStream( void );                                ///< Not available.
                                CBufferedStream( const CBufferedStream &oSource );      ///< Not copyable.
            CBufferedStream    &operator =( const CBufferedStream &oSource );           ///< Not copyable.
	};
} // namespace IASLib

#endif // IASLIB_BUFFEREDSTREAM_H__
