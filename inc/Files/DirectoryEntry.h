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
    class CDirectoryEntry : public CObject
    {
        public:
            CString     m_strFileName;
            CString     m_strEntryName;
            CString     m_strExtension;
            CDate       m_dttLastModified;
            long        m_lSize;
            bool        m_bIsDir;

                        CDirectoryEntry( const char *strDir, const char *strFileName, bool bLightweight = false );
            virtual    ~CDirectoryEntry( void );

                        DEFINE_OBJECT( CDirectoryEntry )
    };
} // namespace IASLib

#endif // __DIRECTORYENTRY_H__
