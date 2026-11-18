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

namespace IASLib
{
    IMPLEMENT_OBJECT( CDirectoryEntry, CObject );

    CDirectoryEntry::CDirectoryEntry( const char *strDir, const char *strFileName, bool bLightweight )
    {
        m_bIsDir = false;
        m_lSize = 0;                    // (was left unset when stat failed)
        m_strFileName = ( strFileName ) ? strFileName : "";
        m_strEntryName = ( strDir ) ? strDir : ".";
        m_strEntryName += "/";
        m_strEntryName += m_strFileName;
        m_strExtension = "";
        m_dttLastModified.SetToCurrent();

        if ( ! bLightweight )
        {
            try
            {
                CFileStat statFile( m_strEntryName );
                m_lSize = statFile.GetSizeBig();
                m_dttLastModified = statFile.GetLastModified();
                m_bIsDir = statFile.IsDir();
            }
            catch ( CFileException & )
            {
                    // Gone since the listing, or not ours to look at.
            }
        }

        if ( ! m_bIsDir )
        {
            size_t nDot = m_strFileName.LastIndexOf( '.' );
            if ( nDot != IASLib::NOT_FOUND && nDot > 0 )
                m_strExtension = m_strFileName.Substring( nDot + (size_t)1 );
        }
    }

    CDirectoryEntry::~CDirectoryEntry( void )
    {
    }
} // namespace IASLib
