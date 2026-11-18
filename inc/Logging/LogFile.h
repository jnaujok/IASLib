/**
 *  @file
 *  Log File Class
 *
 *  Description:
 *      This class defines a log file that can be written to for
 * tracking data. It can be accessed by multiple threads and can date
 * and time stamp each entry. It is also possible to set the default
 * format for the log entry.
 *
 *  Author: Jeffrey R. Naujok
 *  Created: June 1, 2019
 *
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */


#ifndef IASLIB_LOGFILE_H__
#define IASLIB_LOGFILE_H__

#include <mutex>
#include "LogSink.h"
#include "../Files/File.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
        /// A log sink that writes to one file.
        ///
        /// Appends lines to the file (or replaces it, with bDeletePrevious).
        /// With auto-flush, each line is flushed as it is written; lines at
        /// ERROR and above always are. Each line is written in one call
        /// under a lock, so lines from different threads never mix. A file
        /// that cannot be opened leaves the sink closed (IsOpen() is false
        /// and writes fail) instead of throwing.
        ///
        /// @code
        /// CLogFile log( CLogSink::DEBUG, "app.log", false, true );
        /// if ( ! log.IsOpen() ) { ... }
        /// DEBUG_LOG( "value=%d", n );
        /// @endcode
    class CLogFile : public CLogSink
    {
        protected:
            CFile              *m_fileOutputFile;   ///< The open file (owned; NULL if it could not be opened).
            std::mutex          m_mutexProtect;     ///< Guards the file and the flush flag.
            CString             m_strFileName;      ///< The file's name, as given.
            bool                m_bAutoFlush;       ///< True: flush after every line.

        public:
                                DEFINE_OBJECT( CLogFile );
                                /// Opens the log file.
                                /// @param level           The lowest level written.
                                /// @param strFileName     The file to write (NULL is treated as "").
                                /// @param bDeletePrevious True to replace the file; false to append to it.
                                /// @param bAutoFlush      True to flush after every line.
                                /// @param bMakeDefault    True to make this the sink the *_LOG macros use.
                                CLogFile( Level level, const char *strFileName, bool bDeletePrevious = false,
                                          bool bAutoFlush = false, bool bMakeDefault = true );
                                /// Closes and deletes the file object.
                                /// @note The file used to be closed but never deleted.
            virtual            ~CLogFile( void );

                /// Formats and writes a message, flushing it if the level is
                /// ERROR or above.
                /// @copydetails CLogSink::writeLog()
            virtual bool        writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... );
                /// Writes one line and a newline (flushed only with auto-flush).
                /// @param message The line, without its newline (NULL writes an empty line).
                /// @return False if the file is not open or the write was short.
                /// @note It used to report success whatever happened.
            virtual bool        writeLine( const char *message );

            /// Turns flushing after every line on or off.
            void                setAutoFlush( bool bAutoFlush ) { m_bAutoFlush = bAutoFlush; }
            /// Flushes the file (does nothing if it is not open).
            void                Flush( void );
            /// True if the file is open for writing.
            virtual bool        IsOpen( void );
            /// The file's name.
            const CString      &GetFileName( void ) const { return m_strFileName; }

        private:
            /// Writes a line under the lock, flushing it if bFlush or auto-flush is set.
            bool                writeLineFlushed( const char *message, bool bFlush );
    };
} // namespace IASLib
#endif // IASLIB_LOGFILE_H__
