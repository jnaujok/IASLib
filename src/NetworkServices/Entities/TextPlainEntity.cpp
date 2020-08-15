/**
 * Text/Plain Entity class
 *
 * This is the class for returning a body encoded using text/plain encoding.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/Entities/TextPlainEntity.h"

namespace IASLib
{

    IMPLEMENT_OBJECT( CTextPlainEntity, CEntity );
    IASLIB_REGISTER_ENTITY( CTextPlainEntity );

    CTextPlainEntity::CTextPlainEntity( void )
    {

    }

    CTextPlainEntity::CTextPlainEntity( CStream &stream, size_t nContentLength )
    {
        char *achBuffer = new char[ nContentLength + 1 ];
        memset( achBuffer, 0, nContentLength + 1 );
        stream.GetBuffer( achBuffer, nContentLength );
        m_strBody = CString( achBuffer, nContentLength );
        delete [] achBuffer;
    }

    CTextPlainEntity::~CTextPlainEntity( void )
    {

    }

    CString CTextPlainEntity::toString( void )
    {
        return m_strBody;
    }


    CEntity *CTextPlainEntity::generateEntity( CStream &stream, size_t nContentLength )
    {
        return new CTextPlainEntity( stream, nContentLength );
    }

}; // namespace IASLib
#endif // IASLIB_NETWORKING__
