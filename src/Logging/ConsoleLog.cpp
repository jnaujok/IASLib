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


#include "ConsoleLog.h"

#include <stdio.h>
#ifndef IASLIB_WIN32__
#include <stdarg.h>
#else
#include <strsafe.h>
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CConsoleLog, CLogSink );

    CConsoleLog::CConsoleLog( Level level ) : CLogSink( level )
#ifdef IASLIB_MULTI_THREADED__
    , m_mutexProtect()
#endif        
    {
    }

    CConsoleLog::~CConsoleLog( void )
    {
    }

    bool CConsoleLog::writeLine( const char *message )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif
        printf( "%s\n", message );
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif
        return true;
    }
} // namespace IASLib
