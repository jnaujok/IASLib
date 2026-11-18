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

#include <stdio.h>
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CResource, CObject );
    IMPLEMENT_OBJECT( CResourceIndexEntry, CObject );

    CResourceIndexEntry::CResourceIndexEntry( void )
        : m_nFolder( 0 ), m_nResource( 0 ), m_nType( 0 ), m_nOffset( 0 ), m_nLength( 0 ), m_nCRC( 0 ),
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
        m_nCRC = oSource.m_nCRC;
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
        m_aData = oSource.m_aData;
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
        if ( pData == NULL || nLength == 0 )
            m_aData.clear();
        else
            m_aData.assign( (const unsigned char *)pData, (const unsigned char *)pData + nLength );
        m_dttLastModified.SetToCurrent();
    }

    bool CResource::LoadData( const char *strFilename )
    {
        FILE *pFile = ( strFilename ) ? fopen( strFilename, "rb" ) : NULL;
        if ( pFile == NULL )
            return false;

        std::vector<unsigned char> aData;
        unsigned char abyBuffer[ 8192 ];
        size_t nRead;
        while ( ( nRead = fread( abyBuffer, 1, sizeof( abyBuffer ), pFile ) ) > 0 )
            aData.insert( aData.end(), abyBuffer, abyBuffer + nRead );
        bool bOk = ( ferror( pFile ) == 0 );
        fclose( pFile );
        if ( ! bOk )
            return false;

        m_aData.swap( aData );
        m_dttLastModified.SetToCurrent();
        return true;
    }

    bool CResource::SaveData( const char *strFilename ) const
    {
        FILE *pFile = ( strFilename ) ? fopen( strFilename, "wb" ) : NULL;
        if ( pFile == NULL )
            return false;
        bool bOk = m_aData.empty() || fwrite( &m_aData[0], 1, m_aData.size(), pFile ) == m_aData.size();
        bOk = ( fclose( pFile ) == 0 ) && bOk;
        return bOk;
    }
} // namespace IASLib
