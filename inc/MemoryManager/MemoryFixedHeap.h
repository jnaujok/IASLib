/***********************************************************************
** 
**  CMemoryFixedHeap Class
**
**      This class encodes a fixed block size memory heap. This will
** allow us to much more quickly allocate small fixed blocks of memory.
** This will give us some advantages when we need to quickly allocate a
** lot of small blocks of memory, which will basically be carried out in
** O(1) time as opposed to O(n) time for the standard malloc operator
** which has to walk a linked list to find blocks.
**      It will cause a *slight* disadvantage when blocks are re-sized
** (notably the CString class) as it will cause the block to be moved
** and copied often. This may be meaningless, as it's unclear how often
** this happens in standard memory allocators.
**
**  Created:
**      8/24/2004
**  Creator:
**      Jeffrey R. Naujok
**  Revision History:
**      $LOG$
**
************************************************************************/

#ifndef IASLIB_MEMORYFIXEDHEAP_H__
#define IASLIB_MEMORYFIXEDHEAP_H__

#include "../MemoryManager/MemoryHeap.h"
#include "../MemoryManager/MemoryBlock.h"

namespace IASLib
{

    class CMemoryFixedHeap : public CMemoryHeap
    {
        protected:
            CMemoryBlock    **m_aBlocks;
        public:
    };
}


#endif // IASLIB_MEMORYFIXEDHEAP_H__
