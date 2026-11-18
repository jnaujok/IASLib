/***********************************************************************
**  Rotating Log File Class
**
**  Description:
**      This class defines a log file that can be written to for
** tracking data. It can be accessed by multiple threads and can date
** and time stamp each entry. It is also possible to set the default
** format for the log entry.
**
**  Author: Jeffrey R. Naujok
**  Created: August 1, 2019
**
** Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
** [A division of BlackStar Enterprises, LLC.]
***********************************************************************/

/**
 * @file
 * @brief CRotatingLogFile, a log sink that starts a new file every hour,
 * day, week or month.
 */

#ifndef IASLIB_ROTATINGLOGFILE_H__
#define IASLIB_ROTATINGLOGFILE_H__

#include <mutex>
#include "LogSink.h"
#include "../Files/File.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
        /// A log file that starts a new file every hour, day, week or month.
        ///
        /// The file name comes from the template: {date} (YYYYMMDD), {time}
        /// (HHMMSS) and {datetime} are replaced by the time the file was
        /// started; a template with none of them gets _&lt;date&gt;&lt;time&gt; before
        /// its extension ("app.log" -> "app_20261118093000.log"). After each
        /// rotation all but the newest nFilesToKeep files that match the
        /// template are deleted (0 keeps every file). With auto-flush every
        /// line is flushed; lines at ERROR and above always are.
        ///
        /// The rotation check and the write happen under one lock, so
        /// threads never write to a file another has just closed. If a file
        /// cannot be opened, lines are dropped (writes return false) and the
        /// next line tries again.
        ///
        /// @code
        /// CRotatingLogFile log( CLogSink::INFO, "logs/app-{date}.log", CRotatingLogFile::DAILY, 7 );
        /// INFO_LOG( "rotating daily, keeping a week" );
        /// @endcode
    class CRotatingLogFile : public CLogSink
    {
        public:
            /// How often a new file is started.
            enum Interval
            {
                HOURLY,     ///< At the start of each hour.
                DAILY,      ///< At the start of each day.
                WEEKLY,     ///< At the start of each week (the day CDate::GetDOW() calls 0).
                MONTHLY     ///< At the start of each month.
            };

        protected:
            CFile              *m_fileOutputFile;       ///< The current file (owned; NULL if none could be opened).
            std::mutex          m_mutexProtect;         ///< Guards the file, its name and the settings.
            CString             m_strFileNameTemplate;  ///< The file name template.
            CString             m_strCurrentFileName;   ///< The name of the file being written.
            size_t              m_nFilesToKeep;         ///< How many matching files to keep (0: all).
            bool                m_bAutoFlush;           ///< True: flush after every line.
            Interval            m_rotationInterval;     ///< How often to rotate.
            long                m_lCurrentPeriod;       ///< The period (see PeriodOf()) of the current file; -1 if none.

        public:
                                DEFINE_OBJECT( CRotatingLogFile );
                                /// Creates the sink and opens the first file.
                                /// @param level               The lowest level written.
                                /// @param strFileNameTemplate The file name template (NULL gives "log.txt").
                                /// @param rotationInterval    How often to start a new file.
                                /// @param nFilesToKeep        How many matching files to keep; 0 or less keeps all.
                                /// @param bAutoFlush          True to flush after every line.
                                /// @param bMakeDefault        True to make this the sink the *_LOG macros use.
                                /// @note The file pointer used to be read before it was set,
                                /// so the first rotation closed and deleted whatever was in that memory.
                                CRotatingLogFile( Level level, const char *strFileNameTemplate, Interval rotationInterval = DAILY,
                                                  int nFilesToKeep = 3, bool bAutoFlush = false, bool bMakeDefault = true );
                                /// Closes the current file.
            virtual            ~CRotatingLogFile( void );

                /// Formats and writes a message, rotating first if due, and
                /// flushing it if the level is ERROR or above.
                /// @copydetails CLogSink::writeLog()
            virtual bool        writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... );
                /// Writes one line and a newline, rotating first if due.
                /// @param message The line, without its newline (NULL writes an empty line).
                /// @return False if no file is open or the write was short.
            virtual bool        writeLine( const char *message );

                /// Turns flushing after every line on or off.
                /// @note It used to have no effect: every line was flushed regardless.
            void                setAutoFlush( bool bAutoFlush );
                /// The file being written now.
            CString             GetCurrentFileName( void );

                /// Which period a time is in (rotation happens when it
                /// changes).
                /// @param when     The time.
                /// @param interval The rotation interval.
                /// @return A number that is the same for every time in one period.
            static long         PeriodOf( const CDate &when, Interval interval );
                /// The name a file started at this time would get.
                /// @param when The time the file would be started.
            CString             FileNameFor( const CDate &when ) const;

        protected:
                /// For tests: writes as if it were this time.
                /// @param message The line, without its newline.
                /// @param when    The time to rotate by.
                /// @param bFlush  True to flush the line.
                /// @return False if no file is open or the write was short.
            bool                writeLineAt( const char *message, const CDate &when, bool bFlush );

        private:
                /// Under the lock: opens a new file if the period changed (or none is open).
                /// @note The check used to be made outside the lock, so two
                /// threads could rotate at once.
            void                rotateIfDue( const CDate &now );
                /// Deletes all but the newest m_nFilesToKeep files that match the template.
                /// @note Old files used to never be removed.
            void                pruneOldFiles( void );
    };
} // namespace IASLib
#endif // IASLIB_ROTATINGLOGFILE_H__
