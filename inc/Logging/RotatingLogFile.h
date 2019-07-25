/***********************************************************************
**  Log File Class
**
**  Description:
**      This class defines a log file that can be written to for
** tracking data. It can be accessed by multiple threads and can date
** and time stamp each entry. It is also possible to set the default
** format for the log entry.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef IASLIB_LOGFILE_H__
#define IASLIB_LOGFILE_H__

#include "LogSink.h"
#include "../Threading/Mutex.h"
#include "../Files/File.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    class CRotatingLogFile : public CLogSink
    {
        protected:
            CDate       m_lastChanged;
            size_t      m_nInterval;
            CFile      *m_fileOutputFile;
            CMutex      m_mutexProtect;
            CString     m_strFileNameTemplate;

        public:
                        DEFINE_OBJECT( CRotatingLogFile );
                        CRotatingLogFile( const char *strFileNameTemplate, bool bDeletePrevious = false );
            virtual    ~CRotatingLogFile( void );

            virtual bool        Entry( ... );
            virtual bool        Write( const char *strFormat, ... );

            virtual bool        Open( void );
            virtual bool        Close( void );

            virtual bool        Clear( void );

            virtual bool        IsOpen( void ) { return m_fileOutputFile->IsOpen(); }
    };
} // namespace IASLib
#endif // IASLIB_LOGFILE_H__
