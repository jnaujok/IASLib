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
#include <sys/types.h>
#ifndef IASLIB_WIN32__
#include <dirent.h>
#endif

namespace IASLib
{
#ifdef IASLIB_MULTI_THREADED__
    CMutex  CDirectory::m_mutexThreadSafe;
#endif

    IMPLEMENT_OBJECT( CDirectory, CObject );

    CDirectory::CDirectory( const char *strDirPath, const char *strFilter, bool bLightweight )
    {
        m_strFilter = strFilter;
        m_strDirPath = strDirPath;
        m_bLightweight = bLightweight;

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
        if ( nIndex >= m_aDirEntries.Length() )
        {
            return NULL;
        }

        CDirectoryEntry *pEntry = GetEntry( nIndex );
        return (const char *)(pEntry->m_strFileName);
    }

    CDirectoryEntry *CDirectory::GetEntry( size_t nIndex )
    {
        if ( nIndex >= m_aDirEntries.Length() )
        {
            return NULL;
        }

        return (CDirectoryEntry *)m_aDirEntries[ nIndex ];
    }

    void CDirectory::Refresh( void )
    {
    #ifndef IASLIB_WIN32__
        DIR *pDir;
        struct dirent *pStore = (struct dirent *)malloc( sizeof(struct dirent) + 1024 );
        struct dirent *pEntry;

        m_aDirEntries.DeleteAll();

#ifdef IASLIB_MULTI_THREADED__
        m_mutexThreadSafe.Lock();
#endif
        pDir = opendir( m_strDirPath );

    #ifdef IASLIB_SUN__
        while ( ( pEntry = readdir_r( pDir, pStore ) ) != NULL )
    #else
        while ( ( pEntry = readdir( pDir ) ) != NULL )
    #endif
        {
            CString strTemp = pEntry->d_name;
            if ( strTemp.WildcardCompare( m_strFilter ) )
            {
                CDirectoryEntry *pNewEntry;
                try
                {
                    pNewEntry = new CDirectoryEntry( m_strDirPath, pEntry->d_name );
                    m_aDirEntries.Push( pNewEntry );
                }
                catch (CFileException &oException)
                {
                    //ErrorLog( "CacheDir (Init): Exception thrown during new DirEntry [%s]\n", (const char *)oException );
                }
            }
        }
        closedir( pDir );
        free( pStore );
#ifdef IASLIB_MULTI_THREADED__
        m_mutexThreadSafe.Unlock();
#endif
    #else // ifndef IASLIB_WIN32__
        HANDLE hDir;

        LPWIN32_FIND_DATA pStore = (LPWIN32_FIND_DATA)new char[( sizeof(WIN32_FIND_DATA) ) + 1024];

        m_aDirEntries.DeleteAll();

#ifdef IASLIB_MULTI_THREADED__
        m_mutexThreadSafe.Lock();
#endif
        CString strTempDir = m_strDirPath + "/*";
        hDir = FindFirstFile( strTempDir, pStore );

        if ( hDir != INVALID_HANDLE_VALUE )
        {
            do
            {
                CString strTemp = pStore->cFileName;
                if ( strTemp.WildcardCompare( m_strFilter ) )
                {
                    CDirectoryEntry *pNewEntry;
                    try
                    {
                        pNewEntry = new CDirectoryEntry( m_strDirPath, pStore->cFileName, m_bLightweight );
                        m_aDirEntries.Push( pNewEntry );
                    }
                    catch (CFileException &oException)
                    {
                            // The following line is to avoid complaints.
                        oException.GetID();
                    }
                }
            } while ( FindNextFile( hDir, pStore ) );

            FindClose( hDir );
        }
        delete pStore;
#ifdef IASLIB_MULTI_THREADED__
        m_mutexThreadSafe.Unlock();
#endif
    #endif
    }
} // namespace IASLib
