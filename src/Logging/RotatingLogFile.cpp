/***********************************************************************
**  Rotating Log File Class
**
**  Description:
**      This class defines a log file that can be written to for
** tracking data. After a set interval, the log file will roll-over to a
** new log file. The log file format uses a template for the log name
** which can contain a {date} field that will be filled with an ISO-9601 
** date value. 
**
**  Author: Jeffrey R. Naujok
**  Created: September 10, 2019
**
** Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
** [A division of BlackStar Enterprises, LLC.]
***********************************************************************/

#include "RotatingLogFile.h"

namespace IASLib
{ 

IMPLEMENT_OBJECT( CRotatingLogFile, CLogSink );

CRotatingLogFile::CRotatingLogFile( Level level, const char *strFileNameTemplate, Interval rotationInterval, int nFilesToKeep, bool bAutoFlush ) : CLogSink( level )
#ifdef IASLIB_MULTI_THREADED__            
        , m_mutexProtect()
#endif
{
    m_lastChanged = CDate::ParseGMT( "1970-01-01 00:00:00" );
    m_nFilesToKeep = nFilesToKeep;
    m_bAutoFlush = bAutoFlush;
    m_rotationInterval = rotationInterval;
    m_nCurrentDay = -1;
    m_nCurrentDOW = -1;
    m_nCurrentHour = -1;
    m_nCurrentMonth = -1;
    switch ( rotationInterval )
    {
        case HOURLY:
            m_nInterval = 3600;
            break;

        case DAILY:
        case WEEKLY:
        case MONTHLY:
            m_nInterval = 86400;
            break;

        default:
            m_nInterval = 86400;
            break;
    }

    m_strFileNameTemplate = strFileNameTemplate;
    rotateLog();
}

CRotatingLogFile::~CRotatingLogFile( void )
{
    if ( m_fileOutputFile ) 
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif        
        m_fileOutputFile->Close();
        delete m_fileOutputFile;
        m_fileOutputFile = NULL;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif        
    }
}

void CRotatingLogFile::setAutoFlush( bool bAutoFlush )
{
    m_bAutoFlush = bAutoFlush;
}

bool CRotatingLogFile::writeLine( const char *message )
{
    rotateLog();
    if ( m_fileOutputFile )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif        
        m_fileOutputFile->WriteString( message );
        m_fileOutputFile->WriteString( "\n" );
        m_fileOutputFile->Flush();
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif        
    }

    return true;
}

bool CRotatingLogFile::isPastRotation( void )
{
    CDate now;
    bool retVal = false;

    switch( m_rotationInterval )
    {
        case HOURLY:
            if ( m_nCurrentHour != now.GetHour() )
                retVal = true;
            break;

        case DAILY:
            if ( m_nCurrentDay != now.GetDay() )
                retVal = true;
            break;

        case WEEKLY:
            if ( ( m_nCurrentDOW != now.GetDOW() ) && ( now.GetDOW() == 0 ) )
            {
                retVal = true;
            }
            else
            {
                m_nCurrentDOW = now.GetDOW();
                // Don't bother checking again until tomorrow
                m_lastChanged.SetToCurrent();
                m_lastChanged.StartOfDay();
            }
            break;

        case MONTHLY:
            if ( m_nCurrentMonth != now.GetMonth() )
                retVal = true;
            else
            {
                // No point checking again until tomorrow
                m_lastChanged.SetToCurrent();
                m_lastChanged.StartOfDay();
            }
            break;

        default:
            break;
    }
    return retVal;
}

void CRotatingLogFile::rotateLog( void )
{
    CDate now;
    now.SetToCurrent();
    CDate temp = m_lastChanged;
    temp.Add( 0, 0, m_nInterval );
    
    if ( ( m_fileOutputFile == NULL ) 
         || 
         ( 
            ( now >= temp ) 
               && 
            isPastRotation() 
         ) 
       )
    {
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Lock();
#endif        
        // Time to rotate logs
        if ( m_fileOutputFile )   
        {
            m_fileOutputFile->Close();
            delete m_fileOutputFile;
            m_fileOutputFile = NULL;
        }

        CString filename( m_strFileNameTemplate );
        CDate now;

        bool found = false;
        if ( filename.Contains( "{date}" ) )
        {
            filename.Replace( "{date}", now.FormatDate( CDate::DF_YYYYMMDD ) );
            found = true;
        }
        if ( filename.Contains( "{time}" ) )
        {
            filename.Replace( "{time}", now.FormatDate( CDate::DF_HHMMSS ) );
            found = true;
        }
        if ( filename.Contains( "{datetime}" ) )
        {
            filename.Replace( "{datetime}", now.FormatDate( CDate::DF_YYYYMMDD ) + now.FormatDate( CDate::DF_HHMMSS ) );
            found = true;
        }
        if ( ! found )
        {
            CString fileExt = "";

            if ( filename.Contains( "." ) )
            {
                size_t last = filename.LastIndexOf( '.' );
                fileExt = filename.Substring( last );
                filename = filename.Substring( 0, last );
            }
            filename += "_";
            filename += now.FormatDate( CDate::DF_YYYYMMDD ) + now.FormatDate( CDate::DF_HHMMSS );
            filename += fileExt;
        }

        m_fileOutputFile = new CFile( (const char *)filename, CFile::APPEND );

        switch ( m_rotationInterval )
        {
            case DAILY:
            case WEEKLY:
            case MONTHLY:
                m_lastChanged.SetToCurrent();
                m_lastChanged.StartOfDay();
                break;

            case HOURLY:
                m_lastChanged.SetToCurrent();
                m_lastChanged.StartOfHour();
                break;
        }
#ifdef IASLIB_MULTI_THREADED__
        m_mutexProtect.Unlock();
#endif        
    }
}

}; // namespace IASLib