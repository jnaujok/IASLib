/*
 * Settings Class
 *
 *      This class is used to pull various settings out of a settings
 * file. These settings can then be used to drive the various other
 * classes that need those settings to operate.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 09/09/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Settings.h"
#include "File.h"
#include "StringTokenizer.h"
#include <stdlib.h>

namespace IASLib
{
    CSettings   *g_Settings;

    IMPLEMENT_OBJECT( CSettings, CObject );

    CSettings::CSettings( void ) : m_hashSettings( "Normal" )
    {
        m_strSettingsFile = "./Settings.txt";
    }

    CSettings::~CSettings( void )
    {
        m_hashSettings.DeleteAll();
    }

        // Lines are "name = value"; blank lines and lines whose first
        // non-blank character is '#' or ';' are skipped. The value is
        // everything after the first '=', so it may hold '=' itself. A
        // later line replaces an earlier one with the same name.
        // (A missing file threw CFileException out of here; a value was
        // cut at its second '='; an indented comment was read as a
        // setting.)
    bool CSettings::ReadSettings( void )
    {
        CFile   *pFile = NULL;
        try
        {
            pFile = new CFile( m_strSettingsFile, CFile::READ );
        }
        catch ( ... )
        {
            delete pFile;
            return false;
        }

        if ( ! pFile->IsOpen() )
        {
            delete pFile;
            return false;
        }

        while ( ! pFile->IsEOF() )
        {
            CString strLine = pFile->GetLine();
            strLine.Trim();
            if ( strLine.GetLength() == 0 || strLine[0] == '#' || strLine[0] == ';' )
                continue;

            size_t nEquals = strLine.IndexOf( '=' );
            if ( nEquals == NOT_FOUND )
                continue;

            CString strName = strLine.Substring( 0, (int)nEquals );
            CString *pstrValue = new CString( strLine.Substring( nEquals + 1 ) );
            strName.Trim();
            pstrValue->Trim();
            if ( strName.GetLength() == 0 )
            {
                delete pstrValue;
                continue;
            }
            m_hashSettings.Push( strName, pstrValue, true );
        }

        pFile->Close();
        delete pFile;
        return true;
    }

    bool CSettings::HasSetting( const char *strSettingName ) const
    {
        return m_hashSettings.HasKey( strSettingName );
    }

    CString CSettings::GetSetting( const char *strSettingName, const char *strDefault )
    {
        CString     strSetting;
        CString    *pstrReturn;

        pstrReturn = (CString *)m_hashSettings.Get( strSettingName );
        if ( pstrReturn )
        {
            return *pstrReturn;
        }

        strSetting = strDefault;

        return strSetting;
    }

    int CSettings::GetIntSetting( const char *strSettingName, int nDefault )
    {
        CString     strSetting;
        CString    *pstrReturn;

        pstrReturn = (CString *)m_hashSettings.Get( strSettingName );
        if ( pstrReturn )
        {
            return atoi( *pstrReturn );
        }

        return nDefault;
    }
} // namespace IASLib
