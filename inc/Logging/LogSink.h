/***********************************************************************
**  Log Sink Class
**
**  Description:
**      This class defines a generic log sink (a target for logs) that
** can be used to derive log sinks -- for example a log file -- that
** consume log messages and store them in the sink. By building a generic
** log sink, we can abstract away the actual storage of the messages.
**
**  $AUTHOR$
**  $LOG$
**
** Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
** [A division of BlackStar Enterprises, LLC.]
***********************************************************************/

#ifndef IASLIB_LOGSINK_H__
#define IASLIB_LOGSINK_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    #define DEFAULT_LOG_FORMAT "{Date} [{File}:{Line}] - {Message}"

    class CLogSink : public CObject
    {
        protected:
            CString     m_strLogFormat;
            int         m_nDateFormat;
            bool        m_bTimeStamp;

        public:
                        CLogSink( void );
            virtual    ~CLogSink( void );

                                DEFINE_OBJECT( CLogSink );

            virtual void        SetTimeStamp( bool bActivated ) { m_bTimeStamp = bActivated; }
            virtual bool        IsTimeStamped( void ) { return m_bTimeStamp; }

            virtual void        SetDateFormat( int nDateFormat ) { m_nDateFormat = nDateFormat; }
            virtual int         GetDateFormat( void ) { return m_nDateFormat; }

            virtual void        SetLogFormat( const char *strLogFormat ) { m_strLogFormat = strLogFormat; }
            virtual const char *GetLogFormat( void ) { return (const char *)m_strLogFormat; }

            virtual bool        Entry( ... ) = 0;
            virtual bool        Write( const char *strFormat, ... ) = 0;

            virtual bool        Open( void ) = 0;
            virtual bool        Close( void ) = 0;

            virtual bool        Clear( void ) { return false; }

            virtual bool        IsOpen( void ) { return false; }
        protected:
            virtual CString     formatMessage( const char *strFormat, ... );
    };
} // namespace IASLib
#endif // IASLIB_LOGSINK_H__
