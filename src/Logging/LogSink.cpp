/***********************************************************************
**  Log Sink Class
**
**  Description:
**      This class defines a generic log sink (a target for logs) that
** can be used to derive log sinks -- for example a log file -- that
** consume log messages and store them in the sink. By building a generic
** log sink, we can abstract away the actual storage of the messages.
**
**  $AUTHOR$
**  $LOG$
**
** Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
** [A division of BlackStar Enterprises, LLC.]
***********************************************************************/

#include "LogSink.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace IASLib
{
    IMPLEMENT_OBJECT( CLogSink, CObject );

    std::atomic<CLogSink *> CLogSink::m_instance( NULL );

    CLogSink *CLogSink::getInstance( void )
    {
        return m_instance.load();
    }

    CLogSink *CLogSink::setInstance( CLogSink *pSink )
    {
        return m_instance.exchange( pSink );
    }

    CLogSink::CLogSink( Level level, bool bMakeDefault ) : m_level( (int)level )
    {
        if ( bMakeDefault )
            m_instance = this;
    }

        // (Destroying any sink cleared the default, even another one.)
    CLogSink::~CLogSink()
    {
        CLogSink *pThis = this;
        m_instance.compare_exchange_strong( pThis, NULL );
    }

    CString CLogSink::formatV( const char *strFormat, va_list vaArgs )
    {
        if ( strFormat == NULL )
            return CString( "" );

            // (Messages were cut at 4096 bytes.)
        char achBuffer[ 1024 ];
        va_list vaCopy;
        va_copy( vaCopy, vaArgs );
        int nLength = vsnprintf( achBuffer, sizeof( achBuffer ), strFormat, vaCopy );
        va_end( vaCopy );
        if ( nLength < 0 )
            return CString( "" );
        if ( (size_t)nLength < sizeof( achBuffer ) )
            return CString( achBuffer, (size_t)nLength );

        std::vector<char> buffer( (size_t)nLength + 1 );
        va_copy( vaCopy, vaArgs );
        vsnprintf( buffer.data(), buffer.size(), strFormat, vaCopy );
        va_end( vaCopy );
        return CString( buffer.data(), (size_t)nLength );
    }

    CString CLogSink::formatLine( Level logLevel, const char *strFilename, int nLineNum, const char *strMessage )
    {
            // The file's name without its directory, on either kind of path.
            // (Only '/' was stripped, so Windows lines carried the full path.)
        const char *strBase = ( strFilename ) ? strFilename : "";
        for ( const char *p = strBase; *p; p++ )
        {
            if ( *p == '/' || *p == '\\' )
                strBase = p + 1;
        }

        CDate now;
        CString strLine( (const char *)now.FormatDate( CDate::DF_ISO_8601_MS ) );
        strLine += ": [";
        strLine += logLevelToString( logLevel );
        strLine += "] - ";
        strLine += strBase;
        strLine += CString::FormatString( ":%d - ", nLineNum );
        strLine += ( strMessage ) ? strMessage : "";
        return strLine;
    }

    bool CLogSink::writeLogV( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, va_list vaArgs )
    {
        if ( ! isEnabled( logLevel ) )
            return false;
        CString strMessage = formatV( strFormat, vaArgs );
        return writeLine( formatLine( logLevel, strFilename, nLineNum, strMessage ) );
    }

    bool CLogSink::writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... )
    {
        if ( ! isEnabled( logLevel ) )
            return false;
        va_list vaArgs;
        va_start( vaArgs, strFormat );
        bool bRet = writeLogV( logLevel, strFilename, nLineNum, strFormat, vaArgs );
        va_end( vaArgs );
        return bRet;
    }

    bool CLogSink::writeContextLog( Level logLevel, const char *strFilename, int nLineNum, CLogContext &context, const char *strFormat, ... )
    {
        if ( ! isEnabled( logLevel ) )
            return false;
        va_list vaArgs;
        va_start( vaArgs, strFormat );
        CString strMessage = formatV( strFormat, vaArgs );
        va_end( vaArgs );
        return writeLine( formatLine( logLevel, strFilename, nLineNum, context.expand( strMessage ) ) );
    }

    const char *CLogSink::logLevelToString( Level level )
    {
        switch( level )
        {
            case TRACE: return "TRACE";
            case DEBUG: return "DEBUG";
            case INFO:  return "INFO";
            case WARN:  return "WARN";
            case ERROR: return "ERROR";
            case FATAL: return "FATAL";
            default:    return "UNKNOWN";
        }
    }
} // namespace IASLib
