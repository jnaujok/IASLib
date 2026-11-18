/**
 * @file Directory.h
 * @brief CDirectory, a listing of a directory's entries that match a filter.
 *
 * The original header comment follows.
 */
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
#ifdef IASLIB_MULTI_THREADED__
#include "../Threading/Mutex.h"
#endif

namespace IASLib
{
    /// A listing of the entries in a directory whose names match a wildcard filter.
    ///
    /// The listing is read when the object is created, and again by
    /// Refresh(). It includes "." and ".." when they match. The entries
    /// belong to the directory object. Reading directories is serialized
    /// across threads by a shared lock.
    ///
    /// @code
    /// CDirectory oDir( "/var/log", "*.log" );
    /// for ( size_t n = 0; n < oDir.GetEntryCount(); n++ )
    ///     printf( "%s %ld\n", oDir.GetFileName( n ), oDir.GetEntry( n )->m_lSize );
    /// @endcode
    class CDirectory : public CObject
    {
        protected:
            CArray              m_aDirEntries;      ///< The CDirectoryEntry objects (owned).
            CString             m_strDirPath;       ///< The directory's path.
            CString             m_strFilter;        ///< The wildcard filter.
            bool                m_bLightweight;     ///< Skip reading each entry's status.
#ifdef IASLIB_MULTI_THREADED__
            static CMutex       m_mutexThreadSafe;  ///< Serializes directory reads across threads.
#endif
        public:
                /// Reads the directory.
                /// @param strDirPath  The directory's path; it must exist.
                /// @param strFilter  A wildcard filter for the names ("*" for all).
                /// @param bLightweight  Do not read each entry's size, date and
                ///                      type (see CDirectoryEntry).
                /// @note bLightweight is used on Windows only; elsewhere every
                ///       entry's status is read.
                                CDirectory( const char *strDirPath, const char *strFilter = "*", bool bLightweight = false );
                /// Deletes the entries.
            virtual            ~CDirectory( void );

                                DEFINE_OBJECT( CDirectory )

                /// How many entries matched.
            size_t              GetEntryCount( void );
                /// An entry's name (without the directory).
                /// @param nIndex  From 0 to GetEntryCount() - 1.
                /// @return The name, or NULL if the index is out of range.
            const char         *GetFileName( size_t nIndex );
                /// An entry.
                /// @param nIndex  From 0 to GetEntryCount() - 1.
                /// @return The entry, which the directory still owns, or NULL
                ///         if the index is out of range.
            CDirectoryEntry    *GetEntry( size_t nIndex );

                /// Reads the directory again, replacing (and deleting) the old entries.
            void                Refresh( void );
    };
} // namespace IASLib
#endif // __DIRECTORY_H__
