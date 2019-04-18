/***********************************************************************
**  Resource File Class
**
**  Description:
**      This class gives us access to a resource file, which is a file
** composed of one or more resources all bound into a single file. This
** helps us save some space as well as helping us bind our proprietary
** data into a less "hackable" format. 
**
**      This resource file can be used for multiple purposes. In games
** this kind of resource file saves things like graphics and 3D meshes,
** in BaseCamp executables, this file saves HTML and GIF files for use
** in delivering pages.
**
**      Resources are extracted using the Resource type.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef IASLIB_RESOURCEFILE_H__
#define IASLIB_RESOURCEFILE_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/IASLib_String.h"
#include "../BaseTypes/Date.h"
#include "ResourceFile.h"
#include "../Files/File.h"
#include "../Collections/IntArray.h"
#include <map>
#include <list>

namespace IASLib
{
    class CResource;

    typedef std::list<CResource *>      ResourceList;
    typedef std::map<int, CResource *>  ResourceMap;

    class CResourceFile : public CObject
    {
        protected:
            CFile           m_fileSource;
            ResourceList    m_listLRU;
            ResourceMap     m_mapResources;

        public:

    };
} // namespace IASLib
#endif // IASLIB_RESOURCEFILE_H__

