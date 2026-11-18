/**
 *  Resource
 *
 *      See Resource.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Resource.h"
#include "ResourceFile.h"

#include "../../inc/Files/File.h"
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CResource, CObject );
    IMPLEMENT_OBJECT( CResourceIndexEntry, CObject );

    CResourceIndexEntry::CResourceIndexEntry( void )
        : m_nFolder( 0 ), m_nResource( 0 ), m_nType( 0 ), m_nOffset( 0 ), m_nLength( 0 ), m_digest(),
          m_nCompression( RESFILE_COMPRESSION_NONE ), m_nUsage( 0 )
    {
    }

    CResourceIndexEntry::CResourceIndexEntry( const CResourceIndexEntry &oSource ) : CObject()
    {
        *this = oSource;
    }

    CResourceIndexEntry::~CResourceIndexEntry( void )
    {
    }

    CResourceIndexEntry &CResourceIndexEntry::operator =( const CResourceIndexEntry &oSource )
    {
        m_nFolder = oSource.m_nFolder;
        m_nResource = oSource.m_nResource;
        m_strResourceName = oSource.m_strResourceName;
        m_nType = oSource.m_nType;
        m_dttLastModified = oSource.m_dttLastModified;
        m_strModifiedBy = oSource.m_strModifiedBy;
        m_nOffset = oSource.m_nOffset;
        m_nLength = oSource.m_nLength;
        m_digest = oSource.m_digest;
        m_nCompression = oSource.m_nCompression;
        m_nUsage = oSource.m_nUsage;
        return *this;
    }

    CResource::CResource( void ) : m_nFolder( 0 ), m_nResource( 0 ), m_nType( 0 )
    {
    }

    CResource::CResource( short nFolder, short nResource ) : m_nFolder( nFolder ), m_nResource( nResource ), m_nType( 0 )
    {
    }

    CResource::CResource( short nFolder, short nResource, const char *strName, const char *strModifier, int nType, const char *strFilename )
        : m_nFolder( nFolder ), m_nResource( nResource ), m_nType( nType )
    {
        SetName( strName );
        SetModifiedBy( strModifier );
        if ( strFilename )
            LoadData( strFilename );
    }

    CResource::CResource( const CResource &oSource ) : CObject()
    {
        *this = oSource;
    }

    CResource &CResource::operator =( const CResource &oSource )
    {
        m_nFolder = oSource.m_nFolder;
        m_nResource = oSource.m_nResource;
        m_strName = oSource.m_strName;
        m_dttLastModified = oSource.m_dttLastModified;
        m_strModifiedBy = oSource.m_strModifiedBy;
        m_nType = oSource.m_nType;
        m_data = oSource.m_data;
        return *this;
    }

    CResource::~CResource( void )
    {
    }

    bool CResource::Load( CResourceFile &oFile, short nFolder, short nResource )
    {
        return oFile.Read( nFolder, nResource, *this );
    }

    bool CResource::Save( CResourceFile &oFile )
    {
        return oFile.Put( *this );
    }

    void CResource::SetData( const void *pData, size_t nLength )
    {
        if ( pData == NULL )
            nLength = 0;
        m_data.ChangeSize( nLength, false );
        if ( nLength )
            m_data.Copy( pData, nLength );
        m_dttLastModified.SetToCurrent();
    }

        // Through CFile, like the archive.
    bool CResource::LoadData( const char *strFilename )
    {
        if ( strFilename == NULL )
            return false;
        try
        {
            CFile oFile( strFilename, CFile::READ | CFile::BINARY );
            if ( ! oFile.IsOpen() )
                return false;
            int64_t lSize = oFile.SeekBig( 0, CFile::END );
            if ( lSize < 0 || (uint64_t)lSize > (uint64_t)(size_t)-1 || oFile.SeekBig( 0, CFile::BEGIN ) != 0 )
                return false;
            CDataBlock data( (size_t)lSize );
            if ( lSize > 0 && oFile.ReadBig( (char *)data, (size_t)lSize ) != lSize )
                return false;
            m_data = data;
            m_dttLastModified.SetToCurrent();
            return true;
        }
        catch ( ... )
        {
            return false;
        }
    }

    bool CResource::SaveData( const char *strFilename ) const
    {
        if ( strFilename == NULL )
            return false;
        try
        {
            CFile oFile( strFilename, CFile::WRITE | CFile::BINARY );
            if ( ! oFile.IsOpen() )
                return false;
            bool bOk = ( m_data.GetSize() == 0 ) ||
                       oFile.WriteBig( (const char *)m_data, m_data.GetSize() ) == (int64_t)m_data.GetSize();
            return oFile.Close() && bOk;
        }
        catch ( ... )
        {
            return false;
        }
    }
} // namespace IASLib
