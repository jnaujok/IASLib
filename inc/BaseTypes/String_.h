/**
 * @file String_.h
 * @brief CString, the library's string class.
 *
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
    /**
     * A string that acts like a built-in type.
     *
     * The class handles its own memory. Copies share their text until one of
     * them changes (see CStringStub), and the sharing is thread-safe, but one
     * CString object must not be changed on one thread while another thread
     * uses it. Strings hold text without NUL characters: text is taken up to
     * its first NUL.
     *
     * Numbers become text in the C locale ('.' for the decimal point), with
     * doubles and floats in the shortest form that reads back as the same
     * value ("0.1", "2", "1e+100"). Running out of memory throws
     * std::bad_alloc.
     *
     * @code
     * CString strName = "World";
     * CString strGreeting = "Hello, " + strName + "! " + 42;
     * strGreeting.ToUpperCase();                   // "HELLO, WORLD! 42"
     * if ( strGreeting.StartsWith( "hello", false ) )
     *     printf( "%s\n", strGreeting.c_str() );
     * @endcode
     */
    class CString : public CObject
    {
            // Data used to store a string in our current object implementation
            // (NULL for an empty string).
        private:
            CStringStub    *m_pStubData;    ///< The text, shared with copies (NULL for an empty string).

            // Methods used to interface with the string class
        public:
                            /// Default constructor. Creates a zero length string.
                        CString( void );

                            /// String Constructor - Takes a char * and converts
                            /// it to a CString object: its first nLength
                            /// characters (all of it if nLength is negative, or
                            /// the text is shorter).
                            /// @param strSource The text; NULL gives "".
                            /// @param nLength How many characters to take.
                        CString( const IASLibChar__ *strSource, int nLength = -1 );
                            /// Same, with an unsigned length: the first nLength
                            /// characters of strSource (fewer if it is shorter).
                        CString( const IASLibChar__ *strSource, size_t nLength );

                            /// Fixed Buffer String Constructor - a string kept in
                            /// the program's buffer of nSize bytes (so at most
                            /// nSize - 1 characters), starting with its first
                            /// nLength characters (to the first NUL if negative).
                            /// The buffer is never moved: text that does not fit
                            /// is cut off, and every copy of the string shares
                            /// the buffer, so a change through one is seen by
                            /// all. With a NULL buffer, the string allocates one
                            /// for nSize characters itself. A size of 0 or less
                            /// gives an ordinary empty string.
                            /// @param strSource The buffer, or NULL.
                            /// @param nLength Characters already in the buffer.
                            /// @param nSize The buffer's size in bytes.
                        CString( IASLibChar__ *strSource, int nLength, int nSize );
                            /// @copydoc CString(IASLibChar__*,int,int)
                        CString( IASLibChar__ *strSource, size_t nLength, size_t nSize );

                            /// Copy Constructor - shares the source's text.
                        CString( const CString &strSource );
                            /// Move constructor: takes the source's text, leaving
                            /// it empty (a fixed-buffer source keeps sharing its
                            /// buffer).
                        CString( CString &&strSource );

                            /// Destructor - Destroys a string object.
            virtual    ~CString( void );

                        DEFINE_OBJECT( CString )

                // Operator Methods
                            /// Concatenation Operator (+) - Combines two strings.
            friend CString operator +( const CString &strSource1, const CString &strSource2 );
                            /// Concatenation, with text (NULL adds nothing).
            friend CString operator +( const CString &strSource1, const IASLibChar__ *strSource2 );
                            /// Concatenation, after text (NULL adds nothing).
            friend CString operator +( const IASLibChar__ *strSource1, const CString &strSource2 );
                            /// Concatenation, with a character.
            friend CString operator +( const CString &strSource1, const IASLibChar__ chSource2 );
                            /// Concatenation, after a character.
            friend CString operator +( const IASLibChar__ chSource1, const CString &strSource2 );
                            /// Concatenation, with a number as decimal text.
            friend CString operator +( const CString &strSource1, const short nSource2 );
                            /// Concatenation, after a number as decimal text.
            friend CString operator +( const short nSource1, const CString &strSource2 );
                            /// Concatenation, with a number as decimal text.
            friend CString operator +( const CString &strSource1, const int nSource2 );
                            /// Concatenation, after a number as decimal text.
            friend CString operator +( const int nSource1, const CString &strSource2 );
                            /// Concatenation, with a number as decimal text.
            friend CString operator +( const CString &strSource1, const unsigned int nSource2 );
                            /// Concatenation, after a number as decimal text.
            friend CString operator +( const unsigned int nSource1, const CString &strSource2 );
                            /// Concatenation, with a number as decimal text.
            friend CString operator +( const CString &strSource1, const long nSource2 );
                            /// Concatenation, after a number as decimal text.
            friend CString operator +( const long nSource1, const CString &strSource2 );
                            /// Concatenation, with a number as decimal text.
            friend CString operator +( const CString &strSource1, const unsigned long nSource2 );
                            /// Concatenation, after a number as decimal text.
            friend CString operator +( const unsigned long nSource1, const CString &strSource2 );
                            /// Concatenation, with a number as decimal text.
            friend CString operator +( const CString &strSource1, const long long nSource2 );
                            /// Concatenation, after a number as decimal text.
            friend CString operator +( const long long nSource1, const CString &strSource2 );
                            /// Concatenation, with a number as decimal text.
            friend CString operator +( const CString &strSource1, const unsigned long long nSource2 );
                            /// Concatenation, after a number as decimal text.
            friend CString operator +( const unsigned long long nSource1, const CString &strSource2 );
                            /// Concatenation, with a double in its shortest exact form.
            friend CString operator +( const CString &strSource1, const double dSource2 );
                            /// Concatenation, after a double in its shortest exact form.
            friend CString operator +( const double dSource1, const CString &strSource2 );

                            /// Concatenate / Assign - Concatenates the string onto the
                            /// current string and assigns the result.
            CString        &operator +=( const CString &strSource );
                            /// Appends text (NULL appends nothing).
            CString        &operator +=( const IASLibChar__ *strSource );
                            /// Appends a character (NUL appends nothing).
            CString        &operator +=( const IASLibChar__ chSource );
                            /// Appends a number as decimal text.
            CString        &operator +=( const short nSource );
                            /// Appends a number as decimal text.
            CString        &operator +=( const int  nSource );
                            /// Appends a number as decimal text.
            CString        &operator +=( const unsigned int nSource );
                            /// Appends a number as decimal text.
            CString        &operator +=( const long nSource );
                            /// Appends a number as decimal text.
            CString        &operator +=( const unsigned long nSource );
                            /// Appends a number as decimal text.
            CString        &operator +=( const long long nSource );
                            /// Appends a number as decimal text.
            CString        &operator +=( const unsigned long long nSource );
                            /// Appends a double in its shortest exact form.
            CString        &operator +=( const double dSource );

                            /// Assignment Operator (=) - shares the source's text (a
                            /// fixed-buffer string copies it into its buffer instead).
            CString        &operator =( const CString &strSource );
                            /// Move assignment: takes the source's text.
            CString        &operator =( CString &&strSource );
                            /// Assigns text (NULL gives "").
            CString        &operator =( const IASLibChar__ *strSource );
                            /// Assigns one character (NUL gives "").
            CString        &operator =( const IASLibChar__ chSource );
                            /// Assigns a number as decimal text.
            CString        &operator =( const short nSource );
                            /// Assigns a number as decimal text.
            CString        &operator =( const int  nSource );
                            /// Assigns a number as decimal text.
            CString        &operator =( const unsigned int nSource );
                            /// Assigns a number as decimal text.
            CString        &operator =( const long nSource );
                            /// Assigns a number as decimal text.
            CString        &operator =( const unsigned long nSource );
                            /// Assigns a number as decimal text.
            CString        &operator =( const long long nSource );
                            /// Assigns a number as decimal text.
            CString        &operator =( const unsigned long long nSource );
                            /// Assigns a float in its shortest exact form.
            CString        &operator =( const float fSource );
                            /// Assigns a double in its shortest exact form.
            CString        &operator =( const double dSource );
                            /// Assigns "true" or "false".
            CString        &operator =( const bool bSource );

                            /// Typecast Operator - Types a CString to a char *
                            /// (never NULL: "" for an empty string). The pointer
                            /// lasts until the string changes or is destroyed.
                            operator const IASLibChar__ *() const;
                            /// The text, as for std::string (never NULL).
            const IASLibChar__ *c_str( void ) const { return (const IASLibChar__ *)(*this); }

                            /// Comparison Operators - compare the text, case
                            /// sensitively. A NULL char * compares as "".
            bool        operator ==( const CString &strString2 ) const { return Compare( strString2 ) == 0; }
                            /// Whether the text equals strString2.
            bool        operator ==( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) == 0; }
                            /// Whether the texts differ.
            bool        operator !=( const CString &strString2 ) const { return Compare( strString2 ) != 0; }
                            /// Whether the text differs from strString2.
            bool        operator !=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) != 0; }
                            /// Whether this text sorts at or after strString2.
            bool        operator >=( const CString &strString2 ) const { return Compare( strString2 ) >= 0; }
                            /// Whether this text sorts at or after strString2.
            bool        operator >=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) >= 0; }
                            /// Whether this text sorts at or before strString2.
            bool        operator <=( const CString &strString2 ) const { return Compare( strString2 ) <= 0; }
                            /// Whether this text sorts at or before strString2.
            bool        operator <=( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) <= 0; }
                            /// Whether this text sorts after strString2.
            bool        operator >( const CString &strString2 ) const { return Compare( strString2 ) > 0; }
                            /// Whether this text sorts after strString2.
            bool        operator >( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) > 0; }
                            /// Whether this text sorts before strString2.
            bool        operator <( const CString &strString2 ) const { return Compare( strString2 ) < 0; }
                            /// Whether this text sorts before strString2.
            bool        operator <( const IASLibChar__ *strString2 ) const { return Compare( strString2 ) < 0; }

                            /// Whether text equals a string (NULL compares as "").
            friend bool operator ==( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) == 0; }
                            /// Whether text differs from a string.
            friend bool operator !=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) != 0; }
                            /// Whether text sorts at or after a string.
            friend bool operator >=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) <= 0; }
                            /// Whether text sorts at or before a string.
            friend bool operator <=( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) >= 0; }
                            /// Whether text sorts after a string.
            friend bool operator >( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) < 0; }
                            /// Whether text sorts before a string.
            friend bool operator <( const IASLibChar__ *strString1, const CString &strString2 ) { return strString2.Compare( strString1 ) > 0; }

                            /// Lexical Comparison - Compares two strings and returns
                            /// 0 if equal, -1 if this is less than the comparison, and
                            /// +1 if this is greater than the comparison. Characters
                            /// compare as unsigned values; a shorter string that is a
                            /// prefix of the other is less.
                            /// @param strCompare The string to compare with.
                            /// @param bCaseSensitive false compares ignoring case.
            int         Compare( const CString &strCompare, bool bCaseSensitive = true ) const;
                            /// Same, with text (NULL compares as "").
            int         Compare( const IASLibChar__ *strCompare, bool bCaseSensitive = true ) const;

                            /// Array Access Operator - the character at nIndex
                            /// ('\0' past the end, or for a negative index).
            IASLibChar__        operator []( int nIndex ) const;
                            /// The character at nIndex ('\0' past the end).
            IASLibChar__        operator []( size_t nIndex ) const;
                            /// A character that can be changed (in this string
                            /// only); past the end, a scratch character that is
                            /// not part of the string.
            IASLibChar__       &operator []( int nIndex );
                            /// @copydoc operator[](int)
            IASLibChar__       &operator []( size_t nIndex );


                // Standard Methods

                            /// Get the length of a string, in characters.
            size_t      GetLength( void ) const
            {
                return ( m_pStubData ) ? m_pStubData->m_nLength : 0;
            }
                            /// The length; same as GetLength().
            size_t      Length( void ) const { return GetLength(); }
                            /// The length; same as GetLength().
            size_t      GetCount( void ) const { return GetLength(); }
                            /// The length; same as GetLength().
            size_t      Count( void ) const { return GetLength(); }
                            /// Whether the string is "".
            bool        IsEmpty( void ) const { return GetLength() == 0; }

                            /// Makes room for nCapacity characters before the string
                            /// must grow (for building a long string piece by piece).
            void        Reserve( size_t nCapacity );
                            /// Gives back room the string does not use.
            void        Compact( void );

                            /// Get a sub-string of the string: nLength characters
                            /// from nStartIndex (to the end if nLength is negative).
                            /// @return The substring; "" if nStartIndex is past the end.
            CString     Substring( size_t nStartIndex, int nLength = -1 ) const;

                            /// Check if a string is all digits (false if it is empty).
            bool        IsDigits( void ) const;

                            /// Whether the string starts with strPrefix (always for
                            /// "" or NULL).
            bool        StartsWith( const IASLibChar__ *strPrefix, bool bCaseSensitive = true ) const;
                            /// Whether the string ends with strSuffix (always for
                            /// "" or NULL).
            bool        EndsWith( const IASLibChar__ *strSuffix, bool bCaseSensitive = true ) const;

                            /// Convert a string to upper case.
            void        ToUpperCase( void );

                            /// Convert a string to lower case.
            void        ToLowerCase( void );

                            /// Convert a string to Hexadecimal (used in Oracle):
                            /// each character becomes two upper-case digits.
            void        ToHex( void );
                            /// Convert a string back from Hexadecimal: each pair of
                            /// digits becomes a character (a pair that would give
                            /// NUL gives '?').
            void        FromHex( void );

                            /// Trim leading white-space from a string.
                            /// @param pszWhiteSpace The characters to trim.
                            /// @return This string.
            CString    &LeftTrim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );
                            /// Trim trailing white-space from a string.
                            /// @param pszWhiteSpace The characters to trim.
                            /// @return This string.
            CString    &RightTrim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );
                            /// Trim leading/trailing white-space from a string.
                            /// @param pszWhiteSpace The characters to trim.
                            /// @return This string.
            CString    &Trim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );

                            /// Assign this string to a printf formatted string (of any
                            /// length), returning the text. A NULL format gives "".
            const IASLibChar__ *Format( const IASLibChar__ *strFormat, ... )
#if defined( __GNUC__ )
                __attribute__(( format( printf, 2, 3 ) ))
#endif
                ;

                            /// Find a substring at or after nStart: its index, or
                            /// NOT_FOUND. An empty substring is found at nStart.
                            /// @param strSearch The text to find.
                            /// @param nStart Where to start looking.
                            /// @param bCaseInsensitive true ignores case.
            size_t      IndexOf( const CString &strSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
                            /// @copydoc IndexOf(const CString&,size_t,bool) const
            size_t      IndexOf( const IASLibChar__ *strSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
                            /// Same, for a character.
            size_t      IndexOf( const IASLibChar__ chSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
                            /// The last place strSearch starts at or before nStart
                            /// (anywhere, by default), or NOT_FOUND.
                            /// @param strSearch The text to find.
                            /// @param nStart The last start position to consider.
                            /// @param bCaseInsensitive true ignores case.
            size_t      LastIndexOf( const CString &strSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;
                            /// @copydoc LastIndexOf(const CString&,size_t,bool) const
            size_t      LastIndexOf( const IASLibChar__ *strSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;
                            /// Same, for a character.
            size_t      LastIndexOf( const IASLibChar__ chSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;

                // Stream Methods

#ifdef IASLIB_STD_STREAM_SUPPORT__
                            /// Output a string to a stream derived object.
            friend std::ostream &operator <<( std::ostream &oOutputStream, const CString &strSource );

                            /// Input a string from a stream derived object: one
                            /// whitespace-delimited word, as for std::string.
            friend std::istream &operator >>( std::istream &oInputStream, CString &strTarget );
#endif

                            /// URL (form) decoding: "%XX" is the byte XX, "%%" is
                            /// '%', and '+' is a space. A '%' not followed by two
                            /// hex digits (or followed by "00") stays as it is.
            void        Unescape( void );
                            /// URL (form) encoding: control characters, bytes over
                            /// 126 and "\"<>#%{}|\\~^[]`'+" become "%XX", and
                            /// spaces become '+'.
            void        Escape( void );
                            /// Replaces every chReplace with chTo (removing them if
                            /// chTo is NUL).
            void        Replace( const IASLibChar__ chReplace, const IASLibChar__ chTo );
                            /// Replaces every strReplace with strTo (NULL: ""), in
                            /// one pass from the start (text put in is not searched
                            /// again). An empty strReplace does nothing.
            void        Replace( const IASLibChar__ *strReplace, const IASLibChar__ *strTo );

                            /// Removes every chRemove.
            void        Remove( const IASLibChar__ chRemove );
                            /// Removes every strRemove.
            void        Remove( const IASLibChar__ *strRemove );

                            /// Doubles up single quotes for use in Oracle data inserts.
            void        OracleEscape( void );
                            /// Undoes doubled single quotes from Oracle.
            void        OracleUnescape( void );

                            /// Matches a pattern where '*' is any run of characters
                            /// and '?' any one character. An empty pattern matches
                            /// anything; a NULL one matches nothing.
            bool        WildcardCompare( const IASLibChar__ *strPattern, bool bCaseSensitive = true ) const;

                            /// The string padded with chPadChar to nLength
                            /// characters, after the text (bLeftAlign) or before it.
                            /// A string already that long (or a NUL pad character)
                            /// is returned as it is.
            CString     Pad( size_t nLength, IASLibChar__ chPadChar = ' ', bool bLeftAlign = true ) const;

                            /// Whether the string contains strSearch.
            bool        Contains(const CString &strSearch ) const;
                            /// Whether the string contains strSearch.
            bool        Contains(const IASLibChar__ *strSearch ) const;
                            /// Whether the string contains chSearch.
            bool        Contains(const IASLibChar__ chSearch ) const;

                            /// Looks strToken up in astrTokenList (nTokens long, or
                            /// up to a NULL entry or an entry "**", whose token is
                            /// the default), returning the matching entry's token
                            /// from nTokenList, or -9999.
                            /// @param strToken The text to look up.
                            /// @param astrTokenList The texts to match.
                            /// @param nTokenList The token of each text.
                            /// @param nTokens The number of entries (negative: up
                            ///     to a NULL or "**" entry).
                            /// @param bCaseSensitive false matches ignoring case.
                            /// @param bUseWildcards Treat the entries as
                            ///     WildcardCompare() patterns.
            static int  ConvertToToken( const IASLibChar__ *strToken, const IASLibChar__ *astrTokenList[], int nTokenList[], int nTokens = -1, bool bCaseSensitive = true, bool bUseWildcards = false );

                            /// Sets the string to the first nLength characters of
                            /// chSource (to its first NUL if negative; NULL gives "").
            void        Copy( const IASLibChar__ *chSource, int nLength = -1 );

                            /// Makes the string "".
            void        Clear( void );

                            /// A new string from a printf format (of any length).
            static CString FormatString( const char *fmt, ... )
#if defined( __GNUC__ )
                __attribute__(( format( printf, 1, 2 ) ))
#endif
                ;

                            /// A hash of the text: equal strings hash the same, on
                            /// every platform.
            virtual int hashCode( void ) const;
                            /// Same as hashCode().
            int         hashcode( void ) const { return hashCode(); }

                            /// Whether the string has startDelimiter followed
                            /// (later) by endDelimiter.
            bool        hasDelimitedTokens( const char *startDelimiter, const char *endDelimiter ) const;
                            /// Replaces every startDelimiter + name + endDelimiter
                            /// with value (NULL arguments count as "").
                            /// @return This string.
            CString    &replaceDelimitedToken( const char *startDelimiter, const char *endDelimiter, const char *name, const char *value );

                            /// The text as bytes (for hashing and the like).
            const unsigned char *getBytes( void ) const;

        private:
                            /// Makes the stub this string's own (unless it is
                            /// fixed), so it can be changed.
            void        ChangeStub( void );
                            /// Makes room for nLength characters and sets the
                            /// length (less, for a fixed stub): the text up to
                            /// the old length is kept. Returns the buffer, or
                            /// NULL for an empty string.
            IASLibChar__ *SetLength( size_t nLength );
                            /// Sets the text to nLength characters from pData.
            void        Assign( const IASLibChar__ *pData, size_t nLength );
                            /// Appends nLength characters from pData.
            void        Append( const IASLibChar__ *pData, size_t nLength );
                            /// Drops this string's reference to its stub.
            void        Release( void );
    };

}   // namespace IASLib

#endif  // IASLIB_STRING_H__
