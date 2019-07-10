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

#include "BufferedStream.h"

namespace IASLib
{
    CBufferedStream::CBufferedStream( CStream &oStream )
        : m_oStream( oStream )
    {
        m_nSize = m_oStream.GetBuffer( m_chBuffer, 4096 );
        m_nPointer = 0;
    }

    CBufferedStream::~CBufferedStream( void )
    {
    }

	IMPLEMENT_OBJECT( CBufferedStream, CStream );

    CString CBufferedStream::GetLine( void )
    {
        CString strRetVal;

        return strRetVal;
    }

    char CBufferedStream::GetChar( void )
    {
        return (char)GetNextChar( );
    }

    void CBufferedStream::PutChar( const char chPut )
    {
        PutNextChar( (unsigned char)chPut );
    }

    void CBufferedStream::PutLine( const CString &strOutput )
    {
        PutBuffer( (const char *)strOutput, (int)strOutput.GetCount() );
    }

    unsigned char CBufferedStream::GetUChar( void )
    {
        return GetNextChar();
    }

    void CBufferedStream::PutChar( const unsigned char chPut )
    {
        PutNextChar( chPut );
    }

    int CBufferedStream::PutBuffer( const char *achBuffer, int nLength )
    {
        // Magic with memcpy
        memcpy( m_chBuffer, achBuffer, nLength );
        return 0;
    }

    int CBufferedStream::GetBuffer( char *achBuffer, int nLength )
    {
        // More Magic with memcpy
        memcpy( achBuffer, m_chBuffer, nLength );

        return nLength;
    }

    unsigned char CBufferedStream::GetNextChar( void )
    {
        unsigned char chRetVal = m_chBuffer[ m_nPointer ];
        m_nPointer++;

        if ( m_nPointer == 4096 )
        {
            // Rollover logic
        }

        return chRetVal;
    }

    bool CBufferedStream::PutNextChar( const unsigned char chNext )
    {
        m_chBuffer[m_nPointer] = chNext;
        m_nPointer++;

        if ( m_nPointer == 4096 )
        {
            // Rollover logic
        }

        return true;
    }

    bool CBufferedStream::IsEOS( void )
    {
        if ( ( m_nPointer == m_nSize ) && ( m_oStream.IsEOS() ) )
        {
            return true;
        }
        return false;
    }
} // namespace IASLib

