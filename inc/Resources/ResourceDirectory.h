/**
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
	class CResourceDirectoryNode : public CResourceNode
	{
		protected:
			short           m_nFolder;
			uint64_t        m_nDirectorySize;       // bytes of data
			std::vector<CResourceFileNode> m_aResourceNodes;
		public:
			CResourceDirectoryNode( short nFolder = 0 ) : m_nFolder( nFolder ), m_nDirectorySize( 0 ) {}
			DECLARE_OBJECT( CResourceDirectoryNode, CResourceNode );

			virtual bool                IsDirectory( void ) const { return true; }
			short                       GetFolder( void ) const { return m_nFolder; }
			uint64_t                    GetDirectorySize( void ) const { return m_nDirectorySize; }
			size_t                      GetCount( void ) const { return m_aResourceNodes.size(); }
			const CResourceFileNode    &Get( size_t nIndex ) const { return m_aResourceNodes[ nIndex ]; }

			void                        Add( const CResourceIndexEntry &oEntry )
			{
				m_aResourceNodes.push_back( CResourceFileNode( oEntry ) );
				m_nDirectorySize += oEntry.m_nLength;
			}
	};
} // namespace IASLib
#endif // IASLIB_RESOURCEDIRECTORY_H__
