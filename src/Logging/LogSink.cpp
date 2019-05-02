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

namespace IASLib
{
    CLogSink::CLogSink()
    {
        m_strLogFormat  = DEFAULT_LOG_FORMAT;
        m_nDateFormat = CDate::DF_ISO_9601_MS;
        m_bTimeStamp = true;
    }

};

