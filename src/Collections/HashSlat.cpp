/***********************************************************************
**  Hash Slat Object
**
**  Description:
**      Used to store individual slats in a hash bucket.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include <memory>
#include "HashSlat.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHashSlat, CObject );

    CHashSlat::CHashSlat( const char *strKey, CObject *pElement )
    {

        m_pElement = pElement;
        m_strKey = strKey;
    }

    CHashSlat::~CHashSlat( void )
    {
            // Since the hash is likely the only repository of the hashed data, this is
            // a really good place to blow it to smithereens.
        delete m_pElement;
    }
} // namespace IASLib
