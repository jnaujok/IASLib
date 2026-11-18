/**
 * @file Header.h
 * @brief A single named header field with one or more values (CHeader).
 *
 * Header class
 *
 * This class encapsulates a single generic header for HTTP or
 * SIP style transactions. It stores one or more values attached
 * to a named header.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HEADER_H__
#define IASLIB_HEADER_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"
#include "Collections/StringArray.h"

namespace IASLib
{
    /// One header field of an HTTP or SIP style message: a name and its values.
    ///
    /// A header can hold several values; a repeated header (two "Via" lines,
    /// say) is stored as one CHeader with a value per line. The name keeps the
    /// spelling it was given (trimmed); getKey() gives the lower-case form
    /// used for case-insensitive lookups. Values holding CR, LF or NUL are
    /// refused, so a header can always be written out as single lines.
    ///
    /// A CHeader is a plain value object with no locking; normally it is owned
    /// by a CHeaderList.
    /// @code
    /// CHeader oHeader( "Accept" );
    /// oHeader.addValue( "text/html" );
    /// oHeader.addValue( "application/json" );
    /// CString strFirst = oHeader.getValue();   // "text/html"
    /// @endcode
    class CHeader : public CObject
    {
        private:
            CString name;           ///< The name as given, trimmed.
            CString key;            ///< Lower-case version of the name, used for lookups.
            CStringArray values;    ///< The values, in the order they were added.

        public:
            DEFINE_OBJECT(CHeader);

                /// Creates a header with no values.
                /// @param name The header name; surrounding whitespace is trimmed.
                ///             The name is not checked here (see isValidName()).
            CHeader( const CString &name );
                /// Creates a header with an empty name and no values.
            CHeader( void );
                /// Copies the name, key and values of another header.
            CHeader( const CHeader &oSource );
                /// Replaces this header's name, key and values with a copy of another's.
            CHeader &operator =( const CHeader &oSource );

                /// Destroys the header.
            virtual ~CHeader( void );

                /// The header name as given (trimmed).
            virtual CString getName( void ) const;
                /// Renames the header (trimmed) and recomputes its key. The
                /// values are kept and the name is not checked.
            virtual void setName( const CString &name );

                /// The lower-case form of the name. Header names are case-insensitive.
            virtual CString getKey( void ) const;

                /// Returns the first value, or an empty string if there are none.
            virtual CString getValue( void ) const;
                /// Replaces all values with this one.
                ///
                /// A value holding CR, LF or NUL is refused (false, and nothing
                /// changes): written out, it would end the header line early and
                /// let the rest be read as more headers or a body (header
                /// injection).
                /// @return true if the value was stored.
                /// @note Any value used to be stored, so text passed on from a
                ///       client could carry a CR LF and further headers into the
                ///       output.
            virtual bool setValue( const CString &value );
                /// Adds a value after the existing ones. Refused like setValue().
                /// @return true if the value was stored.
            virtual bool addValue( const CString &value );
                /// A copy of all the values, in the order they were added.
            virtual CStringArray getValues( void ) const;
                /// The number of values.
            virtual size_t getValueCount( void ) const;

                /// Used by parsers to append a folded (continuation) line to the
                /// most recently added value.
                ///
                /// The text is joined to the last value with a single space (no
                /// space if the last value is empty); with no values yet it
                /// becomes the first value. Text holding CR, LF or NUL is
                /// silently ignored, as addValue() would refuse it.
            virtual void appendToLastValue( const CString &text );

                /// The lookup key for a name: the name trimmed and in lower case.
            static CString makeKey( const CString &name );

                /// Whether the text is a valid field name.
                ///
                /// A field name is a token (RFC 9110 5.6.2, RFC 3261 25.1):
                /// letters, digits and the characters ! # $ % & ' * + - . ^ _ |
                /// ~ and the backtick, at least one of them.
            static bool isValidName( const CString &name );
                /// Whether the text is a valid field value. A field value may
                /// hold anything but CR, LF and NUL.
            static bool isValidValue( const CString &value );
                /// Whether the text holds CR, LF or NUL (looking at every byte,
                /// past any NUL).
            static bool hasLineBreakOrNul( const CString &text );
                /// Whether the character is a token character (see isValidName()).
            static bool isTokenChar( char ch );
    };
} // namespace IASLib

#endif // IASLIB_HEADER_H__

#endif // IASLIB_NETWORKING__
