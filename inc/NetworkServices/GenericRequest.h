/**
 * Generic Request class
 *
 * This class encapsulates a Generic request, including the request line,
 * headers, and a handle to the stream with the request.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 19, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_GENERICREQUEST_H__
#define IASLIB_GENERICREQUEST_H__

#include "HeaderList.h"
#include "Entity.h"
#include "Sockets/InternetAddress.h"

namespace IASLib
{
    class CGenericRequest : public CObject
    {
        protected:
            bool            m_bIsValid;
            bool            m_bInbound;
            CString         m_requestType;
            CString         m_uri;
            CString         m_version;
            CHeaderList    *m_pHeaders;
            CInternetAddress  m_InternetAddress;
            CEntity        *m_bodyEntity;

        public:
            DEFINE_OBJECT( CGenericRequest );

            CGenericRequest( CInternetAddress &internetAddress );
            CGenericRequest( void );

            virtual ~CGenericRequest( void );

            virtual bool parse( CStream &requestStream );

            virtual CString getRequestType( void ) { return m_requestType; }
            virtual void    setRequestType( const char *requestType );

            virtual CString getUri( void ) { return m_uri; }
            virtual void    setUri( const char *uri );

            virtual CString getVersion( void ) { return m_version; }
            virtual void    setVersion( const char *version );

            virtual CString getHeaderValue( const char *headerName );
            virtual void    setHeaderValue( const char *headerName, const char *headerValue );

            virtual CStringArray getHeaderValues( const char *headerName );
            virtual void    setHeaderValues( const char *headerName, CStringArray headerValues );

            virtual CString getHeaderValue( CString name );
            virtual CStringArray getHeaderValues( CString name );

            virtual CEntity *getEntity( void ) { return m_bodyEntity; }
            virtual void     setEntity( CEntity *entity ) { m_bodyEntity = entity; }

            virtual CInternetAddress &getInternetAddress( void ) { return m_InternetAddress; }

            virtual bool isInbound( void ) { return m_bInbound; }

            virtual CString toString( void ) = 0;
            virtual void toStream( CStream *pStream ) = 0;

            virtual CHeaderList *getHeaderList( CStream &requestStream ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_GENERICREQUEST_H__

#endif // IASLIB_NETWORKING__
