/**
 * Entity class
 *
 * This is the abstract base class for all returnable entity objects.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 23, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "Entity.h"
#include "Logging/LogSink.h"
#include "Streams/Stream.h"
#include "NetworkServices/Entities/NullEntity.h"
#include "NetworkServices/Entities/RawEntity.h"
#include "NetworkServices/Entities/SdpEntity.h"
#include "NetworkServices/Entities/TextPlainEntity.h"

#include <map>
#include <mutex>
#include <string>

namespace IASLib
{
    IMPLEMENT_OBJECT( CEntity, CObject );

    namespace
    {
            // The registry is created on first use so that registrations made
            // during static initialisation in other files are safe.
        std::mutex &registryMutex( void )
        {
            static std::mutex mutex;
            return mutex;
        }

        std::map<std::string, CEntity *> &registry( void )
        {
            static std::map<std::string, CEntity *> entities;
            return entities;
        }

            // Must be called with the registry mutex held.
        void registerLocked( CEntity *entity )
        {
            CString type = CEntity::mediaType( entity->getMimeType() );
            registry()[ std::string( (const char *)type ) ] = entity;
        }

            // Must be called with the registry mutex held.
        void ensureBuiltIns( void )
        {
            static bool registered = false;
            if ( ! registered )
            {
                static CTextPlainEntity textPlain;
                static CSDPEntity sdp;

                    // Do not replace types an application registered first.
                if ( registry().find( "text/plain" ) == registry().end() )
                    registerLocked( &textPlain );
                if ( registry().find( "application/sdp" ) == registry().end() )
                    registerLocked( &sdp );

                registered = true;
            }
        }
    }

    CEntity::CEntity( void )
    {
    }

    CEntity::~CEntity( void )
    {
    }

    CString CEntity::mediaType( const char *contentType )
    {
        CString retVal( contentType ? contentType : "" );
        size_t nSemi = retVal.IndexOf( ';' );
        if ( nSemi != NOT_FOUND )
        {
            retVal = retVal.Substring( 0, (int)nSemi );
        }
        retVal.Trim();
        retVal.ToLowerCase();
        return retVal;
    }

    CEntity *CEntity::getEntity( const char *contentType, const CString &body )
    {
        if ( body.GetLength() == 0 )
        {
            return new CNullEntity();
        }

        CString type = mediaType( contentType );
        CEntity *prototype = NULL;

        {
            std::lock_guard<std::mutex> lock( registryMutex() );
            ensureBuiltIns();
            std::map<std::string, CEntity *>::iterator it = registry().find( std::string( (const char *)type ) );
            if ( it != registry().end() )
            {
                prototype = it->second;
            }
        }

        CEntity *pRetVal = NULL;

        if ( prototype )
        {
            pRetVal = prototype->generateEntity( body );
        }

        if ( pRetVal == NULL )
        {
            CString rawType = ( type.GetLength() > 0 ) ? CString( contentType ) : CString( "application/octet-stream" );
            rawType.Trim();
            pRetVal = new CRawEntity( rawType, body );
        }

        return pRetVal;
    }

    CEntity *CEntity::getEntity( const char *contentType, CStream &stream, size_t nContentLength )
    {
        CString body;

        if ( nContentLength > 0 )
        {
            char *achBuffer = new char[ nContentLength + 1 ];
            int nRead = stream.GetBuffer( achBuffer, (int)nContentLength );
            achBuffer[ ( nRead > 0 ) ? nRead : 0 ] = '\0';
            if ( nRead > 0 )
            {
                body = CString( achBuffer, (size_t)nRead );
            }
            delete [] achBuffer;
        }

        return getEntity( contentType, body );
    }

    void CEntity::registerEntity( CEntity *entity )
    {
        if ( entity )
        {
            std::lock_guard<std::mutex> lock( registryMutex() );
            registerLocked( entity );
        }
    }

    bool CEntity::isValidEntity( const char *mimeType )
    {
        CString type = mediaType( mimeType );

        std::lock_guard<std::mutex> lock( registryMutex() );
        ensureBuiltIns();
        return ( registry().find( std::string( (const char *)type ) ) != registry().end() );
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
