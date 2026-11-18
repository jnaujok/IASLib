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
#ifdef IASLIB_WIN32__
#include <share.h>
#endif
#include <iostream>

#ifndef IASLIB_WIN32__
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

        CString strMode = "";

        if ( nMode & READ )
        {
            m_bInput = true;
            strMode += "r";
        }

        if ( nMode & WRITE )
        {
            m_bOutput = true;
            if ( strMode.GetLength() != 0 )
                strMode += "+";
            else
                strMode += "w";
        }

        if ( nMode & APPEND )
        {
            m_bAppend = true;
            strMode = "a";
            if ( m_bInput )
                strMode += "+";
        }

        if ( nMode & BINARY )
        {
            m_bBinary = true;
            strMode += "b";
        }

        if ( ( nMode & 0x07 ) == 0 )
        {
            m_bInput = true;
            strMode = "r";
        }

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

    bool CFile::Open( const char *strFileName, int nMode )
    {
        if ( m_fpFile )
        {
            Close();
            m_lFileSize = -1;
        }

        m_lCurrentPos = -1;
        m_strFileName = strFileName;

        CString strMode = "";

        if ( nMode & READ )
        {
            m_bInput = true;
            strMode += "r";
        }

        if ( nMode & WRITE )
        {
            m_bOutput = true;
            if ( strMode.GetLength() != 0 )
                strMode += "+";
            else
                strMode += "w";
        }

        if ( nMode & APPEND )
        {
            m_bAppend = true;
            strMode = "a";
            if ( m_bInput )
                strMode += "+";
        }

        if ( nMode & BINARY )
        {
            m_bBinary = true;
            strMode += "b";
        }

        if ( ( nMode & 0x07 ) == 0 )
        {
            m_bInput = true;
            strMode = "r";
        }

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
            fclose( m_fpFile );
            m_fpFile = NULL;
            m_lFileSize = -1;
            m_lCurrentPos = -1;
            return true;
        }
        return false;
    }

    bool CFile::Flush( void )
    {
        if ( ( m_fpFile ) && ( m_bOutput || m_bAppend ) )
        {
            fflush( m_fpFile );
            return true;
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

    bool CFile::IsEOF( void )
    {
        if ( GetPos() == GetSize() )
        {
            return true;
        }
        return false;
    }

    CString CFile::GetLine( void )
    {
        CString     strRetVal;
        static char Buffer[1024];
        bool        bDone = false;

        if ( ( ! m_fpFile ) || ( m_bBinary ) )
        {
            return strRetVal;
        }

        if ( fgets( Buffer, 1024, m_fpFile ) != NULL )
        {
            do
            {
                size_t nTemp = strRetVal.GetLength();
                if ( m_lCurrentPos != -1 )
                {
                    m_lCurrentPos += (int64_t)nTemp;
                    if ( ! m_bBinary )
                    {
                        m_lCurrentPos = tell64( m_fpFile );
                    }
                }
                strRetVal += Buffer;
                nTemp = strRetVal.GetLength() - nTemp;
                if ( nTemp == 1023 )
                {
                    Seek( -1, CURRENT );
                    Read( Buffer, 1 );
                    if ( Buffer[0] == '\n' )
                    {
                        bDone = true;
                    }
                    else
                    {
                        if ( fgets( Buffer, 1024, m_fpFile ) == NULL )
                            bDone = true;
                    }
                }
                else
                    bDone = true;
            } while ( ! bDone );
        }

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
        int nReturn = rename( strOldName, strNewName );
#else
        int nReturn = rename( strOldName, strNewName );
#endif

        if ( nReturn != 0 )
        {
            IASLIB_THROW_FILE_EXCEPTION(errno);
        }

        return true;
    }


} // namespace IASLib
