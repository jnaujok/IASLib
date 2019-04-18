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

#ifdef IASLIB_MULTI_THREADED__
#include "Mutex.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

namespace IASLib
{
#ifdef IASLIB_MULTI_THREADED__
    CMutex oStatLock;
#endif

    IMPLEMENT_OBJECT( CFileStat, CObject );

    CFileStat::CFileStat( const char *strFileName )
    {
#ifdef IASLIB_MULTI_THREADED__
        oStatLock.Lock();
#endif
        m_strFileName = strFileName;
        m_strFileName.Trim();
        if ( stat( m_strFileName, &m_stFileStatus ) != 0 )
        {
#ifdef IASLIB_MULTI_THREADED__
            oStatLock.Unlock();
#endif
            throw CFileException( errno );
        }
#ifdef IASLIB_MULTI_THREADED__
        oStatLock.Unlock();
#endif
        m_bIsDir = false;
        m_bIsFile = false;
        m_lFileSize = 0;

        if ( m_stFileStatus.st_mode & S_IFDIR )
        {
            m_bIsDir = true;
        }
        else
        {
            if ( m_stFileStatus.st_mode & S_IFREG )
            {
                m_bIsFile = true;
                m_lFileSize = (long)m_stFileStatus.st_size;
            }
        }

        m_dttLastAccess = CDate::ParseUnix( m_stFileStatus.st_atime );
        m_dttLastModified = CDate::ParseUnix( m_stFileStatus.st_mtime );
        m_dttLastChanged = CDate::ParseUnix( m_stFileStatus.st_ctime );
    }

    CFileStat::~CFileStat( void )
    {
        // Nothing to do for now.
    }
} // namespace IASLib

