/*
 * File Stream Class
 *
 *      A stream over a CFile. See FileStream.h.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "FileStream.h"
#include "StreamException.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CFileStream, CStream );

    CFileStream::CFileStream( void )
    {
        m_pFile = NULL;
        m_bIsOpen = false;
    }

    CFileStream::CFileStream( CFile *pFile )
    {
        m_pFile = pFile;
        m_bIsOpen = ( pFile != NULL );
    }

    CFileStream::CFileStream( const char *strFileName, bool bRead, bool bWrite )
    {
        int nMode = ( bRead && bWrite ) ? ( CFile::READ | CFile::WRITE ) : bRead ? CFile::READ : CFile::WRITE;
        m_pFile = new CFile( strFileName, nMode );
    }

    CFileStream::~CFileStream( void )
    {
        delete m_pFile;
        m_pFile = NULL;
    }

    void CFileStream::Close( void )
    {
        delete m_pFile;
        m_pFile = NULL;
        m_bIsOpen = false;
    }

    int CFileStream::GetBuffer( char *achBuffer, int nLength )
    {
        if ( m_pFile == NULL || achBuffer == NULL || nLength <= 0 )
            return 0;

        int nReceived = 0;
        while ( nReceived < nLength )
        {
                // In text mode (on Windows) line breaks shrink as they are
                // read, so the file's size cannot say how many bytes will
                // come: read until the file has none left.
            int nRead = m_pFile->Read( achBuffer + nReceived, nLength - nReceived );
            if ( nRead <= 0 )
                break;
            nReceived += nRead;
        }
        return nReceived;
    }

    int CFileStream::PutBuffer( const char *achBuffer, int nLength )
    {
        if ( m_pFile == NULL || achBuffer == NULL || nLength <= 0 )
            return 0;

        int nSent = 0;
        while ( nSent < nLength )
        {
            int nWritten = m_pFile->Write( achBuffer + nSent, nLength - nSent );
            if ( nWritten <= 0 )
                break;          // (it used to try again for ever)
            nSent += nWritten;
        }
        return nSent;
    }

    char CFileStream::PeekChar( void )
    {
        if ( IsEOS() )
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        long lPos = m_pFile->GetPos();
        char chPeek = '\0';
        if ( m_pFile->Read( &chPeek, 1 ) != 1 )
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
            // (It used to seek with SEEK_SET, which is not one of CFile's
            // seek modes.)
        m_pFile->Seek( lPos, CFile::BEGIN );
        return chPeek;
    }

    size_t CFileStream::bytesRemaining( void )
    {
        if ( m_pFile == NULL )
            return 0;
        long lRemaining = m_pFile->GetSize() - m_pFile->GetPos();
        return ( lRemaining > 0 ) ? (size_t)lRemaining : 0;
    }
} // namespace IASLib
