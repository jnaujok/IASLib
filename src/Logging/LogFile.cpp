/**
 *  Log File Class
 *
 *  Description:
 *      This class defines a log file that can be written to for
 * tracking data. It can be accessed by multiple threads and can date
 * and time stamp each entry. It is also possible to set the default
 * format for the log entry.
 *
 *  Author: Jeffrey R. Naujok
 *  Created: June 1, 2019
 *
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */


#include "LogFile.h"
#ifndef IASLIB_WIN32__
#include <stdarg.h>
#else
#include <strsafe.h>
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CLogFile, CLogSink );

    CLogFile::CLogFile( Level level, const char *strFileName, bool bDeletePrevious ) : CLogSink( level )
#ifdef IASLIB_MULTI_THREADED__            
        , m_mutexProtect()
#endif
    {
        m_strFileName = strFileName;
        if ( ! bDeletePrevious )
        {
            m_fileOutputFile = new CFile( m_strFileName, CFile::APPEND );
        }
        else
        {
            m_fileOutputFile = new CFile( m_strFileName, CFile::WRITE );
        }
    }

    CLogFile::~CLogFile( void )
    {
        if (m_fileOutputFile)
        {
            m_fileOutputFile->Close();
        }
        m_fileOutputFile = NULL;
    }

    bool CLogFile::writeLine( const char *message )
    {
        if ( m_fileOutputFile )
        {
#ifdef IASLIB_MULTI_THREADED__            
            m_mutexProtect.Lock();
#endif
            m_fileOutputFile->WriteString( message );
            m_fileOutputFile->WriteString( "\n" );
#ifdef IASLIB_MULTI_THREADED__            
            m_mutexProtect.Unlock();
#endif
        }

        return true;
    }
   
} // namespace IASLib
