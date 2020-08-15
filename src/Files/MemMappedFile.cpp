/*
 * Memory Mapped File Class
 *
 *      This class provides an object to wrap the various memory mapped
 * file functions. Memory mapped files provide a quick, random access
 * means of using files.
 *      Since memory mapped files use operating system calls that are 
 * normally used to page memory, the access is usually very fast and 
 * optimized.
 *      We also allow the choice of maintaining a separate read and
 * write pointer in the same file. 
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

namespace IASLib
{
    CMemMappedFile::CMemMappedFile()
    {
        m_bIsOpen = false;
        m_fpFile = nullptr;
        m_bInput = false;
        m_bOutput = false;
        m_bAppend = false;
        m_bBinary = false;
        m_lFileSize = 0;
        m_hFileHandle = 0;
        m_hMemoryMap = 0;
        m_nReadPointer = 0;
        m_nWritePointer = 0;
        m_bSyncPointers = true;
    }
 
#ifdef IASLIB_WIN32__
    CMemMappedFile::CMemMappedFile( HANDLE nFileHandle, int nMode )
    {
        if ( nFileHandle != NULL )
        {
            m_hFileHandle = nFileHandle;
            if ( nMode & CFile::WRITE )
            {
                CreateFileMapping( m_hFileHandle, NULL, PAGE_READWRITE, 65536, 32768, m_strFileName );
            }
            else
            {
                CreateFileMapping( m_hFileHandle, NULL, PAGE_READONLY, 65536, 32768, m_strFileName );
            }
        }
    }
#endif

    CMemMappedFile::CMemMappedFile( const char *strFileName, int nMode )
    {
    }

    CMemMappedFile::~CMemMappedFile()
    {
    }

    bool CMemMappedFile::Open( const char *strFileName, int nMode )
    {
        return false;
    }

    bool CMemMappedFile::Close( void )
    {
        return false;
    }

    bool CMemMappedFile::Flush( void )
    {
        return false;
    }

    int CMemMappedFile::Read( char *pchBuffer, int nBufferSize )
    {
        return 0;
    }

    int CMemMappedFile::Write( const char *pchBuffer, int nBufferSize )
    {
        return 0;
    }

    int CMemMappedFile::WriteString( const CString &oString )
    {
        return 0;
    }

    long CMemMappedFile::GetPos( void )
    {
        return 0L;
    }

    long CMemMappedFile::GetSize( void )
    {
        return 0L;
    }

    long CMemMappedFile::Seek( long nPos, int nStart )
    {
        return 0L;
    }

    long CMemMappedFile::SeekRead( long nPos, int nStart )
    {
        return 0L;
    }

    long CMemMappedFile::SeekWrite( long nPos, int nStart )
    {
        return 0L;
    }

    bool CMemMappedFile::IsEOF( void )
    {
        return true;
    }

    CString CMemMappedFile::GetLine( void )
    {
        return CString( "" );
    }

} // namespace IASLib

