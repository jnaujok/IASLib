/**
 * Raw Entity class
 *
 * Holds a body of a media type that has no registered entity class, such
 * as multipart/mixed or application/isup. The bytes and the original
 * Content-Type value are kept unchanged so they can be inspected, logged
 * or forwarded.
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_RAWENTITY_H__
#define IASLIB_RAWENTITY_H__

#include "NetworkServices/Entity.h"
#include "BaseTypes/String_.h"

namespace IASLib
{
    class CRawEntity : public CEntity
    {
        private:
            CString m_strContentType;
            CString m_strBody;

        public:
            DEFINE_OBJECT( CRawEntity );

            CRawEntity( const CString &contentType, const CString &body );
            virtual ~CRawEntity( void );

                // The Content-Type value the body arrived with, including any
                // parameters.
            virtual const char *getMimeType( void ) { return (const char *)m_strContentType; }
            virtual CString toString( void ) { return m_strBody; }

        protected:
            virtual CEntity *generateEntity( const CString &body );
    };
}

#endif // IASLIB_RAWENTITY_H__

#endif // IASLIB_NETWORKING__
