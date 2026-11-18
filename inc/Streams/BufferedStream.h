/*
 * Buffered Stream Class
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
	class CBufferedStream : public CStream
	{
        public:
            static const int    BUFFER_SIZE = 4096;

		protected:
			CStream            &m_oStream;
            char                m_chReadBuffer[ BUFFER_SIZE ];
            char                m_chWriteBuffer[ BUFFER_SIZE ];
            int                 m_nReadPos;         // next to hand out
            int                 m_nReadEnd;         // end of what was read
            int                 m_nWriteLength;     // held back

		public:
								CBufferedStream( CStream &oStream );
			virtual            ~CBufferedStream( void );

								DEFINE_OBJECT( CBufferedStream );

			virtual int         GetBuffer( char *achBuffer, int nLength );
            virtual int         PutBuffer( const char *achBuffer, int nLength );
            virtual bool        IsEOS( void );
            virtual char        PeekChar( void );
			virtual char        GetChar( void );
			virtual void        PutChar( const char chPut );
			virtual size_t		bytesRemaining( void );
            virtual void        Flush( void );
			virtual void		Close( void );

        private:
            bool                Fill( void );
                                CBufferedStream( void );
                                CBufferedStream( const CBufferedStream &oSource );
            CBufferedStream    &operator =( const CBufferedStream &oSource );
	};
} // namespace IASLib

#endif // IASLIB_BUFFEREDSTREAM_H__
