/**
 * Entity class
 *
 * This is the abstract base class for all message body (entity) objects
 * carried by HTTP and SIP requests and responses.
 *
 * Entity types register a prototype against a media type such as
 * "application/sdp". When a message is parsed, getEntity() looks up the
 * media type (ignoring parameters such as "; charset=utf-8") and asks the
 * prototype to build an entity from the body bytes. Bodies of types that
 * have no registered entity are kept unchanged in a CRawEntity, so a body
 * is never lost and getEntity() never returns NULL.
 *
 * The built-in entity types (text/plain and application/sdp) are
 * registered automatically. Applications can register their own types,
 * either by calling registerEntity() or with IASLIB_REGISTER_ENTITY in one
 * of their own source files.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 23, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_ENTITY_H__
#define IASLIB_ENTITY_H__

#define IASLIB_SET_ENTITY( mimeType )       virtual const char *getMimeType( void ) { return mimeType; }

    // Registers an entity type from an application's own source file. Note
    // that static registration in a file that is part of a static library is
    // dropped by the linker unless something else in that file is used, so
    // library code calls CEntity::registerEntity() explicitly instead.
#define IASLIB_REGISTER_ENTITY(className)   static className internalEntity; \
                                            static CRegisterEntityStub registerStub( &internalEntity );

#include "BaseTypes/String_.h"
#include "Streams/Stream.h"

namespace IASLib
{
    class CEntity : public CObject
    {
        public:
            DEFINE_OBJECT( CEntity );

            CEntity( void );

            virtual ~CEntity( void );

                // The body exactly as it is sent on the wire.
            virtual CString toString( void ) = 0;

                // The media type of this entity, for example "application/sdp".
            virtual const char *getMimeType( void ) = 0;

                // The length of the body in bytes.
            virtual size_t getLength( void ) { return toString().GetLength(); }

                // Registers a prototype for its media type. The prototype must
                // stay alive for the life of the program (normally a static).
            static void registerEntity( CEntity *entity );

                // Builds an entity for the given Content-Type header value. An
                // empty body gives a CNullEntity; an unregistered type gives a
                // CRawEntity holding the body. The caller owns the result.
            static CEntity *getEntity( const char *contentType, const CString &body );

                // Reads up to nContentLength bytes from the stream and builds an
                // entity from them, as above.
            static CEntity *getEntity( const char *contentType, CStream &stream, size_t nContentLength );

            static bool isValidEntity( const char *mimeType );

                // Returns the lower-case "type/subtype" part of a Content-Type
                // value, without parameters or surrounding whitespace.
            static CString mediaType( const char *contentType );

        protected:
                // Builds a new entity of this type from the body bytes.
            virtual CEntity *generateEntity( const CString &body ) = 0;
    };

    class CRegisterEntityStub
    {
        private:
            CEntity *entity;

        public:
            CRegisterEntityStub( CEntity *entity )
            {
                this->entity = entity;
                CEntity::registerEntity( entity );
            }
    };
} // namespace IASLib

#endif // IASLIB_ENTITY_H__

#endif // IASLIB_NETWORKING__
