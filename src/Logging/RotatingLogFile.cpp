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
#include "../../inc/Files/Directory.h"
#include <algorithm>
#include <string>
#include <vector>

namespace IASLib
{
    IMPLEMENT_OBJECT( CRotatingLogFile, CLogSink );

    namespace
    {
        const char *const s_astrPlaceholders[] = { "{datetime}", "{date}", "{time}" };

            // Splits a path into its directory ("." if none) and file name.
        void splitPath( const std::string &strPath, std::string &strDir, std::string &strName )
        {
            size_t nSlash = strPath.find_last_of( "/\\" );
            if ( nSlash == std::string::npos )
            {
                strDir = ".";
                strName = strPath;
            }
            else
            {
                strDir = ( nSlash == 0 ) ? strPath.substr( 0, 1 ) : strPath.substr( 0, nSlash );
                strName = strPath.substr( nSlash + 1 );
            }
        }

            // The fixed text before and after the date in a template's file
            // names.
        void templateParts( const std::string &strTemplateName, std::string &strPrefix, std::string &strSuffix )
        {
            size_t nFirst = std::string::npos;
            size_t nLastEnd = 0;
            for ( size_t n = 0; n < sizeof( s_astrPlaceholders ) / sizeof( s_astrPlaceholders[0] ); n++ )
            {
                std::string strHolder( s_astrPlaceholders[n] );
                size_t nAt = strTemplateName.find( strHolder );
                while ( nAt != std::string::npos )
                {
                    if ( nFirst == std::string::npos || nAt < nFirst )
                        nFirst = nAt;
                    if ( nAt + strHolder.size() > nLastEnd )
                        nLastEnd = nAt + strHolder.size();
                    nAt = strTemplateName.find( strHolder, nAt + 1 );
                }
            }
            if ( nFirst != std::string::npos )
            {
                strPrefix = strTemplateName.substr( 0, nFirst );
                strSuffix = strTemplateName.substr( nLastEnd );
                return;
            }
            size_t nDot = strTemplateName.rfind( '.' );
            if ( nDot == std::string::npos )
            {
                strPrefix = strTemplateName + "_";
                strSuffix = "";
            }
            else
            {
                strPrefix = strTemplateName.substr( 0, nDot ) + "_";
                strSuffix = strTemplateName.substr( nDot );
            }
        }
    }

        // (The file pointer was read before it was ever set, so the first
        // rotation closed and deleted whatever was in that memory.)
    CRotatingLogFile::CRotatingLogFile( Level level, const char *strFileNameTemplate, Interval rotationInterval,
                                        int nFilesToKeep, bool bAutoFlush, bool bMakeDefault )
        : CLogSink( level, bMakeDefault )
    {
        m_fileOutputFile = NULL;
        m_strFileNameTemplate = ( strFileNameTemplate ) ? strFileNameTemplate : "log.txt";
        m_nFilesToKeep = ( nFilesToKeep > 0 ) ? (size_t)nFilesToKeep : 0;
        m_bAutoFlush = bAutoFlush;
        m_rotationInterval = rotationInterval;
        m_lCurrentPeriod = -1;

        CDate now;
        std::lock_guard<std::mutex> lock( m_mutexProtect );
        rotateIfDue( now );
    }

    CRotatingLogFile::~CRotatingLogFile( void )
    {
        std::lock_guard<std::mutex> lock( m_mutexProtect );
        if ( m_fileOutputFile )
        {
            m_fileOutputFile->Close();
            delete m_fileOutputFile;
            m_fileOutputFile = NULL;
        }
    }

        // (It had no effect: every line was flushed regardless.)
    void CRotatingLogFile::setAutoFlush( bool bAutoFlush )
    {
        std::lock_guard<std::mutex> lock( m_mutexProtect );
        m_bAutoFlush = bAutoFlush;
    }

    CString CRotatingLogFile::GetCurrentFileName( void )
    {
        std::lock_guard<std::mutex> lock( m_mutexProtect );
        return m_strCurrentFileName;
    }

    long CRotatingLogFile::PeriodOf( const CDate &when, Interval interval )
    {
        switch ( interval )
        {
            case HOURLY:
                return when.GetEpochDay() * 24 + when.GetHour();
            case WEEKLY:
                return when.GetEpochDay() - when.GetDOW();
            case MONTHLY:
                return (long)when.GetYear() * 12 + when.GetMonth();
            case DAILY:
            default:
                return when.GetEpochDay();
        }
    }

    CString CRotatingLogFile::FileNameFor( const CDate &when ) const
    {
        CString strDate = when.FormatDate( CDate::DF_YYYYMMDD );
        CString strTime = when.FormatDate( CDate::DF_HHMMSS );
        CString filename( m_strFileNameTemplate );

        bool bFound = false;
        if ( filename.Contains( "{datetime}" ) )
        {
            filename.Replace( "{datetime}", strDate + strTime );
            bFound = true;
        }
        if ( filename.Contains( "{date}" ) )
        {
            filename.Replace( "{date}", strDate );
            bFound = true;
        }
        if ( filename.Contains( "{time}" ) )
        {
            filename.Replace( "{time}", strTime );
            bFound = true;
        }
        if ( ! bFound )
        {
            std::string strDir, strName;
            splitPath( (const char *)m_strFileNameTemplate, strDir, strName );
            std::string strPrefix, strSuffix;
            templateParts( strName, strPrefix, strSuffix );
            std::string strPath( (const char *)m_strFileNameTemplate );
            std::string strBase = strPath.substr( 0, strPath.size() - strName.size() );
            filename = ( strBase + strPrefix + (const char *)strDate + (const char *)strTime + strSuffix ).c_str();
        }
        return filename;
    }

        // Under the lock. (The check was made outside the lock that guarded
        // the file, so two threads could rotate at once, or one could write
        // to a file another had just deleted.)
    void CRotatingLogFile::rotateIfDue( const CDate &now )
    {
        long lPeriod = PeriodOf( now, m_rotationInterval );
        if ( m_fileOutputFile != NULL && lPeriod == m_lCurrentPeriod )
            return;

        if ( m_fileOutputFile )
        {
            m_fileOutputFile->Close();
            delete m_fileOutputFile;
            m_fileOutputFile = NULL;
        }

        m_lCurrentPeriod = lPeriod;
        m_strCurrentFileName = FileNameFor( now );
        try
        {
            m_fileOutputFile = new CFile( (const char *)m_strCurrentFileName, CFile::APPEND );
        }
        catch ( ... )
        {
            m_fileOutputFile = NULL;    // lines are dropped until a file opens
            m_lCurrentPeriod = -1;      // and the next line tries again
            return;
        }
        pruneOldFiles();
    }

        // (nFilesToKeep was stored but old files were never removed.)
    void CRotatingLogFile::pruneOldFiles( void )
    {
        if ( m_nFilesToKeep == 0 )
            return;

        std::string strDir, strName;
        splitPath( (const char *)m_strFileNameTemplate, strDir, strName );
        std::string strPrefix, strSuffix;
        templateParts( strName, strPrefix, strSuffix );

        std::string strCurrentDir, strCurrentName;
        splitPath( (const char *)m_strCurrentFileName, strCurrentDir, strCurrentName );

            // The names hold the date as digits, so they sort by age.
        std::vector<std::string> aMatches;
        CDirectory directory( strDir.c_str(), "*", true );
        for ( size_t n = 0; n < directory.GetEntryCount(); n++ )
        {
            const char *strEntry = directory.GetFileName( n );
            if ( strEntry == NULL )
                continue;
            std::string strFile( strEntry );
            if ( strFile.size() <= strPrefix.size() + strSuffix.size() )
                continue;
            if ( strFile.compare( 0, strPrefix.size(), strPrefix ) != 0 )
                continue;
            if ( strFile.compare( strFile.size() - strSuffix.size(), strSuffix.size(), strSuffix ) != 0 )
                continue;
            std::string strMiddle = strFile.substr( strPrefix.size(), strFile.size() - strPrefix.size() - strSuffix.size() );
            if ( strMiddle.find_first_not_of( "0123456789" ) != std::string::npos )
                continue;       // not one of ours
            aMatches.push_back( strFile );
        }
        std::sort( aMatches.begin(), aMatches.end() );

        size_t nExcess = ( aMatches.size() > m_nFilesToKeep ) ? aMatches.size() - m_nFilesToKeep : 0;
        for ( size_t n = 0; n < nExcess; n++ )
        {
            if ( aMatches[n] == strCurrentName )
                continue;
            std::string strPath = ( strDir == "." ) ? aMatches[n] : strDir + "/" + aMatches[n];
            try
            {
                CFile::Delete( strPath.c_str() );
            }
            catch ( ... )
            {
                    // Gone already, or not ours to delete: leave it.
            }
        }
    }

    bool CRotatingLogFile::writeLineAt( const char *message, const CDate &when, bool bFlush )
    {
        CString strLine( ( message ) ? message : "" );
        strLine += "\n";

        std::lock_guard<std::mutex> lock( m_mutexProtect );
        rotateIfDue( when );
        if ( m_fileOutputFile == NULL || ! m_fileOutputFile->IsOpen() )
            return false;
        bool bOk = m_fileOutputFile->Write( (const char *)strLine, (int)strLine.GetLength() ) == (int)strLine.GetLength();
        if ( bFlush || m_bAutoFlush )
            m_fileOutputFile->Flush();
        return bOk;
    }

    bool CRotatingLogFile::writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... )
    {
        if ( ! isEnabled( logLevel ) )
            return false;
        va_list vaArgs;
        va_start( vaArgs, strFormat );
        CString strMessage = formatV( strFormat, vaArgs );
        va_end( vaArgs );
        CDate now;
        return writeLineAt( formatLine( logLevel, strFilename, nLineNum, strMessage ), now, logLevel >= ERROR );
    }

    bool CRotatingLogFile::writeLine( const char *message )
    {
        CDate now;
        return writeLineAt( message, now, false );
    }
} // namespace IASLib
