/**
 * File Class
 *
 *      This class provides an object to wrap standard file I/O processes
 * to provide the class library with file support. It is used mainly to
 * provide an object oriented file object.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 04/10/1994
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "File.h"
#include "FileException.h"
#include <errno.h>
#include <limits.h>
#include <string.h>
#ifdef IASLIB_WIN32__
#include <share.h>
#endif
#include <iostream>

#ifndef IASLIB_WIN32__
#include <sys/stat.h>
#include <unistd.h> // unlink()
#endif

namespace IASLib
{
    namespace
    {
            // The C library's 64-bit seek and tell.
        int seek64( FILE *pFile, int64_t nPos, int nWhence )
        {
#ifdef IASLIB_WIN32__
            return _fseeki64( pFile, (__int64)nPos, nWhence );
#else
            return fseeko( pFile, (off_t)nPos, nWhence );
#endif
        }

        int64_t tell64( FILE *pFile )
        {
#ifdef IASLIB_WIN32__
            return (int64_t)_ftelli64( pFile );
#else
            return (int64_t)ftello( pFile );
#endif
        }

        long toLong( int64_t n )
        {
            return ( n < 0 || n > (int64_t)LONG_MAX ) ? -1L : (long)n;
        }
    }

    IMPLEMENT_OBJECT( CFile, CObject );

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CFile::CFile()
    {
        m_strFileName = "";
        m_bAppend = false;
        m_bInput = false;
        m_bOutput = false;
        m_bBinary = false;
        m_fpFile = NULL;
        m_lFileSize = -1;
        m_lCurrentPos = -1;
    }

#ifdef IASLIB_WIN32__
    CFile::CFile( int nHandle, int nMode )
    {
        m_strFileName = "Unknown";
        m_bAppend = false;
        m_bInput = false;
        m_bOutput = false;
        m_bBinary = false;
        m_fpFile = NULL;
        m_lFileSize = -1;
        m_lCurrentPos = -1;

        CString strMode = ModeString( nMode );
        m_fpFile = _fdopen( nHandle, strMode );
    }
#endif // IASLIB_WIN32__

    CFile::CFile( const char *strFileName, int nMode )
    {
        m_strFileName = "";
        m_bAppend = false;
        m_bInput = false;
        m_bOutput = false;
        m_bBinary = false;
        m_fpFile = NULL;
        m_lFileSize = -1;
        m_lCurrentPos = -1;
        Open( strFileName, nMode );
    }

    CFile::~CFile()
    {
        m_lCurrentPos = -1;
        Close();
    }

        // (The flags were never reset, so reopening in another mode kept
        // the old ones; and with no READ or WRITE flag the mode fell back to
        // "r", dropping BINARY.)
    CString CFile::ModeString( int nMode )
    {
        m_bInput = false;
        m_bOutput = false;
        m_bAppend = false;
        m_bBinary = ( nMode & BINARY ) != 0;

        if ( ( nMode & ( READ | WRITE | APPEND | CREATE ) ) == 0 )
            nMode |= READ;
        if ( nMode & CREATE )
            nMode |= WRITE;

        CString strMode;
        if ( nMode & APPEND )
        {
            m_bAppend = true;
            m_bInput = ( nMode & READ ) != 0;
            strMode = ( m_bInput ) ? "a+" : "a";
        }
        else if ( ( nMode & READ ) && ( nMode & WRITE ) )
        {
            m_bInput = true;
            m_bOutput = true;
            strMode = ( nMode & CREATE ) ? "w+" : "r+";
        }
        else if ( nMode & WRITE )
        {
            m_bOutput = true;
            strMode = "w";
        }
        else
        {
            m_bInput = true;
            strMode = "r";
        }
        if ( m_bBinary )
            strMode += "b";
        return strMode;
    }

    bool CFile::Open( const char *strFileName, int nMode )
    {
        if ( m_fpFile )
        {
            Close();
            m_lFileSize = -1;
        }

        m_lCurrentPos = -1;
        m_lFileSize = -1;
        m_strFileName = ( strFileName ) ? strFileName : "";

        CString strMode = ModeString( nMode );

#ifdef IASLIB_WIN32__
            // Shared, as fopen() is elsewhere: fopen_s() opens files that no
            // one else can open, so a log could not be read while it was
            // being written.
        m_fpFile = _fsopen( m_strFileName, strMode, _SH_DENYNO );
#else
		m_fpFile = fopen( m_strFileName, strMode );
#endif

        if( ! m_fpFile )
        {
            IASLIB_THROW_FILE_EXCEPTION( errno );
        }

        return true;
    }

    bool CFile::Close( void )
    {
        if ( m_fpFile != NULL )
        {
                // (fclose()'s result was ignored: a failed final write went
                // unnoticed.)
            bool bOk = ( fclose( m_fpFile ) == 0 );
            m_fpFile = NULL;
            m_lFileSize = -1;
            m_lCurrentPos = -1;
            return bOk;
        }
        return false;
    }

    bool CFile::Flush( void )
    {
        if ( ( m_fpFile ) && ( m_bOutput || m_bAppend ) )
        {
            return fflush( m_fpFile ) == 0;
        }
        return false;
    }

    int CFile::Read( char *pchBuffer, int nBufferSize )
    {
        if ( ( m_fpFile ) && ( m_bInput ) )
        {
                // We read nBufferSize records of one byte each
                // to allow us to return how many bytes were
                // actually read instead of getting either
                // 1 record or zero records.
            size_t nRetVal = fread( pchBuffer, 1, (size_t)nBufferSize, m_fpFile );
            if ( m_lCurrentPos != -1 )
            {
                m_lCurrentPos += (int64_t)nRetVal;
                if ( ! m_bBinary )
                {
                        // If we're in text mode, there's no way to know
                        // if a \r\n was made into a \n, so if we find a
                        // \n in the buffer, then we have to do an ftell
                        // to get our position. On windows, this causes
                        // the buffer to invalidate, so it's an expensive
                        // operation. Bugger.
                    for (size_t nFind = 0; nFind < nRetVal; nFind++ )
                    {
                        if ( pchBuffer[ nFind ] == '\n' )
                        {
                            m_lCurrentPos = tell64( m_fpFile );
                            break;
                        }
                    }
                }
            }
            return (int)nRetVal;
        }
        return -1;
    }

    int CFile::Write( const char *pchBuffer, int nBufferSize )
    {
        if ( ( m_fpFile ) && ( m_bOutput || m_bAppend ) )
        {
                // We write nBufferSize records of one byte each
                // to allow us to return how many bytes were
                // actually written instead of getting either
                // 1 record or zero records.
            int nWriteSize = (int)fwrite( pchBuffer, 1, nBufferSize, m_fpFile );

            if ( m_lCurrentPos != -1 )
            {
                m_lCurrentPos += (int64_t)nWriteSize;
                if ( ( m_lCurrentPos > m_lFileSize ) && ( m_lFileSize != -1 ) )
                {
                    m_lFileSize = m_lCurrentPos;
                }
            }
            return nWriteSize;
        }
        return -1;
    }

    int CFile::WriteString( const CString &oString )
    {
        return Write( (const char *)oString, (int)oString.GetLength() );
    }

    long CFile::GetPos( void )
    {
        return toLong( GetPosBig() );
    }

    long CFile::GetSize( void )
    {
        return toLong( GetSizeBig() );
    }

    long CFile::Seek( long nPos, int nStart )
    {
        return toLong( SeekBig( (int64_t)nPos, nStart ) );
    }

    int64_t CFile::GetPosBig( void )
    {
        if ( m_fpFile )
        {
            if ( m_lCurrentPos == -1 )
            {
                m_lCurrentPos = tell64( m_fpFile );
            }
            return m_lCurrentPos;
        }

        return -1;
    }

    int64_t CFile::GetSizeBig( void )
    {
        if ( m_lFileSize == -1 && m_fpFile )
        {
            int64_t lCurrent = GetPosBig();
            if ( SeekBig( 0, END ) >= 0 )
                m_lFileSize = GetPosBig();
            if ( lCurrent >= 0 )
                SeekBig( lCurrent, BEGIN );
        }

        return m_lFileSize;
    }

        // (Seek() went through fseek() and ftell() with a long, so nothing
        // past 2 GB could be reached where long is 32 bits; and a failed
        // seek returned the old position as if it had worked.)
    int64_t CFile::SeekBig( int64_t nPos, int nStart )
    {
        if ( m_fpFile == NULL )
            return -1;

        int nWhence;
        switch ( nStart )
        {
            case BEGIN:     nWhence = SEEK_SET; break;
            case CURRENT:   nWhence = SEEK_CUR; break;
            case END:       nWhence = SEEK_END; break;
            default:        return -1;
        }

        if ( seek64( m_fpFile, nPos, nWhence ) != 0 )
        {
            m_lCurrentPos = -1;
            return -1;
        }
            // (Seeking past the end does not make the file longer: only a
            // write there does, which Write() records.)
        m_lCurrentPos = tell64( m_fpFile );
        return m_lCurrentPos;
    }

    int64_t CFile::ReadBig( void *pBuffer, size_t nLength )
    {
        if ( ! m_fpFile || ! m_bInput )
            return -1;
        char *p = (char *)pBuffer;
        int64_t nDone = 0;
        while ( nLength > 0 )
        {
            int nChunk = ( nLength > (size_t)INT_MAX ) ? INT_MAX : (int)nLength;
            int nRead = Read( p, nChunk );
            if ( nRead <= 0 )
                break;
            nDone += nRead;
            p += nRead;
            nLength -= (size_t)nRead;
            if ( nRead < nChunk )
                break;
        }
        return nDone;
    }

    int64_t CFile::WriteBig( const void *pBuffer, size_t nLength )
    {
        if ( ! m_fpFile || ! ( m_bOutput || m_bAppend ) )
            return -1;
        const char *p = (const char *)pBuffer;
        int64_t nDone = 0;
        while ( nLength > 0 )
        {
            int nChunk = ( nLength > (size_t)INT_MAX ) ? INT_MAX : (int)nLength;
            int nWritten = Write( p, nChunk );
            if ( nWritten <= 0 )
                break;
            nDone += nWritten;
            p += nWritten;
            nLength -= (size_t)nWritten;
            if ( nWritten < nChunk )
                break;
        }
        return nDone;
    }

        // (It compared the long position and size, which are both -1 past
        // 2 GB, so a large file always looked finished.)
    bool CFile::IsEOF( void )
    {
        if ( m_fpFile == NULL )
            return true;
        int64_t nPos = GetPosBig();
        int64_t nSize = GetSizeBig();
        if ( nPos < 0 || nSize < 0 )
            return feof( m_fpFile ) != 0;
        return nPos >= nSize;
    }

        // (It used one static buffer for every file and thread, refused
        // binary files, and handled long lines by seeking back a byte, which
        // text mode does not allow.)
    CString CFile::GetLine( void )
    {
        CString strRetVal;
        if ( m_fpFile == NULL || ! m_bInput )
        {
            return strRetVal;
        }

        char achBuffer[ 1024 ];
        while ( fgets( achBuffer, (int)sizeof( achBuffer ), m_fpFile ) != NULL )
        {
            size_t nLength = strlen( achBuffer );
            strRetVal += achBuffer;
            if ( nLength > 0 && achBuffer[ nLength - 1 ] == '\n' )
                break;
        }
        m_lCurrentPos = tell64( m_fpFile );
        return strRetVal;
    }

    bool CFile::Delete( const char *strFilePath )
    {
#ifdef IASLIB_WIN32__
        int nReturn = _unlink( strFilePath );
#else
        int nReturn = unlink( strFilePath );
#endif

        if ( nReturn != 0 )
        {
            IASLIB_THROW_FILE_EXCEPTION(errno);
        }

        return true;
    }

    bool CFile::Rename( const char *strOldName, const char *strNewName )
    {
#ifdef IASLIB_WIN32__
            // (rename() will not replace a file on Windows.)
        if ( ! MoveFileExA( strOldName, strNewName, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED ) )
        {
            DWORD dwError = GetLastError();
            int nError = ( dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND ) ? ENOENT :
                         ( dwError == ERROR_ACCESS_DENIED || dwError == ERROR_SHARING_VIOLATION ) ? EACCES : EIO;
            IASLIB_THROW_FILE_EXCEPTION( nError );
        }
#else
        if ( rename( strOldName, strNewName ) != 0 )
        {
            IASLIB_THROW_FILE_EXCEPTION(errno);
        }
#endif
        return true;
    }

    bool CFile::Exists( const char *strFileName )
    {
        if ( strFileName == NULL || *strFileName == '\0' )
            return false;
#ifdef IASLIB_WIN32__
        return GetFileAttributesA( strFileName ) != INVALID_FILE_ATTRIBUTES;
#else
        struct stat stInfo;
        return stat( strFileName, &stInfo ) == 0;
#endif
    }

} // namespace IASLib
