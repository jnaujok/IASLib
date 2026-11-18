/**
 *  @file
 *  Resource Directory Node
 *
 *      A folder of a resource archive: its tag and the resources in it, in
 * resource-tag order. A snapshot, from CResourceFile::GetDirectory().
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef ISALIB_RESOURCEDIRECTORY_H__
#define ISALIB_RESOURCEDIRECTORY_H__

#include <vector>
#include "ResourceNode.h"
#include "ResourceFileNode.h"

namespace IASLib
{
	/// A folder of a resource archive, with its resources.
	///
	/// A snapshot from CResourceFile::GetDirectory(): later changes to
	/// the archive do not show in it.
	/// @code
	/// CResourceDirectoryNode oFolder = archive.GetDirectory( 1 );
	/// for ( size_t n = 0; n < oFolder.GetCount(); n++ )
	///     printf( "%s\n", (const char *)oFolder.Get( n ).GetName() );
	/// @endcode
	class CResourceDirectoryNode : public CResourceNode
	{
		protected:
			short           m_nFolder;              ///< The folder tag.
			uint64_t        m_nDirectorySize;       ///< Bytes of data in the folder's resources.
			std::vector<CResourceFileNode> m_aResourceNodes;   ///< The resources, in resource-tag order.
		public:
			/// Creates an empty folder node.
			/// @param nFolder The folder tag.
			CResourceDirectoryNode( short nFolder = 0 ) : m_nFolder( nFolder ), m_nDirectorySize( 0 ) {}
			DECLARE_OBJECT( CResourceDirectoryNode, CResourceNode );

			/// True: this is a folder.
			virtual bool                IsDirectory( void ) const { return true; }
			/// The folder tag.
			short                       GetFolder( void ) const { return m_nFolder; }
			/// Bytes of data in the folder's resources.
			uint64_t                    GetDirectorySize( void ) const { return m_nDirectorySize; }
			/// How many resources the folder has.
			size_t                      GetCount( void ) const { return m_aResourceNodes.size(); }
			/// A resource of the folder (nIndex must be less than GetCount()).
			const CResourceFileNode    &Get( size_t nIndex ) const { return m_aResourceNodes[ nIndex ]; }

			/// Adds a resource to the folder (for CResourceFile).
			/// @param oEntry The resource's index entry.
			void                        Add( const CResourceIndexEntry &oEntry )
			{
				m_aResourceNodes.push_back( CResourceFileNode( oEntry ) );
				m_nDirectorySize += oEntry.m_nLength;
			}
	};
} // namespace IASLib
#endif // IASLIB_RESOURCEDIRECTORY_H__
