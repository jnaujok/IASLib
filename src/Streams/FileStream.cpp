/*
 * File Stream Implementor Class
 *
 *      This class extends the basic stream interface to support streaming
 * data to and from a file data source/sink. This allows us to treat a file
 * like any other stream without the implementing code caring about the 
 * final destination.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "FileStream.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CFileStream, CStream );

    CFileStream::CFileStream( void )
    {
        m_pFile = NULL;
    }

    CFileStream::CFileStream( CFile *pFile )
    {
        m_pFile = pFile;
    }

    CFileStream::CFileStream( const char *strFileName, bool bRead, bool bWrite )
    {
        if ( ( bRead ) && ( ! bWrite ) )
        {
            m_pFile = new CFile( strFileName, CFile::READ );
        }
        else
        {
            if ( bRead && bWrite )
            {
                m_pFile = new CFile( strFileName, CFile::READ | CFile::WRITE );
            }
            else
            {
                m_pFile = new CFile( strFileName, CFile::WRITE );
            }
        }
    }

    CFileStream::~CFileStream( void )
    {
        if ( m_pFile )
        {
            delete m_pFile;
        }
    }

    void CFileStream::Close( void )
    {
        if ( m_pFile )
        {
            delete m_pFile;
            m_pFile = NULL;
        }

        m_bIsOpen = false;
    }

    CString CFileStream::GetLine( void )
    {
        CString strRetVal = "";
        char chWork;

        if ( m_pFile )
        {
            while ( ( ! m_pFile->IsEOF() ) && ( ( chWork = GetChar() ) != '\n' ) )
            {
                if ( ( chWork != '\r' ) && ( chWork != '\0' ) )
                {
                    strRetVal += chWork;
                }
            }
        }

        return strRetVal;
    }

    char CFileStream::GetChar( void )
    {
        if ( m_pFile )
        {
            if ( ! m_pFile->IsEOF() )
            {
                char chRetVal;

                m_pFile->Read( &chRetVal, sizeof( char ) );

                return chRetVal;
            }
        }
        return '\0';
    }

    unsigned char CFileStream::GetUChar( void )
    {
        if ( m_pFile )
        {
            if ( ! m_pFile->IsEOF() )
            {
                unsigned char chRetVal;

                m_pFile->Read( (char *)&chRetVal, sizeof( unsigned char ) );

                return chRetVal;
            }
        }
        return '\0';
    }

    void CFileStream::PutChar( const char chPut )
    {
        if ( m_pFile )
        {
            char chWork = chPut;
            m_pFile->Write( &chWork, sizeof( char ) );
        }
    }

    void CFileStream::PutChar( const unsigned char chPut )
    {
        if ( m_pFile )
        {
            unsigned char chWork = chPut;
            m_pFile->Write( (char *)&chWork, sizeof( unsigned char ) );
        }
    }

    void CFileStream::PutLine( const CString &strOutput )
    {
        if ( m_pFile )
        {
            size_t      nLength = strOutput.GetLength();
            const char *pszOutput = (const char *)strOutput;

            m_pFile->Write( pszOutput, (int)(nLength * sizeof(char)) );
            char chRet = '\n';
            m_pFile->Write( &chRet, sizeof( char ) );
        }
    }

    int CFileStream::PutBuffer( const char *achBuffer, int nLength )
    {
        int nSent = 0;

        while ( nSent < nLength )
        {
            nSent += m_pFile->Write( &(achBuffer[nSent]), nLength - nSent );
        }

        return nSent;
    }

    int CFileStream::GetBuffer( char *achBuffer, int nLength )
    {
        int nReceived = 0;
        int nRemaining = (int)(m_pFile->GetSize() - m_pFile->GetPos());
        if ( nRemaining < nLength )
        {
            nLength = nRemaining;
        }

        while ( nReceived < nLength )
        {
            nReceived += m_pFile->Read( achBuffer + nReceived, nLength - nReceived );
        }

        return nReceived;
    }

    size_t CFileStream::bytesRemaining( void )
    {
        return (m_pFile->GetSize() - m_pFile->GetPos());
    }

} // namespace IASLib
