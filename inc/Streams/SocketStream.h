/*
 * Socket Stream Class
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
	class CSocketStream : public CStream
	{
		protected:
			CSocket            *m_pSocket;
            bool                m_bNoDelete;
            bool                m_bIsPeeked;
            char                m_chPeeked;
            bool                m_bClosed;      // the other side has closed

		public:
								CSocketStream( void );
								CSocketStream( CSocket *pSocket );
			virtual            ~CSocketStream( void );

								DEFINE_OBJECT( CSocketStream );

			virtual int         GetBuffer( char *achBuffer, int nLength );
            virtual int         PutBuffer( const char *achBuffer, int nLength );
                // Waits until a byte can be read or the connection has closed.
            virtual bool        IsEOS( void );
            virtual char        PeekChar( void );
                // A byte that has been peeked (and so is known to be there).
			virtual size_t		bytesRemaining( void ) { return m_bIsPeeked ? 1 : 0; }
			virtual void        Close( void );

			virtual CSocket    *GetSocket( void ) { return m_pSocket; }
            void                SetNoDelete( void ) { m_bNoDelete = true; }

		private:
                                CSocketStream( const CSocketStream & );
            CSocketStream      &operator =( const CSocketStream & );
	};
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SOCKETSTREAM_H__
