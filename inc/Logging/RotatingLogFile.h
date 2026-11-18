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

#ifndef IASLIB_ROTATINGLOGFILE_H__
#define IASLIB_ROTATINGLOGFILE_H__

#include <mutex>
#include "LogSink.h"
#include "../Files/File.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
        // A log file that starts a new file every hour, day, week or month.
        //
        // The file name comes from the template: {date} (YYYYMMDD), {time}
        // (HHMMSS) and {datetime} are replaced by the time the file was
        // started; a template with none of them gets _<date><time> before
        // its extension ("app.log" -> "app_20261118093000.log"). After each
        // rotation all but the newest nFilesToKeep files that match the
        // template are deleted (0 keeps every file). With auto-flush every
        // line is flushed; lines at ERROR and above always are.
    class CRotatingLogFile : public CLogSink
    {
        public:
            enum Interval
            {
                HOURLY,
                DAILY,
                WEEKLY,
                MONTHLY
            };

        protected:
            CFile              *m_fileOutputFile;
            std::mutex          m_mutexProtect;
            CString             m_strFileNameTemplate;
            CString             m_strCurrentFileName;
            size_t              m_nFilesToKeep;
            bool                m_bAutoFlush;
            Interval            m_rotationInterval;
            long                m_lCurrentPeriod;

        public:
                                DEFINE_OBJECT( CRotatingLogFile );
                                CRotatingLogFile( Level level, const char *strFileNameTemplate, Interval rotationInterval = DAILY,
                                                  int nFilesToKeep = 3, bool bAutoFlush = false, bool bMakeDefault = true );
            virtual            ~CRotatingLogFile( void );

            virtual bool        writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... );
            virtual bool        writeLine( const char *message );

            void                setAutoFlush( bool bAutoFlush );
                // The file being written now.
            CString             GetCurrentFileName( void );

                // Which period a time is in (rotation happens when it
                // changes), and the name a file started then would get.
            static long         PeriodOf( const CDate &when, Interval interval );
            CString             FileNameFor( const CDate &when ) const;

        protected:
                // For tests: writes as if it were this time.
            bool                writeLineAt( const char *message, const CDate &when, bool bFlush );

        private:
            void                rotateIfDue( const CDate &now );
            void                pruneOldFiles( void );
    };
} // namespace IASLib
#endif // IASLIB_ROTATINGLOGFILE_H__
