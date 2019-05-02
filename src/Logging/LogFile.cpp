/**
 *  Log File Class
 *
 *  Description:
 *      This class defines a log file that can be written to for
 * tracking data. It can be accessed by multiple threads and can date
 * and time stamp each entry. It is also possible to set the default
 * format for the log entry.
 *
 *  $AUTHOR$
 *  $LOG$
 *
 */


#include "LogFile.h"
#ifndef IASLIB_WIN32__
#include <stdarg.h>
#else
#include <strsafe.h>
#endif

namespace IASLib
{
    DECLARE_OBJECT( CLogFile, CLogSink );

CLogFile::CLogFile( const char *strFileName, bool bDeletePrevious = false ) : m_mutexProtect()
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

bool CLogFile::Entry( ... )
{
    CString output;

    va_list       vaArgList;

    /* format buf using fmt and arguments contained in ap */
    va_start( vaArgList, m_strLogFormat );

    output.Format( m_strLogFormat, vaArgList );

    va_end( vaArgList );
}

bool CLogFile::Write( const char *strFormat, ... )
{
    CString output;

    va_list       vaArgList;

    /* format buf using fmt and arguments contained in ap */
    va_start( vaArgList, strFormat );

    output.Format( strFormat, vaArgList );

    va_end( vaArgList );
}

bool CLogFile::Open( void )
{

}

bool CLogFile::Close( void )
{

}

bool CLogFile::Clear( void )
{

}

} // namespace IASLib
