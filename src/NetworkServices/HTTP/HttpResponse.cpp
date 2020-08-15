/**
 * HTTP Response class
 *
 * This class encapsulates an HTTP based response.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 28, 2019
 *
 */

#include "NetworkServices/HTTP/HttpResponse.h"

namespace IASLib
{
    IMPLEMENT_OBJECT(CHttpResponse, CGenericResponse );

    CHttpResponse::CHttpResponse( void ) : CGenericResponse(), m_remoteAddress()
    {

    }

    CHttpResponse::CHttpResponse( CStream &responseStream, CInternetAddress &internetAddress ) : CGenericResponse( responseStream )
    {
    }

    CHttpResponse::~CHttpResponse( void )
    {
        if ( m_body )
            delete m_body;
        m_body = NULL;
    }

 
    void CHttpResponse::toStream( CStream *pStream )
    {
        CString statusLine;
        statusLine.Format( "HTTP/1.1 %d %s\r\n", (int)m_nStatusCode, (const char *)m_strStatusDescription );
        pStream->PutLine( statusLine );

        pStream->PutLine( this->m_headers.toString() );
        pStream->PutLine( "\r\n" );
        if ( m_body )
        {
            pStream->PutLine( this->m_body->toString() );
        }
    }

    CString CHttpResponse::toString( void )
    {
        CString strRetVal;
        CString statusLine;
        statusLine.Format( "HTTP/1.1 %d %s\n", (int)m_nStatusCode, (const char *)m_strStatusDescription );
        strRetVal += statusLine;

        strRetVal += this->m_headers.toString();
        strRetVal += "\n";
        if ( m_body )
        {
            strRetVal += this->m_body->toString();
        }
        
        return strRetVal;
    }
} // namespace IASLib
