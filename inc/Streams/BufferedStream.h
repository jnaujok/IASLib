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

namespace IASLib
{
	class CBufferedStream : public CStream
	{
		protected:
			CStream            &m_oStream;
            char                m_chBuffer[4096];
            int                 m_nPointer;
            int                 m_nSize;
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
			virtual int         PutBuffer( const char *achBuffer, int nLength );
			virtual int         GetBuffer( char *achBuffer, int nLength );

            virtual bool        IsEOS( void ) { return m_oStream.IsEOS(); }

        private:
                                CBufferedStream( void );
                                CBufferedStream( const CBufferedStream &oSource );
            CBufferedStream    &operator =( const CBufferedStream &oSource );
            unsigned char       GetNextChar( void );
            bool                PutNextChar( const unsigned char chNext );

	};
} // namespace IASLib

#endif // IASLIB_BUFFEREDSTREAM_H__
