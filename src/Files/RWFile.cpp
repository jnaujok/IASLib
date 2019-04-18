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

    CRWFile::CRWFile()
    {
        m_strBaseFileName = "";
        m_bRead = false;
        m_nReadLine = -1;
        m_nWriteLine = -1;
        m_bFileOpen = false;
    }

    CRWFile::CRWFile( const char *strFileName, int nMode )
    {
        m_bRead = false;
        m_nReadLine = -1;
        m_nWriteLine = -1;
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

    bool CRWFile::Open( const char *strFileName, int nMode )
    {
        if ( m_bFileOpen )
        {
            Close();
        }

        m_bRead = false;
        m_nReadLine = 0;
        m_nReadPos = 0;
        m_nWriteLine = 0;
        m_bFileOpen = false;

        m_strBaseFileName = strFileName;

        if ( nMode == WRITE )
        {
            m_oIndexFile.Open( m_strBaseFileName + ".idx", CFile::READ | CFile::WRITE );
            if ( ! m_oIndexFile.IsOpen() )
            {
                m_oIndexFile.Open( m_strBaseFileName + ".idx", CFile::WRITE );
            }

            if ( m_oIndexFile.IsOpen() )
            {
                if ( m_oIndexFile.GetSize() != 0 )
                {
                    m_nWriteLine = atol( m_oIndexFile.GetLine() );
                }
                else
                {
                    m_nWriteLine = 0;
                    CString strOutputLine;
                    strOutputLine.Format( "%15ld\n", (long)0 );
                    m_oIndexFile.WriteString( strOutputLine );
                    m_oIndexFile.WriteString( strOutputLine );
                    m_oIndexFile.Flush();
                }

                m_oDataFile.Open( m_strBaseFileName, CFile::READ | CFile::WRITE );

                if ( ! m_oDataFile.IsOpen() )
                    m_oDataFile.Open( m_strBaseFileName, CFile::WRITE );

                if ( m_oDataFile.IsOpen() )
                {
                    m_oDataFile.Seek( 0, CFile::BEGIN );

                    for ( int nCount = 0; nCount < m_nWriteLine ; nCount++ )
                    {
                        CString strTemp = m_oDataFile.GetLine();
                    }
                    m_nWritePos = m_oDataFile.GetPos();

                    m_bFileOpen = true;
                }
            }
        }
        else
        {
            m_bRead = true;
            m_oIndexFile.Open( m_strBaseFileName + ".idx", CFile::READ | CFile::WRITE );
            if ( m_oIndexFile.IsOpen() )
            {
                if ( m_oIndexFile.GetSize() > 0 )
                {
                        // Read the write index first
                    long lTemp = atol( m_oIndexFile.GetLine() ); lTemp=lTemp; // To aviod compiler warnings.
                        // Read the read index.
                    m_nReadLine = atol( m_oIndexFile.GetLine() );

                    m_oDataFile.Open( m_strBaseFileName, CFile::READ );

                    if ( m_oDataFile.IsOpen() )
                    {
                        for ( int nCount = 0; nCount < m_nReadLine ; nCount++ )
                        {
                            m_oDataFile.GetLine();
                        }

                        m_nReadPos = m_oDataFile.GetPos();

                        m_bFileOpen = true;
                    }
                }
                else
                {
                    m_oIndexFile.Close();
                }
            }
            else
            {
                //DebugLog( "RWFile: Error Number %d\n", errno );
            }
        }

        return m_bFileOpen;
    }

    bool CRWFile::Close( bool bComplete )
    {
        if ( m_bFileOpen )
        {
            if ( ! m_bRead )
            {
                if ( bComplete )
                {
                    m_nWriteLine = -1;
                }
                m_oDataFile.Flush();
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
            m_oDataFile.Flush();
            m_oIndexFile.Flush();
            return true;
        }

        return false;
    }

    bool CRWFile::Write( const char *achBuffer, int nLength )
    {
        if ( ( ! m_bRead ) && ( m_bFileOpen ) )
        {
            m_oDataFile.Seek( m_nWritePos, CFile::BEGIN );
		    if ( m_oDataFile.Write( achBuffer, nLength ) == nLength )
		    {
			    m_nWritePos += nLength;
			    for (int nCount = 0; nCount < nLength ; nCount++ )
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

    bool CRWFile::WriteLine( const CString &oString )
    {
        if ( ( ! m_bRead ) && ( m_bFileOpen ) )
        {
            CString strTemp = oString;
            strTemp += "\n";
            m_oDataFile.Seek( m_nWritePos, CFile::BEGIN );
            m_oDataFile.WriteString( strTemp );
            m_oDataFile.Flush();
            m_nWritePos += (int)strTemp.GetLength();
            m_nWriteLine++;
            UpdateIndex();
            return true;
        }
        return false;
    }

    CString CRWFile::GetLine( void )
    {
        CString strTemp;
        if ( ( m_bRead ) && ( m_bFileOpen ) )
        {
            if ( ! IsEOF() )
            {
                m_oDataFile.Seek( m_nReadPos, CFile::BEGIN );
                strTemp = m_oDataFile.GetLine();
                m_nReadPos = m_oDataFile.GetPos();
                m_nReadLine++;
                UpdateIndex();
            }
        }
        return strTemp;
    }

    long CRWFile::GetPos( void )
    {
        if ( m_bFileOpen )
        {
            if ( m_bRead )
            {
                return m_nReadLine;
            }

            return m_nWriteLine;
        }

        return -1;
    }

    bool CRWFile::IsWriterDone( void )
    {
        if ( m_bFileOpen )
        {
            if ( m_bRead )
            {
                m_oIndexFile.Seek( 0, CFile::BEGIN );
                long lTemp = atol( m_oIndexFile.GetLine() );
                if ( lTemp < 0 )
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool CRWFile::IsEOF( void )
    {
        if ( m_bFileOpen )
        {
            if ( m_bRead )
            {
                long lFileSize = m_oDataFile.Seek( 0, CFile::END );
                m_oDataFile.Seek( m_nReadPos, CFile::BEGIN );
                if ( m_nReadPos == lFileSize )
                {
                    return true;
                }
            }
        }
        return false;
    }

    void CRWFile::UpdateIndex( void )
    {
        CString strTemp;

        if ( m_bFileOpen )
        {
            if ( m_bRead )
            {
                m_oIndexFile.Seek( 16, CFile::BEGIN );
                strTemp.Format( "%15ld\n", m_nReadLine );
                m_oIndexFile.WriteString( strTemp );
                m_oIndexFile.Flush();
            }
            else
            {
                m_oIndexFile.Seek( 0, CFile::BEGIN );
                strTemp.Format( "%15ld\n", m_nWriteLine );
                m_oIndexFile.WriteString( strTemp );
                m_oIndexFile.Flush();
            }
        }
    }
} // namespace IASLib

