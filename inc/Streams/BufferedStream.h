/*
 * Buffered Stream Class
 *
 *      This class allows you to take any other stream type
 * and buffer both the input and output so that the stream
 * is reading and writing in 4K blocks.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/12/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BUFFEREDSTREAM_H__
#define IASLIB_BUFFEREDSTREAM_H__

#include "Stream.h"

#define BUFFER_SIZE 4096

namespace IASLib
{
	class CBufferedStream : public CStream
	{
		protected:
			CStream            &m_oStream;
            char                m_chReadBuffer[BUFFER_SIZE];
            char                m_chWriteBuffer[BUFFER_SIZE];
            int                 m_nReadPointer;
            int                 m_nWritePointer;
            int                 m_nBufferRead;
			int					m_nBufferWritten;
		public:
								CBufferedStream( CStream &oStream );
			virtual            ~CBufferedStream( void );

								DEFINE_OBJECT( CBufferedStream );

			virtual CString     GetLine( void );
			virtual char        GetChar( void );
			virtual void        PutChar( const char chPut );
			virtual void        PutLine( const CString &strOutput );
			virtual unsigned char        GetUChar( void );
			virtual void        PutChar( const unsigned char chPut );
            virtual char        PeekChar( void );
            virtual int         PutBuffer( const char *achBuffer, int nLength );
			virtual int         GetBuffer( char *achBuffer, int nLength );
			virtual size_t		bytesRemaining( void );

            virtual bool        IsEOS( void );

			virtual void		Close( void );
        private:
                                CBufferedStream( void );
                                CBufferedStream( const CBufferedStream &oSource );
            CBufferedStream    &operator =( const CBufferedStream &oSource );
            unsigned char       GetNextChar( void );
            bool                PutNextChar( const unsigned char chNext );

	};
} // namespace IASLib

#endif // IASLIB_BUFFEREDSTREAM_H__
