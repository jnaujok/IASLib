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

#ifndef IASLIB_DIRECTORY_H__
#define IASLIB_DIRECTORY_H__

#include "../BaseTypes/String_.h"
#include "../Collections/Array.h"
#include "DirectoryEntry.h"

namespace IASLib
{
        // The entries of a directory whose names match a wildcard filter
        // ("*", "*.log"), sorted by name. "." and ".." are included when
        // the filter matches them. A directory that cannot be read gives no
        // entries, and IsValid() is false.
    class CDirectory : public CObject
    {
        protected:
            CArray              m_aDirEntries;
            CString             m_strDirPath;
            CString             m_strFilter;
            bool                m_bLightweight;
            bool                m_bValid;

        public:
                                // bLightweight: names only, without each
                                // entry's size, time and kind.
                                CDirectory( const char *strDirPath, const char *strFilter = "*", bool bLightweight = false );
            virtual            ~CDirectory( void );

                                DEFINE_OBJECT( CDirectory )

            size_t              GetEntryCount( void );
            const char         *GetFileName( size_t nIndex );
            CDirectoryEntry    *GetEntry( size_t nIndex );
            bool                IsValid( void ) const { return m_bValid; }
            const CString      &GetPath( void ) const { return m_strDirPath; }

                // Reads the directory again.
            void                Refresh( void );

                // Makes a directory (and, with bParents, any missing parents).
                // True if it exists afterwards.
            static bool         Create( const char *strDirPath, bool bParents = false );
                // Removes an empty directory.
            static bool         Remove( const char *strDirPath );
            static bool         Exists( const char *strDirPath );

        private:
                                CDirectory( const CDirectory & );
            CDirectory         &operator =( const CDirectory & );
    };
} // namespace IASLib

#endif // IASLIB_DIRECTORY_H__
