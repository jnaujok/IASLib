/**
 * Null Entity class
 *
 * This is the class for returning an empty entity body.
 *
 * Author: Jeffrey R. Naujok
 * Created: August 23, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/Entities/NullEntity.h"
#include "Logging/LogSink.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CNullEntity, CEntity );
    IASLIB_REGISTER_ENTITY( CNullEntity );

    CNullEntity::CNullEntity( void )
    {

    }

    CNullEntity::~CNullEntity( void )
    {

    }

    CString CNullEntity::toString( void )
    {
        return CString( "" );
    }

    CEntity *CNullEntity::generateEntity( CStream &stream, size_t nContentLength )
    {
        if ( nContentLength > 0 )
        {
            stream.IsEOS();
            WARN_LOG( "Null Entity generated for a non-zero length content: %d bytes", nContentLength );
        }

        return new CNullEntity();
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING
