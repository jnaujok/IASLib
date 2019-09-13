/**
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

#include "LogSink.h"
#include "../Threading/Mutex.h"
#include "../Files/File.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    class CLogFile : public CLogSink
    {
        protected:
            CFile              *m_fileOutputFile;
#ifdef IASLIB_MULTI_THREADED__            
            CMutex              m_mutexProtect;
#endif
            CString             m_strFileName;

        public:
                                DEFINE_OBJECT( CLogFile );
                                CLogFile( Level level, const char *strFileName, bool bDeletePrevious = false );
            virtual            ~CLogFile( void );

            virtual bool        writeLine( const char *message );


            virtual bool        IsOpen( void ) { return m_fileOutputFile->IsOpen(); }
    };
} // namespace IASLib
#endif // IASLIB_LOGFILE_H__
