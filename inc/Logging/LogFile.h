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

#include "../BaseTypes/Object.h"
#include "../Threading/Mutex.h"
#include "../Files/File.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    class CLogFile : public CObject
    {
        protected:
            CFile       m_fileOutputFile;
            CMutex      m_mutexProtect;
            CString     m_strFileName;
            CString     m_strLogFormat;
            int         m_nDateFormat;
            bool        m_bTimeStamp;
            
        public:
                        CLogFile( const char *strFileName, bool bDeletePrevious = false );
            virtual    ~CLogFile( void );

            void        SetTimeStamp( bool bActivated );
            bool        IsTimeStamped( void ) { return m_bTimeStamp; }

            void        SetDateFormat( int nFormat );
            int         GetDateFormat( void ) { return m_nDateFormat; }

            void        SetLogFormat( const char *strLogFormat );
            const char *GetLogFormat( void ) { return (const char *)m_strLogFormat; }

            bool        Entry( ... );
            bool        Write( const char *strFormat, ... );

            bool        Open( void );
            bool        Close( void );

            bool        Clear( void );

            bool        IsOpen( void ) { return m_fileOutputFile.IsOpen(); }
    };
} // namespace IASLib
#endif // IASLIB_LOGFILE_H__
