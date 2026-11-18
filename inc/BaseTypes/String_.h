/*
 * CString Class
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
    class CString : public CObject
    {
            // Data used to store a string in our current object implementation
            // (NULL for an empty string).
        private:
            CStringStub    *m_pStubData;

            // Methods used to interface with the string class
        public:
                            /// Default constructor. Creates a zero length string.
                        CString( void );

                            /// String Constructor - Takes a char * and converts
                            /// it to a CString object: its first nLength
                            /// characters (all of it if nLength is negative, or
                            /// the text is shorter).
                        CString( const IASLibChar__ *strSource, int nLength = -1 );
                        CString( const IASLibChar__ *strSource, size_t nLength );

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
                        CString( IASLibChar__ *strSource, size_t nLength, size_t nSize );

                            /// Copy Constructor - shares the source's text.
                        CString( const CString &strSource );
                        CString( CString &&strSource );

                            // Destructor - Destroys a string obejct
            virtual    ~CString( void );

                        DEFINE_OBJECT( CString )

                // Operator Methods
                            // Concatenation Operators (+) - Combines two strings
            friend CString operator +( const CString &strSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const IASLibChar__ *strSource2 );
            friend CString operator +( const IASLibChar__ *strSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const IASLibChar__ chSource2 );
            friend CString operator +( const IASLibChar__ chSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const short nSource2 );
            friend CString operator +( const short nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const int nSource2 );
            friend CString operator +( const int nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const unsigned int nSource2 );
            friend CString operator +( const unsigned int nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const long nSource2 );
            friend CString operator +( const long nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const unsigned long nSource2 );
            friend CString operator +( const unsigned long nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const long long nSource2 );
            friend CString operator +( const long long nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const unsigned long long nSource2 );
            friend CString operator +( const unsigned long long nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const double dSource2 );
            friend CString operator +( const double dSource1, const CString &strSource2 );

                            // Concatenate / Assign - Concatenates the string onto the
                            // current string and assigns the result
            CString        &operator +=( const CString &strSource );
            CString        &operator +=( const IASLibChar__ *strSource );
            CString        &operator +=( const IASLibChar__ chSource );
            CString        &operator +=( const short nSource );
            CString        &operator +=( const int  nSource );
            CString        &operator +=( const unsigned int nSource );
            CString        &operator +=( const long nSource );
            CString        &operator +=( const unsigned long nSource );
            CString        &operator +=( const long long nSource );
            CString        &operator +=( const unsigned long long nSource );
            CString        &operator +=( const double dSource );

                            // Assignment Operators (=) - Assigns a string
            CString        &operator =( const CString &strSource );
            CString        &operator =( CString &&strSource );
            CString        &operator =( const IASLibChar__ *strSource );
            CString        &operator =( const IASLibChar__ chSource );
            CString        &operator =( const short nSource );
            CString        &operator =( const int  nSource );
            CString        &operator =( const unsigned int nSource );
            CString        &operator =( const long nSource );
            CString        &operator =( const unsigned long nSource );
            CString        &operator =( const long long nSource );
            CString        &operator =( const unsigned long long nSource );
            CString        &operator =( const float fSource );
            CString        &operator =( const double dSource );
            CString        &operator =( const bool bSource );

                            // Typecast Operator - Types a CString to a char *
                            // (never NULL: "" for an empty string).
                            operator const IASLibChar__ *() const;
            const IASLibChar__ *c_str( void ) const { return (const IASLibChar__ *)(*this); }

                            // Comparison Operators - compare the text, case
                            // sensitively. A NULL char * compares as "".
            bool        operator ==( const CString &strString2 ) const { return Compare( strString2 ) == 0; }
            bool        operator ==( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) == 0; }
            bool        operator !=( const CString &strString2 ) const { return Compare( strString2 ) != 0; }
            bool        operator !=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) != 0; }
            bool        operator >=( const CString &strString2 ) const { return Compare( strString2 ) >= 0; }
            bool        operator >=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) >= 0; }
            bool        operator <=( const CString &strString2 ) const { return Compare( strString2 ) <= 0; }
            bool        operator <=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) <= 0; }
            bool        operator >( const CString &strString2 ) const { return Compare( strString2 ) > 0; }
            bool        operator >( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) > 0; }
            bool        operator <( const CString &strString2 ) const { return Compare( strString2 ) < 0; }
            bool        operator <( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) < 0; }

            friend bool operator ==( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) == 0; }
            friend bool operator !=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) != 0; }
            friend bool operator >=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) <= 0; }
            friend bool operator <=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) >= 0; }
            friend bool operator >( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) < 0; }
            friend bool operator <( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) > 0; }

                            // Lexical Comparison - Compares two strings and returns
                            // 0 if equal, -1 if this is less than the comparison, and
                            // +1 if this is greater than the comparison. Characters
                            // compare as unsigned values.
            int         Compare( const CString &strCompare, bool bCaseSensitive = true ) const;
            int         Compare( const IASLibChar__ *strCompare, bool bCaseSensitive = true ) const;

                            // Array Access Operator - the character at nIndex
                            // ('\0' past the end). The non-const forms give a
                            // character that can be changed (in this string only);
                            // past the end, a scratch character that is not part
                            // of the string.
            IASLibChar__        operator []( int nIndex ) const;
            IASLibChar__        operator []( size_t nIndex ) const;
            IASLibChar__       &operator []( int nIndex );
            IASLibChar__       &operator []( size_t nIndex );


                // Standard Methods

                            // Get the length of a string
            size_t      GetLength( void ) const
            {
                return ( m_pStubData ) ? m_pStubData->m_nLength : 0;
            }
            size_t      Length( void ) const { return GetLength(); }
            size_t      GetCount( void ) const { return GetLength(); }
            size_t      Count( void ) const { return GetLength(); }
            bool        IsEmpty( void ) const { return GetLength() == 0; }

                            // Room for nCapacity characters before the string must
                            // grow (for building a long string piece by piece), and
                            // giving back room the string does not use.
            void        Reserve( size_t nCapacity );
            void        Compact( void );

                            // Get a sub-string of the string: nLength characters
                            // from nStartIndex (to the end if nLength is negative).
            CString     Substring( size_t nStartIndex, int nLength = -1 ) const;

                            // Check if a string is all digits (false if it is empty)
            bool        IsDigits( void ) const;

            bool        StartsWith( const IASLibChar__ *strPrefix, bool bCaseSensitive = true ) const;
            bool        EndsWith( const IASLibChar__ *strSuffix, bool bCaseSensitive = true ) const;

                            // Convert a string to upper case
            void        ToUpperCase( void );

                            // Convert a string to lower case
            void        ToLowerCase( void );

                            // Convert a string to/from Hexadecimal (used in Oracle):
                            // each character becomes two upper-case digits, and
                            // back (a pair that would give NUL gives '?').
            void        ToHex( void );
            void        FromHex( void );

                            // Trim leading white-space from a string
            CString    &LeftTrim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );
                            // Trim trailing white-space from a string
            CString    &RightTrim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );
                            // Trim leading/trailing white-space from a string
            CString    &Trim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );

                            // Assign this string to a printf formatted string (of any
                            // length), returning the text.
            const IASLibChar__ *Format( const IASLibChar__ *strFormat, ... )
#if defined( __GNUC__ )
                __attribute__(( format( printf, 2, 3 ) ))
#endif
                ;

                            // Find a substring or character at or after nStart:
                            // its index, or NOT_FOUND. An empty substring is
                            // found at nStart.
            size_t      IndexOf( const CString &strSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
            size_t      IndexOf( const IASLibChar__ *strSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
            size_t      IndexOf( const IASLibChar__ chSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
                            // The last one that starts at or before nStart
                            // (anywhere, by default).
            size_t      LastIndexOf( const CString &strSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;
            size_t      LastIndexOf( const IASLibChar__ *strSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;
            size_t      LastIndexOf( const IASLibChar__ chSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;

                // Stream Methods

#ifdef IASLIB_STD_STREAM_SUPPORT__
                            // Output a string to a stream derived object.
            friend std::ostream &operator <<( std::ostream &oOutputStream, const CString &strSource );

                            // Input a string from a stream derived object: one
                            // whitespace-delimited word, as for std::string.
            friend std::istream &operator >>( std::istream &oInputStream, CString &strTarget );
#endif

                            // URL (form) decoding and encoding: "%XX" is the byte
                            // XX, "%%" is '%', and '+' is a space; Escape() encodes
                            // control characters, bytes over 126, spaces (as '+')
                            // and "\"<>#%{}|\\~^[]`'+".
            void        Unescape( void );
            void        Escape( void );
                            // Replaces every chReplace with chTo (removing them if
                            // chTo is NUL), or every strReplace with strTo, in one
                            // pass from the start (text put in is not searched
                            // again).
            void        Replace( const IASLibChar__ chReplace, const IASLibChar__ chTo );
            void        Replace( const IASLibChar__ *strReplace, const IASLibChar__ *strTo );

            void        Remove( const IASLibChar__ chRemove );
            void        Remove( const IASLibChar__ *strRemove );

                            // Doubles up single quotes for use in Oracle data inserts.
            void        OracleEscape( void );
                            // Undoes doubled single quotes from Oracle.
            void        OracleUnescape( void );

                            // Matches a pattern where '*' is any run of characters
                            // and '?' any one character. An empty pattern matches
                            // anything.
            bool        WildcardCompare( const IASLibChar__ *strPattern, bool bCaseSensitive = true ) const;

                            // The string padded with chPadChar to nLength
                            // characters, after the text (bLeftAlign) or before it.
            CString     Pad( size_t nLength, IASLibChar__ chPadChar = ' ', bool bLeftAlign = true ) const;

            bool        Contains(const CString &strSearch ) const;
            bool        Contains(const IASLibChar__ *strSearch ) const;
            bool        Contains(const IASLibChar__ chSearch ) const;

                            // Looks strToken up in astrTokenList (nTokens long, or
                            // up to an entry "**", whose token is the default),
                            // returning the matching entry's token from nTokenList,
                            // or -9999.
            static int  ConvertToToken( const IASLibChar__ *strToken, const IASLibChar__ *astrTokenList[], int nTokenList[], int nTokens = -1, bool bCaseSensitive = true, bool bUseWildcards = false );

                            // Sets the string to the first nLength characters of
                            // chSource (to its first NUL if negative).
            void        Copy( const IASLibChar__ *chSource, int nLength = -1 );

            void        Clear( void );

            static CString FormatString( const char *fmt, ... )
#if defined( __GNUC__ )
                __attribute__(( format( printf, 1, 2 ) ))
#endif
                ;

                            // A hash of the text: equal strings hash the same, on
                            // every platform.
            virtual int hashCode( void ) const;
            int         hashcode( void ) const { return hashCode(); }

                            // Whether the string has startDelimiter followed
                            // (later) by endDelimiter, and replacing every
                            // startDelimiter + name + endDelimiter with value.
            bool        hasDelimitedTokens( const char *startDelimiter, const char *endDelimiter ) const;
            CString    &replaceDelimitedToken( const char *startDelimiter, const char *endDelimiter, const char *name, const char *value );

                            // The text as bytes (for hashing and the like).
            const unsigned char *getBytes( void ) const;
                            // A string holding exactly nLength bytes, zero
                            // bytes included (the (text, length) constructor
                            // stops at the first zero).
            static CString      FromBytes( const void *pData, size_t nLength );

        private:
                            // Makes the stub this string's own (unless it is
                            // fixed), so it can be changed.
            void        ChangeStub( void );
                            // Makes room for nLength characters and sets the
                            // length (less, for a fixed stub): the text up to
                            // the old length is kept. Returns the buffer, or
                            // NULL for an empty string.
            IASLibChar__ *SetLength( size_t nLength );
            void        Assign( const IASLibChar__ *pData, size_t nLength );
            void        Append( const IASLibChar__ *pData, size_t nLength );
            void        Release( void );
    };

}   // namespace IASLib

#endif  // IASLIB_STRING_H__
