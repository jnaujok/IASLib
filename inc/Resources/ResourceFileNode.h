/*
 * Resource File Node
 *
 *	This class defines a file stored in the resource file. A node
 * stores the information about the file in the resource and includes
 * physical location and other information about the file within the 
 * resource file.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 09/17/2004
 *	Log:
 *	  $LOG$
 *
 */

#ifndef ISALIB_RESOURCEFILENODE_H__
#define ISALIB_RESOURCEFILENODE_H__

#include "../BaseTypes/Object.h"
#include "ResourceNode.h"

static const int RESFILE_COMPRESSION_NONE		= 0;
static const int RESFILE_COMPRESSION_DEFLATE	= 1;

#ifndef INT64__
#define INT64__ long long
#endif

namespace IASLib
{
	class CResourceFileNode : public CResourceNode
	{
		protected:
			CString			m_strFileName;
			unsigned long	m_nFileLength;
			unsigned long	m_nCRC;
			INT64__			m_nPhysicalOffset;
			unsigned long	m_nPhysicalLength;
			unsigned long	m_nPhysicalCRC;
			bool			m_bCompressed;
			int				m_nCompressionAlgorithm;
		public:
	};
} // namespace IASLib
#endif // IASLIB_RESOURCEFILENODE_H__

