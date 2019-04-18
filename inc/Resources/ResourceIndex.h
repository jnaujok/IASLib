/***********************************************************************
**  Resource Index Class
**
**  Description:
**      This class gives us access to the index generated and stored in
** a resource file. 
**
**      There are two different uses for this index. One is when the 
** index is generated. In this case, the index ties individual resources 
** and their identifiers into a map and tries to organize the index for 
** best-speed retrieval results. In order to do this, it uses a file --
** generated during debugging -- that shows which resources are used 
** most often and clusters them together at the front of the file. The
** resources used least often are clustered at the end of the file.
**
**      In use, the resource Index actually has the ability to write
** this data to a file when the destructor is called, and the code
** has been built in debugging mode.
**
**      The second use is to load the resource index into memory when
** the program is actually running. This use loads and maintains the 
** resource map, and the number of accesses to each item in the map.
**
**      Note that typed resources are derived from this class.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef IASLIB_RESOURCE_INDEX_H__
#define IASLIB_RESOURCE_INDEX_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/IASLib_String.h"

namespace IASLib
{
    class CResourceIndexEntry : public CObject
    {
        protected:
            int         m_nResourceID;
            CString     m_strResourceName;
            long        m_nUsage;
            long        m_nOffset;
        public:

    };
} // namespace IASLib

#endif // IASLIB_RESOURCE_INDEX_H__

