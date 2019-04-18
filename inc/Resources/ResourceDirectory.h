/*
 * Resource Directory Node
 *
 *	This class defines a directory level within a resource file.
 * Directory objects contain a list of files and other directories
 * to create a hierarchical file system within the resource file.
 * This can be used to create a file-system like storage system. It
 * is also possible to address many different objects directly without
 * going through the file/directory structure. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/18/2004
 *	Log:
 *	  $LOG$
 *
 */

#ifndef ISALIB_RESOURCEDIRECTORY_H__
#define ISALIB_RESOURCEDIRECTORY_H__

#include "../BaseTypes/Object.h"
#include "ResourceNode.h"

namespace IASLib
{
	class CResourceDirectoryNode : public CResourceNode
	{
		protected:
			CString			m_strDirectoryName;
			unsigned long	m_nDirectorySize;
			unsigned long	m_nFiles;
			CArray			m_aResourceNodes;
		public:

	};
} // namespace IASLib
#endif // IASLIB_RESOURCEDIRECTORY_H__
