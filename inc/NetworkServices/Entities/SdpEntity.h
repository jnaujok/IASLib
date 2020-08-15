/**
 * SDP Entity class
 *
 * This class encapsulates the Session Description Protocol as an entity
 * within the Request or Response object. 
 *
 * Author: Jeffrey R. Naujok
 * Created: July 30, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SDPENTITY_H__
#define IASLIB_SDPENTITY_H__

#include "NetworkServices/Entity.h"
#include "BaseTypes/String_.h"
#include "Collections/Array.h"
#include "Streams/Stream.h"

namespace IASLib
{
    class CSDPEntity : public CEntity
    {
        private:
            CArray      m_SessionValues;
        public:
                        IASLIB_SET_ENTITY( "application/sdp" );

                        CSDPEntity( CStream &oStream );
                        CSDPEntity( void );
            virtual    ~CSDPEntity( void );

            void        addAttribute( const char *name, const char *value );
            CString     getAttribute( const char *name );

            virtual CString     toString( void );

        protected:
            virtual CEntity *generateEntity( CStream &stream, size_t nContentLength );
    };
} // namespace IASLib

#endif // IASLIB_SDPENTITY_H__

#endif // IASLIB_NETWORKING__
