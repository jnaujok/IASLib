/*
 *  Directory Entry Class
 *
 *      This class provides storage for individual directory entries
 * when a directory is loaded.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/19/2002
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "DirectoryEntry.h"
#include "FileException.h"
#include "FileStat.h"

#ifndef WIN32
#include <dirent.h>
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CDirectoryEntry, CObject );

    CDirectoryEntry::CDirectoryEntry( const char *strDir, const char *strFileName, bool bLightweight )
    {
        m_bIsDir = false;
        m_strFileName = strFileName;
        m_strEntryName = strDir;
        m_strEntryName += "/";
        m_strEntryName += strFileName;

        m_strExtension = "";

        if ( ! bLightweight )
        {
            try
            {
                CFileStat  statFile( m_strEntryName );
                
                m_lSize = statFile.GetSize();
                m_dttLastModified = statFile.GetLastModified();    

                if ( statFile.IsFile() )
                {
                    m_bIsDir = false;
                    if ( m_strFileName.LastIndexOf( '.' ) != IASLib::NOT_FOUND )
                    {
                        m_strExtension = m_strFileName.Substring( m_strFileName.LastIndexOf('.') + (size_t)1 );
                    }
                }
                else 
                {
                    if ( statFile.IsDir() )
                    {
                        m_bIsDir = true;
                        m_strExtension = "";
                    }
                }
            }
            catch ( CFileException &feException )
            {
                    // The following line is to avoid compiler complaints.
                feException.GetID();
            }
        }
        else
        {
            m_lSize = 0;
            m_dttLastModified.SetToCurrent();
            m_bIsDir = false;
            m_strExtension = "";
        }
    }

    CDirectoryEntry::~CDirectoryEntry( void )
    {
    }
} // namespace IASLib
