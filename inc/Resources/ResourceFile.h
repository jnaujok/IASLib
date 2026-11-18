/**
 *  @file
 *  Resource File
 *
 *      An archive of resources (CResource) in one file, for a game or
 * another program with many resources to load. Resources are found by
 * their folder and resource tags, or by name.
 *
 *      The file is a header, the resources' data, and an index at the
 * end, written with CFile and the streams' big-endian numbers; the data
 * and the index each carry a SHA-256 digest (CSHA256). Writing never
 * overwrites anything the header points to: new data goes after the end of
 * the file, and Flush() writes a new index after it and then the header,
 * so a crash part-way leaves the archive as it was at the last Flush().
 * Data that has been replaced or removed, and old indexes, are wasted
 * space until Compact() rewrites the file. Offsets are 64-bit (CFile's
 * Big methods), so an archive can be any size.
 *
 *      With auto-flush (the default) every Put() and Remove() is flushed.
 * Read resources are kept in a TCache, least recently used first out, up
 * to a limit in bytes. Thread-safe.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RESOURCEFILE_H__
#define IASLIB_RESOURCEFILE_H__

#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../Files/File.h"
#include "../Templates/TCache.h"
#include "../Threading/Mutex.h"
#include "Resource.h"
#include "ResourceIndex.h"
#include "ResourceDirectory.h"

namespace IASLib
{
        /// The archive's cache: a TCache that also keeps to a byte limit.
        ///
        /// Keyed by resource ID. A resource larger than the limit is not
        /// kept. Used under CResourceFile's lock; not thread-safe alone.
    class CResourceCache : public TCache< uint32_t, std::shared_ptr<const CResource> >
    {
        protected:
            size_t          m_nBytes;       ///< Bytes of data held.
            size_t          m_nByteLimit;   ///< The most bytes to hold.

        public:
                            /// Creates an empty cache.
                            /// @param nByteLimit The most bytes of data to hold.
                            CResourceCache( size_t nByteLimit );

                /// Adds (or replaces) a resource, then drops the oldest until
                /// within the limit.
                /// @param pResource The resource (not NULL).
            void            Add( const std::shared_ptr<const CResource> &pResource );
                /// Remove() that keeps the byte count.
                /// @param nID The resource's ID.
            void            Erase( uint32_t nID );
                /// Clear() that keeps the byte count.
            void            EraseAll( void );
                /// Sets the byte limit, dropping the oldest resources to keep to it.
            void            SetByteLimit( size_t nBytes );
            /// Bytes of data held.
            size_t          GetBytes( void ) const { return m_nBytes; }

        private:
            /// Drops the oldest resources until within the byte limit.
            void            Trim( void );
    };

        /// An archive of resources in one file.
        ///
        /// See the file description for the format and its crash safety.
        /// Failures return false (or NULL) and set GetLastError(); nothing
        /// throws. Thread-safe (one mutex). Not copyable.
        ///
        /// @code
        /// CResourceFile archive( "game.res" );        // READ_WRITE: created if missing
        /// if ( ! archive.IsOpen() )
        ///     printf( "%s\n", (const char *)archive.GetLastError() );
        /// CResource oLevel( 2, 1, "level1", "tools", 0, "levels/1.dat" );
        /// archive.Put( oLevel );
        /// std::shared_ptr<const CResource> p = archive.Get( 2, 1 );
        /// @endcode
    class CResourceFile : public CObject
    {
        public:
            /// How Open() opens the file.
            enum OpenMode
            {
                READ_ONLY,      ///< Read only; Put(), Remove(), Flush() and Compact() fail.
                READ_WRITE,     ///< Read and write; creates the file if there is none.
                CREATE          ///< Read and write; replaces any file there with an empty archive.
            };

            static const uint32_t FORMAT_VERSION = 1;   ///< The archive format this library writes (and the newest it reads).
            static const size_t HEADER_SIZE = 32;       ///< Bytes in the file's header.

        protected:
            CFile          *m_pFile;            ///< The open file (owned; NULL: closed).
            CString         m_strPath;          ///< The file's path.
            OpenMode        m_nMode;            ///< How it was opened.
            CString         m_strLastError;     ///< Why the last call failed.
            std::map<uint32_t, CResourceIndexEntry> m_mapIndex; ///< The index, by ID.
            uint64_t        m_nAppendAt;        ///< Where new data goes.
            uint64_t        m_nIndexLength;     ///< Length of the index the header points to.
            uint64_t        m_nWasted;          ///< Bytes Compact() would free.
            bool            m_bDirty;           ///< True: the index changed since the last Flush().
            bool            m_bAutoFlush;       ///< True: Put() and Remove() flush.
            CResourceCache  m_cache;            ///< Resources read or written lately.
            mutable CMutex  m_mutex;            ///< Guards everything above.

        public:
                            /// Creates a closed archive (cache limit 16 MB).
                            CResourceFile( void );
                            /// Opens strPath (see Open(); check IsOpen()).
                            CResourceFile( const char *strPath, OpenMode nMode = READ_WRITE );
                            /// Flushes and closes.
            virtual        ~CResourceFile( void );

                            DEFINE_OBJECT( CResourceFile )

                /// Opens an archive, closing any open one first.
                /// A new or replaced file is written as an empty archive at once.
                /// @param strPath The file.
                /// @param nMode   How to open it.
                /// @return False, with GetLastError() saying why, if the file cannot be
                ///         opened or is not a resource archive (or is damaged).
            bool            Open( const char *strPath, OpenMode nMode = READ_WRITE );
                /// Flushes and closes (true if it was not open).
                /// @return False if the flush failed; the file is closed anyway.
            bool            Close( void );
            /// True if an archive is open.
            bool            IsOpen( void ) const;
            /// True if it was opened READ_ONLY.
            bool            IsReadOnly( void ) const { return m_nMode == READ_ONLY; }
            /// Why the last call failed.
            CString         GetLastError( void ) const;
            /// The archive's path.
            CString         GetPath( void ) const;

            /// How many resources the archive has.
            size_t          GetCount( void ) const;
            /// True if there is a resource with these tags.
            bool            Exists( short nFolder, short nResource ) const;
                /// The tags of the first resource (in ID order) with this name.
                /// @param strName   The name.
                /// @param nFolder   Set to its folder tag.
                /// @param nResource Set to its resource tag.
                /// @return False if there is none.
            bool            FindByName( const char *strName, short &nFolder, short &nResource ) const;
                /// A copy of a resource's index entry.
                /// @return False if there is none (oEntry is then unchanged).
            bool            GetEntry( short nFolder, short nResource, CResourceIndexEntry &oEntry ) const;
                /// Every entry, in ID order.
            std::vector<CResourceIndexEntry> GetEntries( void ) const;
                /// The folder tags in use, in ID order (each once).
            std::vector<short> GetFolders( void ) const;
                /// A snapshot of one folder and its resources.
            CResourceDirectoryNode GetDirectory( short nFolder ) const;

                /// The resource (NULL if there is none, or its data is damaged).
                /// From the cache if it is there; otherwise read, checked
                /// against its digest, and cached. A compressed resource is
                /// refused. The pointer stays valid after the archive changes.
                /// @param nFolder   The folder tag.
                /// @param nResource The resource tag.
            std::shared_ptr<const CResource> Get( short nFolder, short nResource );
                /// A copy of it.
                /// @copydetails Get()
                /// @param oResource Set to the resource.
                /// @return False if there is none, or it cannot be read.
            bool            Read( short nFolder, short nResource, CResource &oResource );
                /// Adds the resource, or replaces the one with its tags.
                /// The data goes after the end of the file; the old data, if
                /// any, becomes wasted space. Flushed with auto-flush.
                /// @return False if the archive is closed or read-only, or the write failed.
            bool            Put( const CResource &oResource );
                /// Removes a resource (its data becomes wasted space). Flushed with auto-flush.
                /// @return False if there is none, or the archive is closed or read-only.
            bool            Remove( short nFolder, short nResource );

                /// Writes the index and header if anything changed.
                /// @return False if the archive is closed, read-only with
                ///         changes, or the write failed.
            bool            Flush( void );
                /// Turns flushing after each Put() and Remove() on or off.
            void            SetAutoFlush( bool bAutoFlush );
                /// Rewrites the file without the wasted space.
                /// Writes a new file beside it ("&lt;path&gt;.compact") and renames
                /// it over the archive; the archive is unchanged if that fails.
                /// @return False if it failed (see GetLastError()).
            bool            Compact( void );
                /// Bytes that Compact() would free.
            uint64_t        GetWastedBytes( void ) const;

            /// Sets the cache's byte limit (dropping the oldest resources to keep to it).
            void            SetCacheLimit( size_t nBytes );
            /// Bytes of data in the cache.
            size_t          GetCacheBytes( void ) const;
            /// Resources in the cache.
            size_t          GetCachedCount( void ) const;
            /// Lookups the cache answered.
            uint64_t        GetCacheHits( void ) const;
            /// Lookups the cache could not answer.
            uint64_t        GetCacheMisses( void ) const;

        private:
            /// Sets the last error (printf-style) and returns false.
            bool            Fail( const char *strFormat, ... );
            /// Reads and checks the header and index of the open file.
            bool            ReadIndex( void );
            /// Writes an index at nIndexOffset, then the header that points to it.
            bool            WriteIndexAndHeader( CFile &oFile, const std::map<uint32_t, CResourceIndexEntry> &mapIndex,
                                                 uint64_t nIndexOffset, uint64_t &nIndexLength );
            /// Closes the file without flushing and forgets the index and cache.
            void            CloseFile( void );

                            /// Not copyable.
                            CResourceFile( const CResourceFile & );
                            /// Not assignable.
            CResourceFile  &operator =( const CResourceFile & );
    };
} // namespace IASLib
#endif // IASLIB_RESOURCEFILE_H__
