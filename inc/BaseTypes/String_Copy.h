/*
 * CCString Class
 *
 *      This class provides an interface to a string class which mimics
 * an intrinsic string type for C++. This class handles memory management
 * and dynamic re-allocation of string storage and the like. It also
 * provides a very intrinsic acting interface for using strings.
 *      This class performs deep copies of the string every time it is
 * duplicated. Each instance of the class has its own copy of the string.
 * It is thus totally safe, but somewhat slow. If speed is of the essence,
 * then the CString class should be used instead.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/01/1992
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CSTRING_H__
#define IASLIB_CSTRING_H__

#include "Object.h"
#include <iostream>

namespace IASLib
{
    class CCString : public CObject
    {
        public:
            static const char *END_OF_TOKENS_MARKER;

            // Data used to store a string in our current object implementation
        private:
            char       *m_strData;
            int         m_nLength;
        
            // Methods used to interface with the string class
        public:
                            // Default constructor. Creates a zero length string.
                        CCString( void );

                            // String Constructor - Takes a char * and converts
                            // it to a CCString object.
                        CCString( const char *strSource, int nLength = -1 );

                            // Copy Constructor - Takes another CCString object
                            // and creates a new string object that's an exact
                            // duplicate of the source object.
                        CCString( const CCString &strSource );

                            // Destructor - Destroys a string obejct
            virtual    ~CCString( void );

                // Operator Methods
						DEFINE_OBJECT( CCString );

                            // Concatenation Operators (+) - Combines two strings
            friend CCString operator +( const CCString &strSource1, const int nSource2 );
            friend CCString operator +( const int nSource1, const CCString &strSource2 );
            friend CCString operator +( const CCString &strSource1, const long nSource2 );
            friend CCString operator +( const long nSource1, const CCString &strSource2 );
            friend CCString operator +( const CCString &strSource1, const short nSource2 );
            friend CCString operator +( const short nSource1, const CCString &strSource2 );
            friend CCString operator +( const CCString &strSource1, const CCString &strSource2 );
            friend CCString operator +( const CCString &strSource1, const char *strSource2 );
            friend CCString operator +( const char *strSource1, const CCString &strSource2 );
            friend CCString operator +( const CCString &strSource1, const char chSource2 );
            friend CCString operator +( const char chSource1, const CCString &strSource2 );

                            // Concatenate / Assign - Concatenates the string onto the
                            // current string and assigns the result
            CCString        &operator +=( const CCString &strSource );
            CCString        &operator +=( const char *strSource );
            CCString        &operator +=( const char chSource );
            CCString        &operator +=( const int  nSource );
            CCString        &operator +=( const long nSource );
            CCString        &operator +=( const short nSource );

                            // Assignment Operators (=) - Assigns a string
            CCString        &operator =( const CCString &strSource );
            CCString        &operator =( const char *strSource );
            CCString        &operator =( const char chSource );
            CCString        &operator =( const int  nSource );
            CCString        &operator =( const long nSource );
            CCString        &operator =( const short nSource );

                            // Typecast Operator - Types a CCString to a char *
                            operator const char *() const;

                            // Equality Operators (==) - Compare a string
            bool        operator ==( const CCString &strString2 );
            bool        operator ==( const char *strString2 );
            friend bool operator ==( char *strString1, const CCString &strString2 );

                            // Inequality Operators (!=) - Compare a string
            bool        operator !=( const CCString &strString2 );
            bool        operator !=( const char *strString2 );
            friend bool operator !=( char *strString1, const CCString &strString2 );

                            // Greater than / Equal To Operators (>=) - Compare a string
            bool        operator >=( const CCString &strString2 );
            bool        operator >=( const char *strString2 );
            friend bool operator >=( char *strString1, const CCString &strString2 );

                            // Less than / Equal To Operators (<=) - Compare a string
            bool        operator <=( const CCString &strString2 );
            bool        operator <=( const char *strString2 );
            friend bool operator <=( char *strString1, const CCString &strString2 );

                            // Greater than Operators (>) - Compare a string
            bool        operator >( const CCString &strString2 );
            bool        operator >( const char *strString2 );
            friend bool operator >( char *strString1, const CCString &strString2 );

                            // Less than Operators (<) - Compare a string
            bool        operator <( const CCString &strString2 );
            bool        operator <( const char *strString2 );
            friend bool operator <( char *strString1, const CCString &strString2 );

                            // Lexical Comparison - Compares two strings and returns
                            // 0 if equal, -1 if this is less than the comparison, and
                            // +1 if this is greater than the comparison.
            int         Compare( const CCString &strCompare, bool bCaseSensitive = true ) const;
            int         Compare( const char *strCompare, bool bCaseSensitive = true ) const;

                            // Array Access Operator - Allows us to access the characters
                            // of the string as if they were a char[] buffer.
            char       &operator []( int nIndex ) const;

                // Standard Methods

                            // Get the length of a string
            int         GetLength( void ) const
            {
                return m_nLength;
            }

                            // Get a sub-string of the string
            CCString     Substring( int nStartIndex, int nLength = -1 ) const;

                            // Convert a string to upper case
            void        ToUpperCase( void );

                            // Convert a string to lower case
            void        ToLowerCase( void );

                            // Trim leading/trailing white-space from a string
            CCString    &Trim( const char *pszWhiteSpace = " \t\r\n\f" );

                            // Assign this string to a printf formatted string
            const char *Format( const char *strFormat, ... );
                
                            // Find the occurance of a substring in a string
            int         IndexOf( const CCString &strSearch, int nStart = 0, bool bCaseInsensitive = false ) const;
            int         IndexOf( const char *strSearch, int nStart = 0, bool bCaseInsensitive = false ) const;
            int         IndexOf( const char chSearch, int nStart = 0, bool bCaseInsensitive = false ) const;
            int         LastIndexOf( const CCString &strSearch, int nStart = -1, bool bCaseInsensitive = false ) const;
            int         LastIndexOf( const char *strSearch, int nStart = -1, bool bCaseInsensitive = false ) const;
            int         LastIndexOf( const char chSearch, int nStart = -1, bool bCaseInsensitive = false ) const;

#ifdef IASLIB_STREAM_SUPPORT__
                // Stream Methods

                            // Output a string to a stream derived object.
            friend ostream &operator <<( ostream &oOutputStream, const CCString &strSource );

                            // Input a string from a stream derived object.
            friend istream &operator >>( istream &oInputStream, CCString &strTarget );
#endif // IASLIB_STREAM_SUPPORT__

            void        Unescape( void );
            void        Escape( void );
            void        Replace( const char chReplace, const char chTo );
            void        Replace( const char *strReplace, const char *strTo );

            void        Remove( const char chRemove );
            void        Remove( const char *strRemove );

                            // Doubles up single quotes for use in Oracle data inserts.
            void        OracleEscape( void );
                            // Undoes doubled single quotes from Oracle.
            void        OracleUnescape( void );

            bool        WildcardCompare( const char *strPattern, bool bCaseSensitive = true );

            CCString     Pad( int nLength, char chPadChar = ' ', bool bLeftAlign = true );

            bool        Contains(const CCString &strSearch ) const;
            bool        Contains(const char *strSearch ) const;
            bool        Contains(const char chSearch ) const;

            static int  ConvertToToken( const char *strToken, const char *astrTokenList[], int nTokenList[], int nTokens = -1, bool bCaseSensitive = true, bool bUseWildcards = false );

            void        Copy( const char *chSource, int nLength = -1 );
    };

} // namespace IASLib;

#endif  // IASLIB_CSTRING_H__
