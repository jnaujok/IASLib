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

namespace IASLib
{
    IMPLEMENT_OBJECT( CConsoleLog, CLogSink );

    CConsoleLog::CConsoleLog( Level level, bool bStandardError, bool bMakeDefault )
        : CLogSink( level, bMakeDefault ), m_bStandardError( bStandardError )
    {
    }

    CConsoleLog::~CConsoleLog( void )
    {
    }

    bool CConsoleLog::writeLine( const char *message )
    {
        FILE *pOut = ( m_bStandardError ) ? stderr : stdout;
        std::lock_guard<std::mutex> lock( m_mutexProtect );
            // (printf( "%s\n" ) was not flushed, so lines sat in the buffer
            // when output went to a pipe or file.)
        bool bOk = fputs( ( message ) ? message : "", pOut ) >= 0 && fputc( '\n', pOut ) != EOF;
        fflush( pOut );
        return bOk;
    }
} // namespace IASLib
