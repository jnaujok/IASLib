/*
 * Memory Mapped File Class
 *
 *      See MemMappedFile.h. (Every method here was an empty stub
 * returning 0.)
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 05/29/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "MemMappedFile.h"
#include "FileException.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#ifndef IASLIB_WIN32__
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

namespace IASLib
{

    namespace
    {
        const int64_t GROW_MAX_STEP = 64 * 1024 * 1024;
        const int64_t GROW_MIN = 4096;

        long toLong( int64_t n )
        {
            return ( n < 0 || n > (int64_t)LONG_MAX ) ? -1L : (long)n;
        }

#ifdef IASLIB_WIN32__
        int errnoFromWindows( DWORD dwError )
        {
            switch ( dwError )
            {
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:      return ENOENT;
                case ERROR_ACCESS_DENIED:
                case ERROR_SHARING_VIOLATION:   return EACCES;
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_OUTOFMEMORY:         return ENOMEM;
                case ERROR_DISK_FULL:           return ENOSPC;
                default:                        return EIO;
            }
        }
#endif
    }

    void CMemMappedFile::Init( void )
    {
        m_fpFile = NULL;
        m_bInput = false;
        m_bOutput = false;
        m_bAppend = false;
        m_bBinary = true;
#ifdef IASLIB_WIN32__
        m_hFileHandle = INVALID_HANDLE_VALUE;
        m_hMemoryMap = NULL;
#else
        m_hFileHandle = -1;
#endif
        m_pView = NULL;
        m_nMapped = 0;
        m_nSize = 0;
        m_nReadPointer = 0;
        m_nWritePointer = 0;
        m_bSyncPointers = true;
        m_bIsOpen = false;
        m_bWritable = false;
    }

    CMemMappedFile::CMemMappedFile()
    {
        Init();
    }

    CMemMappedFile::CMemMappedFile( const char *strFileName, int nMode )
    {
        Init();
        Open( strFileName, nMode );
    }

#ifdef IASLIB_WIN32__
    CMemMappedFile::CMemMappedFile( HANDLE nFileHandle, int nMode )
    {
        Init();
        m_strFileName = "Unknown";
        m_hFileHandle = nFileHandle;
        m_bWritable = ( nMode & ( WRITE | APPEND | CREATE ) ) != 0;
        m_bInput = true;
        m_bOutput = m_bWritable;
        m_bAppend = ( nMode & APPEND ) != 0;
        LARGE_INTEGER liSize;
        if ( nFileHandle == INVALID_HANDLE_VALUE || ! GetFileSizeEx( nFileHandle, &liSize ) )
        {
            m_hFileHandle = INVALID_HANDLE_VALUE;
            throw CFileException( EBADF );
        }
        m_nSize = (int64_t)liSize.QuadPart;
        m_bIsOpen = true;
        if ( m_nSize > 0 && ! Map( m_nSize ) )
        {
            Close();
            throw CFileException( ENOMEM );
        }
        if ( m_bAppend )
            m_nWritePointer = m_nSize;
    }
#endif

    CMemMappedFile::~CMemMappedFile()
    {
        Close();
    }

        // Maps the first nBytes of the file (growing the file to that on
        // disk if need be). Leaves nothing mapped for 0 bytes.
    bool CMemMappedFile::Map( int64_t nBytes )
    {
        Unmap();
        if ( nBytes <= 0 )
            return true;
        if ( (uint64_t)nBytes > (uint64_t)(size_t)-1 )
            return false;                   // more than this address space holds

#ifdef IASLIB_WIN32__
        m_hMemoryMap = CreateFileMappingA( m_hFileHandle, NULL, m_bWritable ? PAGE_READWRITE : PAGE_READONLY,
                                           (DWORD)( (uint64_t)nBytes >> 32 ), (DWORD)( (uint64_t)nBytes & 0xFFFFFFFF ), NULL );
        if ( m_hMemoryMap == NULL )
            return false;
        m_pView = (char *)MapViewOfFile( m_hMemoryMap, m_bWritable ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, (SIZE_T)nBytes );
        if ( m_pView == NULL )
        {
            CloseHandle( m_hMemoryMap );
            m_hMemoryMap = NULL;
            return false;
        }
#else
        if ( m_bWritable )
        {
            struct stat stInfo;
            if ( fstat( m_hFileHandle, &stInfo ) != 0 )
                return false;
            if ( (int64_t)stInfo.st_size < nBytes && ftruncate( m_hFileHandle, (off_t)nBytes ) != 0 )
                return false;
        }
        void *pView = mmap( NULL, (size_t)nBytes, m_bWritable ? ( PROT_READ | PROT_WRITE ) : PROT_READ, MAP_SHARED, m_hFileHandle, 0 );
        if ( pView == MAP_FAILED )
            return false;
        m_pView = (char *)pView;
#endif
        m_nMapped = nBytes;
        return true;
    }

    void CMemMappedFile::Unmap( void )
    {
#ifdef IASLIB_WIN32__
        if ( m_pView )
            UnmapViewOfFile( m_pView );
        if ( m_hMemoryMap )
            CloseHandle( m_hMemoryMap );
        m_hMemoryMap = NULL;
#else
        if ( m_pView )
            munmap( m_pView, (size_t)m_nMapped );
#endif
        m_pView = NULL;
        m_nMapped = 0;
    }

        // Room for nNeeded bytes: half again (up to 64 MB more), or what is
        // needed if that is more.
    bool CMemMappedFile::Grow( int64_t nNeeded )
    {
        if ( nNeeded <= m_nMapped )
            return true;
        int64_t nStep = m_nMapped / 2;
        if ( nStep > GROW_MAX_STEP )
            nStep = GROW_MAX_STEP;
        int64_t nNew = m_nMapped + nStep;
        if ( nNew < nNeeded )
            nNew = nNeeded;
        if ( nNew < GROW_MIN )
            nNew = GROW_MIN;
        return Map( nNew ) || Map( nNeeded );
    }

    bool CMemMappedFile::Open( const char *strFileName, int nMode )
    {
        if ( m_bIsOpen )
            Close();
        Init();
        m_strFileName = ( strFileName ) ? strFileName : "";

        if ( ( nMode & ( READ | WRITE | APPEND | CREATE ) ) == 0 )
            nMode |= READ;
        bool bCreate = ( nMode & CREATE ) || ( ( nMode & WRITE ) && ! ( nMode & READ ) );
        bool bAppend = ( nMode & APPEND ) != 0;
        m_bWritable = bCreate || bAppend || ( nMode & WRITE );
        m_bInput = true;
        m_bOutput = m_bWritable;
        m_bAppend = bAppend;

#ifdef IASLIB_WIN32__
        DWORD dwAccess = GENERIC_READ | ( m_bWritable ? GENERIC_WRITE : 0 );
        DWORD dwDisposition = bCreate ? CREATE_ALWAYS : ( bAppend ? OPEN_ALWAYS : OPEN_EXISTING );
        m_hFileHandle = CreateFileA( m_strFileName, dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                     NULL, dwDisposition, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( m_hFileHandle == INVALID_HANDLE_VALUE )
        {
            int nError = errnoFromWindows( GetLastError() );
            Init();
            throw CFileException( nError );
        }
        LARGE_INTEGER liSize;
        if ( ! GetFileSizeEx( m_hFileHandle, &liSize ) )
        {
            CloseHandle( m_hFileHandle );
            Init();
            throw CFileException( EIO );
        }
        m_nSize = (int64_t)liSize.QuadPart;
#else
        int nFlags = m_bWritable ? O_RDWR : O_RDONLY;       // mapping to write needs read access too
        if ( bCreate )
            nFlags |= O_CREAT | O_TRUNC;
        else if ( bAppend )
            nFlags |= O_CREAT;
        m_hFileHandle = open( m_strFileName, nFlags, 0666 );
        if ( m_hFileHandle < 0 )
        {
            int nError = errno;
            Init();
            throw CFileException( nError );
        }
        fcntl( m_hFileHandle, F_SETFD, FD_CLOEXEC );
        struct stat stInfo;
        if ( fstat( m_hFileHandle, &stInfo ) != 0 )
        {
            int nError = errno;
            close( m_hFileHandle );
            Init();
            throw CFileException( nError );
        }
        m_nSize = (int64_t)stInfo.st_size;
#endif
        m_bIsOpen = true;

        if ( m_nSize > 0 && ! Map( m_nSize ) )
        {
            Close();
            throw CFileException( ENOMEM );
        }
        m_nReadPointer = 0;
        m_nWritePointer = ( bAppend ) ? m_nSize : 0;
        return true;
    }

    bool CMemMappedFile::Close( void )
    {
        if ( ! m_bIsOpen )
            return false;

        bool bOk = true;
        if ( m_bWritable && m_pView )
        {
#ifdef IASLIB_WIN32__
            bOk = FlushViewOfFile( m_pView, 0 ) != 0;
#else
            bOk = msync( m_pView, (size_t)m_nMapped, MS_SYNC ) == 0;
#endif
        }
        Unmap();

#ifdef IASLIB_WIN32__
        if ( m_bWritable )
        {
                // Trim off the room grown for writes.
            LARGE_INTEGER liSize;
            liSize.QuadPart = (LONGLONG)m_nSize;
            bOk = SetFilePointerEx( m_hFileHandle, liSize, NULL, FILE_BEGIN ) && SetEndOfFile( m_hFileHandle ) && bOk;
        }
        if ( m_hFileHandle != INVALID_HANDLE_VALUE )
            bOk = CloseHandle( m_hFileHandle ) && bOk;
#else
        if ( m_bWritable )
            bOk = ( ftruncate( m_hFileHandle, (off_t)m_nSize ) == 0 ) && bOk;
        if ( m_hFileHandle >= 0 )
            bOk = ( close( m_hFileHandle ) == 0 ) && bOk;
#endif
        CString strName = m_strFileName;
        Init();
        m_strFileName = strName;
        return bOk;
    }

    bool CMemMappedFile::Flush( void )
    {
        if ( ! m_bIsOpen || ! m_bWritable )
            return false;
        if ( m_pView == NULL )
            return true;
#ifdef IASLIB_WIN32__
        return FlushViewOfFile( m_pView, 0 ) && FlushFileBuffers( m_hFileHandle );
#else
        return msync( m_pView, (size_t)m_nMapped, MS_SYNC ) == 0;
#endif
    }

    int64_t CMemMappedFile::ReadBig( void *pBuffer, size_t nLength )
    {
        if ( ! m_bIsOpen )
            return -1;
        if ( m_nReadPointer >= m_nSize || nLength == 0 )
            return 0;
        uint64_t nAvailable = (uint64_t)( m_nSize - m_nReadPointer );
        size_t nCopy = ( (uint64_t)nLength < nAvailable ) ? nLength : (size_t)nAvailable;
        memcpy( pBuffer, m_pView + m_nReadPointer, nCopy );
        m_nReadPointer += (int64_t)nCopy;
        return (int64_t)nCopy;
    }

    int64_t CMemMappedFile::WriteBig( const void *pBuffer, size_t nLength )
    {
        if ( ! m_bIsOpen || ! m_bWritable )
            return -1;
        if ( nLength == 0 )
            return 0;

        int64_t &nPointer = ( m_bSyncPointers ) ? m_nReadPointer : m_nWritePointer;
        if ( m_bAppend )
            nPointer = m_nSize;
        int64_t nEnd = nPointer + (int64_t)nLength;
        if ( nEnd < nPointer || ! Grow( nEnd ) )
            return -1;
        memcpy( m_pView + nPointer, pBuffer, nLength );
        nPointer = nEnd;
        if ( nEnd > m_nSize )
            m_nSize = nEnd;
        return (int64_t)nLength;
    }

    int CMemMappedFile::Read( char *pchBuffer, int nBufferSize )
    {
        if ( nBufferSize < 0 )
            return -1;
        return (int)ReadBig( pchBuffer, (size_t)nBufferSize );
    }

    int CMemMappedFile::Write( const char *pchBuffer, int nBufferSize )
    {
        if ( nBufferSize < 0 )
            return -1;
        return (int)WriteBig( pchBuffer, (size_t)nBufferSize );
    }

    int CMemMappedFile::WriteString( const CString &oString )
    {
        return Write( (const char *)oString, (int)oString.GetLength() );
    }

    int64_t CMemMappedFile::Seek( int64_t &nPointer, int64_t nPos, int nStart )
    {
        if ( ! m_bIsOpen )
            return -1;
        int64_t nBase;
        switch ( nStart )
        {
            case BEGIN:     nBase = 0; break;
            case CURRENT:   nBase = nPointer; break;
            case END:       nBase = m_nSize; break;
            default:        return -1;
        }
        int64_t nNew = nBase + nPos;
        if ( nNew < 0 )
            return -1;
        nPointer = nNew;
        return nNew;
    }

    int64_t CMemMappedFile::SeekBig( int64_t nPos, int nStart )
    {
        int64_t nNew = Seek( m_nReadPointer, nPos, nStart );
        if ( nNew >= 0 )
            m_nWritePointer = nNew;
        return nNew;
    }

    int64_t CMemMappedFile::SeekReadBig( int64_t nPos, int nStart )
    {
        return Seek( m_nReadPointer, nPos, nStart );
    }

    int64_t CMemMappedFile::SeekWriteBig( int64_t nPos, int nStart )
    {
        return Seek( m_bSyncPointers ? m_nReadPointer : m_nWritePointer, nPos, nStart );
    }

    int64_t CMemMappedFile::GetPosBig( void )
    {
        return ( m_bIsOpen ) ? m_nReadPointer : -1;
    }

    int64_t CMemMappedFile::GetSizeBig( void )
    {
        return ( m_bIsOpen ) ? m_nSize : -1;
    }

    long CMemMappedFile::GetPos( void )             { return toLong( GetPosBig() ); }
    long CMemMappedFile::GetSize( void )            { return toLong( GetSizeBig() ); }
    long CMemMappedFile::Seek( long nPos, int nStart )      { return toLong( SeekBig( nPos, nStart ) ); }
    long CMemMappedFile::SeekRead( long nPos, int nStart )  { return toLong( SeekReadBig( nPos, nStart ) ); }
    long CMemMappedFile::SeekWrite( long nPos, int nStart ) { return toLong( SeekWriteBig( nPos, nStart ) ); }

    void CMemMappedFile::SetSyncPointers( bool bSync )
    {
            // Syncing puts the write pointer where the read pointer is.
        if ( bSync && ! m_bSyncPointers )
            m_nWritePointer = m_nReadPointer;
        else if ( ! bSync && m_bSyncPointers )
            m_nWritePointer = m_nReadPointer;
        m_bSyncPointers = bSync;
    }

    bool CMemMappedFile::Resize( int64_t nBytes )
    {
        if ( ! m_bIsOpen || ! m_bWritable || nBytes < 0 )
            return false;
        if ( nBytes > m_nMapped && ! Grow( nBytes ) )
            return false;
        if ( nBytes > m_nSize )
            memset( m_pView + m_nSize, 0, (size_t)( nBytes - m_nSize ) );
        m_nSize = nBytes;
        return true;
    }

    bool CMemMappedFile::IsEOF( void )
    {
        return ! m_bIsOpen || m_nReadPointer >= m_nSize;
    }

    CString CMemMappedFile::GetLine( void )
    {
        if ( ! m_bIsOpen || m_nReadPointer >= m_nSize )
            return CString( "" );
        const char *pStart = m_pView + m_nReadPointer;
        size_t nAvailable = (size_t)( m_nSize - m_nReadPointer );
        const char *pNewline = (const char *)memchr( pStart, '\n', nAvailable );
        size_t nLength = ( pNewline ) ? (size_t)( pNewline - pStart ) + 1 : nAvailable;
        m_nReadPointer += (int64_t)nLength;
        return CString( pStart, nLength );
    }
} // namespace IASLib
