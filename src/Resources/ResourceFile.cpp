/**
 *  Resource File
 *
 *      See ResourceFile.h. The layout (numbers big-endian, as CStream's
 * PutUInt32() and the rest write them):
 *
 *      header (32 bytes)
 *          "IASRES\r\n"        magic (the CR LF catches text-mode copies)
 *          uint32              format version
 *          uint32              number of resources
 *          uint64              offset of the index
 *          uint64              length of the index
 *      data                    each resource's bytes, anywhere after the header
 *      index                   for each resource:
 *          uint32 id, uint32 type, uint32 compression, 32 bytes SHA-256,
 *          uint64 epoch day, uint32 milliseconds of the day,
 *          uint64 offset, uint64 length,
 *          uint16 + bytes name, uint16 + bytes modified-by
 *                              then 32 bytes: SHA-256 of all of the above
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
#include "../../inc/Streams/StringStream.h"
#include "../../inc/Exceptions/StreamException.h"

#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CResourceFile, CObject );

    namespace
    {
        const char s_achMagic[ 8 ] = { 'I', 'A', 'S', 'R', 'E', 'S', '\r', '\n' };
        const size_t DEFAULT_CACHE_LIMIT = 16 * 1024 * 1024;
        const size_t COPY_CHUNK = 64 * 1024;
        const size_t DIGEST_SIZE = sizeof( CSha256Digest );

        bool readAt( CFile &oFile, uint64_t nOffset, void *pBuffer, size_t nLength )
        {
            if ( nOffset > (uint64_t)INT64_MAX || oFile.SeekBig( (int64_t)nOffset, CFile::BEGIN ) != (int64_t)nOffset )
                return false;
            return nLength == 0 || oFile.ReadBig( pBuffer, nLength ) == (int64_t)nLength;
        }

        bool writeAt( CFile &oFile, uint64_t nOffset, const void *pBuffer, size_t nLength )
        {
            if ( nOffset > (uint64_t)INT64_MAX || oFile.SeekBig( (int64_t)nOffset, CFile::BEGIN ) != (int64_t)nOffset )
                return false;
            return nLength == 0 || oFile.WriteBig( pBuffer, nLength ) == (int64_t)nLength;
        }

            // Opens with CFile, which throws when it cannot.
        CFile *openFile( const char *strPath, int nMode )
        {
            try
            {
                CFile *pFile = new CFile( strPath, nMode );
                if ( pFile->IsOpen() )
                    return pFile;
                delete pFile;
            }
            catch ( ... )
            {
            }
            return NULL;
        }

        bool fileExists( const char *strPath )
        {
            CFile *pFile = openFile( strPath, CFile::READ | CFile::BINARY );
            delete pFile;
            return pFile != NULL;
        }

            // A new empty file (CFile has no "w+" mode), opened for reading
            // and writing.
        CFile *createFile( const char *strPath )
        {
            CFile *pFile = openFile( strPath, CFile::WRITE | CFile::BINARY );
            if ( pFile == NULL )
                return NULL;
            pFile->Close();
            delete pFile;
            return openFile( strPath, CFile::READ | CFile::WRITE | CFile::BINARY );
        }

        void putText( CStringStream &oOut, const CString &str )
        {
            size_t n = str.GetLength();
            if ( n > 0xFFFF )
                n = 0xFFFF;
            oOut.PutUInt16( (uint16_t)n );
            oOut.PutBuffer( (const char *)str, (int)n );
        }

        CString getText( CStringStream &oIn )
        {
            size_t n = oIn.GetUInt16();
            std::string strText( n, '\0' );
            if ( n )
                oIn.GetExact( &strText[0], (int)n );
            return CString( strText.data(), n );
        }

        std::string encodeIndex( const std::map<uint32_t, CResourceIndexEntry> &mapIndex )
        {
            CStringStream oOut;
            for ( std::map<uint32_t, CResourceIndexEntry>::const_iterator it = mapIndex.begin(); it != mapIndex.end(); ++it )
            {
                const CResourceIndexEntry &e = it->second;
                oOut.PutUInt32( e.GetID() );
                oOut.PutUInt32( (uint32_t)e.m_nType );
                oOut.PutUInt32( (uint32_t)e.m_nCompression );
                oOut.PutBuffer( (const char *)e.m_digest.data(), (int)DIGEST_SIZE );
                oOut.PutUInt64( (uint64_t)(int64_t)e.m_dttLastModified.GetEpochDay() );
                long lMillis = (long)e.m_dttLastModified.GetSOD() * 1000 + e.m_dttLastModified.GetMillisecond();
                oOut.PutUInt32( (uint32_t)lMillis );
                oOut.PutUInt64( e.m_nOffset );
                oOut.PutUInt64( e.m_nLength );
                putText( oOut, e.m_strResourceName );
                putText( oOut, e.m_strModifiedBy );
            }
            std::string strIndex = oOut.GetOutput();
            CSha256Digest digest = CSHA256::Digest( strIndex );
            strIndex.append( (const char *)digest.data(), DIGEST_SIZE );
            return strIndex;
        }

        std::string encodeHeader( size_t nCount, uint64_t nIndexOffset, uint64_t nIndexLength )
        {
            CStringStream oOut;
            oOut.PutBuffer( s_achMagic, (int)sizeof( s_achMagic ) );
            oOut.PutUInt32( CResourceFile::FORMAT_VERSION );
            oOut.PutUInt32( (uint32_t)nCount );
            oOut.PutUInt64( nIndexOffset );
            oOut.PutUInt64( nIndexLength );
            return oOut.GetOutput();
        }

        CSha256Digest digestOf( const CResource &oResource )
        {
            return CSHA256::Digest( oResource.GetData(), oResource.GetLength() );
        }
    }

    CResourceCache::CResourceCache( size_t nByteLimit )
        : TCache< uint32_t, std::shared_ptr<const CResource> >( (size_t)1 << 20 ), m_nBytes( 0 ), m_nByteLimit( nByteLimit )
    {
        SetEvictionHandler( [this]( const uint32_t &, std::shared_ptr<const CResource> &pResource ) {
            if ( pResource )
                m_nBytes -= pResource->GetLength();
        } );
    }

    void CResourceCache::Trim( void )
    {
        while ( GetCount() > 0 && m_nBytes > m_nByteLimit )
            EvictOldest();
    }

    void CResourceCache::Add( const std::shared_ptr<const CResource> &pResource )
    {
        uint32_t nID = pResource->GetID();
        Erase( nID );
        if ( pResource->GetLength() > m_nByteLimit )
            return;                 // too big to keep
        Put( nID, pResource );
        m_nBytes += pResource->GetLength();
        Trim();
    }

        // (TCache's Remove() and Clear() do not call the eviction handler.)
    void CResourceCache::Erase( uint32_t nID )
    {
        std::shared_ptr<const CResource> pOld;
        if ( Remove( nID, pOld ) && pOld )
            m_nBytes -= pOld->GetLength();
    }

    void CResourceCache::EraseAll( void )
    {
        Clear();
        m_nBytes = 0;
    }

    void CResourceCache::SetByteLimit( size_t nBytes )
    {
        m_nByteLimit = nBytes;
        Trim();
    }

    CResourceFile::CResourceFile( void ) : m_cache( DEFAULT_CACHE_LIMIT )
    {
        m_pFile = NULL;
        m_nMode = READ_ONLY;
        m_nAppendAt = HEADER_SIZE;
        m_nIndexLength = 0;
        m_nWasted = 0;
        m_bDirty = false;
        m_bAutoFlush = true;
    }

    CResourceFile::CResourceFile( const char *strPath, OpenMode nMode ) : m_cache( DEFAULT_CACHE_LIMIT )
    {
        m_pFile = NULL;
        m_nMode = READ_ONLY;
        m_nAppendAt = HEADER_SIZE;
        m_nIndexLength = 0;
        m_nWasted = 0;
        m_bDirty = false;
        m_bAutoFlush = true;
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
        {
            m_pFile->Close();
            delete m_pFile;
        }
        m_pFile = NULL;
        m_mapIndex.clear();
        m_cache.EraseAll();
        m_nAppendAt = HEADER_SIZE;
        m_nIndexLength = 0;
        m_nWasted = 0;
        m_bDirty = false;
    }

    bool CResourceFile::Open( const char *strPath, OpenMode nMode )
    {
        CMutexLock lock( m_mutex );
        Close();
        m_strLastError = "";
        if ( strPath == NULL || *strPath == '\0' )
            return Fail( "no file name" );

        m_strPath = strPath;
        m_nMode = nMode;

        bool bNew = false;
        if ( nMode == READ_ONLY )
        {
            m_pFile = openFile( strPath, CFile::READ | CFile::BINARY );
        }
        else if ( nMode == CREATE )
        {
            m_pFile = createFile( strPath );
            bNew = true;
        }
        else
        {
            m_pFile = openFile( strPath, CFile::READ | CFile::WRITE | CFile::BINARY );
            if ( m_pFile == NULL )
            {
                if ( fileExists( strPath ) )
                    return Fail( "%s cannot be opened for writing", strPath );
                m_pFile = createFile( strPath );
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
                CString strError = m_strLastError;
                CloseFile();
                m_strLastError = strError;
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
        int64_t lSize = m_pFile->SeekBig( 0, CFile::END );
        if ( lSize < (int64_t)HEADER_SIZE )
            return Fail( "%s is not a resource archive (too short)", (const char *)m_strPath );
        uint64_t nFileSize = (uint64_t)lSize;

        char achHeader[ HEADER_SIZE ];
        if ( ! readAt( *m_pFile, 0, achHeader, HEADER_SIZE ) )
            return Fail( "%s cannot be read", (const char *)m_strPath );
        if ( memcmp( achHeader, s_achMagic, sizeof( s_achMagic ) ) != 0 )
            return Fail( "%s is not a resource archive", (const char *)m_strPath );

        uint32_t nVersion, nCount;
        uint64_t nIndexOffset, nIndexLength;
        {
            CStringStream oHeader( achHeader + sizeof( s_achMagic ), HEADER_SIZE - sizeof( s_achMagic ) );
            nVersion = oHeader.GetUInt32();
            nCount = oHeader.GetUInt32();
            nIndexOffset = oHeader.GetUInt64();
            nIndexLength = oHeader.GetUInt64();
        }
        if ( nVersion == 0 || nVersion > FORMAT_VERSION )
            return Fail( "%s is archive format %u, which this library cannot read", (const char *)m_strPath, (unsigned)nVersion );
        if ( nIndexOffset < HEADER_SIZE || nIndexLength < DIGEST_SIZE || nIndexOffset > nFileSize ||
             nIndexLength > nFileSize - nIndexOffset )
            return Fail( "%s has a damaged header", (const char *)m_strPath );

        std::string strIndex( (size_t)nIndexLength, '\0' );
        if ( ! readAt( *m_pFile, nIndexOffset, &strIndex[0], strIndex.size() ) )
            return Fail( "%s: the index cannot be read", (const char *)m_strPath );

        size_t nBody = strIndex.size() - DIGEST_SIZE;
        CSha256Digest digest = CSHA256::Digest( strIndex.data(), nBody );
        if ( memcmp( digest.data(), strIndex.data() + nBody, DIGEST_SIZE ) != 0 )
            return Fail( "%s: the index is damaged (digest)", (const char *)m_strPath );

        uint64_t nLive = 0;
        try
        {
            CStringStream oIndex( strIndex.data(), nBody );
            for ( uint32_t n = 0; n < nCount; n++ )
            {
                CResourceIndexEntry e;
                uint32_t nID = oIndex.GetUInt32();
                e.m_nFolder = (short)(unsigned short)( nID >> 16 );
                e.m_nResource = (short)(unsigned short)( nID & 0xFFFF );
                e.m_nType = (int)oIndex.GetUInt32();
                e.m_nCompression = (int)oIndex.GetUInt32();
                oIndex.GetExact( (char *)e.m_digest.data(), (int)DIGEST_SIZE );
                long lEpochDay = (long)(int64_t)oIndex.GetUInt64();
                long lMillis = (long)oIndex.GetUInt32();
                e.m_dttLastModified = CDate( lEpochDay, lMillis );
                e.m_nOffset = oIndex.GetUInt64();
                e.m_nLength = oIndex.GetUInt64();
                e.m_strResourceName = getText( oIndex );
                e.m_strModifiedBy = getText( oIndex );
                if ( e.m_nOffset < HEADER_SIZE || e.m_nOffset > nFileSize || e.m_nLength > nFileSize - e.m_nOffset )
                    return Fail( "%s: resource %d/%d lies outside the file", (const char *)m_strPath, (int)e.m_nFolder, (int)e.m_nResource );
                nLive += e.m_nLength;
                m_mapIndex[ nID ] = e;
            }
            if ( oIndex.bytesRemaining() != 0 )
                return Fail( "%s: the index is damaged (long)", (const char *)m_strPath );
        }
        catch ( CStreamException & )
        {
            return Fail( "%s: the index is damaged (short)", (const char *)m_strPath );
        }

        m_nIndexLength = nIndexLength;
        m_nAppendAt = nFileSize;
        uint64_t nUsed = HEADER_SIZE + nLive + nIndexLength;
        m_nWasted = ( nFileSize > nUsed ) ? nFileSize - nUsed : 0;
        m_bDirty = false;
        return true;
    }

    bool CResourceFile::WriteIndexAndHeader( CFile &oFile, const std::map<uint32_t, CResourceIndexEntry> &mapIndex,
                                             uint64_t nIndexOffset, uint64_t &nIndexLength )
    {
        std::string strIndex = encodeIndex( mapIndex );
            // The index first, then the header that points to it.
        if ( ! writeAt( oFile, nIndexOffset, strIndex.data(), strIndex.size() ) || ! oFile.Flush() )
            return Fail( "%s: the index cannot be written", (const char *)m_strPath );
        std::string strHeader = encodeHeader( mapIndex.size(), nIndexOffset, strIndex.size() );
        if ( ! writeAt( oFile, 0, strHeader.data(), strHeader.size() ) || ! oFile.Flush() )
            return Fail( "%s: the header cannot be written", (const char *)m_strPath );
        nIndexLength = strIndex.size();
        return true;
    }

    bool CResourceFile::Flush( void )
    {
        CMutexLock lock( m_mutex );
        if ( m_pFile == NULL )
            return Fail( "not open" );
        if ( ! m_bDirty )
            return true;
        if ( m_nMode == READ_ONLY )
            return Fail( "read-only" );

        uint64_t nOldIndex = m_nIndexLength;
        uint64_t nNewLength = 0;
        if ( ! WriteIndexAndHeader( *m_pFile, m_mapIndex, m_nAppendAt, nNewLength ) )
            return false;
        m_nWasted += nOldIndex;                 // the index it replaces
        m_nIndexLength = nNewLength;
        m_nAppendAt += nNewLength;              // new data goes after it
        m_bDirty = false;
        return true;
    }

    bool CResourceFile::Close( void )
    {
        CMutexLock lock( m_mutex );
        if ( m_pFile == NULL )
            return true;
        bool bOk = ( ! m_bDirty || m_nMode == READ_ONLY ) ? true : Flush();
        CloseFile();
        return bOk;
    }

    bool CResourceFile::IsOpen( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_pFile != NULL;
    }

    CString CResourceFile::GetLastError( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_strLastError;
    }

    CString CResourceFile::GetPath( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_strPath;
    }

    size_t CResourceFile::GetCount( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_mapIndex.size();
    }

    bool CResourceFile::Exists( short nFolder, short nResource ) const
    {
        CMutexLock lock( m_mutex );
        return m_mapIndex.find( CResourceIndexEntry::MakeID( nFolder, nResource ) ) != m_mapIndex.end();
    }

    bool CResourceFile::FindByName( const char *strName, short &nFolder, short &nResource ) const
    {
        CMutexLock lock( m_mutex );
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
        CMutexLock lock( m_mutex );
        std::map<uint32_t, CResourceIndexEntry>::const_iterator it = m_mapIndex.find( CResourceIndexEntry::MakeID( nFolder, nResource ) );
        if ( it == m_mapIndex.end() )
            return false;
        oEntry = it->second;
        return true;
    }

    std::vector<CResourceIndexEntry> CResourceFile::GetEntries( void ) const
    {
        CMutexLock lock( m_mutex );
        std::vector<CResourceIndexEntry> aEntries;
        for ( std::map<uint32_t, CResourceIndexEntry>::const_iterator it = m_mapIndex.begin(); it != m_mapIndex.end(); ++it )
            aEntries.push_back( it->second );
        return aEntries;
    }

    std::vector<short> CResourceFile::GetFolders( void ) const
    {
        CMutexLock lock( m_mutex );
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
        CMutexLock lock( m_mutex );
        CResourceDirectoryNode oDirectory( nFolder );
        uint32_t nFirst = CResourceIndexEntry::MakeID( nFolder, 0 );
        for ( std::map<uint32_t, CResourceIndexEntry>::const_iterator it = m_mapIndex.lower_bound( nFirst );
              it != m_mapIndex.end() && it->second.m_nFolder == nFolder; ++it )
        {
            oDirectory.Add( it->second );
        }
        return oDirectory;
    }

    std::shared_ptr<const CResource> CResourceFile::Get( short nFolder, short nResource )
    {
        CMutexLock lock( m_mutex );
        uint32_t nID = CResourceIndexEntry::MakeID( nFolder, nResource );
        std::map<uint32_t, CResourceIndexEntry>::iterator itEntry = m_mapIndex.find( nID );
        if ( m_pFile == NULL || itEntry == m_mapIndex.end() )
            return std::shared_ptr<const CResource>();
        CResourceIndexEntry &e = itEntry->second;

        std::shared_ptr<const CResource> *pCached = m_cache.Get( nID );
        if ( pCached != NULL )
        {
            e.m_nUsage++;
            return *pCached;
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

        CDataBlock data( (size_t)e.m_nLength );
        if ( ! readAt( *m_pFile, e.m_nOffset, (unsigned char *)data, data.GetSize() ) )
        {
            Fail( "resource %d/%d cannot be read", (int)nFolder, (int)nResource );
            return std::shared_ptr<const CResource>();
        }
        CSha256Digest digest = CSHA256::Digest( (const unsigned char *)data, data.GetSize() );
        if ( digest != e.m_digest )
        {
            Fail( "resource %d/%d is damaged (digest)", (int)nFolder, (int)nResource );
            return std::shared_ptr<const CResource>();
        }

        std::shared_ptr<CResource> pResource( new CResource( nFolder, nResource ) );
        pResource->SetData( (const unsigned char *)data, data.GetSize() );
        pResource->SetName( e.m_strResourceName );
        pResource->SetModifiedBy( e.m_strModifiedBy );
        pResource->SetResourceType( e.m_nType );
        pResource->SetLastModified( e.m_dttLastModified );
        e.m_nUsage++;
        m_cache.Add( pResource );
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
        CMutexLock lock( m_mutex );
        if ( m_pFile == NULL )
            return Fail( "not open" );
        if ( m_nMode == READ_ONLY )
            return Fail( "read-only" );

            // After the end of the file: nothing the header points to is
            // touched until Flush().
        uint64_t nOffset = m_nAppendAt;
        if ( ! writeAt( *m_pFile, nOffset, oResource.GetData(), oResource.GetLength() ) )
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
        e.m_digest = digestOf( oResource );
        e.m_nCompression = RESFILE_COMPRESSION_NONE;
        m_mapIndex[ nID ] = e;
        m_nAppendAt += oResource.GetLength();
        m_bDirty = true;

        m_cache.Add( std::shared_ptr<const CResource>( new CResource( oResource ) ) );
        return ( m_bAutoFlush ) ? Flush() : true;
    }

    bool CResourceFile::Remove( short nFolder, short nResource )
    {
        CMutexLock lock( m_mutex );
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
        m_cache.Erase( nID );
        m_bDirty = true;
        return ( m_bAutoFlush ) ? Flush() : true;
    }

    void CResourceFile::SetAutoFlush( bool bAutoFlush )
    {
        CMutexLock lock( m_mutex );
        m_bAutoFlush = bAutoFlush;
    }

    uint64_t CResourceFile::GetWastedBytes( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_nWasted;
    }

    bool CResourceFile::Compact( void )
    {
        CMutexLock lock( m_mutex );
        if ( m_pFile == NULL )
            return Fail( "not open" );
        if ( m_nMode == READ_ONLY )
            return Fail( "read-only" );
        if ( ! Flush() )
            return false;

            // Everything live into a new file beside this one, then the new
            // file replaces it. Until then this file is untouched.
        std::string strTemp = std::string( (const char *)m_strPath ) + ".compact";
        CFile *pNew = createFile( strTemp.c_str() );
        if ( pNew == NULL )
            return Fail( "%s cannot be created", strTemp.c_str() );

        std::map<uint32_t, CResourceIndexEntry> mapNew = m_mapIndex;
        uint64_t nAt = HEADER_SIZE;
        CDataBlock buffer( COPY_CHUNK );
        bool bOk = true;
        for ( std::map<uint32_t, CResourceIndexEntry>::iterator it = mapNew.begin(); bOk && it != mapNew.end(); ++it )
        {
            uint64_t nDone = 0;
            while ( bOk && nDone < it->second.m_nLength )
            {
                size_t nChunk = (size_t)( ( it->second.m_nLength - nDone < COPY_CHUNK ) ? it->second.m_nLength - nDone : COPY_CHUNK );
                bOk = readAt( *m_pFile, it->second.m_nOffset + nDone, (unsigned char *)buffer, nChunk ) &&
                      writeAt( *pNew, nAt + nDone, (unsigned char *)buffer, nChunk );
                nDone += nChunk;
            }
            it->second.m_nOffset = nAt;
            nAt += it->second.m_nLength;
        }
        uint64_t nIndexLength = 0;
        bOk = bOk && WriteIndexAndHeader( *pNew, mapNew, nAt, nIndexLength );
        bOk = pNew->Close() && bOk;
        delete pNew;

        bool bReplaced = false;
        if ( bOk )
        {
            m_pFile->Close();
            delete m_pFile;
            m_pFile = NULL;
            try
            {
#ifdef IASLIB_WIN32__
                    // Windows will not rename over an existing file.
                CFile::Delete( m_strPath );
#endif
                bReplaced = CFile::Rename( strTemp.c_str(), m_strPath );
            }
            catch ( ... )
            {
                bReplaced = false;
            }
            m_pFile = openFile( m_strPath, CFile::READ | CFile::WRITE | CFile::BINARY );
        }
        if ( ! bReplaced )
        {
            try { CFile::Delete( strTemp.c_str() ); } catch ( ... ) {}
            if ( m_pFile == NULL )
            {
                CloseFile();
                return Fail( "%s: compacting failed, and the archive could not be reopened", (const char *)m_strPath );
            }
            return Fail( "%s: compacting failed; the archive is unchanged", (const char *)m_strPath );
        }
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
        CMutexLock lock( m_mutex );
        m_cache.SetByteLimit( nBytes );
    }

    size_t CResourceFile::GetCacheBytes( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_cache.GetBytes();
    }

    size_t CResourceFile::GetCachedCount( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_cache.GetCount();
    }

    uint64_t CResourceFile::GetCacheHits( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_cache.GetHits();
    }

    uint64_t CResourceFile::GetCacheMisses( void ) const
    {
        CMutexLock lock( m_mutex );
        return m_cache.GetMisses();
    }
} // namespace IASLib
