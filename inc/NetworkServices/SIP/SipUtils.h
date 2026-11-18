/**
 * @file SipUtils.h
 * @brief Helpers for SIP header values, framing and reason phrases (CSipUtils).
 *
 * SIP Utilities
 *
 * Helper functions for working with SIP header values (RFC 3261): header
 * parameters, Via and CSeq values, tags, comma-separated lists, URI
 * schemes, random tokens and standard reason phrases.
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPUTILS_H__
#define IASLIB_SIPUTILS_H__

#include "BaseTypes/String_.h"
#include "Collections/StringArray.h"

namespace IASLib
{
    /// Static helpers for SIP (RFC 3261) header values and message framing.
    ///
    /// They work on the text of header values: lists, parameters, Via and
    /// CSeq values, tags and URI schemes. All functions are stateless (except
    /// randomToken(), which locks its generator) and thread-safe.
    /// @code
    /// CString strVia = "SIP/2.0/UDP pc33.example.com:5060;branch=z9hG4bK776asdhds;rport";
    /// CString strBranch = CSipUtils::getParameter( strVia, "branch" );   // "z9hG4bK776asdhds"
    /// bool bRport = false;
    /// CSipUtils::getParameter( strVia, "rport", &bRport );                 // bRport is true
    /// strVia = CSipUtils::setParameter( strVia, "received", "192.0.2.4" );
    /// @endcode
    class CSipUtils
    {
        public:
                /// The branch prefix that marks an RFC 3261 compliant branch ID ("z9hG4bK").
            static const char *MAGIC_COOKIE;

                /// Splits a header value on commas that are not inside a quoted
                /// string or angle brackets. Each element is trimmed, and empty
                /// elements are dropped.
            static CStringArray splitList( const CString &value );

                /// Returns the value of a ";name=value" parameter of a header
                /// value.
                ///
                /// Parameters inside <...> are ignored, and the name is matched
                /// without regard to case. For a parameter with no value
                /// (";rport") the result is "" and *pFound is true.
                /// @param headerValue The header value.
                /// @param name        The parameter name.
                /// @param pFound      If not NULL, set to whether the parameter is present.
                /// @return The parameter's value (trimmed), or "".
            static CString getParameter( const CString &headerValue, const char *name, bool *pFound = NULL );

                /// Returns the header value with the parameter set, replacing an
                /// existing one (in place) or adding it at the end.
                /// A NULL value sets a parameter with no value.
            static CString setParameter( const CString &headerValue, const char *name, const char *value );

                /// The tag parameter of a From or To value, or "".
            static CString getTag( const CString &fromOrToValue ) { return getParameter( fromOrToValue, "tag" ); }

                /// Splits a Via value into its transport, host and port.
                /// @param via       One Via value (one element of the list).
                /// @param transport Set to the transport in upper case ("UDP", "TCP", ...).
                /// @param host      Set to the host, without brackets for IPv6.
                /// @param port      Set to the port, or 0 if the Via has none.
                /// @return false if the value is not a valid Via.
            static bool parseVia( const CString &via, CString &transport, CString &host, int &port );

                /// Splits a CSeq value into its sequence number and method.
                /// @return false if the number is missing or over 2**31 - 1
                ///         (RFC 3261 8.1.1.5), or the method is missing.
            static bool parseCSeq( const CString &cseq, unsigned long &number, CString &method );

                /// The lower-case scheme of a URI ("sip", "sips", "tel", ...),
                /// or "" if there is none. A leading '<' is skipped.
            static CString uriScheme( const CString &uri );

                /// Random hexadecimal string of nBytes bytes (2 * nBytes digits),
                /// from a properly seeded generator. Used for tags and nonces.
            static CString randomToken( size_t nBytes = 8 );

                /// The RFC 3261 reason phrase for a status code.
                ///
                /// An unlisted code gets a generic phrase for its class, such as
                /// "Request Failure". Never NULL.
            static const char *reasonPhrase( int nStatusCode );

                /// True for methods defined by RFC 3261 and its extensions, used
                /// to choose between 405 and 501 for methods with no handler.
                /// The match is case-sensitive.
            static bool isKnownMethod( const CString &method );

                /// Finds the end of the first complete message in a stream buffer.
                ///
                /// RFC 3261 18.3: headers end at a blank line and the body is
                /// Content-Length (or "l") bytes long; no Content-Length means
                /// no body. Leading CRLFs are skipped and counted in
                /// nLeadingCrlf.
                /// @param pBuffer      The bytes received so far.
                /// @param nLength      The number of bytes in pBuffer.
                /// @param nLeadingCrlf Set to the number of line-end bytes before
                ///                     the message.
                /// @param nMaxMessage  The largest message accepted, in bytes.
                /// @return The length of the message including the leading
                ///         CRLFs, 0 if more data is needed, or -1 if the message
                ///         is too long or the Content-Length is invalid.
            static long frameMessage( const char *pBuffer, size_t nLength, size_t &nLeadingCrlf, size_t nMaxMessage );
    };
} // namespace IASLib

#endif // IASLIB_SIPUTILS_H__

#endif // IASLIB_NETWORKING__
