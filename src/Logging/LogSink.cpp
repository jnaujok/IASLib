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

#include "Logging/LogSink.h"

#ifndef IASLIB_WIN32__
#include <stdarg.h>
#else
#include <strsafe.h>
#endif

namespace IASLib
{
    CLogSink::CLogSink()
    {
        m_strLogFormat  = DEFAULT_LOG_FORMAT;
        m_nDateFormat = CDate::DF_ISO_9601_MS;
        m_bTimeStamp = true;
    }

    CString CLogSink::formatMessage( CLogContext &context, const char *strFormat, ... )
    {
        CString output;

        va_list       vaArgList;

        /* format buf using fmt and arguments contained in ap */
        va_start( vaArgList, strFormat );

        output.Format( strFormat, vaArgList );

        va_end( vaArgList );

        // Replace any context variables
        
    }
};

