/**
 *  Resource Sources
 *
 *      See ResourceSource.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "ResourceSource.h"
#include "../../inc/Files/File.h"

#include <limits.h>

namespace IASLib
{
    CArchiveSource::CArchiveSource( const std::shared_ptr<CResourceFile> &pArchive, bool bKeepArchiveCache )
        : m_pArchive( pArchive )
    {
        if ( m_pArchive && ! bKeepArchiveCache )
            m_pArchive->SetCacheLimit( 0 );
        Refresh();
    }

    CString CArchiveSource::GetName( void ) const
    {
        return CString( "archive " ) + ( ( m_pArchive ) ? m_pArchive->GetPath() : CString( "(none)" ) );
    }

    void CArchiveSource::Refresh( void )
    {
        std::map<std::string, uint32_t> mapNames;
        if ( m_pArchive )
        {
            std::vector<CResourceIndexEntry> aEntries = m_pArchive->GetEntries();
                // The first resource (lowest ID) with a name wins, as in
                // CResourceFile::FindByName().
            for ( size_t n = 0; n < aEntries.size(); n++ )
            {
                std::string strName( (const char *)aEntries[n].m_strResourceName );
                if ( ! strName.empty() && mapNames.find( strName ) == mapNames.end() )
                    mapNames[ strName ] = aEntries[n].GetID();
            }
        }
        CMutexLock lock( m_mutex );
        m_mapNames.swap( mapNames );
    }

    bool CArchiveSource::Exists( const CString &strPath )
    {
        CMutexLock lock( m_mutex );
        return m_mapNames.find( std::string( (const char *)strPath ) ) != m_mapNames.end();
    }

    bool CArchiveSource::Read( const CString &strPath, CDataBlock &oData, CString &strError )
    {
        uint32_t nID;
        {
            CMutexLock lock( m_mutex );
            std::map<std::string, uint32_t>::const_iterator it = m_mapNames.find( std::string( (const char *)strPath ) );
            if ( it == m_mapNames.end() )
            {
                strError = "not in the archive";
                return false;
            }
            nID = it->second;
        }
        std::shared_ptr<const CResource> pResource = m_pArchive->Get( (short)(unsigned short)( nID >> 16 ), (short)(unsigned short)( nID & 0xFFFF ) );
        if ( ! pResource )
        {
            strError = m_pArchive->GetLastError();
            return false;
        }
        oData = pResource->GetDataBlock();
        return true;
    }

    CDirectorySource::CDirectorySource( const char *strRoot )
    {
        m_strRoot = ( strRoot && *strRoot ) ? strRoot : ".";
    }

    CString CDirectorySource::GetName( void ) const
    {
        return CString( "directory " ) + m_strRoot;
    }

    CString CDirectorySource::FullPath( const CString &strPath ) const
    {
        CString strFull( m_strRoot );
        if ( strFull.GetLength() && strFull[ strFull.GetLength() - 1 ] != '/' && strFull[ strFull.GetLength() - 1 ] != '\\' )
            strFull += "/";
        strFull += strPath;
        return strFull;
    }

    bool CDirectorySource::Exists( const CString &strPath )
    {
        try
        {
            CFile oFile( FullPath( strPath ), CFile::READ | CFile::BINARY );
            return oFile.IsOpen();
        }
        catch ( ... )
        {
            return false;
        }
    }

    bool CDirectorySource::Read( const CString &strPath, CDataBlock &oData, CString &strError )
    {
        try
        {
            CFile oFile( FullPath( strPath ), CFile::READ | CFile::BINARY );
            if ( ! oFile.IsOpen() )
            {
                strError = "cannot be opened";
                return false;
            }
            int64_t lSize = oFile.SeekBig( 0, CFile::END );
            if ( lSize < 0 || (uint64_t)lSize > (uint64_t)(size_t)-1 || oFile.SeekBig( 0, CFile::BEGIN ) != 0 )
            {
                strError = "cannot be sized";
                return false;
            }
            CDataBlock oRead( (size_t)lSize );
            if ( lSize > 0 && oFile.ReadBig( (char *)oRead, (size_t)lSize ) != lSize )
            {
                strError = "cannot be read";
                return false;
            }
            oData = oRead;
            return true;
        }
        catch ( ... )
        {
            strError = "cannot be opened";
            return false;
        }
    }
} // namespace IASLib
