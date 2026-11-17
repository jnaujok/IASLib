/**
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
    class CSipUtils
    {
        public:
                // The branch prefix that marks an RFC 3261 compliant branch ID.
            static const char *MAGIC_COOKIE;

                // Splits a header value on commas that are not inside a quoted
                // string or angle brackets. Each element is trimmed.
            static CStringArray splitList( const CString &value );

                // Returns the value of a ";name=value" parameter of a header
                // value (parameters inside <...> are ignored). For a parameter
                // with no value (";rport") the result is "" and *pFound is true.
            static CString getParameter( const CString &headerValue, const char *name, bool *pFound = NULL );

                // Returns the header value with the parameter set, replacing an
                // existing one. A NULL value sets a parameter with no value.
            static CString setParameter( const CString &headerValue, const char *name, const char *value );

            static CString getTag( const CString &fromOrToValue ) { return getParameter( fromOrToValue, "tag" ); }

                // Splits a Via value into its transport ("UDP", "TCP", ...), host
                // and port. The port is 0 if the Via has none.
            static bool parseVia( const CString &via, CString &transport, CString &host, int &port );

                // Splits a CSeq value into its sequence number and method.
            static bool parseCSeq( const CString &cseq, unsigned long &number, CString &method );

                // The lower-case scheme of a URI ("sip", "sips", "tel", ...).
            static CString uriScheme( const CString &uri );

                // Random hexadecimal string of nBytes bytes (2 * nBytes digits),
                // from a properly seeded generator. Used for tags and nonces.
            static CString randomToken( size_t nBytes = 8 );

                // The RFC 3261 reason phrase for a status code.
            static const char *reasonPhrase( int nStatusCode );

                // True for methods defined by RFC 3261 and its extensions, used
                // to choose between 405 and 501 for methods with no handler.
            static bool isKnownMethod( const CString &method );

                // Finds the end of the first complete message in a stream buffer
                // (RFC 3261 18.3: headers end at a blank line and the body is
                // Content-Length bytes long). Leading CRLFs are skipped and
                // counted in nLeadingCrlf. Returns the length of the message
                // including the leading CRLFs, 0 if more data is needed, or -1 if
                // the headers are too long or the Content-Length is invalid.
            static long frameMessage( const char *pBuffer, size_t nLength, size_t &nLeadingCrlf, size_t nMaxMessage );
    };
} // namespace IASLib

#endif // IASLIB_SIPUTILS_H__

#endif // IASLIB_NETWORKING__
