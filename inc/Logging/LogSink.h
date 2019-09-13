/***********************************************************************
**  Log Sink Class
**
**  Description:
**      This class defines a generic log sink (a target for logs) that
** can be used to derive log sinks -- for example a log file -- that
** consume log messages and store them in the sink. By building a generic
** log sink, we can abstract away the actual storage of the messages.
**
**  Author: Jeffrey R. Naujok
**  Created: June 1, 2019
**
** Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
** [A division of BlackStar Enterprises, LLC.]
***********************************************************************/

#ifndef IASLIB_LOGSINK_H__
#define IASLIB_LOGSINK_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"
#include "LogContext.h"

namespace IASLib
{
    class CLogSink : public CObject
    {
        public:
            enum Level
            {
                TRACE,
                DEBUG,
                INFO,
                WARN,
                ERROR,
                FATAL
            };

        protected:
            static CLogSink *m_instance;
            Level   m_level;

        public:
                                CLogSink( Level level );
            virtual            ~CLogSink( void );

                                DEFINE_OBJECT( CLogSink );

            static CLogSink    *getInstance( void );

            virtual bool        writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... );

            virtual bool        writeLine( const char *message ) = 0;
        protected:
            //virtual CString     formatMessage( CLogContext &context, const char *strFormat, ... );
            const char         *logLevelToString( Level level );
    };

    #define TRACE_LOG( ... ) if ( CLogSink::getInstance() )  { CLogSink::getInstance()->writeLog( CLogSink::Level::TRACE, __FILE__, __LINE__, __VA_ARGS__ ); }
    #define DEBUG_LOG( ... ) if ( CLogSink::getInstance() )  { CLogSink::getInstance()->writeLog( CLogSink::Level::DEBUG, __FILE__, __LINE__, __VA_ARGS__ ); }
    #define  INFO_LOG( ... ) if ( CLogSink::getInstance() )  { CLogSink::getInstance()->writeLog( CLogSink::Level::INFO,  __FILE__, __LINE__, __VA_ARGS__ ); }
    #define  WARN_LOG( ... ) if ( CLogSink::getInstance() )  { CLogSink::getInstance()->writeLog( CLogSink::Level::WARN,  __FILE__, __LINE__, __VA_ARGS__ ); }
    #define ERROR_LOG( ... ) if ( CLogSink::getInstance() )  { CLogSink::getInstance()->writeLog( CLogSink::Level::ERROR, __FILE__, __LINE__, __VA_ARGS__ ); }
    #define FATAL_LOG( ... ) if ( CLogSink::getInstance() )  { CLogSink::getInstance()->writeLog( CLogSink::Level::FATAL, __FILE__, __LINE__, __VA_ARGS__ ); }

} // namespace IASLib
#endif // IASLIB_LOGSINK_H__
