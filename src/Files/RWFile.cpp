/*
 *  Read/Write File Class
 *
 *      This class, inherited from CFile, provides for a file with an
 * associated index that stores where the file is being written or read
 * from. This allows two different processes or threads to read from a
 * file and coordinate their actions.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/21/2002
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "RWFile.h"
#include <errno.h>
#include <stdlib.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CRWFile, CObject );

    namespace
    {
        const int RECORD = 16;          // "%15ld\n"
    }

    CRWFile::CRWFile()
    {
        m_strBaseFileName = "";
        m_bRead = false;
        m_nReadLine = 0;
        m_nWriteLine = 0;
        m_nReadPos = 0;
        m_nWritePos = 0;
        m_bFileOpen = false;
    }

    CRWFile::CRWFile( const char *strFileName, int nMode )
    {
        m_bRead = false;
        m_nReadLine = 0;
        m_nWriteLine = 0;
        m_nReadPos = 0;
        m_nWritePos = 0;
        m_bFileOpen = false;

        Open( strFileName, nMode );
    }

    CRWFile::~CRWFile()
    {
        if ( m_bFileOpen )
        {
            Close();
        }
    }

        // Binary, so the index records are 16 bytes on every system (in
        // text mode Windows made the newline two bytes, and the reader's
        // record was written over the writer's). CFile throws when a file
        // cannot be opened, so the "else create it" steps never ran.
    bool CRWFile::OpenFile( CFile &oFile, const CString &strName, bool bCreate )
    {
        try
        {
            if ( oFile.Open( strName, CFile::READ | CFile::WRITE | CFile::BINARY ) )
                return true;
        }
        catch ( ... )
        {
        }
        if ( ! bCreate )
            return false;
        try
        {
            return oFile.Open( strName, CFile::READ | CFile::WRITE | CFile::CREATE | CFile::BINARY );
        }
        catch ( ... )
        {
            return false;
        }
    }

    bool CRWFile::ReadIndex( long &nWriteLine, long &nReadLine )
    {
        char achIndex[ RECORD * 2 + 1 ];
        if ( m_oIndexFile.SeekBig( 0, CFile::BEGIN ) != 0 ||
             m_oIndexFile.ReadBig( achIndex, RECORD * 2 ) != RECORD * 2 )
            return false;
        achIndex[ RECORD * 2 ] = '\0';
        nWriteLine = atol( achIndex );
        nReadLine = atol( achIndex + RECORD );
        return true;
    }

    bool CRWFile::Open( const char *strFileName, int nMode )
    {
        if ( m_bFileOpen )
        {
            Close();
        }

        m_bRead = ( nMode != WRITE );
        m_nReadLine = 0;
        m_nReadPos = 0;
        m_nWriteLine = 0;
        m_nWritePos = 0;
        m_bFileOpen = false;
        m_strBaseFileName = ( strFileName ) ? strFileName : "";
        CString strIndex = m_strBaseFileName + ".idx";

        if ( ! m_bRead )
        {
            if ( ! OpenFile( m_oIndexFile, strIndex, true ) )
                return false;
            if ( ! OpenFile( m_oDataFile, m_strBaseFileName, true ) )
            {
                m_oIndexFile.Close();
                return false;
            }

            long nWriteLine = 0, nReadLine = 0;
            bool bHaveIndex = ReadIndex( nWriteLine, nReadLine );
            if ( ! bHaveIndex )
                nReadLine = 0;

                // Where the last writer stopped: after its last line, or (if
                // it finished: -1) after everything. (Reopening a finished
                // file started again at line 0, over the old lines.)
            m_oDataFile.SeekBig( 0, CFile::BEGIN );
            long nLines = 0;
            int64_t nPos = 0;
            while ( nWriteLine < 0 || nLines < nWriteLine )
            {
                CString strLine = m_oDataFile.GetLine();
                if ( strLine.GetLength() == 0 || strLine[ strLine.GetLength() - 1 ] != '\n' )
                    break;                      // the end, or a line never finished
                nLines++;
                nPos = m_oDataFile.GetPosBig();
            }
            m_nWriteLine = nLines;
            m_nWritePos = nPos;
            m_nReadLine = nReadLine;
            m_bFileOpen = true;

            CString strRecords;
            strRecords.Format( "%15ld\n%15ld\n", m_nWriteLine, m_nReadLine );
            m_oIndexFile.SeekBig( 0, CFile::BEGIN );
            m_oIndexFile.WriteString( strRecords );
            m_oIndexFile.Flush();
        }
        else
        {
            if ( ! OpenFile( m_oIndexFile, strIndex, false ) )
                return false;
            long nWriteLine = 0, nReadLine = 0;
            if ( ! ReadIndex( nWriteLine, nReadLine ) )
            {
                m_oIndexFile.Close();
                return false;
            }
            try
            {
                if ( ! m_oDataFile.Open( m_strBaseFileName, CFile::READ | CFile::BINARY ) )
                    throw 0;
            }
            catch ( ... )
            {
                m_oIndexFile.Close();
                return false;
            }

                // Skip the lines the last reader took.
            m_oDataFile.SeekBig( 0, CFile::BEGIN );
            for ( long nCount = 0; nCount < nReadLine; nCount++ )
            {
                CString strLine = m_oDataFile.GetLine();
                if ( strLine.GetLength() == 0 || strLine[ strLine.GetLength() - 1 ] != '\n' )
                    break;
                m_nReadLine++;
                m_nReadPos = m_oDataFile.GetPosBig();
            }
            m_bFileOpen = true;
        }

        return m_bFileOpen;
    }

    bool CRWFile::Close( bool bComplete )
    {
        if ( m_bFileOpen )
        {
            if ( ! m_bRead )
            {
                m_oDataFile.Flush();
                if ( bComplete )
                {
                    m_nWriteLine = -1;
                }
            }
            UpdateIndex();
            m_oDataFile.Close();
            m_oIndexFile.Close();

            m_bFileOpen = false;

            return true;
        }
        return false;
    }

    bool CRWFile::Flush( void )
    {
        if ( m_bFileOpen )
        {
            bool bOk = m_bRead || m_oDataFile.Flush();
            return m_oIndexFile.Flush() && bOk;
        }

        return false;
    }

    bool CRWFile::Write( const char *achBuffer, int nLength )
    {
        if ( ( ! m_bRead ) && ( m_bFileOpen ) && achBuffer && nLength >= 0 )
        {
            m_oDataFile.SeekBig( m_nWritePos, CFile::BEGIN );
		    if ( m_oDataFile.WriteBig( achBuffer, (size_t)nLength ) == nLength )
		    {
                m_oDataFile.Flush();
			    m_nWritePos += nLength;
			    for ( int nCount = 0; nCount < nLength ; nCount++ )
			    {
				    if ( achBuffer[nCount] == '\n' )
				    {
					    m_nWriteLine++;
				    }
			    }
			    UpdateIndex();
			    return true;
		    }
        }
        return false;
    }

        // (A line with newlines in it counted as one.)
    bool CRWFile::WriteLine( const CString &oString )
    {
        CString strTemp = oString;
        strTemp += "\n";
        return Write( (const char *)strTemp, (int)strTemp.GetLength() );
    }

    CString CRWFile::GetLine( void )
    {
        CString strTemp;
        if ( ( m_bRead ) && ( m_bFileOpen ) && ! IsEOF() )
        {
            m_oDataFile.SeekBig( m_nReadPos, CFile::BEGIN );
            strTemp = m_oDataFile.GetLine();
            if ( strTemp.GetLength() == 0 || strTemp[ strTemp.GetLength() - 1 ] != '\n' )
            {
                    // Not finished yet: leave it for the next call.
                return CString( "" );
            }
            m_nReadPos = m_oDataFile.GetPosBig();
            m_nReadLine++;
            UpdateIndex();
        }
        return strTemp;
    }

    long CRWFile::GetPos( void )
    {
        if ( m_bFileOpen )
        {
            return ( m_bRead ) ? m_nReadLine : m_nWriteLine;
        }
        return -1;
    }

    bool CRWFile::IsWriterDone( void )
    {
        if ( m_bFileOpen && m_bRead )
        {
            long nWriteLine = 0, nReadLine = 0;
            return ReadIndex( nWriteLine, nReadLine ) && nWriteLine < 0;
        }
        return false;
    }

    bool CRWFile::IsEOF( void )
    {
        if ( m_bFileOpen && m_bRead )
        {
                // The size now (the writer may have added to it).
            int64_t nFileSize = m_oDataFile.SeekBig( 0, CFile::END );
            m_oDataFile.SeekBig( m_nReadPos, CFile::BEGIN );
            return m_nReadPos >= nFileSize;
        }
        return false;
    }

    void CRWFile::UpdateIndex( void )
    {
        if ( m_bFileOpen )
        {
            CString strTemp;
            strTemp.Format( "%15ld\n", ( m_bRead ) ? m_nReadLine : m_nWriteLine );
            m_oIndexFile.SeekBig( ( m_bRead ) ? RECORD : 0, CFile::BEGIN );
            m_oIndexFile.WriteString( strTemp );
            m_oIndexFile.Flush();
        }
    }
} // namespace IASLib
