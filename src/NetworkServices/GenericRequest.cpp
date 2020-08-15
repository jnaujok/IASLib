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

#include "NetworkServices/GenericRequest.h"
#include "NetworkServices/GenericServer.h"
#include "BaseTypes/StringTokenizer.h"
#include "Logging/LogSink.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CGenericRequest, CObject );

    CGenericRequest::CGenericRequest( CInternetAddress &internetAddress ) : m_InternetAddress( internetAddress )
    {
    }

    bool CGenericRequest::parse( CStream &requestStream )
    {
        m_bIsValid = false;
        m_bInbound = true;
        m_bIsValid = false;
        CString line = requestStream.GetLine();

        // Skip preceding blank lines
        while ( (line.Length() == 0) && (!requestStream.IsEOS() ))
        {
            line = requestStream.GetLine();
        }

        // Did we get a non-blank line before the end of stream?
        if ( line.Length() > 0 )
        {
            // Now that we have a non-blank line, break it into tokens based on spaces.
            // The SIP protocol (RFC 3261) explicitly requires space characters between
            // elements of the Request Line.
            // Request-Line  =  Method SP Request-URI SP SIP-Version CRLF
            CStringTokenizer stTokens(line.Trim(), " ");

            // Did we get tokens?
            if (stTokens.HasMoreTokens() )
            {
                // SIP commands should have 3 parts
                if ( stTokens.CountTokens() == 3 )
                {
                    m_requestType = stTokens.NextToken();
                    m_uri = stTokens.NextToken();
                    m_version = stTokens.NextToken();

                    // Once we have the request, we can go ahead and get the headers
                    // We use the pure-virtual to get the correct header list type
                    m_pHeaders = this->getHeaderList( requestStream );

                    // Look for the content-type header
                    CString contentType = m_pHeaders->firstValue( "content-type" );
                    if ( contentType.Length() > 0 )
                    {
                        contentType.ToLowerCase();
                        DEBUG_LOG( "Content-Type=[%s]", (const char *)contentType );
                    }
                    else
                    {
                        DEBUG_LOG( "No content-type found - defaulting to [text/plain]." );
                        contentType = "text/plain";
                    }
                    CString contentLength = m_pHeaders->firstValue( "content-length" );
                    size_t nContentLength = 0;

                    if ( contentLength.Length() > 0 )
                    {
                        nContentLength = (size_t)atoi( contentLength );
                    }
                    else
                    {
                        nContentLength = requestStream.bytesRemaining();
                    }

                    m_bodyEntity = CEntity::getEntity( contentType, requestStream, nContentLength );

                    m_bIsValid = true;
                }
                else
                {
                    // Return invalid request response
                    m_bIsValid = false;
                }

            }
            else
            {
                // Return invalid request response
                m_bIsValid = false;
            }
        }

        return m_bIsValid;
    }

    CGenericRequest::CGenericRequest( void )
    {
        m_bIsValid = true;
        m_bInbound = false;
    }

    CGenericRequest::~CGenericRequest( void )
    {
        if ( m_pHeaders )
            delete m_pHeaders;
        m_pHeaders = NULL;
        if ( m_bodyEntity )
            delete m_bodyEntity;
        m_bodyEntity = NULL;
    }

    void CGenericRequest::setRequestType( const char *requestType )
    {
        m_requestType = requestType;
    }

    void CGenericRequest::setUri( const char *uri )
    {

    }

    void CGenericRequest::setVersion( const char *version )
    {

    }

    CString CGenericRequest::getHeaderValue( const char *headerName )
    {

    }
    void CGenericRequest::setHeaderValue( const char *headerName, const char *headerValue )
    {

    }

    CStringArray CGenericRequest::getHeaderValues( const char *headerName )
    {

    }
    void CGenericRequest::setHeaderValues( const char *headerName, CStringArray headerValues )
    {

    }

    CString CGenericRequest::getHeaderValue( CString name )
    {

    }

    CStringArray CGenericRequest::getHeaderValues( CString name )
    {

    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
