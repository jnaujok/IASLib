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

#include <stdint.h>
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"

namespace IASLib
{
        // One name in a directory. Unless lightweight, its size, time and
        // kind come from CFileStat (left as 0, now and "not a directory" if
        // that fails).
    class CDirectoryEntry : public CObject
    {
        public:
            CString     m_strFileName;      // the name alone
            CString     m_strEntryName;     // <directory>/<name>
            CString     m_strExtension;     // after the last '.', for files
            CDate       m_dttLastModified;
            int64_t     m_lSize;            // (a long: wrong past 2 GB on Windows)
            bool        m_bIsDir;

                        CDirectoryEntry( const char *strDir, const char *strFileName, bool bLightweight = false );
            virtual    ~CDirectoryEntry( void );

                        DEFINE_OBJECT( CDirectoryEntry )

            bool        IsDirectory( void ) const { return m_bIsDir; }
                // "." and "..".
            bool        IsDotEntry( void ) const { return m_strFileName == "." || m_strFileName == ".."; }
    };
} // namespace IASLib

#endif // IASLIB_DIRECTORYENTRY_H__
