/**
 * @file Entity.h
 * @brief The base class of message bodies, and the media type registry (CEntity).
 *
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

    /// Declares getMimeType() in an entity class, returning the given media type.
    /// Use it inside the class body, for example IASLIB_SET_ENTITY( "text/plain" ).
#define IASLIB_SET_ENTITY( mimeType )       virtual const char *getMimeType( void ) { return mimeType; }

    /// Registers an entity type from an application's own source file.
    ///
    /// It defines a static prototype of the class and a CRegisterEntityStub
    /// that registers it during static initialisation. Use it at most once per
    /// source file, at namespace scope.
    /// @note Static registration in a file that is part of a static library is
    ///       dropped by the linker unless something else in that file is used,
    ///       so library code calls CEntity::registerEntity() explicitly instead.
#define IASLIB_REGISTER_ENTITY(className)   static className internalEntity; \
                                            static CRegisterEntityStub registerStub( &internalEntity );

#include "BaseTypes/String_.h"
#include "Streams/Stream.h"

namespace IASLib
{
    /// The abstract base of every message body (entity) in HTTP and SIP messages.
    ///
    /// A derived class gives the body text (toString()), its media type
    /// (getMimeType(), usually through IASLIB_SET_ENTITY) and a way to build a
    /// new instance from body bytes (generateEntity()). A prototype instance of
    /// each type is registered against its media type; getEntity() then turns a
    /// Content-Type value and a body into the right entity object.
    ///
    /// The registry is guarded by a mutex, so registerEntity(), getEntity() and
    /// isValidEntity() may be called from any thread. Entity objects themselves
    /// do no locking.
    /// @code
    /// CEntity *pBody = CEntity::getEntity( "application/sdp; charset=utf-8", strBody );
    /// CString strType = pBody->getMimeType();   // "application/sdp"
    /// delete pBody;
    /// @endcode
    class CEntity : public CObject
    {
        public:
            DEFINE_OBJECT( CEntity );

                /// Creates an entity.
            CEntity( void );
                /// Destroys the entity.
            virtual ~CEntity( void );

                /// The body exactly as it is sent on the wire.
            virtual CString toString( void ) = 0;

                /// The media type of this entity, for example "application/sdp".
            virtual const char *getMimeType( void ) = 0;

                /// The length of the body in bytes.
            virtual size_t getLength( void ) { return toString().GetLength(); }

                /// Registers a prototype for its media type.
                ///
                /// The type is taken from the prototype's getMimeType() (lower
                /// case, without parameters) and replaces any prototype already
                /// registered for it. NULL is ignored. The prototype is not
                /// copied and must stay alive for the life of the program
                /// (normally a static).
            static void registerEntity( CEntity *entity );

                /// Builds an entity for the given Content-Type header value.
                ///
                /// An empty body gives a CNullEntity; an unregistered type (or
                /// one whose prototype returns NULL) gives a CRawEntity holding
                /// the body, typed "application/octet-stream" when no type was
                /// given. Never returns NULL.
                /// @param contentType The Content-Type value; parameters are
                ///                    ignored for the lookup. May be NULL.
                /// @param body        The body bytes.
                /// @return A new entity, owned by the caller.
            static CEntity *getEntity( const char *contentType, const CString &body );

                /// Reads up to nContentLength bytes from the stream and builds an
                /// entity from them, as above.
                ///
                /// The stream is read in pieces until the length is reached or
                /// the stream ends, so a body that arrives in parts is complete;
                /// zero bytes are kept. A stream that ends early gives a shorter
                /// body.
                /// @return A new entity, owned by the caller.
                /// @note This used to be one read, so a body arriving in parts was
                ///       cut short, and the body stopped at the first zero byte.
            static CEntity *getEntity( const char *contentType, CStream &stream, size_t nContentLength );

                /// Whether a prototype is registered for the media type of this
                /// Content-Type value (parameters ignored).
            static bool isValidEntity( const char *mimeType );

                /// Returns the lower-case "type/subtype" part of a Content-Type
                /// value, without parameters or surrounding whitespace.
            static CString mediaType( const char *contentType );

        protected:
                /// Builds a new entity of this type from the body bytes.
                /// @return A new entity owned by the caller, or NULL if the body
                ///         cannot be read as this type (getEntity() then keeps it
                ///         in a CRawEntity).
            virtual CEntity *generateEntity( const CString &body ) = 0;
    };

    /// Registers an entity prototype when constructed; used by IASLIB_REGISTER_ENTITY.
    class CRegisterEntityStub
    {
        private:
            CEntity *entity;    ///< The registered prototype (not owned).

        public:
                /// Registers the prototype with CEntity::registerEntity().
            CRegisterEntityStub( CEntity *entity )
            {
                this->entity = entity;
                CEntity::registerEntity( entity );
            }
    };
} // namespace IASLib

#endif // IASLIB_ENTITY_H__

#endif // IASLIB_NETWORKING__
