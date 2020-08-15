/**
 * Entity class
 *
 * This is the abstract base class for all returnable entity objects.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 23, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "Entity.h"
#include "Logging/LogSink.h"
#include "Streams/Stream.h"

namespace IASLib
{
    CHash CEntity::m_entityMap( CHash::SMALL );
    size_t CEntity::entityCount = 0;
    CEntity *CEntity::entities[512];
    bool CEntity::m_initialized = false;

    IMPLEMENT_OBJECT( CEntity, CObject );

    CEntity::CEntity( void )
    {

    }

    CEntity::~CEntity( void )
    {

    }

    CEntity *CEntity::getEntity( const char *mimeType, CStream &stream, size_t nContentLength )
    {
        initHash();
        CEntity *pRetVal = NULL;

        DEBUG_LOG( "Getting Entity mimeType=[%s]", 
                (const char *)mimeType );

        if ( m_entityMap.HasKey( mimeType ) )
        {
            CEntity *temp = (CEntity *)m_entityMap.Get( mimeType );
            pRetVal = temp->generateEntity( stream, nContentLength );
        }

        if ( pRetVal == NULL )
        {
            ERROR_LOG( "No Entity Found! mimeType=[%s]", 
                    mimeType );
        }

        return pRetVal;
    }

    void CEntity::registerEntity( CEntity *entity )
    {
        size_t nX = 0;

        while ( nX < IASLIB_MAX_DEFINED_ENTITIES && entities[ nX ] != NULL )
        {
            nX++;
        }
        entities[nX] = entity;
    }

    bool CEntity::isValidEntity( const char *mimeType )
    {
        initHash();
        CString strMimeType(mimeType);
        strMimeType.ToLowerCase();
        if ( m_entityMap.HasKey( strMimeType ) )
            return true;

        return false;
    }

    void CEntity::initHash( void )
    {
        if ( m_initialized == false )
        {
            size_t x = 0;

            while ( entities[x] )
            {
                m_entityMap.Push( entities[x]->getMimeType(), entities[x]);
                x++;
            }
            m_initialized = true;
        }
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
