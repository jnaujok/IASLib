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

#include <stdarg.h>
#include <atomic>
#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"
#include "LogContext.h"

    // wingdi.h defines ERROR as a macro; Platform.h keeps it out, but a
    // program may include <windows.h> itself first.
#ifdef ERROR
    #undef ERROR
#endif

namespace IASLib
{
        // Where log lines go. A sink formats each message as
        //     <ISO 8601 time>: [LEVEL] - <file>:<line> - <message>
        // and hands the line to writeLine(). The newest sink created (or the
        // one given to setInstance()) is the one the *_LOG macros use.
        // Messages below the sink's level are dropped before they are
        // formatted. Writing is thread-safe in every sink here; destroy a
        // sink only once no thread can still be logging to it.
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
            static std::atomic<CLogSink *> m_instance;
            std::atomic<int>    m_level;

        public:
                                // bMakeDefault: the macros log here from now on.
                                CLogSink( Level level, bool bMakeDefault = true );
            virtual            ~CLogSink( void );

                                DEFINE_OBJECT( CLogSink );

                // The sink the macros use (NULL: none).
            static CLogSink    *getInstance( void );
                // Makes pSink the one the macros use; returns the previous one.
            static CLogSink    *setInstance( CLogSink *pSink );

            void                setLevel( Level level ) { m_level = (int)level; }
            Level               getLevel( void ) const { return (Level)m_level.load(); }
            bool                isEnabled( Level level ) const { return (int)level >= m_level.load(); }

                // Formats and writes a message (of any length). False if it
                // was below the level or could not be written.
            virtual bool        writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... );
            virtual bool        writeLogV( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, va_list vaArgs );
                // The same, with ${name} in the message replaced by the
                // context's values (see CLogContext::expand()).
            virtual bool        writeContextLog( Level logLevel, const char *strFilename, int nLineNum, CLogContext &context, const char *strFormat, ... );

            virtual bool        writeLine( const char *message ) = 0;

            static const char  *logLevelToString( Level level );
                // The line writeLog() writes, for sinks that want to format
                // differently.
            static CString      formatLine( Level logLevel, const char *strFilename, int nLineNum, const char *strMessage );
                // printf into a CString, of any length.
            static CString      formatV( const char *strFormat, va_list vaArgs );

        private:
                                CLogSink( const CLogSink & );
            CLogSink           &operator =( const CLogSink & );
    };

        // Logs to the current sink when it takes the level. The sink is read
        // once, and the arguments are not evaluated when nothing will be
        // written. (The macros read the sink twice, so it could change in
        // between, and were bare if statements that captured a following
        // else.)
    #define IASLIB_LOG_AT__( lvl, ... ) \
        do { \
            IASLib::CLogSink *pIASLibLogSink__ = IASLib::CLogSink::getInstance(); \
            if ( pIASLibLogSink__ && pIASLibLogSink__->isEnabled( lvl ) ) \
                pIASLibLogSink__->writeLog( lvl, __FILE__, __LINE__, __VA_ARGS__ ); \
        } while ( 0 )

    #define TRACE_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::TRACE, __VA_ARGS__ )
    #define DEBUG_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::DEBUG, __VA_ARGS__ )
    #define  INFO_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::INFO,  __VA_ARGS__ )
    #define  WARN_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::WARN,  __VA_ARGS__ )
    #define ERROR_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::ERROR, __VA_ARGS__ )
    #define FATAL_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::FATAL, __VA_ARGS__ )

} // namespace IASLib
#endif // IASLIB_LOGSINK_H__
