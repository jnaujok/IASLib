/**
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
 * space until Compact() rewrites the file. (CFile seeks with a long, so on
 * Windows an archive is limited to 2 GB.)
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
        // The archive's cache: a TCache that also keeps to a byte limit.
    class CResourceCache : public TCache< uint32_t, std::shared_ptr<const CResource> >
    {
        protected:
            size_t          m_nBytes;
            size_t          m_nByteLimit;

        public:
                            CResourceCache( size_t nByteLimit );

            void            Add( const std::shared_ptr<const CResource> &pResource );
                // Remove() and Clear() that keep the byte count.
            void            Erase( uint32_t nID );
            void            EraseAll( void );
            void            SetByteLimit( size_t nBytes );
            size_t          GetBytes( void ) const { return m_nBytes; }

        private:
            void            Trim( void );
    };

    class CResourceFile : public CObject
    {
        public:
            enum OpenMode
            {
                READ_ONLY,
                READ_WRITE,     // creates the file if there is none
                CREATE          // replaces any file there
            };

            static const uint32_t FORMAT_VERSION = 1;
            static const size_t HEADER_SIZE = 32;

        protected:
            CFile          *m_pFile;
            CString         m_strPath;
            OpenMode        m_nMode;
            CString         m_strLastError;
            std::map<uint32_t, CResourceIndexEntry> m_mapIndex;
            uint64_t        m_nAppendAt;        // where new data goes
            uint64_t        m_nIndexLength;     // of the index the header points to
            uint64_t        m_nWasted;
            bool            m_bDirty;
            bool            m_bAutoFlush;
            CResourceCache  m_cache;
            mutable CMutex  m_mutex;

        public:
                            CResourceFile( void );
                            // Opens strPath (see Open()).
                            CResourceFile( const char *strPath, OpenMode nMode = READ_WRITE );
            virtual        ~CResourceFile( void );

                            DEFINE_OBJECT( CResourceFile )

                // False, with GetLastError() saying why, if the file cannot be
                // opened or is not a resource archive (or is damaged).
            bool            Open( const char *strPath, OpenMode nMode = READ_WRITE );
                // Flushes and closes.
            bool            Close( void );
            bool            IsOpen( void ) const;
            bool            IsReadOnly( void ) const { return m_nMode == READ_ONLY; }
            CString         GetLastError( void ) const;
            CString         GetPath( void ) const;

            size_t          GetCount( void ) const;
            bool            Exists( short nFolder, short nResource ) const;
                // The tags of the first resource with this name.
            bool            FindByName( const char *strName, short &nFolder, short &nResource ) const;
            bool            GetEntry( short nFolder, short nResource, CResourceIndexEntry &oEntry ) const;
                // Every entry, in ID order.
            std::vector<CResourceIndexEntry> GetEntries( void ) const;
            std::vector<short> GetFolders( void ) const;
            CResourceDirectoryNode GetDirectory( short nFolder ) const;

                // The resource (NULL if there is none, or its data is damaged).
            std::shared_ptr<const CResource> Get( short nFolder, short nResource );
                // A copy of it.
            bool            Read( short nFolder, short nResource, CResource &oResource );
                // Adds the resource, or replaces the one with its tags.
            bool            Put( const CResource &oResource );
            bool            Remove( short nFolder, short nResource );

                // Writes the index and header if anything changed.
            bool            Flush( void );
            void            SetAutoFlush( bool bAutoFlush );
                // Rewrites the file without the wasted space.
            bool            Compact( void );
                // Bytes that Compact() would free.
            uint64_t        GetWastedBytes( void ) const;

            void            SetCacheLimit( size_t nBytes );
            size_t          GetCacheBytes( void ) const;
            size_t          GetCachedCount( void ) const;
            uint64_t        GetCacheHits( void ) const;
            uint64_t        GetCacheMisses( void ) const;

        private:
            bool            Fail( const char *strFormat, ... );
            bool            ReadIndex( void );
            bool            WriteIndexAndHeader( CFile &oFile, const std::map<uint32_t, CResourceIndexEntry> &mapIndex,
                                                 uint64_t nIndexOffset, uint64_t &nIndexLength );
            void            CloseFile( void );

                            CResourceFile( const CResourceFile & );
            CResourceFile  &operator =( const CResourceFile & );
    };
} // namespace IASLib
#endif // IASLIB_RESOURCEFILE_H__
