/**
 * @file HeaderList.h
 * @brief An ordered, case-insensitive set of header fields (CHeaderList).
 *
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
    /// The header fields of an HTTP or SIP style message.
    ///
    /// Header names are matched case-insensitively. Headers keep the order in
    /// which they were first seen, and the values of a repeated header are kept
    /// in the order they arrived, each as a value of one CHeader. The list owns
    /// its CHeader objects and deletes them when cleared or destroyed.
    ///
    /// Every way of adding a header refuses a name that is not a token and a
    /// value holding CR, LF or NUL, so toString() always writes one line per
    /// value. Derived classes (CHttpHeaderList, CSipHeaderList) change how names
    /// are canonicalised and which CHeader type is created.
    ///
    /// The list does no locking; use one from one thread at a time.
    /// @code
    /// CHeaderList oHeaders;
    /// oHeaders.addHeader( "Content-Type", "text/plain" );
    /// oHeaders.appendHeader( "Via", "SIP/2.0/UDP a.example.com" );
    /// oHeaders.appendHeader( "via", "SIP/2.0/UDP b.example.com" );  // same header
    /// CString strType = oHeaders.firstValue( "content-type" );     // "text/plain"
    /// CString strWire = oHeaders.toString();  // "Content-Type: text/plain\r\nVia: ...\r\nVia: ...\r\n"
    /// @endcode
    class CHeaderList : public CObject
    {
        protected:
            CArray  m_aHeaders;         ///< CHeader * objects, owned, in first-seen order.
            bool    m_bModifiable;      ///< Set false by the parsing constructor, true otherwise. Nothing checks it.
            bool    m_bComplete;        ///< Parsing ended on the blank line that terminates headers.
            bool    m_bMalformed;       ///< Parsing found a line that is not a valid header.

        public:
            DEFINE_OBJECT( CHeaderList );

                /// Parses headers from the stream using the generic rules.
                ///
                /// Derived classes that need their own parsing rules should use
                /// the default constructor and then call parse() from their own
                /// constructor so that their overrides of canonicalName() are
                /// used.
                /// @param headerStream The stream to read from, positioned at the
                ///                     first header line.
            CHeaderList( CStream &headerStream );
                /// Makes a deep copy of another list, including its complete and
                /// malformed flags.
            CHeaderList( const CHeaderList &oSource );
                /// Creates an empty list.
            CHeaderList( void );
                /// Replaces this list's headers with deep copies of another's.
            CHeaderList &operator =( const CHeaderList &oSource );

                /// Deletes all the headers.
            virtual ~CHeaderList( void );

                /// Reads header lines up to and including the blank line that
                /// ends them.
                ///
                /// Handles CRLF or LF line endings, whitespace around the colon
                /// and folded continuation lines. A line whose name is not a
                /// token, that has no colon, or that holds a CR other than the
                /// one ending it, marks the list malformed and is skipped.
                /// Headers already in the list are kept; values of a header seen
                /// again are appended to it.
                /// @return true if the blank line was found before the end of the
                ///         stream (also available afterwards as isComplete()).
            virtual bool parse( CStream &headerStream );

                /// Maps a header name as received to the name it should be stored
                /// under.
                ///
                /// The default returns the name unchanged; SIP uses this to
                /// expand compact header forms.
            virtual CString canonicalName( const CString &name );

                /// Finds a header by name (case-insensitive, after canonicalName()).
                /// @return The header, still owned by the list, or NULL if there
                ///         is none.
            virtual CHeader *getHeader( const char *key );

                /// Sets a header to a single value, replacing any existing values
                /// of the header.
                ///
                /// The name is trimmed and passed through canonicalName(). A name
                /// that is not a token or a value holding CR, LF or NUL is
                /// refused, which would otherwise let a value smuggle extra
                /// header lines or a body into the message (see
                /// CHeader::isValidName() and CHeader::isValidValue()).
                /// @param key   The header name.
                /// @param value The value; NULL is taken as an empty value.
                /// @return true if stored, false (nothing changed) if refused.
                /// @note Names and values used to be stored as given, CR LF
                ///       included, so text from a client could add headers to a
                ///       response.
            virtual bool addHeader( const char *key, const char *value );
                /// Appends the values to the header, creating it if needed.
                ///
                /// If the name or any one of the values is refused, none of them
                /// are added.
                /// @return true if all values were added, false if refused.
            virtual bool addHeader( const char *key, const CStringArray &values );
                /// Appends one value to the header, creating it if needed.
                /// Refused like addHeader().
                /// @param key   The header name.
                /// @param value The value; NULL is taken as an empty value.
                /// @return true if stored, false (nothing changed) if refused.
            virtual bool appendHeader( const char *key, const char *value );

                /// Removes and deletes the named header, if present.
            virtual void removeHeader( const char *key );
                /// Whether the named header is present (case-insensitive).
            virtual bool hasHeader( const char *key );
                /// Removes and deletes all headers. The complete and malformed
                /// flags are not changed.
            virtual void clear( void );

                /// Returns all of the values of the named header (empty if none).
            virtual const CStringArray allValues( const char *name );

                /// Returns the first value of the named header, or an empty string.
            virtual CString     firstValue( const char *name );

                /// Returns the first value of the named header as a number, or 0.
                ///
                /// The value must be all digits once trimmed; anything else
                /// (a sign, a fraction, trailing text) gives 0.
            virtual long        firstValueAsLong( const char *name );

                /// Returns a map of header name -> CStringArray * of values.
                ///
                /// The hash map and its contents must be deleted by the caller.
            virtual CHash      *map( void );

                /// Iterates over the CHeader objects in order.
                ///
                /// The iterator must be deleted by the caller; the headers belong
                /// to the list.
            virtual CIterator  *Enumerate( void );

                /// The number of distinct headers (not values).
            virtual size_t      getHeaderCount( void ) { return m_aHeaders.Count(); }

                /// Every header value as a `"Name: value\r\n"` line, in order.
                ///
                /// A header with several values gives several lines. Names or
                /// values that are not valid (possible only through a derived
                /// class) are skipped rather than written.
            virtual CString     toString( void );

                /// Whether the last parse() ended on the blank line after the headers.
            virtual bool        isComplete( void ) { return m_bComplete; }
                /// Whether parsing skipped any line that was not a valid header.
            virtual bool        isMalformed( void ) { return m_bMalformed; }

        protected:
                /// Creates the header object used to store a new header.
                ///
                /// Derived lists override this to use their own CHeader type.
                /// The list takes ownership of the result.
            virtual CHeader    *newHeader( const CString &name );

                /// The index in m_aHeaders of the header with this lower-case key, or -1.
            int                 findIndex( const CString &key );
                /// Appends deep copies of another list's headers and copies its flags.
            void                copyFrom( const CHeaderList &oSource );
                /// The trimmed name, and whether it is a valid field name.
                /// @param key  The name to check; NULL is taken as empty.
                /// @param name Receives the trimmed name.
                /// @return true if the trimmed name is a token.
            static bool         validName( const char *key, CString &name );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__

#endif // IASLIB_HEADERLIST_H__
