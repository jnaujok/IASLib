/**
 * @file DirectoryEntry.h
 * @brief CDirectoryEntry, one entry of a CDirectory listing.
 *
 * The original header comment follows.
 */
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

#ifndef IASLIB_DIRECTORYENTRY_H__
#define IASLIB_DIRECTORYENTRY_H__

#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
    /// One entry of a directory listing (CDirectory), with its size, date and type.
    ///
    /// The fields are read once, when the entry is made. If the entry's
    /// status cannot be read, the size and date are left unset.
    class CDirectoryEntry : public CObject
    {
        public:
            CString     m_strFileName;      ///< The entry's name, without the directory.
            CString     m_strEntryName;     ///< The full path: directory, "/", name.
            CString     m_strExtension;     ///< For a file, what follows the last "." in the name; otherwise "".
            CDate       m_dttLastModified;  ///< When it was last modified (the current time if lightweight).
            long        m_lSize;            ///< A file's size in bytes (0 for a directory, or if lightweight).
            bool        m_bIsDir;           ///< Whether it is a directory (always false if lightweight).

                /// Reads an entry's details.
                /// @param strDir  The directory it is in.
                /// @param strFileName  The entry's name.
                /// @param bLightweight  Skip reading the status: the size is 0,
                ///                      the date is now and it counts as a file.
                        CDirectoryEntry( const char *strDir, const char *strFileName, bool bLightweight = false );
                /// Destroys the entry.
            virtual    ~CDirectoryEntry( void );

                        DEFINE_OBJECT( CDirectoryEntry )
    };
} // namespace IASLib

#endif // __DIRECTORYENTRY_H__
