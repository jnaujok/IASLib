/*
 * Profile Class
 *
 *      See Profile.h. (Rewritten: the constructor threw for a missing file,
 * the const char * read returned storage a later call could free, the
 * modified check was never reset so every read re-parsed the file, and
 * WriteInt() and ReadSection() were declared but never written.)
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/09/1994
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Profile.h"
#include "File.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

namespace IASLib
{
        // Allows for a global profile object.
    CProfile *g_Profile = NULL;

    IMPLEMENT_OBJECT( CProfile, CObject );

    namespace
    {
        bool SameName( const CString &strA, const char *pszB )
        {
            return strA.Compare( ( pszB ) ? pszB : "", false ) == 0;
        }

        std::string Trimmed( const std::string &str )
        {
            size_t nStart = str.find_first_not_of( " \t\r\n\f\v" );
            if ( nStart == std::string::npos )
                return std::string();
            size_t nEnd = str.find_last_not_of( " \t\r\n\f\v" );
            return str.substr( nStart, nEnd - nStart + 1 );
        }
    }

    CProfile::CProfile( const char *pszFileName, bool bCacheFile )
    {
        m_strFileName = ( pszFileName ) ? pszFileName : "";
        m_bCached = bCacheFile;
        m_bExistsFlag = false;
        m_bLoaded = false;
        m_llFileStamp = -1;
        m_llFileSize = -1;
        Load();
    }

    CProfile::~CProfile( void )
    {
    }

        // The file's modification time and size, to see whether it changed.
    bool CProfile::Stamp( long long &llTime, long long &llSize ) const
    {
    #ifdef IASLIB_WIN32__
        struct _stat64 oStat;
        if ( _stat64( (const char *)m_strFileName, &oStat ) != 0 )
            return false;
        llTime = (long long)oStat.st_mtime;
    #else
        struct stat oStat;
        if ( stat( (const char *)m_strFileName, &oStat ) != 0 )
            return false;
      #if defined( __APPLE__ )
        llTime = (long long)oStat.st_mtimespec.tv_sec * 1000000000LL + oStat.st_mtimespec.tv_nsec;
      #else
        llTime = (long long)oStat.st_mtim.tv_sec * 1000000000LL + oStat.st_mtim.tv_nsec;
      #endif
    #endif
        llSize = (long long)oStat.st_size;
        return true;
    }

    bool CProfile::Load( void )
    {
        m_aSections.clear();
        SSection oRoot;
        m_aSections.push_back( oRoot );
        m_bLoaded = true;

        long long llTime = -1, llSize = -1;
        m_bExistsFlag = Stamp( llTime, llSize );
        m_llFileStamp = llTime;
        m_llFileSize = llSize;
        if ( ! m_bExistsFlag )
            return false;

        std::ifstream oIn( (const char *)m_strFileName, std::ios::in | std::ios::binary );
        if ( ! oIn )
        {
            m_bExistsFlag = false;
            return false;
        }

        std::string strLine;
        while ( std::getline( oIn, strLine ) )
        {
            if ( ! strLine.empty() && strLine[ strLine.size() - 1 ] == '\r' )
                strLine.erase( strLine.size() - 1 );
            std::string strTrim = Trimmed( strLine );

            if ( strTrim.size() >= 2 && strTrim[0] == '[' && strTrim[ strTrim.size() - 1 ] == ']' )
            {
                SSection oSection;
                oSection.strName = Trimmed( strTrim.substr( 1, strTrim.size() - 2 ) ).c_str();
                oSection.strHeader = strLine.c_str();
                m_aSections.push_back( oSection );
                continue;
            }

            SLine oLine;
            size_t nEquals = strTrim.find( '=' );
            if ( strTrim.empty() || strTrim[0] == ';' || strTrim[0] == '#' || nEquals == std::string::npos || nEquals == 0 )
            {
                oLine.bEntry = false;
                oLine.strRaw = strLine.c_str();
            }
            else
            {
                oLine.bEntry = true;
                oLine.strName = Trimmed( strTrim.substr( 0, nEquals ) ).c_str();
                oLine.strValue = Trimmed( strTrim.substr( nEquals + 1 ) ).c_str();
                oLine.strIndent = strLine.substr( 0, strLine.find_first_not_of( " \t" ) ).c_str();
            }
            m_aSections.back().aLines.push_back( oLine );
        }
        return true;
    }

    void CProfile::Refresh( void )
    {
        if ( m_bLoaded && m_bCached )
            return;
        long long llTime = -1, llSize = -1;
        bool bExists = Stamp( llTime, llSize );
        if ( m_bLoaded && bExists == m_bExistsFlag && llTime == m_llFileStamp && llSize == m_llFileSize )
            return;
        Load();
    }

    void CProfile::Reload( void )
    {
        Load();
    }

    bool CProfile::Save( void )
    {
        if ( m_strFileName.GetLength() == 0 )
            return false;

        CString strTemp = m_strFileName + ".tmp";
        {
            std::ofstream oOut( (const char *)strTemp, std::ios::out | std::ios::binary | std::ios::trunc );
            if ( ! oOut )
                return false;
            for ( size_t nSection = 0; nSection < m_aSections.size(); nSection++ )
            {
                const SSection &oSection = m_aSections[ nSection ];
                if ( nSection > 0 )
                {
                    if ( oSection.strHeader.GetLength() )
                        oOut << (const char *)oSection.strHeader << "\n";
                    else
                        oOut << "[" << (const char *)oSection.strName << "]\n";
                }
                for ( size_t n = 0; n < oSection.aLines.size(); n++ )
                {
                    const SLine &oLine = oSection.aLines[ n ];
                    if ( oLine.bEntry )
                        oOut << (const char *)oLine.strIndent << (const char *)oLine.strName << " = " << (const char *)oLine.strValue << "\n";
                    else
                        oOut << (const char *)oLine.strRaw << "\n";
                }
            }
            oOut.flush();
            if ( ! oOut )
            {
                oOut.close();
                std::remove( (const char *)strTemp );
                return false;
            }
        }

        bool bRenamed = false;
        try
        {
            bRenamed = CFile::Rename( strTemp, m_strFileName );
        }
        catch ( ... )
        {
            bRenamed = false;
        }
        if ( ! bRenamed )
        {
            std::remove( (const char *)strTemp );
            return false;
        }

        long long llTime = -1, llSize = -1;
        m_bExistsFlag = Stamp( llTime, llSize );
        m_llFileStamp = llTime;
        m_llFileSize = llSize;
        return true;
    }

    CProfile::SSection *CProfile::FindSection( const char *pszSectionName )
    {
        if ( pszSectionName == NULL || *pszSectionName == '\0' )
            return &m_aSections[0];
        for ( size_t n = 1; n < m_aSections.size(); n++ )
        {
            if ( SameName( m_aSections[ n ].strName, pszSectionName ) )
                return &m_aSections[ n ];
        }
        return NULL;
    }

    CProfile::SLine *CProfile::FindEntry( const char *pszSectionName, const char *pszEntryName )
    {
        SSection *pSection = FindSection( pszSectionName );
        if ( pSection == NULL || pszEntryName == NULL )
            return NULL;
        for ( size_t n = 0; n < pSection->aLines.size(); n++ )
        {
            SLine &oLine = pSection->aLines[ n ];
            if ( oLine.bEntry && SameName( oLine.strName, pszEntryName ) )
                return &oLine;
        }
        return NULL;
    }

    size_t CProfile::GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, char *pszBuffer, int nBufferLen )
    {
        if ( pszBuffer == NULL || nBufferLen <= 0 )
            return 0;
        CString strValue;
        GetString( pszSectionName, pszEntryName, pszDefault, strValue );
        size_t nCopy = strValue.GetLength();
        if ( nCopy > (size_t)nBufferLen - 1 )
            nCopy = (size_t)nBufferLen - 1;
        memcpy( pszBuffer, (const char *)strValue, nCopy );
        pszBuffer[ nCopy ] = '\0';
        return nCopy;
    }

    size_t CProfile::GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault, CString &strBuffer )
    {
        Refresh();
        SLine *pLine = FindEntry( pszSectionName, pszEntryName );
        if ( pLine )
            strBuffer = pLine->strValue;
        else
            strBuffer = ( pszDefault ) ? pszDefault : "";
        return strBuffer.GetLength();
    }

    const char *CProfile::GetString( const char *pszSectionName, const char *pszEntryName, const char *pszDefault )
    {
        Refresh();
        SLine *pLine = FindEntry( pszSectionName, pszEntryName );
        if ( pLine == NULL )
            return pszDefault;
        std::string strKey = std::string( ( pszSectionName ) ? pszSectionName : "" ) + '\n' + ( ( pszEntryName ) ? pszEntryName : "" );
        CString &strKept = m_mapReturned[ strKey ];
        strKept = pLine->strValue;
        return (const char *)strKept;
    }

    int CProfile::GetInt( const char *pszSectionName, const char *pszEntryName, int nDefault )
    {
        Refresh();
        SLine *pLine = FindEntry( pszSectionName, pszEntryName );
        if ( pLine == NULL )
            return nDefault;
        const char *pszValue = pLine->strValue;
        char *pszEnd = NULL;
        long lValue = strtol( pszValue, &pszEnd, 0 );
        if ( pszEnd == pszValue )
            return nDefault;        // (atoi() gave 0 for text)
        return (int)lValue;
    }

    bool CProfile::WriteString( const char *pszSectionName, const char *pszEntryName, const char *pszBuffer )
    {
        if ( pszEntryName == NULL || *pszEntryName == '\0' )
            return false;
            // A name with '=', '[' or a line break, or a value with a
            // line break, would read back as something else.
        if ( strpbrk( pszEntryName, "=\r\n" ) || ( pszSectionName && strpbrk( pszSectionName, "[]\r\n" ) ) || ( pszBuffer && strpbrk( pszBuffer, "\r\n" ) ) )
            return false;

            // Start from the file as it is now, so changes made by others
            // since it was read are not lost.
        if ( ! m_bCached )
            Load();

        SLine *pLine = FindEntry( pszSectionName, pszEntryName );
        if ( pLine == NULL )
        {
            SSection *pSection = FindSection( pszSectionName );
            if ( pSection == NULL )
            {
                SSection oSection;
                oSection.strName = pszSectionName;
                m_aSections.push_back( oSection );
                pSection = &m_aSections.back();
            }
            SLine oLine;
            oLine.bEntry = true;
            oLine.strName = pszEntryName;
                // After the section's last entry, before its trailing
                // blank lines and comments.
            size_t nAt = pSection->aLines.size();
            while ( nAt > 0 && ! pSection->aLines[ nAt - 1 ].bEntry )
                nAt--;
                // No entries yet: straight after a section's header, or
                // after the root's opening comments.
            if ( nAt == 0 && pSection == &m_aSections[0] )
                nAt = pSection->aLines.size();
            pSection->aLines.insert( pSection->aLines.begin() + (long)nAt, oLine );
            pLine = &pSection->aLines[ nAt ];
        }
        pLine->strValue = ( pszBuffer ) ? pszBuffer : "";
        return Save();
    }

    bool CProfile::WriteInt( const char *pszSectionName, const char *pszEntryName, const int nValue )
    {
        char achValue[ 32 ];
        snprintf( achValue, sizeof( achValue ), "%d", nValue );
        return WriteString( pszSectionName, pszEntryName, achValue );
    }

    bool CProfile::DeleteEntry( const char *pszSectionName, const char *pszEntryName )
    {
        if ( ! m_bCached )
            Load();
        SSection *pSection = FindSection( pszSectionName );
        if ( pSection == NULL || pszEntryName == NULL )
            return false;
        for ( size_t n = 0; n < pSection->aLines.size(); n++ )
        {
            if ( pSection->aLines[ n ].bEntry && SameName( pSection->aLines[ n ].strName, pszEntryName ) )
            {
                pSection->aLines.erase( pSection->aLines.begin() + (long)n );
                return Save();
            }
        }
        return false;
    }

    bool CProfile::DeleteSection( const char *pszSectionName )
    {
        if ( pszSectionName == NULL || *pszSectionName == '\0' )
            return false;
        if ( ! m_bCached )
            Load();
        for ( size_t n = 1; n < m_aSections.size(); n++ )
        {
            if ( SameName( m_aSections[ n ].strName, pszSectionName ) )
            {
                m_aSections.erase( m_aSections.begin() + (long)n );
                return Save();
            }
        }
        return false;
    }

    bool CProfile::HasEntry( const char *pszSectionName, const char *pszEntryName )
    {
        Refresh();
        return FindEntry( pszSectionName, pszEntryName ) != NULL;
    }

    std::vector<CString> CProfile::GetSectionNames( void )
    {
        Refresh();
        std::vector<CString> aNames;
        for ( size_t n = 1; n < m_aSections.size(); n++ )
            aNames.push_back( m_aSections[ n ].strName );
        return aNames;
    }

    std::vector<CString> CProfile::GetEntryNames( const char *pszSectionName )
    {
        Refresh();
        std::vector<CString> aNames;
        SSection *pSection = FindSection( pszSectionName );
        if ( pSection )
        {
            for ( size_t n = 0; n < pSection->aLines.size(); n++ )
            {
                if ( pSection->aLines[ n ].bEntry )
                    aNames.push_back( pSection->aLines[ n ].strName );
            }
        }
        return aNames;
    }
} // namespace IASLib
