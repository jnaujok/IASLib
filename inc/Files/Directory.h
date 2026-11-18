/**
 * @file
 * @brief Directory Class.
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
        /// The entries of a directory whose names match a wildcard filter
        /// ("*", "*.log"), sorted by name. "." and ".." are included when
        /// the filter matches them. A directory that cannot be read gives no
        /// entries, and IsValid() is false.
        ///
        /// The CDirectory owns its CDirectoryEntry objects; they are deleted
        /// by Refresh() and the destructor. It cannot be copied.
        ///
        /// @code
        /// CDirectory oDir( "logs", "*.log" );
        /// for ( size_t n = 0; n < oDir.GetEntryCount(); n++ )
        ///     printf( "%s\n", oDir.GetFileName( n ) );
        /// @endcode
    class CDirectory : public CObject
    {
        protected:
            CArray              m_aDirEntries;      ///< The CDirectoryEntry objects (owned).
            CString             m_strDirPath;       ///< The directory's path.
            CString             m_strFilter;        ///< The wildcard filter.
            bool                m_bLightweight;     ///< Names only, without size, time and kind.
            bool                m_bValid;           ///< Whether the directory could be read.

        public:
                                /// Reads a directory.
                                /// @param strDirPath The directory ("." if NULL or empty).
                                /// @param strFilter A WildcardCompare() pattern ("*" if NULL or empty).
                                /// @param bLightweight Names only, without each
                                ///        entry's size, time and kind.
                                CDirectory( const char *strDirPath, const char *strFilter = "*", bool bLightweight = false );
                                /// Destructor. Deletes the entries.
            virtual            ~CDirectory( void );

                                DEFINE_OBJECT( CDirectory )

            size_t              GetEntryCount( void );                  ///< The number of entries.
                                /// The name of entry @p nIndex, or NULL if there is none.
            const char         *GetFileName( size_t nIndex );
                                /// Entry @p nIndex (still owned by the directory), or NULL.
            CDirectoryEntry    *GetEntry( size_t nIndex );
            bool                IsValid( void ) const { return m_bValid; }      ///< Whether the directory could be read.
            const CString      &GetPath( void ) const { return m_strDirPath; }  ///< The directory's path.

                /// Reads the directory again (deleting the old entries).
                /// @note A directory that could not be opened used to crash.
            void                Refresh( void );

                /// Makes a directory (and, with bParents, any missing parents).
                /// True if it exists afterwards.
            static bool         Create( const char *strDirPath, bool bParents = false );
                /// Removes an empty directory. True if it worked.
            static bool         Remove( const char *strDirPath );
                /// Whether @p strDirPath is an existing directory.
            static bool         Exists( const char *strDirPath );

        private:
                                CDirectory( const CDirectory & );       ///< Not copyable.
            CDirectory         &operator =( const CDirectory & );       ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_DIRECTORY_H__
