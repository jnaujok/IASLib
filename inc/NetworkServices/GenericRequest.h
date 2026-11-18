/**
 * @file GenericRequest.h
 * @brief The shared base of HTTP and SIP requests (CGenericRequest).
 *
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
    /// A request message: request line, headers and body.
    ///
    /// This is the shared base of CHttpRequest and CSipRequest. An inbound
    /// request is built with the peer's address and filled by parse(); an
    /// outbound one is built empty, filled with the setters, and written with
    /// toString() or toStream().
    ///
    /// The request owns its header list and body entity and deletes them, so
    /// it cannot be copied. It does no locking.
    ///
    /// The request line is checked when set as well as when parsed: a method
    /// must be a token, and a request URI or version may not hold spaces or
    /// control characters (a CR LF in a URI would start a new header line).
    class CGenericRequest : public CObject
    {
        protected:
            bool            m_bIsValid;         ///< Whether the last parse succeeded (true for an outbound request).
            bool            m_bInbound;         ///< Whether the request was received rather than built to send.
            CString         m_requestType;      ///< The method, such as "GET" or "INVITE".
            CString         m_uri;              ///< The request URI.
            CString         m_version;          ///< The protocol version, such as "HTTP/1.1".
            CHeaderList    *m_pHeaders;         ///< The headers (owned), or NULL until parsed or first set.
            CInternetAddress  m_InternetAddress;  ///< The peer the request came from (inbound requests).
            CEntity        *m_bodyEntity;       ///< The body (owned), or NULL if none was parsed or set.
            CString         m_strParseError;    ///< The first reason parse() failed, or empty.

        public:
            DEFINE_OBJECT( CGenericRequest );

                /// Creates an inbound request from a peer, to be filled by
                /// parse(). It is not valid until parsed.
            CGenericRequest( const CInternetAddress &internetAddress );
                /// Creates an empty outbound request, to be filled with the setters.
            CGenericRequest( void );

                /// Deletes the header list and the body.
            virtual ~CGenericRequest( void );

                /// Not copyable: the request owns its header list and body.
                /// @note The compiler-made copies used to share the header list
                ///       and body, so the second destructor deleted them again.
            CGenericRequest( const CGenericRequest & ) = delete;
                /// Not assignable, for the same reason.
            CGenericRequest &operator =( const CGenericRequest & ) = delete;

                /// Parses one complete message.
                ///
                /// Blank lines before the request line (keep-alive CRLFs) are
                /// skipped. Any previous headers and body are deleted first, and
                /// the request is marked inbound. The body is Content-Length
                /// bytes when that header is present (bytes beyond it are
                /// ignored), otherwise all of the remaining bytes. It is read in
                /// pieces, so a stream that delivers it in parts gives all of it.
                ///
                /// Whatever could be parsed (request line, headers) is still
                /// available after a failure.
                /// @return true if the message is well formed; false, with the
                ///         reason in getParseError(), if not (empty message, bad
                ///         request line, malformed or unterminated headers,
                ///         invalid Content-Length, or a body shorter than it).
                /// @note A message cut short in its headers used to be accepted
                ///       with the headers it had, and the body was one read of a
                ///       Content-Length buffer, so a body arriving in parts over
                ///       a socket looked too short.
            virtual bool parse( CStream &requestStream );
                /// Same, for a message held in a string.
            virtual bool parse( const CString &message );

                /// Whether the request is well formed: true for an outbound
                /// request, otherwise the result of the last parse().
            virtual bool    isValid( void ) { return m_bIsValid; }
                /// Why the last parse() failed, or an empty string.
            virtual CString getParseError( void ) { return m_strParseError; }

                /// The method, such as "GET" or "INVITE".
            virtual CString getRequestType( void ) { return m_requestType; }
                /// Sets the method.
                /// @return false (nothing changes) if the method is not a token.
            virtual bool    setRequestType( const char *requestType );

                /// The request URI.
            virtual CString getUri( void ) { return m_uri; }
                /// Sets the request URI.
                /// @return false (nothing changes) if the URI is empty or holds a
                ///         space or a control character.
                /// @note Any text used to be taken, so a URI built from user
                ///       input could carry a CR LF and inject header lines.
            virtual bool    setUri( const char *uri );

                /// The protocol version, such as "HTTP/1.1" or "SIP/2.0".
            virtual CString getVersion( void ) { return m_version; }
                /// Sets the protocol version.
                /// @return false (nothing changes) if it is empty or holds a
                ///         space or a control character.
            virtual bool    setVersion( const char *version );

                /// Returns the first value of the header, or "" if it is absent.
            virtual CString getHeaderValue( const char *headerName );
                /// Replaces all values of the header with this one.
                /// @return false if the name or value is refused (see
                ///         CHeaderList::addHeader()).
            virtual bool    setHeaderValue( const char *headerName, const char *headerValue );

                /// All values of the header, in order (empty if absent).
            virtual CStringArray getHeaderValues( const char *headerName );
                /// Replaces all values of the header with these.
                /// @return false, leaving the header as it was, if the name or
                ///         any value is refused.
            virtual bool    setHeaderValues( const char *headerName, CStringArray headerValues );

                /// Whether the header is present.
            virtual bool    hasHeader( const char *headerName );
                /// Removes the header, if present.
            virtual void    removeHeader( const char *headerName );

                /// The headers of this request, created empty if needed. Never
                /// NULL; the request keeps ownership.
            virtual CHeaderList *getHeaders( void );

                /// The body, or NULL if none was parsed or set. The request
                /// keeps ownership. A parsed request always has one (a
                /// CNullEntity when the body is empty).
            virtual CEntity *getEntity( void ) { return m_bodyEntity; }
                /// Takes ownership of the entity and deletes any previous one.
            virtual void     setEntity( CEntity *entity );

                /// The peer the request came from.
            virtual CInternetAddress &getInternetAddress( void ) { return m_InternetAddress; }

                /// Whether the request was received (parsed) rather than built to send.
            virtual bool isInbound( void ) { return m_bInbound; }

                /// The whole message as it is sent on the wire.
            virtual CString toString( void ) = 0;
                /// Writes the whole message to the stream.
            virtual void toStream( CStream *pStream ) = 0;

                /// Creates the header list for a parsed message, reading the
                /// header lines from the stream. The caller takes ownership.
            virtual CHeaderList *getHeaderList( CStream &requestStream ) = 0;

        protected:
                /// Creates an empty header list of the right type for this
                /// protocol.
                ///
                /// Used when headers are set on a request that was not parsed.
                /// The caller takes ownership.
            virtual CHeaderList *newHeaderList( void ) { return new CHeaderList(); }

                /// Marks the request invalid and records the reason, unless an
                /// earlier reason is already recorded.
            void setParseError( const char *reason );

                /// The line without its trailing CR, if it has one.
            static CString stripLineEnding( const CString &line );
                /// Appends up to nWanted bytes from the stream (NOT_FOUND: to
                /// its end), reading in pieces until done or the stream ends.
            static void readBody( CStream &stream, std::string &body, size_t nWanted );

        public:
                /// Whether the text is a valid method: a token.
            static bool isValidMethod( const CString &method );
                /// Whether the text is a valid request URI: non-empty printable
                /// text without spaces (no CR, LF, NUL or other control
                /// characters).
            static bool isValidUri( const CString &uri );
                /// Whether the text is a valid version; the same rule as isValidUri().
            static bool isValidVersion( const CString &version ) { return isValidUri( version ); }
    };
} // namespace IASLib

#endif // IASLIB_GENERICREQUEST_H__

#endif // IASLIB_NETWORKING__
