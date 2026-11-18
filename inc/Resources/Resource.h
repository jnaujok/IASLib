/**
 *  Resource
 *
 *      One resource of a resource-heavy program (a game's textures,
 * sounds, levels...): a block of data with a folder tag and a resource tag
 * (together its ID), a name, a type number for the program's own use, and
 * when and by whom it was last changed. Resources are kept in a
 * CResourceFile archive: Load() reads one, Save() writes one.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RESOURCE_H__
#define IASLIB_RESOURCE_H__

#include <stdint.h>
#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"
#include "../BaseTypes/DataBlock.h"
#include "ResourceIndex.h"

namespace IASLib
{
    class CResourceFile;

    class CResource : public CObject
    {
        protected:
            short                   m_nFolder;
            short                   m_nResource;
            CString                 m_strName;
            CDate                   m_dttLastModified;
            CString                 m_strModifiedBy;
            int                     m_nType;
            CDataBlock              m_data;

        public:
                                    DEFINE_OBJECT(CResource);

                                    CResource( void );
                                    CResource( short nFolder, short nResource );
                                    // With the data read from strFilename
                                    // (none if it cannot be read: see
                                    // LoadData()).
                                    CResource( short nFolder, short nResource, const char *strName, const char *strModifier, int nType, const char *strFilename );
                                    CResource( const CResource &oSource );
            CResource              &operator =( const CResource &oSource );
            virtual                ~CResource( void );

                // Reads the resource with these tags from the archive (false
                // if there is none, or its data is damaged).
            virtual bool            Load( CResourceFile &oFile, short nFolder, short nResource );
                // Writes this resource to the archive, replacing one with the
                // same tags.
            virtual bool            Save( CResourceFile &oFile );

            short                   GetFolder( void ) const { return m_nFolder; }
            short                   GetResource( void ) const { return m_nResource; }
            uint32_t                GetID( void ) const { return CResourceIndexEntry::MakeID( m_nFolder, m_nResource ); }
            void                    SetTags( short nFolder, short nResource ) { m_nFolder = nFolder; m_nResource = nResource; }

            const CString          &GetName( void ) const { return m_strName; }
            void                    SetName( const char *strName ) { m_strName = ( strName ) ? strName : ""; }
            const CString          &GetModifiedBy( void ) const { return m_strModifiedBy; }
            void                    SetModifiedBy( const char *strWho ) { m_strModifiedBy = ( strWho ) ? strWho : ""; }
            const CDate            &GetLastModified( void ) const { return m_dttLastModified; }
            void                    SetLastModified( const CDate &dttWhen ) { m_dttLastModified = dttWhen; }
                // The program's own kind of resource. (Not GetType(), which
                // every CObject has: the class name.)
            int                     GetResourceType( void ) const { return m_nType; }
            void                    SetResourceType( int nType ) { m_nType = nType; }

                // The data (GetData() is NULL when there is none).
            const unsigned char    *GetData( void ) const { return ( m_data.GetSize() ) ? (const unsigned char *)m_data : NULL; }
            size_t                  GetLength( void ) const { return m_data.GetSize(); }
            const CDataBlock       &GetDataBlock( void ) const { return m_data; }
            virtual long            getLength() { return (long)m_data.GetSize(); }
                // Replaces the data (and marks it changed now).
            void                    SetData( const void *pData, size_t nLength );
                // Replaces the data with a file's contents; false if it cannot
                // be read.
            bool                    LoadData( const char *strFilename );
                // Writes the data to a file.
            bool                    SaveData( const char *strFilename ) const;
    };
} // namespace IASLib

#endif // IASLIB_RESOURCE_H__
