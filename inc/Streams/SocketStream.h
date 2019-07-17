/*
 * Socket Stream Class
 *
 *      This class provides the basis for streaming data to a socket.
 * This allows a socket and other data source/sinks to be treated as
 * a stream without differentiating what we're actually writing to. It
 * should be noted that a socket stream is one of the few types of stream
 * which is always both read and write capable.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/18/1997
 *	Log:
 *	  $LOG$
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
		public:
								CSocketStream( void );
								CSocketStream( CSocket *pSocket );
			virtual            ~CSocketStream( void );

								DEFINE_OBJECT( CSocketStream );

			virtual CString     GetLine( void );
			virtual char        GetChar( void );
			virtual void        PutChar( const char chPut );
			virtual void        PutLine( const CString &strOutput );
			virtual unsigned char        GetUChar( void );
			virtual void        PutChar( const unsigned char chPut );
			virtual int         PutBuffer( const char *achBuffer, int nLength );
			virtual int         GetBuffer( char *achBuffer, int nLength );
			virtual CSocket    *GetSocket( void ) { return m_pSocket; }

            void                SetNoDelete( void ) { m_bNoDelete = true; }

            virtual bool        IsEOS( void );

			virtual void        Close( void );
	};
} // namespace IASLib
#endif // IASLIB_NETWORKING__
#endif // IASLIB_SOCKETSTREAM_H__
