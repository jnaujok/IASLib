/**
 * Generic Response class
 *
 * This class encapsulates a Generic response: a status code and reason,
 * headers and an optional body. It is the shared base of the HTTP and SIP
 * responses, which supply the status line.
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
            CStream    *m_pResponseStream;
            bool        m_bInbound;

        public:
            DEFINE_OBJECT( CGenericResponse );

            CGenericResponse( void );
            CGenericResponse( CStream &responseStream );
            CGenericResponse( CStream &inputStream, CInternetAddress &internetAddress );

            virtual ~CGenericResponse( void );

            virtual void setResponseStream( CStream &responseStream )
            {
                m_pResponseStream = &responseStream;
            }

                // The stream given at construction, or a null stream if none was.
            virtual CStream &getResponseStream( void );

            virtual void setStatus( int nCode, const char *desc );

                // Takes ownership of the entity and deletes any previous one.
            virtual void SetEntity( CEntity *entity );
            virtual CEntity *getEntity( void ) { return m_body; }

            virtual int getStatusCode( void ) { return m_nStatusCode; }
            virtual CString getStatusDescription( void ) { return m_strStatusDescription; }

                // Replaces any values of the header with this one.
            virtual void addHeader( const char *name, const char *value );
            virtual void addHeader( const char *name, size_t nValue );
                // Appends the values to the header.
            virtual void addHeaders( const char *name, CStringArray values );
            virtual void removeHeader( const char *name );
            virtual bool hasHeader( const char *name );

                // Returns the first value of the header, or "" if it is absent.
            virtual CString getHeaderValue( const char *name );
            virtual CStringArray getHeaderValues( const char *name );

            virtual CHeaderList &getHeaders( void ) { return m_headers; }

            virtual bool isInbound( void ) { return m_bInbound; }

            virtual void toStream( CStream *pStream ) = 0;
            virtual CString toString( void ) = 0;

        protected:
                // Builds the full message: the status line (which must end in
                // CRLF), the headers, a blank line and the body. Content-Length is
                // always set from the body, and Content-Type is filled in from the
                // entity when there is a body but no Content-Type header.
            CString serialize( const CString &statusLine );
    };
} // namespace IASLib

#endif // IASLIB_GENERICRESPONSE_H__

#endif // IASLIB_NETWORKING__
