/*
 *  Directory Class
 *
 *      This class encapsulates a directory listing with a filter.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/19/2002
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Directory.h"
#include "FileException.h"
#include "FileStat.h"

#include <errno.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef IASLIB_WIN32__
    #include "Platform.h"
    #include <direct.h>
#else
    #include <dirent.h>
    #include <unistd.h>
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CDirectory, CObject );

    CDirectory::CDirectory( const char *strDirPath, const char *strFilter, bool bLightweight )
    {
        m_strFilter = ( strFilter && *strFilter ) ? strFilter : "*";
        m_strDirPath = ( strDirPath && *strDirPath ) ? strDirPath : ".";
        m_bLightweight = bLightweight;
        m_bValid = false;

        Refresh();
    }

    CDirectory::~CDirectory( void )
    {
        m_aDirEntries.DeleteAll();
    }

    size_t CDirectory::GetEntryCount( void )
    {
        return m_aDirEntries.Length();
    }

    const char *CDirectory::GetFileName( size_t nIndex )
    {
        CDirectoryEntry *pEntry = GetEntry( nIndex );
        return ( pEntry ) ? (const char *)( pEntry->m_strFileName ) : NULL;
    }

    CDirectoryEntry *CDirectory::GetEntry( size_t nIndex )
    {
        if ( nIndex >= m_aDirEntries.Length() )
        {
            return NULL;
        }
        return (CDirectoryEntry *)m_aDirEntries[ nIndex ];
    }

        // (A directory that could not be opened crashed: readdir() was given
        // NULL. On Windows the TCHAR calls broke UNICODE builds, and a new[]
        // buffer was freed with delete. Lightweight was ignored elsewhere.)
    void CDirectory::Refresh( void )
    {
        m_aDirEntries.DeleteAll();
        m_bValid = false;

        std::vector<std::string> aNames;
    #ifndef IASLIB_WIN32__
        DIR *pDir = opendir( m_strDirPath );
        if ( pDir == NULL )
            return;
        struct dirent *pEntry;
        while ( ( pEntry = readdir( pDir ) ) != NULL )      // its own stream: thread-safe
        {
            aNames.push_back( pEntry->d_name );
        }
        closedir( pDir );
    #else
        WIN32_FIND_DATAA findData;
        CString strPattern = m_strDirPath + "/*";
        HANDLE hFind = FindFirstFileA( strPattern, &findData );
        if ( hFind == INVALID_HANDLE_VALUE )
            return;
        do
        {
            aNames.push_back( findData.cFileName );
        } while ( FindNextFileA( hFind, &findData ) );
        FindClose( hFind );
    #endif
        m_bValid = true;

            // The same order everywhere (the system's order is arbitrary).
        std::sort( aNames.begin(), aNames.end() );
        for ( size_t n = 0; n < aNames.size(); n++ )
        {
            CString strName( aNames[n].c_str() );
            if ( ! strName.WildcardCompare( m_strFilter ) )
                continue;
            m_aDirEntries.Push( new CDirectoryEntry( m_strDirPath, aNames[n].c_str(), m_bLightweight ) );
        }
    }

    bool CDirectory::Exists( const char *strDirPath )
    {
        if ( strDirPath == NULL || *strDirPath == '\0' )
            return false;
        try
        {
            CFileStat oStat( strDirPath );
            return oStat.IsDir();
        }
        catch ( CFileException & )
        {
            return false;
        }
    }

    bool CDirectory::Create( const char *strDirPath, bool bParents )
    {
        if ( strDirPath == NULL || *strDirPath == '\0' )
            return false;
        if ( Exists( strDirPath ) )
            return true;

        if ( bParents )
        {
            std::string strPath( strDirPath );
            size_t nSlash = strPath.find_last_of( "/\\" );
            while ( nSlash != std::string::npos && nSlash + 1 == strPath.size() )
            {
                strPath.erase( nSlash );                        // trailing separators
                nSlash = strPath.find_last_of( "/\\" );
            }
            if ( nSlash != std::string::npos && nSlash > 0 )
            {
                std::string strParent = strPath.substr( 0, nSlash );
                if ( ! ( strParent.size() == 2 && strParent[1] == ':' ) )     // not "C:"
                    Create( strParent.c_str(), true );
            }
        }

    #ifdef IASLIB_WIN32__
        int nResult = _mkdir( strDirPath );
    #else
        int nResult = mkdir( strDirPath, 0777 );
    #endif
        return nResult == 0 || Exists( strDirPath );
    }

    bool CDirectory::Remove( const char *strDirPath )
    {
        if ( strDirPath == NULL || *strDirPath == '\0' )
            return false;
    #ifdef IASLIB_WIN32__
        return _rmdir( strDirPath ) == 0;
    #else
        return rmdir( strDirPath ) == 0;
    #endif
    }
} // namespace IASLib
