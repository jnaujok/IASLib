/**
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
	class CResourceFileNode : public CResourceNode
	{
		protected:
			CResourceIndexEntry m_entry;
		public:
			CResourceFileNode( void ) {}
			CResourceFileNode( const CResourceIndexEntry &oEntry ) : m_entry( oEntry ) {}
			DECLARE_OBJECT( CResourceFileNode, CResourceNode );

			virtual bool                IsDirectory( void ) const { return false; }
			const CResourceIndexEntry  &GetEntry( void ) const { return m_entry; }
			const CString              &GetName( void ) const { return m_entry.m_strResourceName; }
			uint64_t                    GetLength( void ) const { return m_entry.m_nLength; }
			const CSha256Digest        &GetDigest( void ) const { return m_entry.m_digest; }
			short                       GetResource( void ) const { return m_entry.m_nResource; }
			bool                        IsCompressed( void ) const { return m_entry.m_nCompression != RESFILE_COMPRESSION_NONE; }
	};
} // namespace IASLib
#endif // IASLIB_RESOURCEFILENODE_H__
