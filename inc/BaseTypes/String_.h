/*
 * CString Class
 *
 *      This class provides an interface to a string class which mimics
 * an intrinsic string type for C++. This class handles memory management
 * and dynamic re-allocation of string storage and the like. It also
 * provides a very intrinsic acting interface for using strings.
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
#ifdef IASLIB_WIN32__
    #if ( _MSC_VER >= 1300 )
    #include <ostream>
    #include <istream>
    #else
    #include <iostream.h>
    #endif
#else
    #include <iostream>
#endif
#endif

namespace IASLib
{
    class CString : public CObject
    {
            // Data used to store a string in our current object implementation
        private:
            CStringStub    *m_pStubData;

            // Methods used to interface with the string class
        public:
                            /// Default constructor. Creates a zero length string.
                        CString( void );

                            /// String Constructor - Takes a char * and converts
                            /// it to a CString object.
                        CString( const IASLibChar__ *strSource, int nLength = -1 );

                            /// String Constructor - Takes a char * and converts
                            /// it to a CString object.
                        CString( const IASLibChar__ *strSource, size_t nLength );

                            /// Fixed Buffer String Constructor - Takes a char * as
                            /// the fixed location of a data buffer. The length of
                            /// the string, and the overall size of the buffer.
                        CString( IASLibChar__ *strSource, int nLength, int nSize );

                            /// Fixed Buffer String Constructor - Takes a char * as
                            /// the fixed location of a data buffer. The length of
                            /// the string, and the overall size of the buffer.
                        CString( IASLibChar__ *strSource, size_t nLength, size_t nSize );

                            /// Copy Constructor - Takes another CString object
                            /// and creates a new string object that's an exact
                            /// duplicate of the source object.
                        CString( const CString &strSource );

                            // Destructor - Destroys a string obejct
            virtual    ~CString( void );

                        DEFINE_OBJECT( CString )

                // Operator Methods
                            // Concatenation Operators (+) - Combines two strings
            friend CString operator +( const CString &strSource1, const int nSource2 );
            friend CString operator +( const int nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const long nSource2 );
            friend CString operator +( const long nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const short nSource2 );
            friend CString operator +( const short nSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const IASLibChar__ *strSource2 );
            friend CString operator +( const IASLibChar__ *strSource1, const CString &strSource2 );
            friend CString operator +( const CString &strSource1, const IASLibChar__ chSource2 );
            friend CString operator +( const IASLibChar__ chSource1, const CString &strSource2 );

                            // Concatenate / Assign - Concatenates the string onto the
                            // current string and assigns the result
            CString        &operator +=( const CString &strSource );
            CString        &operator +=( const IASLibChar__ *strSource );
            CString        &operator +=( const IASLibChar__ chSource );
            CString        &operator +=( const int  nSource );
            CString        &operator +=( const long nSource );
            CString        &operator +=( const short nSource );

                            // Assignment Operators (=) - Assigns a string
            CString        &operator =( const CString &strSource );
            CString        &operator =( const IASLibChar__ *strSource );
            CString        &operator =( const IASLibChar__ chSource );
            CString        &operator =( const int  nSource );
            CString        &operator =( const long nSource );
            CString        &operator =( const short nSource );
            CString        &operator =( const float nSource );
            CString        &operator =( const double nSource );
            CString        &operator =( const bool nSource );

                            // Typecast Operator - Types a CString to a char *
                            operator const IASLibChar__ *() const;

                            // Equality Operators (==) - Compare a string
            bool        operator ==( const CString &strString2 );
            bool        operator ==( const IASLibChar__ *strString2 );
            friend bool operator ==( IASLibChar__ *strString1, const CString &strString2 );

                            // Inequality Operators (!=) - Compare a string
            bool        operator !=( const CString &strString2 );
            bool        operator !=( const IASLibChar__ *strString2 );
            friend bool operator !=( IASLibChar__ *strString1, const CString &strString2 );

                            // Greater than / Equal To Operators (>=) - Compare a string
            bool        operator >=( const CString &strString2 );
            bool        operator >=( const IASLibChar__ *strString2 );
            friend bool operator >=( IASLibChar__ *strString1, const CString &strString2 );

                            // Less than / Equal To Operators (<=) - Compare a string
            bool        operator <=( const CString &strString2 );
            bool        operator <=( const IASLibChar__ *strString2 );
            friend bool operator <=( IASLibChar__ *strString1, const CString &strString2 );

                            // Greater than Operators (>) - Compare a string
            bool        operator >( const CString &strString2 );
            bool        operator >( const IASLibChar__ *strString2 );
            friend bool operator >( IASLibChar__ *strString1, const CString &strString2 );

                            // Less than Operators (<) - Compare a string
            bool        operator <( const CString &strString2 );
            bool        operator <( const IASLibChar__ *strString2 );
            friend bool operator <( IASLibChar__ *strString1, const CString &strString2 );

                            // Lexical Comparison - Compares two strings and returns
                            // 0 if equal, -1 if this is less than the comparison, and
                            // +1 if this is greater than the comparison.
            int         Compare( const CString &strCompare, bool bCaseSensitive = true ) const;
            int         Compare( const IASLibChar__ *strCompare, bool bCaseSensitive = true ) const;

                            // Array Access Operator - Allows us to access the characters
                            // of the string as if they were a char[] buffer.
            IASLibChar__       &operator []( int nIndex ) const;
                            // Array Access Operator - Allows us to access the characters
                            // of the string as if they were a char[] buffer.
            IASLibChar__       &operator []( size_t nIndex ) const;


                // Standard Methods

                            // Get the length of a string
            size_t      GetLength( void ) const
            {
                return (size_t)( ( m_pStubData ) ? m_pStubData->m_nLength:0);
            }
            size_t      Length( void ) const
            {
                return (size_t)( ( m_pStubData ) ? m_pStubData->m_nLength:0);
            }
            size_t      GetCount( void ) const
            {
                return (size_t)( ( m_pStubData ) ? m_pStubData->m_nLength:0);
            }
            size_t      Count( void ) const
            {
                return (size_t)( ( m_pStubData ) ? m_pStubData->m_nLength:0);
            }

                            // Get a sub-string of the string
            CString     Substring( size_t nStartIndex, int nLength = -1 ) const;

                            // Check if a string is all digits
            bool        IsDigits( void );

                            // Convert a string to upper case
            void        ToUpperCase( void );

                            // Convert a string to lower case
            void        ToLowerCase( void );

                            // Convert a string to/from Hexadecimal (used in Oracle)
            void        ToHex( void );
            void        FromHex( void );

                            // Trim leading white-space from a string
            CString    &LeftTrim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );
                            // Trim trailing white-space from a string
            CString    &RightTrim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );
                            // Trim leading/trailing white-space from a string
            CString    &Trim( const IASLibChar__ *pszWhiteSpace = " \t\r\n\f" );

                            // Assign this string to a printf formatted string
            const IASLibChar__ *Format( const IASLibChar__ *strFormat, ... );

                            // Find the occurance of a substring in a string
            size_t      IndexOf( const CString &strSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
            size_t      IndexOf( const IASLibChar__ *strSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
            size_t      IndexOf( const IASLibChar__ chSearch, size_t nStart = 0, bool bCaseInsensitive = false ) const;
            size_t      LastIndexOf( const CString &strSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;
            size_t      LastIndexOf( const IASLibChar__ *strSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;
            size_t      LastIndexOf( const IASLibChar__ chSearch, size_t nStart = (size_t)-1, bool bCaseInsensitive = false ) const;

                // Stream Methods

#ifdef IASLIB_STD_STREAM_SUPPORT__
                            // Output a string to a stream derived object.
            friend std::ostream &operator <<( std::ostream &oOutputStream, const CString &strSource );

                            // Input a string from a stream derived object.
            friend std::istream &operator >>( std::istream &oInputStream, CString &strTarget );
#endif

            void        Unescape( void );
            void        Escape( void );
            void        Replace( const IASLibChar__ chReplace, const IASLibChar__ chTo );
            void        Replace( const IASLibChar__ *strReplace, const IASLibChar__ *strTo );

            void        Remove( const IASLibChar__ chRemove );
            void        Remove( const IASLibChar__ *strRemove );

                            // Doubles up single quotes for use in Oracle data inserts.
            void        OracleEscape( void );
                            // Undoes doubled single quotes from Oracle.
            void        OracleUnescape( void );

            bool        WildcardCompare( const IASLibChar__ *strPattern, bool bCaseSensitive = true );

            CString     Pad( size_t nLength, IASLibChar__ chPadChar = ' ', bool bLeftAlign = true );

            bool        Contains(const CString &strSearch ) const;
            bool        Contains(const IASLibChar__ *strSearch ) const;
            bool        Contains(const IASLibChar__ chSearch ) const;

            static int  ConvertToToken( const IASLibChar__ *strToken, const IASLibChar__ *astrTokenList[], int nTokenList[], int nTokens = -1, bool bCaseSensitive = true, bool bUseWildcards = false );

            void        Copy( const IASLibChar__ *chSource, int nLength = -1 );

            void        Clear( void );

            static CString FormatString( const char *fmt, ... );

            virtual int hashcode( void );

            void        dumpStubMemory( void );

            bool        hasDelimitedTokens( const char *startDelimiter, const char *endDelimiter );
            CString    &replaceDelimitedToken( const char *startDelimiter, const char *endDelimiter, const char *name, const char *value );

        private:
            void        ChangeStub( void );
            void        ResizeString( int nNewSize );
    };

}   // namespace IASLib

#endif  // IASLIB_STRING_H__
