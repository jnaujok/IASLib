/**
 *  @file
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

        /// One resource: a block of data with its tags, name and history.
        ///
        /// A value type: copying copies the data. Not thread-safe; the
        /// archive (CResourceFile) does its own locking.
        ///
        /// @code
        /// CResource oHero( 1, 7, "hero.png", "art", TYPE_TEXTURE, "art/hero.png" );
        /// oHero.Save( archive );
        /// CResource oLoaded;
        /// if ( oLoaded.Load( archive, 1, 7 ) )
        ///     Upload( oLoaded.GetData(), oLoaded.GetLength() );
        /// @endcode
    class CResource : public CObject
    {
        protected:
            short                   m_nFolder;          ///< The folder tag.
            short                   m_nResource;        ///< The resource tag within the folder.
            CString                 m_strName;          ///< The resource's name.
            CDate                   m_dttLastModified;  ///< When it was last changed.
            CString                 m_strModifiedBy;    ///< Who last changed it.
            int                     m_nType;            ///< The program's own kind of resource.
            CDataBlock              m_data;             ///< The data.

        public:
                                    DEFINE_OBJECT(CResource);

                                    /// Creates an empty resource with tags 0/0.
                                    CResource( void );
                                    /// Creates an empty resource with these tags.
                                    /// @param nFolder   The folder tag.
                                    /// @param nResource The resource tag.
                                    CResource( short nFolder, short nResource );
                                    /// Creates a resource with the data read from strFilename
                                    /// (none if it cannot be read: see LoadData()).
                                    /// @param nFolder     The folder tag.
                                    /// @param nResource   The resource tag.
                                    /// @param strName     The name (NULL: "").
                                    /// @param strModifier Who made it (NULL: "").
                                    /// @param nType       The program's own kind of resource.
                                    /// @param strFilename The file to read the data from (NULL: no data).
                                    CResource( short nFolder, short nResource, const char *strName, const char *strModifier, int nType, const char *strFilename );
                                    /// Copies a resource, data included.
                                    CResource( const CResource &oSource );
            /// Copies a resource, data included.
            CResource              &operator =( const CResource &oSource );
                                    /// Destroys the resource.
            virtual                ~CResource( void );

                /// Reads the resource with these tags from the archive.
                /// @param oFile     The archive.
                /// @param nFolder   The folder tag.
                /// @param nResource The resource tag.
                /// @return False if there is none, or its data is damaged
                ///         (oFile.GetLastError() says why); this resource is then unchanged.
            virtual bool            Load( CResourceFile &oFile, short nFolder, short nResource );
                /// Writes this resource to the archive, replacing one with the
                /// same tags (see CResourceFile::Put()).
                /// @param oFile The archive.
                /// @return False if it could not be written.
            virtual bool            Save( CResourceFile &oFile );

            /// The folder tag.
            short                   GetFolder( void ) const { return m_nFolder; }
            /// The resource tag.
            short                   GetResource( void ) const { return m_nResource; }
            /// The tags as one number (see CResourceIndexEntry::MakeID()).
            uint32_t                GetID( void ) const { return CResourceIndexEntry::MakeID( m_nFolder, m_nResource ); }
            /// Sets the folder and resource tags.
            void                    SetTags( short nFolder, short nResource ) { m_nFolder = nFolder; m_nResource = nResource; }

            /// The resource's name.
            const CString          &GetName( void ) const { return m_strName; }
            /// Sets the name (NULL: "").
            void                    SetName( const char *strName ) { m_strName = ( strName ) ? strName : ""; }
            /// Who last changed the resource.
            const CString          &GetModifiedBy( void ) const { return m_strModifiedBy; }
            /// Sets who last changed the resource (NULL: "").
            void                    SetModifiedBy( const char *strWho ) { m_strModifiedBy = ( strWho ) ? strWho : ""; }
            /// When the resource was last changed.
            const CDate            &GetLastModified( void ) const { return m_dttLastModified; }
            /// Sets when the resource was last changed.
            void                    SetLastModified( const CDate &dttWhen ) { m_dttLastModified = dttWhen; }
                /// The program's own kind of resource.
                /// (Not GetType(), which every CObject has: the class name.)
            int                     GetResourceType( void ) const { return m_nType; }
            /// Sets the program's own kind of resource.
            void                    SetResourceType( int nType ) { m_nType = nType; }

                /// The data (NULL when there is none).
            const unsigned char    *GetData( void ) const { return ( m_data.GetSize() ) ? (const unsigned char *)m_data : NULL; }
            /// The data's length in bytes.
            size_t                  GetLength( void ) const { return m_data.GetSize(); }
            /// The data as a block.
            const CDataBlock       &GetDataBlock( void ) const { return m_data; }
            /// The data's length, as a long (kept for older callers; see GetLength()).
            virtual long            getLength() { return (long)m_data.GetSize(); }
                /// Replaces the data (and marks it changed now).
                /// @param pData   The new data (NULL: none).
                /// @param nLength Its length.
            void                    SetData( const void *pData, size_t nLength );
                /// Replaces the data with a file's contents (and marks it changed now).
                /// @param strFilename The file.
                /// @return False if it cannot be read; the data is then unchanged.
            bool                    LoadData( const char *strFilename );
                /// Writes the data to a file, replacing it.
                /// @param strFilename The file.
                /// @return False if it cannot be written.
            bool                    SaveData( const char *strFilename ) const;
    };
} // namespace IASLib

#endif // IASLIB_RESOURCE_H__
