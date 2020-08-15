/**
 * Entity class
 *
 * This is the abstract base class for all returnable entity objects.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 23, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_ENTITY_H__
#define IASLIB_ENTITY_H__

#define IASLIB_MAX_DEFINED_ENTITIES 512

#define IASLIB_SET_ENTITY( mimeType )       virtual const char *getMimeType( void ) { return mimeType; }

#define IASLIB_REGISTER_ENTITY(className)   static className internalEntity; \
                                            static CRegisterEntityStub registerStub( &internalEntity );

#include "Collections/Hash.h"
#include "Streams/Stream.h"

namespace IASLib
{
    class CEntity : public CObject
    {
        private:
            static size_t entityCount;
            static CEntity *entities[IASLIB_MAX_DEFINED_ENTITIES];
            static CHash   m_entityMap;
            static bool    m_initialized;
        public:
            DEFINE_OBJECT( CEntity );

            CEntity( void );
            virtual ~CEntity( void );

            virtual CString toString( void ) = 0;

            static void registerEntity( CEntity *entity );

            static CEntity *getEntity( const char *mimeType, CStream &stream, size_t nContentLength );
            static bool isValidEntity( const char *mimeType );

            virtual const char *getMimeType( void ) = 0;

        protected:
            virtual CEntity *generateEntity( CStream &stream, size_t nContentLength ) = 0;
        private:
            static void initHash( void );

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
