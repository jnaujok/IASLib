/**
 *  @file
 *  Resource File Node
 *
 *      A resource in a view of an archive folder: its index entry.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef ISALIB_RESOURCEFILENODE_H__
#define ISALIB_RESOURCEFILENODE_H__

#include "ResourceNode.h"
#include "ResourceIndex.h"

namespace IASLib
{
	/// A resource in a CResourceDirectoryNode: a copy of its index entry.
	class CResourceFileNode : public CResourceNode
	{
		protected:
			CResourceIndexEntry m_entry;    ///< The resource's index entry.
		public:
			/// Creates a node with an empty entry.
			CResourceFileNode( void ) {}
			/// Creates a node for this entry (copied).
			CResourceFileNode( const CResourceIndexEntry &oEntry ) : m_entry( oEntry ) {}
			DECLARE_OBJECT( CResourceFileNode, CResourceNode );

			/// False: this is a resource, not a folder.
			virtual bool                IsDirectory( void ) const { return false; }
			/// The resource's index entry.
			const CResourceIndexEntry  &GetEntry( void ) const { return m_entry; }
			/// The resource's name.
			const CString              &GetName( void ) const { return m_entry.m_strResourceName; }
			/// The length of the resource's data.
			uint64_t                    GetLength( void ) const { return m_entry.m_nLength; }
			/// The SHA-256 digest of the resource's data.
			const CSha256Digest        &GetDigest( void ) const { return m_entry.m_digest; }
			/// The resource tag.
			short                       GetResource( void ) const { return m_entry.m_nResource; }
			/// True if the archive says the data is compressed.
			bool                        IsCompressed( void ) const { return m_entry.m_nCompression != RESFILE_COMPRESSION_NONE; }
	};
} // namespace IASLib
#endif // IASLIB_RESOURCEFILENODE_H__
