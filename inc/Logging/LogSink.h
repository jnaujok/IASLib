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

/**
 * @file
 * @brief CLogSink, the base class of every log target, and the TRACE_LOG()
 * ... FATAL_LOG() macros that write to the current sink.
 */

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
        /// Where log lines go: the abstract base of every log sink.
        ///
        /// A sink formats each message as
        ///
        ///     &lt;ISO 8601 time&gt;: [LEVEL] - &lt;file&gt;:&lt;line&gt; - &lt;message&gt;
        ///
        /// and hands the line to writeLine(). The newest sink created (or the
        /// one given to setInstance()) is the one the *_LOG macros use.
        /// Messages below the sink's level are dropped before they are
        /// formatted. Writing is thread-safe in every sink here; destroy a
        /// sink only once no thread can still be logging to it. Sinks are
        /// not copyable.
        ///
        /// A new sink only has to implement writeLine():
        /// @code
        /// class CMemoryLog : public CLogSink
        /// {
        ///     public:
        ///         CMemoryLog( void ) : CLogSink( CLogSink::DEBUG ) {}
        ///         virtual bool writeLine( const char *message ) { m_lines.push_back( message ); return true; }
        ///         std::vector<std::string> m_lines;   // (lock it if threads log)
        /// };
        /// @endcode
    class CLogSink : public CObject
    {
        public:
            /// Message severities, lowest first.
            enum Level
            {
                TRACE,      ///< Detailed tracing.
                DEBUG,      ///< Debugging information.
                INFO,       ///< Normal operation.
                WARN,       ///< Something unexpected, but handled.
                ERROR,      ///< A failure; file sinks flush these at once.
                FATAL       ///< A failure the program cannot continue after.
            };

        protected:
            static std::atomic<CLogSink *> m_instance;  ///< The sink the macros use (NULL: none).
            std::atomic<int>    m_level;                ///< The lowest Level written, as an int.

        public:
                                /// Creates a sink.
                                /// @param level        The lowest level written.
                                /// @param bMakeDefault True: the macros log here from now on.
                                CLogSink( Level level, bool bMakeDefault = true );
                                /// Destroys the sink; if it was the default, there is then no default.
                                /// @note Destroying any sink used to clear the default, even another one.
            virtual            ~CLogSink( void );

                                DEFINE_OBJECT( CLogSink );

                /// The sink the macros use (NULL: none).
            static CLogSink    *getInstance( void );
                /// Makes pSink the one the macros use.
                /// @param pSink The new default sink (NULL: none). Not owned.
                /// @return The previous one.
            static CLogSink    *setInstance( CLogSink *pSink );

            /// Sets the lowest level written.
            void                setLevel( Level level ) { m_level = (int)level; }
            /// The lowest level written.
            Level               getLevel( void ) const { return (Level)m_level.load(); }
            /// True if a message at this level would be written.
            bool                isEnabled( Level level ) const { return (int)level >= m_level.load(); }

                /// Formats and writes a message (of any length).
                /// @param logLevel    The message's level.
                /// @param strFilename The source file (usually __FILE__); only its name is written.
                /// @param nLineNum    The source line (usually __LINE__).
                /// @param strFormat   A printf format, followed by its arguments.
                /// @return False if it was below the level or could not be written.
                /// @note Messages used to be cut at 4096 bytes.
            virtual bool        writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... );
                /// The same as writeLog(), with the arguments in a va_list.
                /// @param logLevel    The message's level.
                /// @param strFilename The source file.
                /// @param nLineNum    The source line.
                /// @param strFormat   A printf format.
                /// @param vaArgs      Its arguments.
                /// @return False if it was below the level or could not be written.
            virtual bool        writeLogV( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, va_list vaArgs );
                /// The same, with ${name} in the message replaced by the
                /// context's values (see CLogContext::expand()).
                /// The expansion is applied after the printf formatting.
                /// @param logLevel    The message's level.
                /// @param strFilename The source file.
                /// @param nLineNum    The source line.
                /// @param context     The values to put in the message.
                /// @param strFormat   A printf format, followed by its arguments.
                /// @return False if it was below the level or could not be written.
            virtual bool        writeContextLog( Level logLevel, const char *strFilename, int nLineNum, CLogContext &context, const char *strFormat, ... );

                /// Stores one finished line (without a newline). Every sink implements this.
                /// @param message The line.
                /// @return False if it could not be written.
            virtual bool        writeLine( const char *message ) = 0;

                /// The level's name ("TRACE" ... "FATAL"; "UNKNOWN" for any other value).
            static const char  *logLevelToString( Level level );
                /// The line writeLog() writes, for sinks that want to format
                /// differently.
                /// The time is now, in ISO 8601 with milliseconds; the file
                /// loses its directory (either '/' or '\\').
                /// @param logLevel    The message's level.
                /// @param strFilename The source file (NULL is treated as "").
                /// @param nLineNum    The source line.
                /// @param strMessage  The formatted message (NULL is treated as "").
                /// @note Only '/' used to be stripped, so Windows lines carried the full path.
            static CString      formatLine( Level logLevel, const char *strFilename, int nLineNum, const char *strMessage );
                /// printf into a CString, of any length.
                /// @param strFormat A printf format (NULL gives "").
                /// @param vaArgs    Its arguments.
                /// @return The text, or "" if the format was bad.
            static CString      formatV( const char *strFormat, va_list vaArgs );

        private:
                                /// Not copyable.
                                CLogSink( const CLogSink & );
                                /// Not assignable.
            CLogSink           &operator =( const CLogSink & );
    };

        /// Logs to the current sink when it takes the level.
        ///
        /// The sink is read once, and the arguments are not evaluated when
        /// nothing will be written. It is a single statement, safe in an
        /// unbraced if/else.
        /// @note The macros used to read the sink twice, so it could change
        /// in between, and were bare if statements that captured a
        /// following else.
    #define IASLIB_LOG_AT__( lvl, ... ) \
        do { \
            IASLib::CLogSink *pIASLibLogSink__ = IASLib::CLogSink::getInstance(); \
            if ( pIASLibLogSink__ && pIASLibLogSink__->isEnabled( lvl ) ) \
                pIASLibLogSink__->writeLog( lvl, __FILE__, __LINE__, __VA_ARGS__ ); \
        } while ( 0 )

    /// Logs a printf-style message at TRACE to the current sink (see IASLIB_LOG_AT__).
    #define TRACE_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::TRACE, __VA_ARGS__ )
    /// Logs a printf-style message at DEBUG to the current sink.
    #define DEBUG_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::DEBUG, __VA_ARGS__ )
    /// Logs a printf-style message at INFO to the current sink.
    #define  INFO_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::INFO,  __VA_ARGS__ )
    /// Logs a printf-style message at WARN to the current sink.
    #define  WARN_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::WARN,  __VA_ARGS__ )
    /// Logs a printf-style message at ERROR to the current sink.
    #define ERROR_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::ERROR, __VA_ARGS__ )
    /// Logs a printf-style message at FATAL to the current sink.
    #define FATAL_LOG( ... ) IASLIB_LOG_AT__( IASLib::CLogSink::FATAL, __VA_ARGS__ )

} // namespace IASLib
#endif // IASLIB_LOGSINK_H__
