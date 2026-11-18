/*
 * File Status Class
 *
 *      This class wraps the status function found in Unix. This 
 * function can be used to detemine information about a file 
 * without actually having to open that file.
 *      In Windows, or other operating systems, this command 
 * recreates the functionality of the Unix file-stat.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/15/2002
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "FileStat.h"
#include "FileException.h"

#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CFileStat, CObject );

        // (It took a global lock that stat() does not need, trimmed spaces
        // a name may really have, tested the type with "mode & S_IFDIR",
        // which is also true of sockets and block devices, and kept the size
        // in a long: wrong past 2 GB on Windows.)
    CFileStat::CFileStat( const char *strFileName )
    {
        m_strFileName = ( strFileName ) ? strFileName : "";
        m_bIsDir = false;
        m_bIsFile = false;
        m_lFileSize = 0;

#ifdef IASLIB_WIN32__
        struct _stat64 stStatus;
        if ( _stat64( m_strFileName, &stStatus ) != 0 )
            throw CFileException( errno );
        int nType = stStatus.st_mode & _S_IFMT;
        m_bIsDir = ( nType == _S_IFDIR );
        m_bIsFile = ( nType == _S_IFREG );
#else
        struct stat stStatus;
        if ( stat( m_strFileName, &stStatus ) != 0 )
            throw CFileException( errno );
        m_bIsDir = S_ISDIR( stStatus.st_mode );
        m_bIsFile = S_ISREG( stStatus.st_mode );
#endif
        if ( m_bIsFile )
            m_lFileSize = (int64_t)stStatus.st_size;

        m_dttLastAccess = CDate::ParseUnix( (time_t)stStatus.st_atime );
        m_dttLastModified = CDate::ParseUnix( (time_t)stStatus.st_mtime );
        m_dttLastChanged = CDate::ParseUnix( (time_t)stStatus.st_ctime );
    }

    CFileStat::~CFileStat( void )
    {
    }

    long CFileStat::GetSize( void ) const
    {
        return ( m_lFileSize > (int64_t)LONG_MAX ) ? -1L : (long)m_lFileSize;
    }
} // namespace IASLib
