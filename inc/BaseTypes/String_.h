/**
 * @file
 * @brief CString Class.
 *
 *      This class provides an interface to a string class which mimics
 * an intrinsic string type for C++. This class handles memory management
 * and dynamic re-allocation of string storage and the like. It also
 * provides a very intrinsic acting interface for using strings.
 *
 *      Copies share their text until one of them changes (see
 * CStringStub), and the sharing is thread-safe. Strings hold text without
 * NUL characters: text is taken up to its first NUL.
 *
 *      Numbers become text in the C locale ('.' for the decimal point),
 * with doubles and floats in the shortest form that reads back as the same
 * value ("0.1", "2", "1e+100").
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/02/1992
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STRING_H__
#define IASLIB_STRING_H__

#include "Object.h"
#include "StringStub.h"

#ifdef IASLIB_STD_STREAM_SUPPORT__
#include <istream>
#include <ostream>
#endif

namespace IASLib
{
    /// A reference-counted, copy-on-write text string.
    ///
    /// CString manages its own storage and grows as needed. Copies share
    /// the same CStringStub until one of them is changed; the sharing is
    /// thread-safe, but one CString object must not be changed from two
    /// threads at once. Text never holds NUL characters: text given to a
    /// CString is taken up to its first NUL. Numbers become text in the C
    /// locale.
    ///
    /// @code
    /// CString strName = "world";
    /// CString strGreeting = "Hello, " + strName + "!";   // "Hello, world!"
    /// strGreeting += 42;                                  // "Hello, world!42"
    /// size_t nAt = strGreeting.IndexOf( "world" );        // 7
    /// @endcode
    class CString : public CObject
    {
            // Data used to store a string in our current object implementation
            // (NULL for an empty string).
        private:
            CStringStub    *m_pStubData;        ///< The shared text, or NULL for an empty string.

            // Methods used to interface with the string class
        public:
                            /// Default constructor. Creates a zero length string.
                        CString( void );

                            /// String Constructor - Takes a char * and converts
                            /// it to a CString object: its first nLength
                            /// characters (all of it if nLength is negative, or
                            /// the text is shorter).
                        CString( const IASLibChar__ *strSource, int nLength = -1 );
                        CString( const IASLibChar__ *strSource, size_t nLength );     ///< Same, with a size_t length.

                            /// Fixed Buffer String Constructor - a string kept in
                            /// the program's buffer of nSize bytes (so at most
                            /// nSize - 1 characters), starting with its first
                            /// nLength characters (to the first NUL if negative).
                            /// The buffer is never moved: text that does not fit
                            /// is cut off, and every copy of the string shares
                            /// the buffer, so a change through one is seen by
                            /// all. With a NULL buffer, the string allocates one
                            /// for nSize characters itself.
                        CString( IASLibChar__ *strSource, int nLength, int nSize );
                        CString( IASLibChar__ *strSource, size_t nLength, size_t nSize );  ///< Same, with size_t sizes.

                            /// Copy Constructor - shares the source's text.
                        CString( const CString &strSource );
                            /// Move Constructor - takes the source's text, leaving
                            /// it empty.
                        CString( CString &&strSource );

                            /// Destructor - Destroys a string obejct
            virtual    ~CString( void );

                        DEFINE_OBJECT( CString )

                // Operator Methods
            /// @name Concatenation
            /// Concatenation Operators (+) - Combines two strings into a new
            /// one. A number is added as its text; a NUL character or a NULL
            /// char * adds nothing.
            ///@{
                            /// The two strings, one after the other.
            friend CString operator +( const CString &strSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const IASLibChar__ *strSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const IASLibChar__ *strSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const IASLibChar__ chSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const IASLibChar__ chSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const short nSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const short nSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const int nSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const int nSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const unsigned int nSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const unsigned int nSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const long nSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const long nSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const unsigned long nSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const unsigned long nSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const long long nSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const long long nSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const unsigned long long nSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const unsigned long long nSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            friend CString operator +( const CString &strSource1, const double dSource2 );   ///< Same, with a value on the right.
            friend CString operator +( const double dSource1, const CString &strSource2 );   ///< Same, with a value on the left.
            ///@}

            /// @name Appending
            /// Concatenate / Assign - Concatenates the value onto the
            /// current string and assigns the result. A number is appended
            /// as its text; a NUL character or a NULL char * appends nothing.
            ///@{
                            /// Appends @p strSource. Returns this string.
            CString        &operator +=( const CString &strSource );   ///< Same, for another type.
            CString        &operator +=( const IASLibChar__ *strSource );   ///< Same, for another type.
            CString        &operator +=( const IASLibChar__ chSource );   ///< Same, for another type.
            CString        &operator +=( const short nSource );   ///< Same, for another type.
            CString        &operator +=( const int  nSource );   ///< Same, for another type.
            CString        &operator +=( const unsigned int nSource );   ///< Same, for another type.
            CString        &operator +=( const long nSource );   ///< Same, for another type.
            CString        &operator +=( const unsigned long nSource );   ///< Same, for another type.
            CString        &operator +=( const long long nSource );   ///< Same, for another type.
            CString        &operator +=( const unsigned long long nSource );   ///< Same, for another type.
            CString        &operator +=( const double dSource );   ///< Same, for another type.

            ///@}

            /// @name Assignment
            /// Assignment Operators (=) - Assigns a string. A number is stored
            /// as its text, a bool as "true" or "false", and a NUL character or
            /// a NULL char * gives an empty string.
            ///@{
                            /// Shares @p strSource's text (a fixed-buffer string
                            /// copies it into its buffer instead). Returns this string.
            CString        &operator =( const CString &strSource );   ///< Same, for another type.
                            /// Takes @p strSource's text, leaving it empty.
            CString        &operator =( CString &&strSource );   ///< Same, for another type.
            CString        &operator =( const IASLibChar__ *strSource );   ///< Same, for another type.
            CString        &operator =( const IASLibChar__ chSource );   ///< Same, for another type.
            CString        &operator =( const short nSource );   ///< Same, for another type.
            CString        &operator =( const int  nSource );   ///< Same, for another type.
            CString        &operator =( const unsigned int nSource );   ///< Same, for another type.
            CString        &operator =( const long nSource );   ///< Same, for another type.
            CString        &operator =( const unsigned long nSource );   ///< Same, for another type.
            CString        &operator =( const long long nSource );   ///< Same, for another type.
            CString        &operator =( const unsigned long long nSource );   ///< Same, for another type.
            CString        &operator =( const float fSource );   ///< Same, for another type.
            CString        &operator =( const double dSource );   ///< Same, for another type.
            CString        &operator =( const bool bSource );   ///< Same, for another type.
            ///@}

                            /// Typecast Operator - Types a CString to a char *
                            /// (never NULL: "" for an empty string). The pointer
                            /// is valid until the string is changed or destroyed.
                            operator const IASLibChar__ *() const;
                            /// The text as a char *; same as the typecast.
            const IASLibChar__ *c_str( void ) const { return (const IASLibChar__ *)(*this); }

            /// @name Comparison
            /// Comparison Operators - compare the text, case
            /// sensitively. A NULL char * compares as "".
            ///@{
                            /// True if the texts are the same.
            bool        operator ==( const CString &strString2 ) const { return Compare( strString2 ) == 0; }
            bool        operator ==( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) == 0; }   ///< Same, for this operator.
            bool        operator !=( const CString &strString2 ) const { return Compare( strString2 ) != 0; }   ///< Same, for this operator.
            bool        operator !=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) != 0; }   ///< Same, for this operator.
            bool        operator >=( const CString &strString2 ) const { return Compare( strString2 ) >= 0; }   ///< Same, for this operator.
            bool        operator >=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) >= 0; }   ///< Same, for this operator.
            bool        operator <=( const CString &strString2 ) const { return Compare( strString2 ) <= 0; }   ///< Same, for this operator.
            bool        operator <=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) <= 0; }   ///< Same, for this operator.
            bool        operator >( const CString &strString2 ) const { return Compare( strString2 ) > 0; }   ///< Same, for this operator.
            bool        operator >( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) > 0; }   ///< Same, for this operator.
            bool        operator <( const CString &strString2 ) const { return Compare( strString2 ) < 0; }   ///< Same, for this operator.
            bool        operator <( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) < 0; }   ///< Same, for this operator.

            friend bool operator ==( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) == 0; }   ///< Same, with the char * on the left.
            friend bool operator !=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) != 0; }   ///< Same, with the char * on the left.
            friend bool operator >=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) <= 0; }   ///< Same, with the char * on the left.
            friend bool operator <=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) >= 0; }   ///< Same, with the char * on the left.
            friend bool operator >( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) < 0; }   ///< Same, with the char * on the left.
            friend bool operator <( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) > 0; }   ///< Same, with the char * on the left.
            ///@}

                            /// Lexical Comparison - Compares two strings and returns
                            /// 0 if equal, -1 if this is less than the comparison, and
                            /// +1 if this is greater than the comparison. Characters
                            /// compare as unsigned values.
                            /// @param strCompare The string to compare with.
                            /// @param bCaseSensitive False to ignore letter case.
            int         Compare( const CString &strCompare, bool bCaseSensitive = true ) const;
                            /// Same, with a char * (NULL compares as "").
            int         Compare( const IASLibChar__ *strCompare, bool bCaseSensitive = true ) const;

                            /// Array Access Operator - the character at nIndex
                            /// ('\0' past the end).
            IASLibChar__        operator []( int nIndex ) const;
            IASLibChar__        operator []( size_t nIndex ) const;     ///< Same, with a size_t index.
                            /// The character at nIndex, as one that can be changed
                            /// (in this string only); past the end, a scratch
                            /// character that is not part of the string.
            IASLibChar__       &operator []( int nIndex );
            IASLibChar__       &operator []( size_t nIndex );           ///< Same, with a size_t index.


                // Standard Methods

                            /// Get the length of a string, in characters.
            size_t      GetLength( void ) const
            {
                return ( m_pStubData ) ? m_pStubData->m_nLength : 0;
            }
            size_t      Length( void ) const { return GetLength(); }      ///< Same as GetLength().
            size_t      GetCount( void ) const { return GetLength(); }    ///< Same as GetLength().
            size_t      Count( void ) const { return GetLength(); }       ///< Same as GetLength().
            bool        IsEmpty( void ) const { return GetLength() == 0; } ///< True if the string has no characters.

                            /// Room for nCapacity characters before the string must
                            /// grow (for building a long string piece by piece).
            void        Reserve( size_t nCapacity );
                            /// Gives back room the string does not use (an empty
                            /// string frees its storage; shared text is left alone).
            void        Compact( void );

                            /// Get a sub-string of the string: nLength characters
                            /// from nStartIndex (to the end if nLength is negative,
                            /// or longer than what is left). Empty if nStartIndex
                            /// is at or past the end.
            CString     Substring( size_t nStartIndex, int nLength = -1 ) const;

                            /// Check if a string is all digits (false if it is empty)
            bool        IsDigits( void ) const;

                            /// True if the string begins with @p strPrefix (always
                            /// true for an empty or NULL prefix).
            bool        StartsWith( const IASLibChar__ *strPrefix, bool bCaseSensitive = true ) const;
                            /// True if the string ends with @p strSuffix (always
                            /// true for an empty or NULL suffix).
            bool        EndsWith( const IASLibChar__ *strSuffix, bool bCaseSensitive = true ) const;

                            /// Convert a string to upper case
            void        ToUpperCase( void );

                            /// Convert a string to lower case
            void        ToLowerCase( void );

                            /// Convert a string to Hexadecimal (used in Oracle):
                            /// each character becomes two upper-case digits.
            void        ToHex( void );
                            /// Convert a string back from Hexadecimal (a pair
                            /// that would give NUL gives '?').
            void        FromHex( void );

                            /// Trim leading white-space from a string. Returns this string.
            CString    &LeftTrim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );
                            /// Trim trailing white-space from a string. Returns this string.
            CString    &RightTrim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );
                            /// Trim leading/trailing white-space from a string. Returns this string.
            CString    &Trim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );

                            /// Assign this string to a printf formatted string (of any
                            /// length), returning the text.
            const IASLibChar__ *Format( const IASLibChar__ *strFormat, ... )
#if defined( __GNUC__ )
                __attribute__(( format( printf, 2, 3 ) ))
#endif
                ;

                            /// Find a substring or character at or after nStart:
                            /// its index, or NOT_FOUND. An empty substring is
                            /// found at nStart.
            size_t      IndexOf( const CString &strSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
            size_t      IndexOf( const IASLibChar__ *strSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;   ///< Same, for a char *.
            size_t      IndexOf( const IASLibChar__ chSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;    ///< Same, for one character.
                            /// The last one that starts at or before nStart
                            /// (anywhere, by default): its index, or NOT_FOUND.
            size_t      LastIndexOf( const CString &strSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;
            size_t      LastIndexOf( const IASLibChar__ *strSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;   ///< Same, for a char *.
            size_t      LastIndexOf( const IASLibChar__ chSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;    ///< Same, for one character.

                // Stream Methods

#ifdef IASLIB_STD_STREAM_SUPPORT__
                            /// Output a string to a stream derived object.
            friend std::ostream &operator <<( std::ostream &oOutputStream, const CString &strSource );

                            /// Input a string from a stream derived object: one
                            /// whitespace-delimited word, as for std::string.
            friend std::istream &operator >>( std::istream &oInputStream, CString &strTarget );
#endif

                            /// URL (form) decoding: "%XX" is the byte XX, "%%" is
                            /// '%', and '+' is a space.
            void        Unescape( void );
                            /// URL (form) encoding: encodes control characters,
                            /// bytes over 126, spaces (as '+') and
                            /// "\"<>#%{}|\\~^[]`'+".
            void        Escape( void );
                            /// Replaces every chReplace with chTo (removing them if
                            /// chTo is NUL), in one pass from the start.
            void        Replace( const IASLibChar__ chReplace, const IASLibChar__ chTo );
                            /// Replaces every strReplace with strTo, in one pass
                            /// from the start (text put in is not searched again).
            void        Replace( const IASLibChar__ *strReplace, const IASLibChar__ *strTo );

                            /// Removes every @p chRemove (nothing for NUL).
            void        Remove( const IASLibChar__ chRemove );
                            /// Removes every @p strRemove.
            void        Remove( const IASLibChar__ *strRemove );

                            /// Doubles up single quotes for use in Oracle data inserts.
            void        OracleEscape( void );
                            /// Undoes doubled single quotes from Oracle.
            void        OracleUnescape( void );

                            /// Matches a pattern where '*' is any run of characters
                            /// and '?' any one character. An empty pattern matches
                            /// anything.
            bool        WildcardCompare( const IASLibChar__ *strPattern, bool bCaseSensitive = true ) const;

                            /// The string padded with chPadChar to nLength
                            /// characters, after the text (bLeftAlign) or before it.
                            /// Unchanged if it is already that long, or chPadChar is NUL.
            CString     Pad( size_t nLength, IASLibChar__ chPadChar = ' ', bool bLeftAlign = true ) const;

                            /// True if the string holds @p strSearch (case sensitive).
            bool        Contains(const CString &strSearch ) const;
            bool        Contains(const IASLibChar__ *strSearch ) const;     ///< Same, for a char *.
            bool        Contains(const IASLibChar__ chSearch ) const;       ///< Same, for one character.

                            /// Looks strToken up in astrTokenList (nTokens long, or
                            /// up to a NULL entry or an entry "**", whose token is
                            /// the default), returning the matching entry's token
                            /// from nTokenList, or -9999. With @p bUseWildcards the
                            /// list entries are WildcardCompare() patterns.
            static int  ConvertToToken( const IASLibChar__ *strToken, const IASLibChar__ *astrTokenList[], int nTokenList[], int nTokens = -1, bool bCaseSensitive = true, bool bUseWildcards = false );

                            /// Sets the string to the first nLength characters of
                            /// chSource (to its first NUL if negative; empty for NULL).
            void        Copy( const IASLibChar__ *chSource, int nLength = -1 );

                            /// Makes the string empty.
            void        Clear( void );

                            /// A new string holding a printf formatted text (of any length).
            static CString FormatString( const char *fmt, ... )
#if defined( __GNUC__ )
                __attribute__(( format( printf, 1, 2 ) ))
#endif
                ;

                            /// A hash of the text: equal strings hash the same, on
                            /// every platform.
            virtual int hashCode( void ) const;
            int         hashcode( void ) const { return hashCode(); }    ///< Same as hashCode().

                            /// Whether the string has startDelimiter followed
                            /// (later) by endDelimiter.
            bool        hasDelimitedTokens( const char *startDelimiter, const char *endDelimiter ) const;
                            /// Replaces every startDelimiter + name + endDelimiter
                            /// with value. Returns this string.
            CString    &replaceDelimitedToken( const char *startDelimiter, const char *endDelimiter, const char *name, const char *value );

                            /// The text as bytes (for hashing and the like).
            const unsigned char *getBytes( void ) const;
                            /// A string holding exactly nLength bytes, zero
                            /// bytes included (the (text, length) constructor
                            /// stops at the first zero).
            static CString      FromBytes( const void *pData, size_t nLength );

        private:
                            /// Makes the stub this string's own (unless it is
                            /// fixed), so it can be changed.
            void        ChangeStub( void );
                            /// Makes room for nLength characters and sets the
                            /// length (less, for a fixed stub): the text up to
                            /// the old length is kept. Returns the buffer, or
                            /// NULL for an empty string.
            IASLibChar__ *SetLength( size_t nLength );
            void        Assign( const IASLibChar__ *pData, size_t nLength );    ///< Replaces the text with nLength bytes of pData.
            void        Append( const IASLibChar__ *pData, size_t nLength );    ///< Adds nLength bytes of pData to the end.
            void        Release( void );                                        ///< Drops this string's hold on its stub.
    };

}   // namespace IASLib

#endif  // IASLIB_STRING_H__
