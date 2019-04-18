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

    bool CSettings::ReadSettings( void )
    {
        CFile   oFile( m_strSettingsFile, CFile::READ );

        if ( oFile.IsOpen() )
        {
            CString     strLine;

            while ( ! oFile.IsEOF() )
            {
                strLine = oFile.GetLine();
                if ( strLine[0] != '#' )
                {
                    strLine.Trim();

                    if ( strLine.IndexOf( '=' ) != -1 )
                    {
                        CStringTokenizer stTokens( strLine, "=", false );
                        CString strName = stTokens.NextToken();
                        CString *pstrValue = new CString;
                        *pstrValue = stTokens.NextToken();
                        pstrValue->Trim();
                        strName.Trim();
                        m_hashSettings.Push( strName, pstrValue );
                    }
                }
            }

            oFile.Close();
            return true;
        }

        return false;
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
