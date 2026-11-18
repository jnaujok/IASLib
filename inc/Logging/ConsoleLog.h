/***********************************************************************
**  Console Log Class
**
**  Description:
**      This class defines a log file that writes to the console (screen)
** rather than a permanent store.
**
**  Author: Jeffrey R. Naujok
**  Created: September 12, 2019
**
** Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
** [A division of BlackStar Enterprises, LLC.]
***********************************************************************/

/**
 * @file
 * @brief CConsoleLog, a log sink that writes to standard output or
 * standard error.
 */

#ifndef IASLIB_CONSOLELOG_H__
#define IASLIB_CONSOLELOG_H__

#include <mutex>
#include "LogSink.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
        /// A log sink that writes to the console.
        ///
        /// Writes each line to standard output (or standard error), whole,
        /// and flushes it, so lines appear at once even when the output is
        /// a pipe or a file. Lines from different threads never mix.
        ///
        /// @code
        /// CConsoleLog log( CLogSink::INFO );     // becomes the default sink
        /// INFO_LOG( "started on port %d", nPort );
        /// @endcode
    class CConsoleLog : public CLogSink
    {
        protected:
            std::mutex  m_mutexProtect;     ///< Serializes writes to the stream.
            bool        m_bStandardError;   ///< True: write to stderr; false: stdout.

        public:
                                DEFINE_OBJECT( CConsoleLog );
                                /// Creates a console sink.
                                /// @param level          The lowest level written.
                                /// @param bStandardError True to write to standard error instead of standard output.
                                /// @param bMakeDefault   True to make this the sink the *_LOG macros use.
                                CConsoleLog( Level level, bool bStandardError = false, bool bMakeDefault = true );
                                /// Destroys the sink (and stops it being the default, if it was).
            virtual            ~CConsoleLog( void );

                /// Writes one line and a newline, then flushes the stream.
                /// @param message The line, without its newline (NULL writes an empty line).
                /// @return False if the stream reported an error.
                /// @note printf( "%s\n" ) used to be used without a flush, so
                /// lines sat in the buffer when output went to a pipe or file.
            virtual bool        writeLine( const char *message );
    };
} // namespace IASLib
#endif // IASLIB_CONSOLELOG_H__
