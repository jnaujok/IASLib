/***********************************************************************
** 
**  CMemoryHeap Class
**
**      This file defines the base abstract class for memory heaps. It
** sets up some basic allocator types that all derived memory heap
** classes must override.
**      All Heap classes must be derived from this class.
**
**  Created:
**      8/24/2004
**  Creator:
**      Jeffrey R. Naujok
**  Revision History:
**      $LOG$
**
************************************************************************/

#ifndef IASLIB_MEMORYHEAP_H__
#define IASLIB_MEMORYHEAP_H__

#include "MemoryBlock.h"

namespace IASLib
{
    class CMemoryHeap : public CObject
    {
        protected:
        public:
    };
}


#endif // IASLIB_MEMORYHEAP_H__
