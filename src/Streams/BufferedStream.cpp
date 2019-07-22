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
#include "StreamException.h"

#define MIN(x,y) ((x < y) ? x:y)

namespace IASLib
{
    CBufferedStream::CBufferedStream( CStream &oStream )
        : m_oStream( oStream )
    {
        memset( m_chReadBuffer, 0, BUFFER_SIZE );
        memset( m_chWriteBuffer, 0, BUFFER_SIZE );
        m_nReadPointer = 0;
        m_nWritePointer = 0;
        m_nBufferRead = 0;
        m_nBufferWritten = 0;
    }

    CBufferedStream::~CBufferedStream( void )
    {
        Close();
    }

	IMPLEMENT_OBJECT( CBufferedStream, CStream );

    CString CBufferedStream::GetLine( void )
    {
        CString strRetVal;
        char inChar;

        while ( (!IsEOS() ) && (inChar = GetChar() ) != '\n' )
        {
            strRetVal += inChar;
        }

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
        PutNextChar( '\n' );
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
        int nOffset = 0, nSize = 0;

        while ( nOffset < nLength )
        {
            nSize = MIN( nLength - nOffset, BUFFER_SIZE - m_nWritePointer );

            memcpy( m_chWriteBuffer + m_nWritePointer, achBuffer + nOffset, nSize );
            m_nWritePointer += nSize;
            m_nBufferWritten += nSize;
            nOffset += nSize;

            if ( m_nWritePointer == BUFFER_SIZE )
            {
                m_oStream.PutBuffer( m_chWriteBuffer, m_nBufferWritten );
                m_nWritePointer = 0;
                m_nBufferWritten = 0;
            }
        }
        return nLength;
    }

    int CBufferedStream::GetBuffer( char *achBuffer, int nLength )
    {
        int retBytes = 0;
        // Test for cases
        if ( (m_nBufferRead - m_nReadPointer) >= nLength )
        {
            // We already have the data in memory
            memcpy( achBuffer, m_chReadBuffer + m_nReadPointer, nLength );
            m_nReadPointer += nLength;
            retBytes = nLength;
        }
        else
        {
            int nOffset = 0;
            while ( ( ( m_nBufferRead - m_nReadPointer ) < ( nLength - nOffset ) ) && ( ! IsEOS() ) )
            {
                memcpy( achBuffer + nOffset, m_chReadBuffer + m_nReadPointer, m_nBufferRead - m_nReadPointer );
                nOffset += m_nBufferRead - m_nReadPointer;
                m_nReadPointer = 0;
                m_nBufferRead = m_oStream.GetBuffer( m_chReadBuffer, BUFFER_SIZE );
            }

            if ( ( nOffset < nLength ) && ( ! IsEOS() ) )
            {
                int nSize = MIN( m_nBufferRead - m_nReadPointer, nLength - nOffset );
                memcpy( achBuffer + nOffset, m_chReadBuffer + m_nReadPointer, nSize );
                nOffset += nSize;
                m_nReadPointer = nSize;
            }
            retBytes = nOffset;
        }

        return retBytes;
    }

    unsigned char CBufferedStream::GetNextChar( void )
    {
        if (  ( m_nReadPointer == m_nBufferRead ) && ( ! IsEOS() ) )
        {
            m_nReadPointer = 0;
            m_nBufferRead = m_oStream.GetBuffer( m_chReadBuffer, BUFFER_SIZE );
        }

        if ( m_nReadPointer < m_nBufferRead )
        {
            unsigned char chRetVal = m_chReadBuffer[ m_nReadPointer ];
            m_nReadPointer++;
            return chRetVal;
        }

        IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        return '\0';
    }

    bool CBufferedStream::PutNextChar( const unsigned char chNext )
    {
        m_chWriteBuffer[m_nWritePointer] = chNext;
        m_nWritePointer++;

        if ( m_nWritePointer == BUFFER_SIZE )
        {
            m_oStream.PutBuffer( m_chWriteBuffer, m_nWritePointer );
            m_nWritePointer = 0;
            memset(m_chWriteBuffer, 0, BUFFER_SIZE );
        }

        return true;
    }

    bool CBufferedStream::IsEOS( void )
    {
        if ( ( m_nReadPointer == m_nBufferRead ) && ( m_oStream.IsEOS() ) )
        {
            return true;
        }
        return false;
    }

    void CBufferedStream::Close( void )
    {
        if ( ( m_oStream.IsOpen() ) && IsOpen() )
        {
            if ( m_nBufferWritten > 0 )
            {
                m_oStream.PutBuffer( m_chWriteBuffer, m_nBufferWritten );
                m_nWritePointer = 0;
                m_nBufferWritten = 0;
            }
            m_oStream.Close();
            m_bIsOpen = false;
        }
    }
} // namespace IASLib

