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

#ifndef IASLIB_WIN32__
#include <stdarg.h>
#else
#include <strsafe.h>
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CLogSink, CObject );


    CLogSink *CLogSink::m_instance = NULL;

    CLogSink *CLogSink::getInstance( void )
    {
        return m_instance;
    }

    CLogSink::CLogSink( Level level )
    {
        m_instance = this;
        m_level = level;
    }

    CLogSink::~CLogSink()
    {
        m_instance = NULL;
    }

    bool CLogSink::writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... )
    {
        if ( logLevel >= m_level )
        {
            va_list       vaArgList;
            IASLibChar__   szBuffer[4096];

            /* format buf using fmt and arguments contained in ap */
            va_start( vaArgList, strFormat );
    #ifdef IASLIB_WIN32__
            StringCbVPrintf( szBuffer, 4096, strFormat, vaArgList );
    #else
            vsprintf( szBuffer, strFormat, vaArgList );
    #endif
            va_end( vaArgList );

            CString filename = strFilename;
            if ( filename.Contains( '/' ) )
                filename = filename.Substring( filename.LastIndexOf( '/' ) + 1 );
            CDate   now;
            CString workString;
            workString.Format( "%s: [%s] - %s:%d - %s", (const char *)now.FormatDate( CDate::DF_ISO_8601_MS ), logLevelToString( logLevel ), (const char *)filename, nLineNum, szBuffer );
            return writeLine( workString );
        }
        return false;
    }

    const char *CLogSink::logLevelToString( Level level )
    {
        switch( level )
        {
            case TRACE:
                return "TRACE";

            case DEBUG:
                return "DEBUG";
                
            case INFO:
                return "INFO";
                
            case WARN:
                return "WARN";
                
            case ERROR:
                return "ERROR";
                
            case FATAL:
                return "FATAL";
                
            default:
                return "UNKNOWN";
        }
    }

/*
    CString CLogSink::formatMessage( CLogContext &context, const char *strFormat, ... )
    {
        CString output;

        va_list       vaArgList;

        // format buf using fmt and arguments contained in ap 
        va_start( vaArgList, strFormat );

        output.Format( strFormat, vaArgList );

        va_end( vaArgList );

        // Replace any context variables
        if ( output.hasDelimitedTokens( "${", "}" ) )
        {
            context = context;
            // output.replaceDelimitedToken( "${", "}", )
        }
        return output;   
    }
*/

};

