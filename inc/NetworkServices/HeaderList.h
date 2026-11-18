/**
 * Header List class
 *
 * This class encapsulates a set of header fields in an HTTP or SIP
 * style request or response. Header names are case-insensitive, the
 * order in which headers were first seen is preserved, and the values of
 * repeated headers are kept in the order they were received (which matters
 * for headers such as SIP's Via and Record-Route).
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_HEADERLIST_H__
#define IASLIB_HEADERLIST_H__

#ifdef IASLIB_NETWORKING__

#include "BaseTypes/String_.h"
#include "Collections/Array.h"
#include "Collections/Hash.h"
#include "Collections/StringArray.h"
#include "Streams/Stream.h"
#include "NetworkServices/Header.h"

namespace IASLib
{
    class CHeaderList : public CObject
    {
        protected:
            CArray  m_aHeaders;         // CHeader * objects, owned, in first-seen order.
            bool    m_bModifiable;
            bool    m_bComplete;        // Parsing ended on the blank line that terminates headers.
            bool    m_bMalformed;       // Parsing found a line that is not a valid header.

        public:
            DEFINE_OBJECT( CHeaderList );

                // Parses headers from the stream using the generic rules. Derived
                // classes that need their own parsing rules should use the default
                // constructor and then call parse() from their own constructor so
                // that their overrides of canonicalName() are used.
            CHeaderList( CStream &headerStream );
            CHeaderList( const CHeaderList &oSource );
            CHeaderList( void );
            CHeaderList &operator =( const CHeaderList &oSource );

            virtual ~CHeaderList( void );

                // Reads header lines up to and including the blank line that ends
                // them. Handles CRLF or LF line endings, whitespace around the
                // colon and folded continuation lines. Returns true if the blank
                // line was found before the end of the stream. A line whose
                // name is not a token, or that holds a CR other than the one
                // ending it, marks the list malformed and is skipped.
            virtual bool parse( CStream &headerStream );

                // Maps a header name as received to the name it should be stored
                // under. The default returns the name unchanged; SIP uses this to
                // expand compact header forms.
            virtual CString canonicalName( const CString &name );

            virtual CHeader *getHeader( const char *key );

                // These refuse (return false, changing nothing) a name that is
                // not a token or a value holding CR, LF or NUL, which would
                // otherwise let a value smuggle extra header lines or a body
                // into the message (see CHeader::isValidName/isValidValue).

                // Replaces any existing values of the header with this one.
            virtual bool addHeader( const char *key, const char *value );
                // Appends the values to the header, creating it if needed (none
                // of them if any one is refused).
            virtual bool addHeader( const char *key, const CStringArray &values );
                // Appends one value to the header, creating it if needed.
            virtual bool appendHeader( const char *key, const char *value );

            virtual void removeHeader( const char *key );
            virtual bool hasHeader( const char *key );
            virtual void clear( void );

                // Returns all of the values of the named header (empty if none).
            virtual const CStringArray allValues( const char *name );

                // Returns the first value of the named header, or an empty string.
            virtual CString     firstValue( const char *name );

                // Returns the first value of the named header as a number, or 0.
            virtual long        firstValueAsLong( const char *name );

                // Returns a map of header name -> CStringArray * of values. The
                // hash map and its contents must be deleted by the caller.
            virtual CHash      *map( void );

                // Iterates over the CHeader objects in order. The iterator must be
                // deleted by the caller; the headers belong to the list.
            virtual CIterator  *Enumerate( void );

            virtual size_t      getHeaderCount( void ) { return m_aHeaders.Count(); }

                // Every header value as a "Name: value\r\n" line, in order.
            virtual CString     toString( void );

            virtual bool        isComplete( void ) { return m_bComplete; }
            virtual bool        isMalformed( void ) { return m_bMalformed; }

        protected:
                // Creates the header object used to store a new header.
            virtual CHeader    *newHeader( const CString &name );

            int                 findIndex( const CString &key );
            void                copyFrom( const CHeaderList &oSource );
                // The trimmed name, and whether it is a valid field name.
            static bool         validName( const char *key, CString &name );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__

#endif // IASLIB_HEADERLIST_H__
