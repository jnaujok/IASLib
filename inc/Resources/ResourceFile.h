/**
 *  Resource File
 *
 *      An archive of resources (CResource) in one file. Resources are
 * found by their folder and resource tags, or by name.
 *
 *      The file is a header, the resources' data, and an index at the
 * end; numbers are big-endian, and the data and the index each carry a
 * CRC-32. Writing never overwrites anything the header points to: new data
 * goes after the end of the file, and Flush() writes a new index after it
 * and then the header, so a crash part-way leaves the archive as it was at
 * the last Flush(). Data that has been replaced or removed, and old
 * indexes, are wasted space until Compact() rewrites the file.
 *
 *      With auto-flush (the default) every Put() and Remove() is flushed.
 * Read resources are cached, least recently used first out, up to a byte
 * limit. Thread-safe.
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

#include <stdio.h>
#include <stdint.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "Resource.h"
#include "ResourceIndex.h"
#include "ResourceDirectory.h"

namespace IASLib
{
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
            FILE           *m_pFile;
            CString         m_strPath;
            OpenMode        m_nMode;
            CString         m_strLastError;
            std::map<uint32_t, CResourceIndexEntry> m_mapIndex;
            uint64_t        m_nAppendAt;        // where new data goes
            uint64_t        m_nIndexLength;     // of the index the header points to
            uint64_t        m_nWasted;
            bool            m_bDirty;
            bool            m_bAutoFlush;

                // The cache: resources by ID, most recently used at the front.
            typedef std::list<uint32_t> ResourceList;
            struct CCached
            {
                std::shared_ptr<const CResource> pResource;
                ResourceList::iterator itLRU;
            };
            ResourceList    m_listLRU;
            std::map<uint32_t, CCached> m_mapResources;
            size_t          m_nCacheBytes;
            size_t          m_nCacheLimit;

            mutable std::recursive_mutex m_mutex;

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

                // The CRC-32 (as zlib and PNG compute it) of some bytes.
            static uint32_t CRC32( const void *pData, size_t nLength, uint32_t nCRC = 0 );

        private:
            bool            Fail( const char *strFormat, ... );
            bool            ReadIndex( void );
            bool            WriteIndexAndHeader( FILE *pFile, const std::map<uint32_t, CResourceIndexEntry> &mapIndex,
                                                 uint64_t nIndexOffset, uint64_t &nIndexLength );
            void            CacheInsert( const std::shared_ptr<const CResource> &pResource );
            void            CacheErase( uint32_t nID );
            void            CloseFile( void );

                            CResourceFile( const CResourceFile & );
            CResourceFile  &operator =( const CResourceFile & );
    };
} // namespace IASLib
#endif // IASLIB_RESOURCEFILE_H__
