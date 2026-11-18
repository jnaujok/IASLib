/**
 *  @file
 *  Resource Sources
 *
 *      Where CResourceManager finds a resource's bytes, by path
 * ("textures/hero.png"):
 *
 *      CArchiveSource      a CResourceFile archive, by resource name
 *      CDirectorySource    loose files under a directory (during
 *                          development, or for patches and mods)
 *
 *      A program can add its own (a network source, a compressed pack).
 * Sources must be safe to call from several threads at once: the manager's
 * background loaders do.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RESOURCESOURCE_H__
#define IASLIB_RESOURCESOURCE_H__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "../BaseTypes/String_.h"
#include "../BaseTypes/DataBlock.h"
#include "../Threading/Mutex.h"
#include "ResourceFile.h"

namespace IASLib
{
        /// Where CResourceManager finds a resource's bytes: the interface.
        ///
        /// Implementations must be thread-safe: the manager calls them from
        /// its worker threads, without its own lock.
    class CResourceSource
    {
        public:
            /// Destroys the source.
            virtual            ~CResourceSource( void ) {}

                /// A name for messages.
            virtual CString     GetName( void ) const = 0;
                /// Whether the source has this path (normalised: '/'
                /// separators, no leading '/', no "." or "..").
            virtual bool        Exists( const CString &strPath ) = 0;
                /// Reads the bytes of a path.
                /// @param strPath  The path (normalised, as for Exists()).
                /// @param oData    Set to the bytes.
                /// @param strError Set to why, on failure.
                /// @return False if they cannot be read.
            virtual bool        Read( const CString &strPath, CDataBlock &oData, CString &strError ) = 0;
    };

        /// Resources by name from an archive.
        ///
        /// A path is a resource's name in the archive; if several share a
        /// name, the one with the lowest ID wins. The manager does the
        /// caching, so the archive's own cache is turned off unless
        /// bKeepArchiveCache. The names are read when the source is made:
        /// call Refresh() after changing the archive.
    class CArchiveSource : public CResourceSource
    {
        protected:
            std::shared_ptr<CResourceFile>      m_pArchive;     ///< The archive (shared).
            std::map<std::string, uint32_t>     m_mapNames;     ///< Name to resource ID.
            mutable CMutex                      m_mutex;        ///< Guards m_mapNames.

        public:
                                /// Creates a source over an open archive and reads its names.
                                /// @param pArchive          The archive (may be NULL: the source is then empty).
                                /// @param bKeepArchiveCache False to set the archive's cache limit to 0.
                                CArchiveSource( const std::shared_ptr<CResourceFile> &pArchive, bool bKeepArchiveCache = false );

            /// "archive " and the archive's path.
            virtual CString     GetName( void ) const;
            /// Whether a resource has this name.
            virtual bool        Exists( const CString &strPath );
            /// Reads the named resource's data.
            /// @copydetails CResourceSource::Read()
            virtual bool        Read( const CString &strPath, CDataBlock &oData, CString &strError );

                /// Rereads the archive's names (after resources are added to
                /// or removed from it).
            void                Refresh( void );
            /// The archive.
            std::shared_ptr<CResourceFile> GetArchive( void ) const { return m_pArchive; }
    };

        /// Files under a directory: "textures/hero.png" is
        /// &lt;root&gt;/textures/hero.png.
    class CDirectorySource : public CResourceSource
    {
        protected:
            CString             m_strRoot;      ///< The directory.

        public:
                                /// Creates a source over a directory.
                                /// @param strRoot The directory (NULL or "": the current one).
                                CDirectorySource( const char *strRoot );

            /// "directory " and the root.
            virtual CString     GetName( void ) const;
            /// Whether the file exists and can be opened for reading.
            virtual bool        Exists( const CString &strPath );
            /// Reads the whole file.
            /// @copydetails CResourceSource::Read()
            virtual bool        Read( const CString &strPath, CDataBlock &oData, CString &strError );

            /// The file's path: the root, a '/' if it needs one, then strPath.
            CString             FullPath( const CString &strPath ) const;
    };
} // namespace IASLib

#endif // IASLIB_RESOURCESOURCE_H__
