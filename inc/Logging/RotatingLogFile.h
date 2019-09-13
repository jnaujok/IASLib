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

#include "LogSink.h"
#include "../Threading/Mutex.h"
#include "../Files/File.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
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
            CDate               m_lastChanged;
            size_t              m_nInterval;
            CFile              *m_fileOutputFile;
#ifdef IASLIB_MULTI_THREADED__            
            CMutex              m_mutexProtect;
#endif
            CString             m_strFileNameTemplate;
            size_t              m_nFilesToKeep;
            bool                m_bAutoFlush;
            Interval            m_rotationInterval;
            int                 m_nCurrentDOW;
            int                 m_nCurrentMonth;
            int                 m_nCurrentDay;
            int                 m_nCurrentHour;

        public:
                                DEFINE_OBJECT( CRotatingLogFile );
                                CRotatingLogFile( Level level, const char *strFileNameTemplate, Interval rotationInterval = Interval::DAILY, int nFilesToKeep = 3, bool bAutoFlush = false );
            virtual            ~CRotatingLogFile( void );

            virtual bool        writeLine( const char *message );

            void                setAutoFlush( bool bAutoFlush );

        private:
            void                rotateLog( void );
            bool                isPastRotation( void );
    };
} // namespace IASLib
#endif // IASLIB_ROTATINGLOGFILE_H__
