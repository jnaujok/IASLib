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

#include <mutex>
#include "LogSink.h"
#include "../Files/File.h"
#include "../BaseTypes/String_.h"

namespace IASLib
{
        // Appends lines to one file (or replaces it, with bDeletePrevious).
        // With auto-flush, each line is flushed as it is written; lines at
        // ERROR and above always are.
    class CLogFile : public CLogSink
    {
        protected:
            CFile              *m_fileOutputFile;
            std::mutex          m_mutexProtect;
            CString             m_strFileName;
            bool                m_bAutoFlush;

        public:
                                DEFINE_OBJECT( CLogFile );
                                CLogFile( Level level, const char *strFileName, bool bDeletePrevious = false,
                                          bool bAutoFlush = false, bool bMakeDefault = true );
            virtual            ~CLogFile( void );

            virtual bool        writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... );
            virtual bool        writeLine( const char *message );

            void                setAutoFlush( bool bAutoFlush ) { m_bAutoFlush = bAutoFlush; }
            void                Flush( void );
            virtual bool        IsOpen( void );
            const CString      &GetFileName( void ) const { return m_strFileName; }

        private:
            bool                writeLineFlushed( const char *message, bool bFlush );
    };
} // namespace IASLib
#endif // IASLIB_LOGFILE_H__
