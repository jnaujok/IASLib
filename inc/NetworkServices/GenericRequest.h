/**
 * Generic Request class
 *
 * This class encapsulates a Generic request, including the request line,
 * headers and body. It is the shared base of the HTTP and SIP requests.
 *
 * parse() expects the stream (or string) to hold exactly one complete
 * message, as a UDP datagram does or as a stream-transport reader produces
 * after framing the message. The body is taken from the bytes following
 * the headers: Content-Length bytes of it when that header is present, or
 * all of the remaining bytes when it is not (for a stream that cannot say
 * how many remain, everything up to its end).
 *
 * A request owns its header list and body, so it cannot be copied.
 *
 * The request line is checked when set as well as when parsed: a method
 * must be a token, and a request URI or version may not hold spaces or
 * control characters (a CR LF in a URI would start a new header line).
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

#include <string>

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
            CString         m_strParseError;

        public:
            DEFINE_OBJECT( CGenericRequest );

            CGenericRequest( const CInternetAddress &internetAddress );
            CGenericRequest( void );

            virtual ~CGenericRequest( void );

                // (The compiler-made copies shared the header list and body,
                // so the second destructor deleted them again.)
            CGenericRequest( const CGenericRequest & ) = delete;
            CGenericRequest &operator =( const CGenericRequest & ) = delete;

                // Parses one complete message. Returns false, and records the
                // reason in getParseError(), if the message is malformed. Whatever
                // could be parsed (request line, headers) is still available.
            virtual bool parse( CStream &requestStream );
            virtual bool parse( const CString &message );

            virtual bool    isValid( void ) { return m_bIsValid; }
            virtual CString getParseError( void ) { return m_strParseError; }

                // The setters refuse (false, nothing changes) a method that is
                // not a token, and a URI or version that is empty or holds a
                // space or a control character.
            virtual CString getRequestType( void ) { return m_requestType; }
            virtual bool    setRequestType( const char *requestType );

            virtual CString getUri( void ) { return m_uri; }
            virtual bool    setUri( const char *uri );

            virtual CString getVersion( void ) { return m_version; }
            virtual bool    setVersion( const char *version );

                // Returns the first value of the header, or "" if it is absent.
            virtual CString getHeaderValue( const char *headerName );
                // Replaces all values of the header with this one. False if
                // the name or value is refused (see CHeaderList::addHeader).
            virtual bool    setHeaderValue( const char *headerName, const char *headerValue );

            virtual CStringArray getHeaderValues( const char *headerName );
                // Replaces all values of the header with these (or, if any is
                // refused, leaves the header as it was and returns false).
            virtual bool    setHeaderValues( const char *headerName, CStringArray headerValues );

            virtual bool    hasHeader( const char *headerName );
            virtual void    removeHeader( const char *headerName );

                // The headers of this request. Never NULL.
            virtual CHeaderList *getHeaders( void );

            virtual CEntity *getEntity( void ) { return m_bodyEntity; }
                // Takes ownership of the entity and deletes any previous one.
            virtual void     setEntity( CEntity *entity );

            virtual CInternetAddress &getInternetAddress( void ) { return m_InternetAddress; }

            virtual bool isInbound( void ) { return m_bInbound; }

            virtual CString toString( void ) = 0;
            virtual void toStream( CStream *pStream ) = 0;

                // Creates the header list for a parsed message.
            virtual CHeaderList *getHeaderList( CStream &requestStream ) = 0;

        protected:
                // Creates an empty header list of the right type for this
                // protocol. Used when headers are set on a request that was not
                // parsed.
            virtual CHeaderList *newHeaderList( void ) { return new CHeaderList(); }

            void setParseError( const char *reason );

            static CString stripLineEnding( const CString &line );
                // Appends up to nWanted bytes from the stream (NOT_FOUND: to
                // its end).
            static void readBody( CStream &stream, std::string &body, size_t nWanted );

        public:
                // A method is a token; a URI or version is printable text
                // without spaces (no CR, LF, NUL or other control characters).
            static bool isValidMethod( const CString &method );
            static bool isValidUri( const CString &uri );
            static bool isValidVersion( const CString &version ) { return isValidUri( version ); }
    };
} // namespace IASLib

#endif // IASLIB_GENERICREQUEST_H__

#endif // IASLIB_NETWORKING__
