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


#include "LogFile.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CLogFile, CLogSink );

    CLogFile::CLogFile( Level level, const char *strFileName, bool bDeletePrevious, bool bAutoFlush, bool bMakeDefault )
        : CLogSink( level, bMakeDefault ), m_bAutoFlush( bAutoFlush )
    {
        m_strFileName = ( strFileName ) ? strFileName : "";
        m_fileOutputFile = NULL;
            // A file that cannot be opened leaves the log closed (IsOpen()
            // is false) rather than throwing out of the constructor.
        try
        {
            m_fileOutputFile = new CFile( m_strFileName, ( bDeletePrevious ) ? CFile::WRITE : CFile::APPEND );
        }
        catch ( ... )
        {
            m_fileOutputFile = NULL;
        }
    }

        // (The file was closed but never deleted.)
    CLogFile::~CLogFile( void )
    {
        std::lock_guard<std::mutex> lock( m_mutexProtect );
        if ( m_fileOutputFile )
        {
            m_fileOutputFile->Close();
            delete m_fileOutputFile;
            m_fileOutputFile = NULL;
        }
    }

    bool CLogFile::IsOpen( void )
    {
        std::lock_guard<std::mutex> lock( m_mutexProtect );
        return m_fileOutputFile != NULL && m_fileOutputFile->IsOpen();
    }

    void CLogFile::Flush( void )
    {
        std::lock_guard<std::mutex> lock( m_mutexProtect );
        if ( m_fileOutputFile )
            m_fileOutputFile->Flush();
    }

    bool CLogFile::writeLog( Level logLevel, const char *strFilename, int nLineNum, const char *strFormat, ... )
    {
        if ( ! isEnabled( logLevel ) )
            return false;
        va_list vaArgs;
        va_start( vaArgs, strFormat );
        CString strMessage = formatV( strFormat, vaArgs );
        va_end( vaArgs );
        return writeLineFlushed( formatLine( logLevel, strFilename, nLineNum, strMessage ), logLevel >= ERROR );
    }

    bool CLogFile::writeLine( const char *message )
    {
        return writeLineFlushed( message, false );
    }

        // One write per line, so lines from different threads never mix.
        // (It reported success whatever happened.)
    bool CLogFile::writeLineFlushed( const char *message, bool bFlush )
    {
        CString strLine( ( message ) ? message : "" );
        strLine += "\n";

        std::lock_guard<std::mutex> lock( m_mutexProtect );
        if ( m_fileOutputFile == NULL || ! m_fileOutputFile->IsOpen() )
            return false;
        bool bOk = m_fileOutputFile->Write( (const char *)strLine, (int)strLine.GetLength() ) == (int)strLine.GetLength();
        if ( bFlush || m_bAutoFlush )
            m_fileOutputFile->Flush();
        return bOk;
    }
} // namespace IASLib
