/**
 *  Resource File
 *
 *      See ResourceFile.h. The layout (numbers big-endian):
 *
 *      header (32 bytes)
 *          "IASRES\r\n"        magic (the CR LF catches text-mode copies)
 *          uint32              format version
 *          uint32              number of resources
 *          uint64              offset of the index
 *          uint64              length of the index
 *      data                    each resource's bytes, anywhere after the header
 *      index                   for each resource:
 *          uint32 id, int32 type, uint32 compression, uint32 CRC-32,
 *          int64 epoch day, int32 milliseconds of the day,
 *          uint64 offset, uint64 length,
 *          uint16 + bytes name, uint16 + bytes modified-by
 *                              then uint32 CRC-32 of all of the above
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "ResourceFile.h"

#include <stdarg.h>
#include <string.h>
#include <string>
#ifdef IASLIB_WIN32__
#include "Platform.h"
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CResourceFile, CObject );

    namespace
    {
        const unsigned char s_abyMagic[ 8 ] = { 'I', 'A', 'S', 'R', 'E', 'S', '\r', '\n' };
        const size_t DEFAULT_CACHE_LIMIT = 4 * 1024 * 1024;
        const size_t COPY_CHUNK = 64 * 1024;

        bool seekTo( FILE *pFile, uint64_t nOffset )
        {
#ifdef IASLIB_WIN32__
            return _fseeki64( pFile, (__int64)nOffset, SEEK_SET ) == 0;
#else
            return fseeko( pFile, (off_t)nOffset, SEEK_SET ) == 0;
#endif
        }

        bool fileSize( FILE *pFile, uint64_t &nSize )
        {
#ifdef IASLIB_WIN32__
            if ( _fseeki64( pFile, 0, SEEK_END ) != 0 )
                return false;
            __int64 nAt = _ftelli64( pFile );
#else
            if ( fseeko( pFile, 0, SEEK_END ) != 0 )
                return false;
            off_t nAt = ftello( pFile );
#endif
            if ( nAt < 0 )
                return false;
            nSize = (uint64_t)nAt;
            return true;
        }

        bool readAt( FILE *pFile, uint64_t nOffset, void *pBuffer, size_t nLength )
        {
            return seekTo( pFile, nOffset ) && ( nLength == 0 || fread( pBuffer, 1, nLength, pFile ) == nLength );
        }

        bool writeAt( FILE *pFile, uint64_t nOffset, const void *pBuffer, size_t nLength )
        {
            return seekTo( pFile, nOffset ) && ( nLength == 0 || fwrite( pBuffer, 1, nLength, pFile ) == nLength );
        }

            // Big-endian encoding into a byte vector, and decoding from one.
        struct CWriter
        {
            std::vector<unsigned char> aBytes;
            void u8( unsigned v ) { aBytes.push_back( (unsigned char)v ); }
            void u16( unsigned v ) { u8( v >> 8 ); u8( v ); }
            void u32( uint32_t v ) { u16( v >> 16 ); u16( v & 0xFFFF ); }
            void u64( uint64_t v ) { u32( (uint32_t)( v >> 32 ) ); u32( (uint32_t)v ); }
            void text( const CString &str )
            {
                size_t n = str.GetLength();
                if ( n > 0xFFFF )
                    n = 0xFFFF;
                u16( (unsigned)n );
                aBytes.insert( aBytes.end(), (const unsigned char *)(const char *)str, (const unsigned char *)(const char *)str + n );
            }
        };

        struct CReader
        {
            const unsigned char *p;
            size_t nLeft;
            bool bOk;
            CReader( const unsigned char *pData, size_t n ) : p( pData ), nLeft( n ), bOk( true ) {}
            bool take( size_t n ) { if ( ! bOk || nLeft < n ) { bOk = false; return false; } return true; }
            unsigned u8( void ) { if ( ! take( 1 ) ) return 0; nLeft--; return *p++; }
            unsigned u16( void ) { unsigned a = u8(); return ( a << 8 ) | u8(); }
            uint32_t u32( void ) { uint32_t a = u16(); return ( a << 16 ) | u16(); }
            uint64_t u64( void ) { uint64_t a = u32(); return ( a << 32 ) | u32(); }
            CString text( void )
            {
                size_t n = u16();
                if ( ! take( n ) )
                    return CString( "" );
                CString str( (const char *)p, n );
                p += n;
                nLeft -= n;
                return str;
            }
        };

        std::vector<unsigned char> encodeIndex( const std::map<uint32_t, CResourceIndexEntry> &mapIndex )
        {
            CWriter w;
            for ( std::map<uint32_t, CResourceIndexEntry>::const_iterator it = mapIndex.begin(); it != mapIndex.end(); ++it )
            {
                const CResourceIndexEntry &e = it->second;
                w.u32( e.GetID() );
                w.u32( (uint32_t)e.m_nType );
                w.u32( (uint32_t)e.m_nCompression );
                w.u32( e.m_nCRC );
                w.u64( (uint64_t)(int64_t)e.m_dttLastModified.GetEpochDay() );
                long lMillis = (long)e.m_dttLastModified.GetSOD() * 1000 + e.m_dttLastModified.GetMillisecond();
                w.u32( (uint32_t)lMillis );
                w.u64( e.m_nOffset );
                w.u64( e.m_nLength );
                w.text( e.m_strResourceName );
                w.text( e.m_strModifiedBy );
            }
            w.u32( CResourceFile::CRC32( w.aBytes.empty() ? NULL : &w.aBytes[0], w.aBytes.size() ) );
            return w.aBytes;
        }

        std::vector<unsigned char> encodeHeader( size_t nCount, uint64_t nIndexOffset, uint64_t nIndexLength )
        {
            CWriter w;
            w.aBytes.assign( s_abyMagic, s_abyMagic + sizeof( s_abyMagic ) );
            w.u32( CResourceFile::FORMAT_VERSION );
            w.u32( (uint32_t)nCount );
            w.u64( nIndexOffset );
            w.u64( nIndexLength );
            return w.aBytes;
        }

        bool replaceFile( const char *strFrom, const char *strTo )
        {
#ifdef IASLIB_WIN32__
            return MoveFileExA( strFrom, strTo, MOVEFILE_REPLACE_EXISTING ) != 0;
#else
            return rename( strFrom, strTo ) == 0;
#endif
        }
    }

    uint32_t CResourceFile::CRC32( const void *pData, size_t nLength, uint32_t nCRC )
    {
        static uint32_t s_anTable[ 256 ];
        static bool s_bTable = ( [](){
            for ( uint32_t n = 0; n < 256; n++ )
            {
                uint32_t c = n;
                for ( int k = 0; k < 8; k++ )
                    c = ( c & 1 ) ? 0xEDB88320U ^ ( c >> 1 ) : ( c >> 1 );
                s_anTable[n] = c;
            }
            return true;
        } )();
        (void)s_bTable;

        const unsigned char *p = (const unsigned char *)pData;
        nCRC = ~nCRC;
        for ( size_t n = 0; n < nLength; n++ )
            nCRC = s_anTable[ ( nCRC ^ p[n] ) & 0xFF ] ^ ( nCRC >> 8 );
        return ~nCRC;
    }

    CResourceFile::CResourceFile( void )
    {
        m_pFile = NULL;
        m_nMode = READ_ONLY;
        m_nAppendAt = HEADER_SIZE;
        m_nIndexLength = 0;
        m_nWasted = 0;
        m_bDirty = false;
        m_bAutoFlush = true;
        m_nCacheBytes = 0;
        m_nCacheLimit = DEFAULT_CACHE_LIMIT;
    }

    CResourceFile::CResourceFile( const char *strPath, OpenMode nMode )
    {
        m_pFile = NULL;
        m_nMode = READ_ONLY;
        m_nAppendAt = HEADER_SIZE;
        m_nIndexLength = 0;
        m_nWasted = 0;
        m_bDirty = false;
        m_bAutoFlush = true;
        m_nCacheBytes = 0;
        m_nCacheLimit = DEFAULT_CACHE_LIMIT;
        Open( strPath, nMode );
    }

    CResourceFile::~CResourceFile( void )
    {
        Close();
    }

    bool CResourceFile::Fail( const char *strFormat, ... )
    {
        char achMessage[ 512 ];
        va_list vaArgs;
        va_start( vaArgs, strFormat );
        vsnprintf( achMessage, sizeof( achMessage ), strFormat, vaArgs );
        va_end( vaArgs );
        m_strLastError = achMessage;
        return false;
    }

    void CResourceFile::CloseFile( void )
    {
        if ( m_pFile )
            fclose( m_pFile );
        m_pFile = NULL;
        m_mapIndex.clear();
        m_listLRU.clear();
        m_mapResources.clear();
        m_nCacheBytes = 0;
        m_nAppendAt = HEADER_SIZE;
        m_nIndexLength = 0;
        m_nWasted = 0;
        m_bDirty = false;
    }

    bool CResourceFile::Open( const char *strPath, OpenMode nMode )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        Close();
        m_strLastError = "";
        if ( strPath == NULL || *strPath == '\0' )
            return Fail( "no file name" );

        m_strPath = strPath;
        m_nMode = nMode;

        bool bNew = false;
        if ( nMode == READ_ONLY )
        {
            m_pFile = fopen( strPath, "rb" );
        }
        else if ( nMode == CREATE )
        {
            m_pFile = fopen( strPath, "w+b" );
            bNew = true;
        }
        else
        {
            m_pFile = fopen( strPath, "r+b" );
            if ( m_pFile == NULL )
            {
                FILE *pExisting = fopen( strPath, "rb" );
                if ( pExisting )
                {
                    fclose( pExisting );
                    return Fail( "%s cannot be opened for writing", strPath );
                }
                m_pFile = fopen( strPath, "w+b" );
                bNew = true;
            }
        }
        if ( m_pFile == NULL )
            return Fail( "%s cannot be opened", strPath );

        if ( bNew )
        {
                // An empty archive: the header and an empty index.
            m_nAppendAt = HEADER_SIZE;
            m_bDirty = true;
            if ( ! Flush() )
            {
                CloseFile();
                return false;
            }
            return true;
        }

        if ( ! ReadIndex() )
        {
            CString strError = m_strLastError;
            CloseFile();
            m_strLastError = strError;
            return false;
        }
        return true;
    }

    bool CResourceFile::ReadIndex( void )
    {
        uint64_t nFileSize = 0;
        if ( ! fileSize( m_pFile, nFileSize ) || nFileSize < HEADER_SIZE )
            return Fail( "%s is not a resource archive (too short)", (const char *)m_strPath );

        unsigned char abyHeader[ HEADER_SIZE ];
        if ( ! readAt( m_pFile, 0, abyHeader, HEADER_SIZE ) )
            return Fail( "%s cannot be read", (const char *)m_strPath );
        if ( memcmp( abyHeader, s_abyMagic, sizeof( s_abyMagic ) ) != 0 )
            return Fail( "%s is not a resource archive", (const char *)m_strPath );

        CReader header( abyHeader + sizeof( s_abyMagic ), HEADER_SIZE - sizeof( s_abyMagic ) );
        uint32_t nVersion = header.u32();
        uint32_t nCount = header.u32();
        uint64_t nIndexOffset = header.u64();
        uint64_t nIndexLength = header.u64();
        if ( nVersion == 0 || nVersion > FORMAT_VERSION )
            return Fail( "%s is archive format %u, which this library cannot read", (const char *)m_strPath, (unsigned)nVersion );
        if ( nIndexOffset < HEADER_SIZE || nIndexLength < 4 || nIndexOffset > nFileSize || nIndexLength > nFileSize - nIndexOffset ||
             nIndexLength > (uint64_t)(size_t)-1 )
            return Fail( "%s has a damaged header", (const char *)m_strPath );

        std::vector<unsigned char> aIndex( (size_t)nIndexLength );
        if ( ! readAt( m_pFile, nIndexOffset, &aIndex[0], aIndex.size() ) )
            return Fail( "%s: the index cannot be read", (const char *)m_strPath );

        size_t nBody = aIndex.size() - 4;
        CReader crc( &aIndex[ nBody ], 4 );
        if ( crc.u32() != CRC32( &aIndex[0], nBody ) )
            return Fail( "%s: the index is damaged (CRC)", (const char *)m_strPath );

        CReader r( &aIndex[0], nBody );
        uint64_t nLive = 0;
        for ( uint32_t n = 0; n < nCount; n++ )
        {
            CResourceIndexEntry e;
            uint32_t nID = r.u32();
            e.m_nFolder = (short)(unsigned short)( nID >> 16 );
            e.m_nResource = (short)(unsigned short)( nID & 0xFFFF );
            e.m_nType = (int)r.u32();
            e.m_nCompression = (int)r.u32();
            e.m_nCRC = r.u32();
            long lEpochDay = (long)(int64_t)r.u64();
            long lMillis = (long)r.u32();
            e.m_dttLastModified = CDate( lEpochDay, lMillis );
            e.m_nOffset = r.u64();
            e.m_nLength = r.u64();
            e.m_strResourceName = r.text();
            e.m_strModifiedBy = r.text();
            if ( ! r.bOk )
                return Fail( "%s: the index is damaged (short)", (const char *)m_strPath );
            if ( e.m_nOffset < HEADER_SIZE || e.m_nOffset > nFileSize || e.m_nLength > nFileSize - e.m_nOffset )
                return Fail( "%s: resource %d/%d lies outside the file", (const char *)m_strPath, (int)e.m_nFolder, (int)e.m_nResource );
            nLive += e.m_nLength;
            m_mapIndex[ nID ] = e;
        }
        if ( r.nLeft != 0 )
            return Fail( "%s: the index is damaged (long)", (const char *)m_strPath );

        m_nIndexLength = nIndexLength;
        m_nAppendAt = nFileSize;
        uint64_t nUsed = HEADER_SIZE + nLive + nIndexLength;
        m_nWasted = ( nFileSize > nUsed ) ? nFileSize - nUsed : 0;
        m_bDirty = false;
        return true;
    }

    bool CResourceFile::WriteIndexAndHeader( FILE *pFile, const std::map<uint32_t, CResourceIndexEntry> &mapIndex,
                                             uint64_t nIndexOffset, uint64_t &nIndexLength )
    {
        std::vector<unsigned char> aIndex = encodeIndex( mapIndex );
            // The index first, then the header that points to it.
        if ( ! writeAt( pFile, nIndexOffset, &aIndex[0], aIndex.size() ) || fflush( pFile ) != 0 )
            return Fail( "%s: the index cannot be written", (const char *)m_strPath );
        std::vector<unsigned char> aHeader = encodeHeader( mapIndex.size(), nIndexOffset, aIndex.size() );
        if ( ! writeAt( pFile, 0, &aHeader[0], aHeader.size() ) || fflush( pFile ) != 0 )
            return Fail( "%s: the header cannot be written", (const char *)m_strPath );
        nIndexLength = aIndex.size();
        return true;
    }

    bool CResourceFile::Flush( void )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        if ( m_pFile == NULL )
            return Fail( "not open" );
        if ( ! m_bDirty )
            return true;
        if ( m_nMode == READ_ONLY )
            return Fail( "read-only" );

        uint64_t nOldIndex = m_nIndexLength;
        uint64_t nNewLength = 0;
        if ( ! WriteIndexAndHeader( m_pFile, m_mapIndex, m_nAppendAt, nNewLength ) )
            return false;
        m_nWasted += nOldIndex;                 // the index it replaces
        m_nIndexLength = nNewLength;
        m_nAppendAt += nNewLength;              // new data goes after it
        m_bDirty = false;
        return true;
    }

    bool CResourceFile::Close( void )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        if ( m_pFile == NULL )
            return true;
        bool bOk = ( ! m_bDirty || m_nMode == READ_ONLY ) ? true : Flush();
        CloseFile();
        return bOk;
    }

    bool CResourceFile::IsOpen( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_pFile != NULL;
    }

    CString CResourceFile::GetLastError( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_strLastError;
    }

    CString CResourceFile::GetPath( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_strPath;
    }

    size_t CResourceFile::GetCount( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_mapIndex.size();
    }

    bool CResourceFile::Exists( short nFolder, short nResource ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_mapIndex.find( CResourceIndexEntry::MakeID( nFolder, nResource ) ) != m_mapIndex.end();
    }

    bool CResourceFile::FindByName( const char *strName, short &nFolder, short &nResource ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        if ( strName == NULL )
            return false;
        for ( std::map<uint32_t, CResourceIndexEntry>::const_iterator it = m_mapIndex.begin(); it != m_mapIndex.end(); ++it )
        {
            if ( it->second.m_strResourceName == strName )
            {
                nFolder = it->second.m_nFolder;
                nResource = it->second.m_nResource;
                return true;
            }
        }
        return false;
    }

    bool CResourceFile::GetEntry( short nFolder, short nResource, CResourceIndexEntry &oEntry ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        std::map<uint32_t, CResourceIndexEntry>::const_iterator it = m_mapIndex.find( CResourceIndexEntry::MakeID( nFolder, nResource ) );
        if ( it == m_mapIndex.end() )
            return false;
        oEntry = it->second;
        return true;
    }

    std::vector<CResourceIndexEntry> CResourceFile::GetEntries( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        std::vector<CResourceIndexEntry> aEntries;
        for ( std::map<uint32_t, CResourceIndexEntry>::const_iterator it = m_mapIndex.begin(); it != m_mapIndex.end(); ++it )
            aEntries.push_back( it->second );
        return aEntries;
    }

    std::vector<short> CResourceFile::GetFolders( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        std::vector<short> aFolders;
        for ( std::map<uint32_t, CResourceIndexEntry>::const_iterator it = m_mapIndex.begin(); it != m_mapIndex.end(); ++it )
        {
            if ( aFolders.empty() || aFolders.back() != it->second.m_nFolder )
                aFolders.push_back( it->second.m_nFolder );
        }
        return aFolders;
    }

    CResourceDirectoryNode CResourceFile::GetDirectory( short nFolder ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        CResourceDirectoryNode oDirectory( nFolder );
        uint32_t nFirst = CResourceIndexEntry::MakeID( nFolder, 0 );
        for ( std::map<uint32_t, CResourceIndexEntry>::const_iterator it = m_mapIndex.lower_bound( nFirst );
              it != m_mapIndex.end() && it->second.m_nFolder == nFolder; ++it )
        {
            oDirectory.Add( it->second );
        }
        return oDirectory;
    }

    void CResourceFile::CacheErase( uint32_t nID )
    {
        std::map<uint32_t, CCached>::iterator it = m_mapResources.find( nID );
        if ( it == m_mapResources.end() )
            return;
        m_nCacheBytes -= it->second.pResource->GetLength();
        m_listLRU.erase( it->second.itLRU );
        m_mapResources.erase( it );
    }

    void CResourceFile::CacheInsert( const std::shared_ptr<const CResource> &pResource )
    {
        uint32_t nID = pResource->GetID();
        CacheErase( nID );
        size_t nSize = pResource->GetLength();
        if ( nSize > m_nCacheLimit )
            return;
        while ( ! m_listLRU.empty() && m_nCacheBytes + nSize > m_nCacheLimit )
            CacheErase( m_listLRU.back() );
        m_listLRU.push_front( nID );
        CCached cached;
        cached.pResource = pResource;
        cached.itLRU = m_listLRU.begin();
        m_mapResources[ nID ] = cached;
        m_nCacheBytes += nSize;
    }

    std::shared_ptr<const CResource> CResourceFile::Get( short nFolder, short nResource )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        uint32_t nID = CResourceIndexEntry::MakeID( nFolder, nResource );
        std::map<uint32_t, CResourceIndexEntry>::iterator itEntry = m_mapIndex.find( nID );
        if ( m_pFile == NULL || itEntry == m_mapIndex.end() )
            return std::shared_ptr<const CResource>();
        CResourceIndexEntry &e = itEntry->second;

        std::map<uint32_t, CCached>::iterator itCached = m_mapResources.find( nID );
        if ( itCached != m_mapResources.end() )
        {
            m_listLRU.splice( m_listLRU.begin(), m_listLRU, itCached->second.itLRU );
            e.m_nUsage++;
            return itCached->second.pResource;
        }

        if ( e.m_nCompression != RESFILE_COMPRESSION_NONE )
        {
            Fail( "resource %d/%d is compressed (method %d), which this library cannot read", (int)nFolder, (int)nResource, e.m_nCompression );
            return std::shared_ptr<const CResource>();
        }
        if ( e.m_nLength > (uint64_t)(size_t)-1 )
        {
            Fail( "resource %d/%d is too large to load", (int)nFolder, (int)nResource );
            return std::shared_ptr<const CResource>();
        }

        std::vector<unsigned char> aData( (size_t)e.m_nLength );
        if ( ! readAt( m_pFile, e.m_nOffset, aData.empty() ? NULL : &aData[0], aData.size() ) )
        {
            Fail( "resource %d/%d cannot be read", (int)nFolder, (int)nResource );
            return std::shared_ptr<const CResource>();
        }
        if ( CRC32( aData.empty() ? NULL : &aData[0], aData.size() ) != e.m_nCRC )
        {
            Fail( "resource %d/%d is damaged (CRC)", (int)nFolder, (int)nResource );
            return std::shared_ptr<const CResource>();
        }

        std::shared_ptr<CResource> pResource( new CResource( nFolder, nResource ) );
        pResource->SetData( aData.empty() ? NULL : &aData[0], aData.size() );
        pResource->SetName( e.m_strResourceName );
        pResource->SetModifiedBy( e.m_strModifiedBy );
        pResource->SetResourceType( e.m_nType );
        pResource->SetLastModified( e.m_dttLastModified );
        e.m_nUsage++;
        CacheInsert( pResource );
        return pResource;
    }

    bool CResourceFile::Read( short nFolder, short nResource, CResource &oResource )
    {
        std::shared_ptr<const CResource> pResource = Get( nFolder, nResource );
        if ( ! pResource )
            return false;
        oResource = *pResource;
        return true;
    }

    bool CResourceFile::Put( const CResource &oResource )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        if ( m_pFile == NULL )
            return Fail( "not open" );
        if ( m_nMode == READ_ONLY )
            return Fail( "read-only" );

            // After the end of the file: nothing the header points to is
            // touched until Flush().
        uint64_t nOffset = m_nAppendAt;
        if ( ! writeAt( m_pFile, nOffset, oResource.GetData(), oResource.GetLength() ) )
            return Fail( "resource %d/%d cannot be written", (int)oResource.GetFolder(), (int)oResource.GetResource() );

        uint32_t nID = oResource.GetID();
        std::map<uint32_t, CResourceIndexEntry>::iterator it = m_mapIndex.find( nID );
        if ( it != m_mapIndex.end() )
            m_nWasted += it->second.m_nLength;

        CResourceIndexEntry e;
        e.m_nFolder = oResource.GetFolder();
        e.m_nResource = oResource.GetResource();
        e.m_strResourceName = oResource.GetName();
        e.m_nType = oResource.GetResourceType();
        e.m_dttLastModified = oResource.GetLastModified();
        e.m_strModifiedBy = oResource.GetModifiedBy();
        e.m_nOffset = nOffset;
        e.m_nLength = oResource.GetLength();
        e.m_nCRC = CRC32( oResource.GetData(), oResource.GetLength() );
        e.m_nCompression = RESFILE_COMPRESSION_NONE;
        m_mapIndex[ nID ] = e;
        m_nAppendAt += oResource.GetLength();
        m_bDirty = true;

        CacheInsert( std::shared_ptr<const CResource>( new CResource( oResource ) ) );
        return ( m_bAutoFlush ) ? Flush() : true;
    }

    bool CResourceFile::Remove( short nFolder, short nResource )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        if ( m_pFile == NULL )
            return Fail( "not open" );
        if ( m_nMode == READ_ONLY )
            return Fail( "read-only" );
        uint32_t nID = CResourceIndexEntry::MakeID( nFolder, nResource );
        std::map<uint32_t, CResourceIndexEntry>::iterator it = m_mapIndex.find( nID );
        if ( it == m_mapIndex.end() )
            return false;
        m_nWasted += it->second.m_nLength;
        m_mapIndex.erase( it );
        CacheErase( nID );
        m_bDirty = true;
        return ( m_bAutoFlush ) ? Flush() : true;
    }

    void CResourceFile::SetAutoFlush( bool bAutoFlush )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        m_bAutoFlush = bAutoFlush;
    }

    uint64_t CResourceFile::GetWastedBytes( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_nWasted;
    }

    bool CResourceFile::Compact( void )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        if ( m_pFile == NULL )
            return Fail( "not open" );
        if ( m_nMode == READ_ONLY )
            return Fail( "read-only" );
        if ( ! Flush() )
            return false;

            // Everything live into a new file beside this one, then the new
            // file replaces it. Until then this file is untouched.
        std::string strTemp = std::string( (const char *)m_strPath ) + ".compact";
        FILE *pNew = fopen( strTemp.c_str(), "w+b" );
        if ( pNew == NULL )
            return Fail( "%s cannot be created", strTemp.c_str() );

        std::map<uint32_t, CResourceIndexEntry> mapNew = m_mapIndex;
        uint64_t nAt = HEADER_SIZE;
        std::vector<unsigned char> aBuffer( COPY_CHUNK );
        bool bOk = true;
        for ( std::map<uint32_t, CResourceIndexEntry>::iterator it = mapNew.begin(); bOk && it != mapNew.end(); ++it )
        {
            uint64_t nDone = 0;
            while ( bOk && nDone < it->second.m_nLength )
            {
                size_t nChunk = (size_t)( ( it->second.m_nLength - nDone < COPY_CHUNK ) ? it->second.m_nLength - nDone : COPY_CHUNK );
                bOk = readAt( m_pFile, it->second.m_nOffset + nDone, &aBuffer[0], nChunk ) &&
                      writeAt( pNew, nAt + nDone, &aBuffer[0], nChunk );
                nDone += nChunk;
            }
            it->second.m_nOffset = nAt;
            nAt += it->second.m_nLength;
        }
        uint64_t nIndexLength = 0;
        bOk = bOk && WriteIndexAndHeader( pNew, mapNew, nAt, nIndexLength );
        bOk = ( fclose( pNew ) == 0 ) && bOk;
        if ( ! bOk )
        {
            remove( strTemp.c_str() );
            return Fail( "%s: compacting failed; the archive is unchanged", (const char *)m_strPath );
        }

        fclose( m_pFile );
        m_pFile = NULL;
        if ( ! replaceFile( strTemp.c_str(), m_strPath ) )
        {
            m_pFile = fopen( m_strPath, "r+b" );
            remove( strTemp.c_str() );
            return Fail( "%s could not be replaced; the archive is unchanged", (const char *)m_strPath );
        }
        m_pFile = fopen( m_strPath, "r+b" );
        if ( m_pFile == NULL )
        {
            CloseFile();
            return Fail( "%s cannot be reopened after compacting", (const char *)m_strPath );
        }

            // The cached resources are still right: only the offsets moved.
        for ( std::map<uint32_t, CResourceIndexEntry>::iterator it = mapNew.begin(); it != mapNew.end(); ++it )
            it->second.m_nUsage = m_mapIndex[ it->first ].m_nUsage;
        m_mapIndex.swap( mapNew );
        m_nIndexLength = nIndexLength;
        m_nAppendAt = nAt + nIndexLength;
        m_nWasted = 0;
        m_bDirty = false;
        return true;
    }

    void CResourceFile::SetCacheLimit( size_t nBytes )
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        m_nCacheLimit = nBytes;
        while ( ! m_listLRU.empty() && m_nCacheBytes > m_nCacheLimit )
            CacheErase( m_listLRU.back() );
    }

    size_t CResourceFile::GetCacheBytes( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_nCacheBytes;
    }

    size_t CResourceFile::GetCachedCount( void ) const
    {
        std::lock_guard<std::recursive_mutex> lock( m_mutex );
        return m_mapResources.size();
    }
} // namespace IASLib
