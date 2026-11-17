/***********************************************************************
** 
**  CMemoryBlock Class
**
**      This class handles allocating a single page of memory (4K) and
** is basically a self-contained item. 
**
**  Created:
**      8/24/2004
**  Creator:
**      Jeffrey R. Naujok
**  Revision History:
**      $LOG$
**
************************************************************************/

#ifndef IASLIB_MEMORYBLOCK_H__
#define IASLIB_MEMORYBLOCK_H__

#include "MemoryHeap.h"
#include "MemoryBlock.h"

    // This is for GCC to handle the packing pragma.
#ifndef HANDLE_PRAGMA_PACK_PUSH_POP
#define HANDLE_PRAGMA_PACK_PUSH_POP
#endif

    // Save and restore the packing on every platform, so that it does not
    // leak into the headers included after this one.
#pragma pack( push, _standard_packing_ )
#pragma pack(1)

namespace IASLib
{

    class CMemoryBlock
    {
        protected:
            char        m_Data[4096];
        public:
                        CMemoryBlock( void );
            
    };
}

#pragma pack( pop, _standard_packing_ )


#endif // IASLIB_MEMORYFIXEDHEAP_H__
