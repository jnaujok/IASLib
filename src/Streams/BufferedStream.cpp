/*
 * Buffered Stream Class
 *
 *      Reads and writes another stream in blocks. See BufferedStream.h.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "BufferedStream.h"
#include "StreamException.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CBufferedStream, CStream );

    CBufferedStream::CBufferedStream( CStream &oStream )
        : m_oStream( oStream ), m_nReadPos( 0 ), m_nReadEnd( 0 ), m_nWriteLength( 0 )
    {
    }

    CBufferedStream::~CBufferedStream( void )
    {
            // Flushed, but the stream underneath is not ours to close.
        try
        {
            Flush();
        }
        catch ( ... )
        {
        }
    }

        // Reads the next block, if the buffer is empty.
    bool CBufferedStream::Fill( void )
    {
        if ( m_nReadPos < m_nReadEnd )
            return true;
        m_nReadPos = 0;
        m_nReadEnd = 0;
        if ( m_oStream.IsEOS() )
            return false;
        int nRead = m_oStream.GetBuffer( m_chReadBuffer, BUFFER_SIZE );
        m_nReadEnd = ( nRead > 0 ) ? nRead : 0;
        return m_nReadEnd > 0;
    }

    bool CBufferedStream::IsEOS( void )
    {
        return m_nReadPos >= m_nReadEnd && m_oStream.IsEOS();
    }

    char CBufferedStream::PeekChar( void )
    {
        if ( ! Fill() )
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        return m_chReadBuffer[ m_nReadPos ];
    }

    char CBufferedStream::GetChar( void )
    {
        if ( ! Fill() )
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        return m_chReadBuffer[ m_nReadPos++ ];
    }

    int CBufferedStream::GetBuffer( char *achBuffer, int nLength )
    {
        if ( achBuffer == NULL || nLength <= 0 )
            return 0;

        int nCopied = 0;
        while ( nCopied < nLength )
        {
            if ( m_nReadPos >= m_nReadEnd )
            {
                    // Large reads go straight through.
                if ( nLength - nCopied >= BUFFER_SIZE )
                {
                    if ( m_oStream.IsEOS() )
                        break;
                    int nRead = m_oStream.GetBuffer( achBuffer + nCopied, nLength - nCopied );
                    if ( nRead <= 0 )
                        break;
                    nCopied += nRead;
                    continue;
                }
                if ( ! Fill() )
                    break;
            }
            int nTake = m_nReadEnd - m_nReadPos;
            if ( nTake > nLength - nCopied )
                nTake = nLength - nCopied;
            memcpy( achBuffer + nCopied, m_chReadBuffer + m_nReadPos, (size_t)nTake );
            m_nReadPos += nTake;
            nCopied += nTake;
        }
        return nCopied;
    }

    void CBufferedStream::PutChar( const char chPut )
    {
        if ( m_nWriteLength == BUFFER_SIZE )
            Flush();
        m_chWriteBuffer[ m_nWriteLength++ ] = chPut;
    }

    int CBufferedStream::PutBuffer( const char *achBuffer, int nLength )
    {
        if ( achBuffer == NULL || nLength <= 0 )
            return 0;

        if ( nLength >= BUFFER_SIZE )
        {
                // Large writes go straight through, after what is held.
            Flush();
            return m_oStream.PutBuffer( achBuffer, nLength );
        }

        int nOffset = 0;
        while ( nOffset < nLength )
        {
            if ( m_nWriteLength == BUFFER_SIZE )
                Flush();
            int nTake = BUFFER_SIZE - m_nWriteLength;
            if ( nTake > nLength - nOffset )
                nTake = nLength - nOffset;
            memcpy( m_chWriteBuffer + m_nWriteLength, achBuffer + nOffset, (size_t)nTake );
            m_nWriteLength += nTake;
            nOffset += nTake;
        }
        return nLength;
    }

    void CBufferedStream::Flush( void )
    {
        if ( m_nWriteLength > 0 )
        {
            int nLength = m_nWriteLength;
            m_nWriteLength = 0;
            m_oStream.PutBuffer( m_chWriteBuffer, nLength );
        }
        m_oStream.Flush();
    }

    void CBufferedStream::Close( void )
    {
        if ( IsOpen() )
        {
            Flush();
            m_oStream.Close();
            m_bIsOpen = false;
        }
    }

    size_t CBufferedStream::bytesRemaining( void )
    {
        size_t nBelow = m_oStream.bytesRemaining();
        if ( nBelow == NOT_FOUND )
            return NOT_FOUND;
        return nBelow + (size_t)( m_nReadEnd - m_nReadPos );
    }
} // namespace IASLib
