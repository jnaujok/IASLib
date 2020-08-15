/**
 * Generic Response class
 *
 * This class encapsulates a Generic response.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 19, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_GENERICRESPONSE_H__
#define IASLIB_GENERICRESPONSE_H__

#include "HeaderList.h"
#include "Entity.h"
#include "Sockets/InternetAddress.h"

namespace IASLib
{
    class CGenericResponse : public CObject
    {
        protected:
            int         m_nStatusCode;
            CString     m_strStatusDescription;
            CHeaderList m_headers;
            CEntity    *m_body;
            CStream    &m_responseStream;
            bool        m_bInbound;
        public:
            DEFINE_OBJECT( CGenericResponse );

            CGenericResponse( void );
            CGenericResponse( CStream &responseStream );
            CGenericResponse( CStream &inputStream, CInternetAddress &internetAddress );

            virtual void setResponseStream( CStream &responseStream )
            {
                m_responseStream = responseStream;
            }

            virtual CStream &getResponseStream( void )
            {
                return m_responseStream;
            }

            virtual ~CGenericResponse( void );

            virtual void setStatus( int nCode, const char *desc );

            virtual void SetEntity( CEntity *entity )
            {
                m_body = entity;
            }

            virtual int getStatusCode( void ) { return m_nStatusCode; }
            virtual CString getStatusDescription( void ) { return m_strStatusDescription; }

            virtual void addHeader( const char *name, const char *value );
            virtual void addHeader( const char *name, size_t nValue );
            virtual void addHeaders( const char *name, CStringArray values );

            virtual CString getHeaderValue( const char *name );
            virtual CStringArray getHeaderValues( const char *name );

            virtual bool isInbound( void ) { return m_bInbound; }

            virtual void toStream( CStream *pStream ) = 0;
            virtual CString toString( void ) = 0;
    };
} // namespace IASLib
#endif // IASLIB_GENERICRESPONSE_H__

#endif // IASLIB_NETWORKING__
