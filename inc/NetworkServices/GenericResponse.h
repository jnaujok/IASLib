/**
 * @file GenericResponse.h
 * @brief The shared base of HTTP and SIP responses (CGenericResponse).
 *
 * Generic Response class
 *
 * This class encapsulates a Generic response: a status code and reason,
 * headers and an optional body. It is the shared base of the HTTP and SIP
 * responses, which supply the status line.
 *
 * A response owns its body, so it cannot be copied.
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
    /// A response message: status code and reason, headers and an optional body.
    ///
    /// This is the shared base of CHttpResponse and CSipResponse, which supply
    /// the status line and write the message. A new response starts as
    /// 500 "Internal Server Error" with no headers and no body, so a handler
    /// that forgets to set a status does not report success.
    ///
    /// The response owns its body entity and deletes it, so it cannot be
    /// copied. The stream it is given is not owned. It does no locking.
    class CGenericResponse : public CObject
    {
        protected:
            int         m_nStatusCode;          ///< The status code, 500 until set.
            CString     m_strStatusDescription; ///< The reason phrase.
            CHeaderList m_headers;              ///< The headers.
            CEntity    *m_body;                 ///< The body (owned), or NULL for none.
            CStream    *m_pResponseStream;      ///< The stream the response is written to or read from (not owned), or NULL.
            bool        m_bInbound;             ///< Whether the response was received rather than built to send.

        public:
            DEFINE_OBJECT( CGenericResponse );

                /// Creates an outbound response with no stream.
            CGenericResponse( void );
                /// Creates an outbound response to be written to the stream.
                /// The stream is not owned and must outlive the response.
            CGenericResponse( CStream &responseStream );
                /// Creates an inbound response read from the stream. The address
                /// is not stored.
            CGenericResponse( CStream &inputStream, CInternetAddress &internetAddress );

                /// Deletes the body.
            virtual ~CGenericResponse( void );

                /// Not copyable: the response owns its body.
                /// @note The compiler-made copies used to share the body, so the
                ///       second destructor deleted it again.
            CGenericResponse( const CGenericResponse & ) = delete;
                /// Not assignable, for the same reason.
            CGenericResponse &operator =( const CGenericResponse & ) = delete;

                /// Sets the stream the response is written to. The stream is not
                /// owned and must outlive the response.
            virtual void setResponseStream( CStream &responseStream )
            {
                m_pResponseStream = &responseStream;
            }

                /// The stream given at construction or by setResponseStream(), or
                /// a shared null stream (which discards writes) if none was.
            virtual CStream &getResponseStream( void );

                /// Sets the status code and reason phrase.
                ///
                /// A reason phrase holding CR, LF or NUL (or NULL) is replaced by
                /// an empty one, as it would otherwise end the status line early.
                /// @note Any text used to be taken, so a reason with a CR LF in it
                ///       could add header lines to the response.
            virtual void setStatus( int nCode, const char *desc );

                /// Takes ownership of the entity and deletes any previous one.
            virtual void SetEntity( CEntity *entity );
                /// The body, or NULL if none was set. The response keeps ownership.
            virtual CEntity *getEntity( void ) { return m_body; }

                /// The status code.
            virtual int getStatusCode( void ) { return m_nStatusCode; }
                /// The reason phrase.
            virtual CString getStatusDescription( void ) { return m_strStatusDescription; }

                /// Replaces any values of the header with this one.
                /// @return false, and nothing changes, for a name that is not a
                ///         token or a value holding CR, LF or NUL (see
                ///         CHeaderList::addHeader()).
            virtual bool addHeader( const char *name, const char *value );
                /// Same, with the value written as a decimal number.
            virtual bool addHeader( const char *name, size_t nValue );
                /// Appends the values to the header (none if the name or any
                /// value is refused).
                /// @return false if refused.
            virtual bool addHeaders( const char *name, CStringArray values );
                /// Removes the header, if present.
            virtual void removeHeader( const char *name );
                /// Whether the header is present.
            virtual bool hasHeader( const char *name );

                /// Returns the first value of the header, or "" if it is absent.
            virtual CString getHeaderValue( const char *name );
                /// All values of the header, in order (empty if absent).
            virtual CStringArray getHeaderValues( const char *name );

                /// The headers of this response.
            virtual CHeaderList &getHeaders( void ) { return m_headers; }

                /// Whether the response was received rather than built to send.
            virtual bool isInbound( void ) { return m_bInbound; }

                /// Writes the whole message to the stream.
            virtual void toStream( CStream *pStream ) = 0;
                /// The whole message as it is sent on the wire.
            virtual CString toString( void ) = 0;

        protected:
                /// Builds the full message: the status line, the headers, a blank
                /// line and the body.
                ///
                /// Content-Length is set from the body unless there is a
                /// Transfer-Encoding header (the body is then sent as it is,
                /// already encoded, and any Content-Length is removed: RFC 9112
                /// 6.2 forbids both). Content-Type is filled in from the entity
                /// when there is a body but no Content-Type header. Both are
                /// stored in the headers, so this changes the response.
                /// @param statusLine The status line, which must end in CRLF.
                /// @note Content-Length used to be added always, so a chunked
                ///       response had both, which lets a proxy and a client
                ///       disagree on where the message ends.
            CString serialize( const CString &statusLine );
    };
} // namespace IASLib

#endif // IASLIB_GENERICRESPONSE_H__

#endif // IASLIB_NETWORKING__
