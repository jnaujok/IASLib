/**
 * @file
 * @brief Socket Stream Class.
 *
 *      A stream over a connected socket, which the stream owns (it deletes
 * the socket when closed or destroyed) unless SetNoDelete(). Reading waits
 * for data; the stream ends (IsEOS()) when the other side has closed the
 * connection and everything it sent has been read. For many small reads
 * or writes, put a CBufferedStream over it.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997 (reworked 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SOCKETSTREAM_H__
#define IASLIB_SOCKETSTREAM_H__

#include "Stream.h"
#include "../Sockets/Socket.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{
	/// A stream over a connected CSocket.
	///
	/// The stream owns its socket and deletes it on Close() or destruction,
	/// unless SetNoDelete() was called. Socket errors do not throw: a read
	/// error ends the stream as a close does, and a write error stops the write.
	/// Reads block until data arrives. Not thread-safe.
	///
	/// @code
	/// CSocketStream oStream( pSocket );   // takes ownership of pSocket
	/// oStream.PutLine( "PING" );
	/// CString strReply = oStream.GetLine();
	/// @endcode
	class CSocketStream : public CStream
	{
		protected:
			CSocket            *m_pSocket;      ///< The socket, or NULL once closed.
            bool                m_bNoDelete;    ///< If true, Close() does not delete the socket.
            bool                m_bIsPeeked;    ///< True if m_chPeeked holds a byte already read.
            char                m_chPeeked;     ///< A byte read ahead by IsEOS() or PeekChar().
            bool                m_bClosed;      ///< the other side has closed

		public:
								/// Makes a closed stream with no socket.
								CSocketStream( void );
								/// Wraps a socket. The stream takes ownership of @p pSocket.
								CSocketStream( CSocket *pSocket );
			/// Calls Close().
			virtual            ~CSocketStream( void );

								DEFINE_OBJECT( CSocketStream );

			/// Reads up to @p nLength bytes, blocking until they arrive or the
			/// connection closes. (It used to ask again for ever, and add -1 on errors.)
			/// @return Bytes read; fewer than asked (maybe 0) when the connection
			///         closed or failed.
			virtual int         GetBuffer( char *achBuffer, int nLength );
            /// Sends @p nLength bytes, stopping at the first failed send.
            /// @return Bytes sent; 0 with no socket.
            virtual int         PutBuffer( const char *achBuffer, int nLength );
            /// Waits until a byte can be read or the connection has closed.
            /// The byte read is kept for the next read. (This used to answer
            /// "true" exactly when data was waiting.)
            virtual bool        IsEOS( void );
            /// The next byte, without taking it; waits for it if needed.
            /// A second peek returns the same byte (it used to take it).
            /// @throws CStreamException if the connection has closed.
            virtual char        PeekChar( void );
            /// A byte that has been peeked (and so is known to be there).
            /// @return 1 if a byte is held, else 0.
			virtual size_t		bytesRemaining( void ) { return m_bIsPeeked ? 1 : 0; }
			/// Deletes the socket (unless SetNoDelete() was called) and marks the stream closed.
			virtual void        Close( void );

			/// The socket, or NULL once closed.
			virtual CSocket    *GetSocket( void ) { return m_pSocket; }
            /// Keeps the socket alive when the stream closes; the caller then owns it.
            void                SetNoDelete( void ) { m_bNoDelete = true; }

		private:
                                CSocketStream( const CSocketStream & );     ///< Not copyable.
            CSocketStream      &operator =( const CSocketStream & );        ///< Not copyable.
	};
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SOCKETSTREAM_H__
