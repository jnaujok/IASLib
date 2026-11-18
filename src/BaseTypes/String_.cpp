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

#include "String_.h"

#include <ctype.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <new>
#include <string>
#include <vector>

#ifdef IASLIB_STD_STREAM_SUPPORT__
#include <istream>
#include <ostream>
#endif

namespace IASLib
{
    IMPLEMENT_OBJECT( CString, CObject );

    namespace
    {
            // The length of text up to its first NUL, looking no further
            // than nMax characters (the text need not be NUL terminated
            // if it is at least that long).
        size_t BoundedLength( const IASLibChar__ *pText, size_t nMax )
        {
            if ( pText == NULL )
            {
                return 0;
            }
            const void *pNul = memchr( pText, 0, nMax );
            return ( pNul != NULL ) ? (size_t)( (const IASLibChar__ *)pNul - pText ) : nMax;
        }

        size_t TextLength( const IASLibChar__ *pText )
        {
            return ( pText != NULL ) ? strlen( pText ) : 0;
        }

        inline unsigned char Byte( IASLibChar__ ch )
        {
            return (unsigned char)ch;
        }

        inline int Upper( IASLibChar__ ch )
        {
            return toupper( Byte( ch ) );
        }

        inline int Lower( IASLibChar__ ch )
        {
            return tolower( Byte( ch ) );
        }

        int CompareText( const IASLibChar__ *p1, size_t n1, const IASLibChar__ *p2, size_t n2, bool bCaseSensitive )
        {
            size_t nCommon = ( n1 < n2 ) ? n1 : n2;
            for ( size_t nCount = 0; nCount < nCommon; nCount++ )
            {
                int c1 = bCaseSensitive ? Byte( p1[ nCount ] ) : Upper( p1[ nCount ] );
                int c2 = bCaseSensitive ? Byte( p2[ nCount ] ) : Upper( p2[ nCount ] );
                if ( c1 != c2 )
                {
                    return ( c1 < c2 ) ? -1 : 1;
                }
            }
            if ( n1 == n2 )
            {
                return 0;
            }
            return ( n1 < n2 ) ? -1 : 1;
        }

        bool SameText( const IASLibChar__ *p1, const IASLibChar__ *p2, size_t nLength, bool bCaseSensitive )
        {
            return CompareText( p1, nLength, p2, nLength, bCaseSensitive ) == 0;
        }

            // Where pNeedle (nNeedle long) first appears in pText at or after
            // nStart, or NOT_FOUND.
        size_t FindText( const IASLibChar__ *pText, size_t nText, const IASLibChar__ *pNeedle, size_t nNeedle, size_t nStart, bool bCaseSensitive )
        {
            if ( nStart > nText || nNeedle > nText - nStart )
            {
                return NOT_FOUND;
            }
            for ( size_t nPos = nStart; nPos + nNeedle <= nText; nPos++ )
            {
                if ( SameText( pText + nPos, pNeedle, nNeedle, bCaseSensitive ) )
                {
                    return nPos;
                }
            }
            return NOT_FOUND;
        }

        std::string FormatV( const char *strFormat, va_list vaArgs )
        {
            if ( strFormat == NULL )
            {
                return std::string();
            }

            char achBuffer[ 256 ];
            va_list vaCopy;
            va_copy( vaCopy, vaArgs );
            int nLength = vsnprintf( achBuffer, sizeof( achBuffer ), strFormat, vaCopy );
            va_end( vaCopy );

            if ( nLength < 0 )
            {
                return std::string();
            }
            if ( (size_t)nLength < sizeof( achBuffer ) )
            {
                return std::string( achBuffer, (size_t)nLength );
            }

            std::vector<char> aBuffer( (size_t)nLength + 1 );
            va_copy( vaCopy, vaArgs );
            vsnprintf( &aBuffer[ 0 ], aBuffer.size(), strFormat, vaCopy );
            va_end( vaCopy );
            return std::string( &aBuffer[ 0 ], (size_t)nLength );
        }

        std::string FormatSigned( long long nValue )
        {
            char achBuffer[ 32 ];
            snprintf( achBuffer, sizeof( achBuffer ), "%lld", nValue );
            return achBuffer;
        }

        std::string FormatUnsigned( unsigned long long nValue )
        {
            char achBuffer[ 32 ];
            snprintf( achBuffer, sizeof( achBuffer ), "%llu", nValue );
            return achBuffer;
        }

            // printf follows the C locale's decimal point; ours is always '.'.
        void UseDecimalPoint( char *pszNumber )
        {
            const struct lconv *pLocale = localeconv();
            char chPoint = ( pLocale != NULL && pLocale->decimal_point != NULL && pLocale->decimal_point[0] != 0 ) ? pLocale->decimal_point[0] : '.';
            if ( chPoint != '.' )
            {
                for ( char *p = pszNumber; *p; p++ )
                {
                    if ( *p == chPoint )
                    {
                        *p = '.';
                    }
                }
            }
        }

            // The shortest %g form that reads back as the same value.
        std::string FormatDouble( double dValue )
        {
            char achBuffer[ 40 ];
            for ( int nPrecision = 1; nPrecision <= 17; nPrecision++ )
            {
                snprintf( achBuffer, sizeof( achBuffer ), "%.*g", nPrecision, dValue );
                if ( nPrecision == 17 || strtod( achBuffer, NULL ) == dValue || dValue != dValue )
                {
                    break;
                }
            }
            UseDecimalPoint( achBuffer );
            return achBuffer;
        }

        std::string FormatFloat( float fValue )
        {
            char achBuffer[ 40 ];
            for ( int nPrecision = 1; nPrecision <= 9; nPrecision++ )
            {
                snprintf( achBuffer, sizeof( achBuffer ), "%.*g", nPrecision, (double)fValue );
                if ( nPrecision == 9 || strtof( achBuffer, NULL ) == fValue || fValue != fValue )
                {
                    break;
                }
            }
            UseDecimalPoint( achBuffer );
            return achBuffer;
        }

        int HexDigit( IASLibChar__ ch )
        {
            if ( ch >= '0' && ch <= '9' ) return ch - '0';
            if ( ch >= 'A' && ch <= 'F' ) return 10 + ( ch - 'A' );
            if ( ch >= 'a' && ch <= 'f' ) return 10 + ( ch - 'a' );
            return -1;
        }

        const char *HEX_DIGITS = "0123456789ABCDEF";
        const char *DEFAULT_WHITESPACE = " \t\r\n\f";

        bool WildcardMatch( const IASLibChar__ *pText, size_t nText, const IASLibChar__ *pPattern, size_t nPattern, bool bCaseSensitive )
        {
                // The usual greedy matcher: on a mismatch, go back to the
                // last '*' and let it take one more character.
            size_t nT = 0, nP = 0;
            size_t nStarP = NOT_FOUND, nStarT = 0;
            while ( nT < nText )
            {
                if ( nP < nPattern && pPattern[ nP ] == '*' )
                {
                    nStarP = nP++;
                    nStarT = nT;
                }
                else if ( nP < nPattern && ( pPattern[ nP ] == '?' || SameText( pText + nT, pPattern + nP, 1, bCaseSensitive ) ) )
                {
                    nP++;
                    nT++;
                }
                else if ( nStarP != NOT_FOUND )
                {
                    nP = nStarP + 1;
                    nT = ++nStarT;
                }
                else
                {
                    return false;
                }
            }
            while ( nP < nPattern && pPattern[ nP ] == '*' )
            {
                nP++;
            }
            return nP == nPattern;
        }
    }

        // Default constructor. Creates a zero length string.
    CString::CString( void )
    {
        m_pStubData = NULL;
    }

    CString::CString( const IASLibChar__ *strSource, int nLength )
    {
        m_pStubData = NULL;
        if ( strSource != NULL && nLength != 0 )
        {
            Assign( strSource, ( nLength < 0 ) ? strlen( strSource ) : BoundedLength( strSource, (size_t)nLength ) );
        }
    }

    CString::CString( const IASLibChar__ *strSource, size_t nLength )
    {
        m_pStubData = NULL;
        if ( strSource != NULL && nLength != 0 )
        {
            Assign( strSource, BoundedLength( strSource, nLength ) );
        }
    }

    CString::CString( IASLibChar__ *strSource, int nLength, int nSize )
    {
        m_pStubData = NULL;
        if ( nSize <= 0 )
        {
            return;
        }

        if ( strSource == NULL )
        {
                // Room for nSize characters and the NUL.
            IASLibChar__ *pBuffer = (IASLibChar__ *)malloc( (size_t)nSize + 1 );
            if ( pBuffer == NULL )
            {
                throw std::bad_alloc();
            }
            memset( pBuffer, 0, (size_t)nSize + 1 );
            m_pStubData = new CStringStub( pBuffer, 0, (size_t)nSize + 1, true );
        }
        else
        {
            size_t nTextLength = ( nLength < 0 ) ? BoundedLength( strSource, (size_t)nSize ) : BoundedLength( strSource, (size_t)nLength );
            m_pStubData = new CStringStub( strSource, nTextLength, (size_t)nSize, false );
        }
        m_pStubData->AddRef();
    }

    CString::CString( IASLibChar__ *strSource, size_t nLength, size_t nSize )
    {
        m_pStubData = NULL;
        if ( nSize == 0 )
        {
            return;
        }

        if ( strSource == NULL )
        {
            if ( nSize == (size_t)-1 )
            {
                throw std::bad_alloc();
            }
            IASLibChar__ *pBuffer = (IASLibChar__ *)malloc( nSize + 1 );
            if ( pBuffer == NULL )
            {
                throw std::bad_alloc();
            }
            memset( pBuffer, 0, nSize + 1 );
            m_pStubData = new CStringStub( pBuffer, 0, nSize + 1, true );
        }
        else
        {
            m_pStubData = new CStringStub( strSource, BoundedLength( strSource, ( nLength < nSize ) ? nLength : nSize ), nSize, false );
        }
        m_pStubData->AddRef();
    }

    CString::CString( const CString &strSource ) : CObject( strSource )
    {
        m_pStubData = strSource.m_pStubData;
        if ( m_pStubData )
            m_pStubData->AddRef();
    }

    CString::CString( CString &&strSource ) : CObject( strSource )
    {
        m_pStubData = strSource.m_pStubData;
        if ( m_pStubData != NULL && m_pStubData->m_bFixedStub )
        {
                // The source still uses its fixed buffer: share it.
            m_pStubData->AddRef();
        }
        else
        {
            strSource.m_pStubData = NULL;
        }
    }

    CString::~CString( void )
    {
        Release();
    }

        // The helpers everything else is built on.

    void CString::Release( void )
    {
        if ( m_pStubData )
        {
            m_pStubData->RemoveRef();
            m_pStubData = NULL;
        }
    }

    void CString::ChangeStub( void )
    {
        if ( m_pStubData != NULL && ! m_pStubData->m_bFixedStub && m_pStubData->IsShared() )
        {
            CStringStub *pCopy = new CStringStub( m_pStubData->m_strData, m_pStubData->m_nLength );
            pCopy->AddRef();
            m_pStubData->RemoveRef();
            m_pStubData = pCopy;
        }
    }

    IASLibChar__ *CString::SetLength( size_t nLength )
    {
        if ( m_pStubData == NULL )
        {
            if ( nLength == 0 )
            {
                return NULL;
            }
            m_pStubData = new CStringStub( NULL, nLength );
            m_pStubData->AddRef();
            return m_pStubData->m_strData;
        }

        if ( nLength == 0 && ! m_pStubData->m_bFixedStub )
        {
            Release();
            return NULL;
        }

        if ( m_pStubData->IsShared() && ! m_pStubData->m_bFixedStub )
        {
                // Copy only what will be kept.
            size_t nKeep = ( m_pStubData->m_nLength < nLength ) ? m_pStubData->m_nLength : nLength;
            CStringStub *pCopy = new CStringStub( m_pStubData->m_strData, nKeep, nLength );
            pCopy->AddRef();
            m_pStubData->RemoveRef();
            m_pStubData = pCopy;
        }
        m_pStubData->ChangeSize( nLength );
        return m_pStubData->m_strData;
    }

    CString CString::FromBytes( const void *pData, size_t nLength )
    {
        CString strResult;
        if ( pData != NULL && nLength > 0 )
            strResult.Assign( (const IASLibChar__ *)pData, nLength / sizeof( IASLibChar__ ) );
        return strResult;
    }

    void CString::Assign( const IASLibChar__ *pData, size_t nLength )
    {
        if ( m_pStubData != NULL && m_pStubData->m_bFixedStub )
        {
                // Written into the fixed buffer (memmove: the text may come
                // from the buffer itself).
            if ( nLength > m_pStubData->m_nCapacity )
            {
                nLength = m_pStubData->m_nCapacity;
            }
            if ( nLength > 0 )
            {
                memmove( m_pStubData->m_strData, pData, nLength * sizeof( IASLibChar__ ) );
            }
            m_pStubData->ChangeSize( nLength );
            return;
        }

        if ( nLength == 0 )
        {
            Release();
            return;
        }

            // Make the new stub before letting go of the old one, which the
            // text may be part of.
        CStringStub *pNew = new CStringStub( pData, nLength );
        pNew->AddRef();
        Release();
        m_pStubData = pNew;
    }

    void CString::Append( const IASLibChar__ *pData, size_t nLength )
    {
        if ( nLength == 0 )
        {
            return;
        }

        size_t nOld = GetLength();
        if ( m_pStubData != NULL && pData >= m_pStubData->m_strData && pData < m_pStubData->m_strData + m_pStubData->m_nLength )
        {
                // Appending part of ourselves: the buffer may move.
            std::string strCopy( pData, nLength );
            Append( strCopy.data(), strCopy.size() );
            return;
        }

        if ( nOld > (size_t)-1 - nLength - 1 )
        {
            throw std::bad_alloc();
        }
        IASLibChar__ *pBuffer = SetLength( nOld + nLength );
        size_t nNew = GetLength();          // less, if a fixed buffer is full
        if ( pBuffer != NULL && nNew > nOld )
        {
            memcpy( pBuffer + nOld, pData, ( nNew - nOld ) * sizeof( IASLibChar__ ) );
        }
    }

    void CString::Reserve( size_t nCapacity )
    {
        if ( m_pStubData == NULL )
        {
            if ( nCapacity > 0 )
            {
                    // An empty string is normally stub-less; this one keeps
                    // its stub so the room is there.
                m_pStubData = new CStringStub( NULL, 0, nCapacity );
                m_pStubData->AddRef();
            }
            return;
        }
        ChangeStub();
        m_pStubData->Reserve( nCapacity );
    }

    void CString::Compact( void )
    {
        if ( m_pStubData == NULL )
        {
            return;
        }
        if ( m_pStubData->m_nLength == 0 && ! m_pStubData->m_bFixedStub )
        {
            Release();
            return;
        }
        if ( ! m_pStubData->IsShared() )
        {
            m_pStubData->Compact();
        }
    }

                // Operator Methods

    CString::operator const IASLibChar__ *() const
    {
        if ( m_pStubData )
            return m_pStubData->m_strData;
        return "";
    }

            // Concatenation Operators (+) - Combines two strings
    CString operator +( const CString &strSource1, const CString &strSource2 )
    {
        CString strReturn;
        strReturn.Reserve( strSource1.GetLength() + strSource2.GetLength() );
        strReturn += strSource1;
        strReturn += strSource2;
        return strReturn;
    }

    CString operator +( const CString &strSource1, const IASLibChar__ *strSource2 )
    {
        CString strReturn;
        size_t nLength2 = TextLength( strSource2 );
        strReturn.Reserve( strSource1.GetLength() + nLength2 );
        strReturn += strSource1;
        strReturn += strSource2;
        return strReturn;
    }

    CString operator +( const IASLibChar__ *strSource1, const CString &strSource2 )
    {
        CString strReturn;
        size_t nLength1 = TextLength( strSource1 );
        strReturn.Reserve( nLength1 + strSource2.GetLength() );
        strReturn += strSource1;
        strReturn += strSource2;
        return strReturn;
    }

    CString operator +( const CString &strSource1, const IASLibChar__ chSource2 )
    {
        CString strReturn( strSource1 );
        strReturn += chSource2;
        return strReturn;
    }

    CString operator +( const IASLibChar__ chSource1, const CString &strSource2 )
    {
        CString strReturn;
        strReturn = chSource1;
        strReturn += strSource2;
        return strReturn;
    }

#define IASLIB_STRING_NUMBER_OPERATORS__( TYPE, FORMAT ) \
    CString operator +( const CString &strSource1, const TYPE nSource2 ) \
    { \
        std::string strNumber = FORMAT( nSource2 ); \
        CString strReturn( strSource1 ); \
        strReturn += strNumber.c_str(); \
        return strReturn; \
    } \
    CString operator +( const TYPE nSource1, const CString &strSource2 ) \
    { \
        std::string strNumber = FORMAT( nSource1 ); \
        CString strReturn( strNumber.c_str() ); \
        strReturn += strSource2; \
        return strReturn; \
    } \
    CString &CString::operator +=( const TYPE nSource ) \
    { \
        std::string strNumber = FORMAT( nSource ); \
        Append( strNumber.data(), strNumber.size() ); \
        return *this; \
    } \
    CString &CString::operator =( const TYPE nSource ) \
    { \
        std::string strNumber = FORMAT( nSource ); \
        Assign( strNumber.data(), strNumber.size() ); \
        return *this; \
    }

    IASLIB_STRING_NUMBER_OPERATORS__( short, FormatSigned )
    IASLIB_STRING_NUMBER_OPERATORS__( int, FormatSigned )
    IASLIB_STRING_NUMBER_OPERATORS__( unsigned int, FormatUnsigned )
    IASLIB_STRING_NUMBER_OPERATORS__( long, FormatSigned )
    IASLIB_STRING_NUMBER_OPERATORS__( unsigned long, FormatUnsigned )
    IASLIB_STRING_NUMBER_OPERATORS__( long long, FormatSigned )
    IASLIB_STRING_NUMBER_OPERATORS__( unsigned long long, FormatUnsigned )
    IASLIB_STRING_NUMBER_OPERATORS__( double, FormatDouble )

#undef IASLIB_STRING_NUMBER_OPERATORS__

                    // Concatenate / Assign - Concatenates the string onto the
                    // current string and assigns the result
    CString &CString::operator +=( const CString &strSource )
    {
        if ( strSource.m_pStubData == NULL || strSource.m_pStubData->m_nLength == 0 )
        {
            return *this;
        }
        if ( m_pStubData == NULL )
        {
                // Nothing to add to: share the source's text.
            return *this = strSource;
        }
        Append( strSource.m_pStubData->m_strData, strSource.m_pStubData->m_nLength );
        return *this;
    }

    CString &CString::operator +=( const IASLibChar__ *strSource )
    {
        Append( strSource, TextLength( strSource ) );
        return *this;
    }

    CString &CString::operator +=( const IASLibChar__ chSource )
    {
        if ( chSource != 0 )
        {
            Append( &chSource, 1 );
        }
        return *this;
    }

                    // Assignment Operators (=) - Assigns a string
    CString &CString::operator =( const CString &strSource )
    {
        if ( this == &strSource || m_pStubData == strSource.m_pStubData )
        {
            return *this;
        }

        if ( m_pStubData != NULL && m_pStubData->m_bFixedStub )
        {
                // A fixed string keeps its buffer and takes a copy.
            Assign( strSource.c_str(), strSource.GetLength() );
            return *this;
        }

        if ( strSource.m_pStubData != NULL )
        {
            strSource.m_pStubData->AddRef();
        }
        Release();
        m_pStubData = strSource.m_pStubData;
        return *this;
    }

    CString &CString::operator =( CString &&strSource )
    {
        if ( this == &strSource )
        {
            return *this;
        }
        if ( ( m_pStubData != NULL && m_pStubData->m_bFixedStub ) ||
             ( strSource.m_pStubData != NULL && strSource.m_pStubData->m_bFixedStub ) )
        {
            return *this = (const CString &)strSource;
        }
        Release();
        m_pStubData = strSource.m_pStubData;
        strSource.m_pStubData = NULL;
        return *this;
    }

    CString &CString::operator =( const IASLibChar__ *strSource )
    {
        Assign( strSource, TextLength( strSource ) );
        return *this;
    }

    CString &CString::operator =( const IASLibChar__ chSource )
    {
        Assign( &chSource, ( chSource != 0 ) ? 1 : 0 );
        return *this;
    }

    CString &CString::operator =( const float fSource )
    {
        std::string strNumber = FormatFloat( fSource );
        Assign( strNumber.data(), strNumber.size() );
        return *this;
    }

    CString &CString::operator =( const bool bSource )
    {
        return *this = ( bSource ) ? "true" : "false";
    }

            // Lexical Comparison
    int CString::Compare( const CString &strCompare, bool bCaseSensitive ) const
    {
        if ( m_pStubData == strCompare.m_pStubData )
        {
            return 0;
        }
        return CompareText( c_str(), GetLength(), strCompare.c_str(), strCompare.GetLength(), bCaseSensitive );
    }

    int CString::Compare( const IASLibChar__ *strCompare, bool bCaseSensitive ) const
    {
        return CompareText( c_str(), GetLength(), ( strCompare != NULL ) ? strCompare : "", TextLength( strCompare ), bCaseSensitive );
    }

            // Array Access Operators
    IASLibChar__ CString::operator []( size_t nIndex ) const
    {
        if ( m_pStubData != NULL && nIndex < m_pStubData->m_nLength )
            return m_pStubData->m_strData[ nIndex ];
        return '\0';
    }

    IASLibChar__ CString::operator []( int nIndex ) const
    {
        if ( nIndex < 0 )
            return '\0';
        return (*this)[ (size_t)nIndex ];
    }

    IASLibChar__ &CString::operator []( size_t nIndex )
    {
        if ( m_pStubData != NULL && nIndex < m_pStubData->m_nLength )
        {
            ChangeStub();
            return m_pStubData->m_strData[ nIndex ];
        }

            // Past the end: somewhere harmless to read or write.
        static thread_local IASLibChar__ chScratch;
        chScratch = '\0';
        return chScratch;
    }

    IASLibChar__ &CString::operator []( int nIndex )
    {
        return (*this)[ ( nIndex < 0 ) ? (size_t)-1 : (size_t)nIndex ];
    }

                // Standard Methods

    CString CString::Substring( size_t nStartIndex, int nLength ) const
    {
        size_t nTotal = GetLength();
        if ( nStartIndex >= nTotal || nLength == 0 )
        {
            return CString();
        }

        size_t nAvailable = nTotal - nStartIndex;
        size_t nCount = ( nLength < 0 || (size_t)nLength > nAvailable ) ? nAvailable : (size_t)nLength;
        if ( nStartIndex == 0 && nCount == nTotal )
        {
            return *this;
        }
        return CString( m_pStubData->m_strData + nStartIndex, nCount );
    }

    void CString::ToUpperCase( void )
    {
        if ( m_pStubData )
        {
            ChangeStub();
            for ( size_t nCount = 0; nCount < m_pStubData->m_nLength; nCount++ )
            {
                m_pStubData->m_strData[ nCount ] = (IASLibChar__)Upper( m_pStubData->m_strData[ nCount ] );
            }
        }
    }

    void CString::ToLowerCase( void )
    {
        if ( m_pStubData )
        {
            ChangeStub();
            for ( size_t nCount = 0; nCount < m_pStubData->m_nLength; nCount++ )
            {
                m_pStubData->m_strData[ nCount ] = (IASLibChar__)Lower( m_pStubData->m_strData[ nCount ] );
            }
        }
    }

    void CString::ToHex( void )
    {
        size_t nLength = GetLength();
        if ( nLength == 0 )
        {
            return;
        }

        std::string strHex;
        strHex.reserve( nLength * 2 );
        for ( size_t nCount = 0; nCount < nLength; nCount++ )
        {
            unsigned char ch = Byte( m_pStubData->m_strData[ nCount ] );
            strHex += HEX_DIGITS[ ch >> 4 ];
            strHex += HEX_DIGITS[ ch & 0x0F ];
        }
        Assign( strHex.data(), strHex.size() );
    }

    void CString::FromHex( void )
    {
        size_t nLength = GetLength();
        if ( nLength == 0 )
        {
            return;
        }

        std::string strBytes;
        strBytes.reserve( nLength / 2 );
        for ( size_t nCount = 0; nCount + 1 < nLength; nCount += 2 )
        {
            int nHigh = HexDigit( m_pStubData->m_strData[ nCount ] );
            int nLow = HexDigit( m_pStubData->m_strData[ nCount + 1 ] );
            int nByte = ( ( nHigh < 0 ) ? 0 : nHigh ) * 16 + ( ( nLow < 0 ) ? 0 : nLow );
            strBytes += ( nByte != 0 ) ? (char)nByte : '?';
        }
        Assign( strBytes.data(), strBytes.size() );
    }

    CString &CString::LeftTrim( const IASLibChar__ *pszWhiteSpace )
    {
        if ( pszWhiteSpace == NULL )
            pszWhiteSpace = DEFAULT_WHITESPACE;

        size_t nLength = GetLength();
        size_t nStart = 0;
        while ( nStart < nLength && strchr( pszWhiteSpace, m_pStubData->m_strData[ nStart ] ) != NULL )
            nStart++;

        if ( nStart > 0 )
            Assign( m_pStubData->m_strData + nStart, nLength - nStart );
        return *this;
    }

    CString &CString::RightTrim( const IASLibChar__ *pszWhiteSpace )
    {
        if ( pszWhiteSpace == NULL )
            pszWhiteSpace = DEFAULT_WHITESPACE;

        size_t nLength = GetLength();
        size_t nEnd = nLength;
        while ( nEnd > 0 && strchr( pszWhiteSpace, m_pStubData->m_strData[ nEnd - 1 ] ) != NULL )
            nEnd--;

        if ( nEnd < nLength )
            SetLength( nEnd );
        return *this;
    }

    CString &CString::Trim( const IASLibChar__ *pszWhiteSpace )
    {
        RightTrim( pszWhiteSpace );
        return LeftTrim( pszWhiteSpace );
    }

    const IASLibChar__ *CString::Format( const IASLibChar__ *strFormat, ... )
    {
        va_list vaArgList;
        va_start( vaArgList, strFormat );
        std::string strText = FormatV( strFormat, vaArgList );
        va_end( vaArgList );

        Assign( strText.data(), strText.size() );
        return c_str();
    }

    CString CString::FormatString( const char *strFormat, ... )
    {
        va_list vaArgList;
        va_start( vaArgList, strFormat );
        std::string strText = FormatV( strFormat, vaArgList );
        va_end( vaArgList );

        return CString( strText.data(), strText.size() );
    }

            // Searching
    size_t CString::IndexOf( const CString &strSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        return FindText( c_str(), GetLength(), strSearch.c_str(), strSearch.GetLength(), ( nStart == NOT_FOUND ) ? 0 : nStart, ! bCaseInsensitive );
    }

    size_t CString::IndexOf( const IASLibChar__ *strSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        if ( strSearch == NULL )
            return NOT_FOUND;
        return FindText( c_str(), GetLength(), strSearch, strlen( strSearch ), ( nStart == NOT_FOUND ) ? 0 : nStart, ! bCaseInsensitive );
    }

    size_t CString::IndexOf( const IASLibChar__ chSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        if ( chSearch == 0 )
            return NOT_FOUND;
        return FindText( c_str(), GetLength(), &chSearch, 1, ( nStart == NOT_FOUND ) ? 0 : nStart, ! bCaseInsensitive );
    }

    size_t CString::LastIndexOf( const CString &strSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        return LastIndexOf( strSearch.c_str(), nStart, bCaseInsensitive );
    }

    size_t CString::LastIndexOf( const IASLibChar__ *strSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        if ( strSearch == NULL )
            return NOT_FOUND;

        size_t nLength = GetLength();
        size_t nSearch = strlen( strSearch );
        if ( nSearch > nLength )
            return NOT_FOUND;

        size_t nPos = nLength - nSearch;
        if ( nStart < nPos )
            nPos = nStart;

        const IASLibChar__ *pText = c_str();
        for ( ;; nPos-- )
        {
            if ( SameText( pText + nPos, strSearch, nSearch, ! bCaseInsensitive ) )
                return nPos;
            if ( nPos == 0 )
                break;
        }
        return NOT_FOUND;
    }

    size_t CString::LastIndexOf( const IASLibChar__ chSearch, size_t nStart, bool bCaseInsensitive ) const
    {
        if ( chSearch == 0 )
            return NOT_FOUND;
        IASLibChar__ achSearch[2] = { chSearch, 0 };
        return LastIndexOf( achSearch, nStart, bCaseInsensitive );
    }

    bool CString::StartsWith( const IASLibChar__ *strPrefix, bool bCaseSensitive ) const
    {
        size_t nPrefix = TextLength( strPrefix );
        return nPrefix <= GetLength() && SameText( c_str(), strPrefix, nPrefix, bCaseSensitive );
    }

    bool CString::EndsWith( const IASLibChar__ *strSuffix, bool bCaseSensitive ) const
    {
        size_t nSuffix = TextLength( strSuffix );
        return nSuffix <= GetLength() && SameText( c_str() + GetLength() - nSuffix, strSuffix, nSuffix, bCaseSensitive );
    }

    bool CString::Contains( const CString &strSearch ) const
    {
        return IndexOf( strSearch ) != NOT_FOUND;
    }

    bool CString::Contains( const IASLibChar__ *strSearch ) const
    {
        return IndexOf( strSearch ) != NOT_FOUND;
    }

    bool CString::Contains( const IASLibChar__ chSearch ) const
    {
        return IndexOf( chSearch ) != NOT_FOUND;
    }

#ifdef IASLIB_STD_STREAM_SUPPORT__
    std::ostream &operator <<( std::ostream &oOutputStream, const CString &strSource )
    {
        oOutputStream.write( strSource.c_str(), (std::streamsize)strSource.GetLength() );
        return oOutputStream;
    }

    std::istream &operator >>( std::istream &oInputStream, CString &strTarget )
    {
        std::string strWord;
        oInputStream >> strWord;
        strTarget = strWord.c_str();
        return oInputStream;
    }
#endif

            // Escaping
    void CString::Unescape( void )
    {
        size_t nLength = GetLength();
        if ( nLength == 0 )
            return;

        const IASLibChar__ *pText = c_str();
        std::string strResult;
        strResult.reserve( nLength );
        for ( size_t nCount = 0; nCount < nLength; nCount++ )
        {
            IASLibChar__ ch = pText[ nCount ];
            if ( ch == '%' && nCount + 1 < nLength && pText[ nCount + 1 ] == '%' )
            {
                strResult += '%';
                nCount++;
            }
            else if ( ch == '%' && nCount + 2 < nLength && HexDigit( pText[ nCount + 1 ] ) >= 0 && HexDigit( pText[ nCount + 2 ] ) >= 0 &&
                      ( HexDigit( pText[ nCount + 1 ] ) | HexDigit( pText[ nCount + 2 ] ) ) != 0 )
            {
                strResult += (char)( HexDigit( pText[ nCount + 1 ] ) * 16 + HexDigit( pText[ nCount + 2 ] ) );
                nCount += 2;
            }
            else if ( ch == '+' )
            {
                strResult += ' ';
            }
            else
            {
                    // Includes a '%' not followed by two hex digits (or by
                    // "00", which would end the string), which stays as it is.
                strResult += ch;
            }
        }
        Assign( strResult.data(), strResult.size() );
    }

    void CString::Escape( void )
    {
        size_t nLength = GetLength();
        if ( nLength == 0 )
            return;

        static const char *ESCAPABLE = "\"<>#%{}|\\~^[]`'+";
        const IASLibChar__ *pText = c_str();
        std::string strResult;
        strResult.reserve( nLength + nLength / 2 );
        for ( size_t nCount = 0; nCount < nLength; nCount++ )
        {
            unsigned char ch = Byte( pText[ nCount ] );
            if ( ch < 32 || ch > 126 || strchr( ESCAPABLE, ch ) != NULL )
            {
                strResult += '%';
                strResult += HEX_DIGITS[ ch >> 4 ];
                strResult += HEX_DIGITS[ ch & 0x0F ];
            }
            else if ( ch == ' ' )
            {
                strResult += '+';
            }
            else
            {
                strResult += (char)ch;
            }
        }
        Assign( strResult.data(), strResult.size() );
    }

    void CString::Replace( const IASLibChar__ chReplace, const IASLibChar__ chTo )
    {
        if ( chTo == 0 )
        {
            Remove( chReplace );
            return;
        }
        if ( chReplace == 0 || IndexOf( chReplace ) == NOT_FOUND )
            return;

        ChangeStub();
        for ( size_t nCount = 0; nCount < m_pStubData->m_nLength; nCount++ )
        {
            if ( m_pStubData->m_strData[ nCount ] == chReplace )
                m_pStubData->m_strData[ nCount ] = chTo;
        }
    }

    void CString::Replace( const IASLibChar__ *strReplace, const IASLibChar__ *strTo )
    {
        size_t nReplace = TextLength( strReplace );
        if ( nReplace == 0 )
            return;

        size_t nFound = IndexOf( strReplace );
        if ( nFound == NOT_FOUND )
            return;

        if ( strTo == NULL )
            strTo = "";
        size_t nTo = strlen( strTo );

        const IASLibChar__ *pText = c_str();
        size_t nLength = GetLength();
        std::string strResult;
        size_t nFrom = 0;
        while ( nFound != NOT_FOUND )
        {
            strResult.append( pText + nFrom, nFound - nFrom );
            strResult.append( strTo, nTo );
            nFrom = nFound + nReplace;
            nFound = FindText( pText, nLength, strReplace, nReplace, nFrom, true );
        }
        strResult.append( pText + nFrom, nLength - nFrom );
        Assign( strResult.data(), strResult.size() );
    }

    void CString::Remove( const IASLibChar__ chRemove )
    {
        if ( chRemove == 0 || IndexOf( chRemove ) == NOT_FOUND )
            return;

        ChangeStub();
        size_t nInsert = 0;
        for ( size_t nCount = 0; nCount < m_pStubData->m_nLength; nCount++ )
        {
            if ( m_pStubData->m_strData[ nCount ] != chRemove )
                m_pStubData->m_strData[ nInsert++ ] = m_pStubData->m_strData[ nCount ];
        }
        SetLength( nInsert );
    }

    void CString::Remove( const IASLibChar__ *strRemove )
    {
        Replace( strRemove, "" );
    }

    bool CString::WildcardCompare( const IASLibChar__ *strPattern, bool bCaseSensitive ) const
    {
        if ( strPattern == NULL )
            return false;
        size_t nPattern = strlen( strPattern );
        if ( nPattern == 0 )
            return true;
        return WildcardMatch( c_str(), GetLength(), strPattern, nPattern, bCaseSensitive );
    }

    void CString::OracleEscape( void )
    {
        if ( IndexOf( '\'' ) != NOT_FOUND )
            Replace( "'", "''" );
    }

    void CString::OracleUnescape( void )
    {
        if ( IndexOf( "''" ) != NOT_FOUND )
            Replace( "''", "'" );
    }

    CString CString::Pad( size_t nLength, IASLibChar__ chPadChar, bool bLeftAlign ) const
    {
        size_t nCurrent = GetLength();
        if ( nLength <= nCurrent || chPadChar == 0 )
            return *this;

        std::string strPadding( nLength - nCurrent, chPadChar );
        CString strRetVal;
        strRetVal.Reserve( nLength );
        if ( bLeftAlign )
        {
            strRetVal += *this;
            strRetVal += strPadding.c_str();
        }
        else
        {
            strRetVal += strPadding.c_str();
            strRetVal += *this;
        }
        return strRetVal;
    }

    int CString::ConvertToToken( const IASLibChar__ *strToken, const IASLibChar__ *astrTokenList[], int nTokenList[], int nTokens, bool bCaseSensitive, bool bUseWildcards )
    {
        if ( astrTokenList == NULL || nTokenList == NULL )
            return -9999;
        if ( nTokens < 0 )
            nTokens = 65535;

        CString strCompare = strToken;
        for ( int nCount = 0; nCount < nTokens && astrTokenList[ nCount ] != NULL; nCount++ )
        {
            const IASLibChar__ *strWork = astrTokenList[ nCount ];

                // The end of the list, with the default token.
            if ( strcmp( strWork, "**" ) == 0 )
                return nTokenList[ nCount ];

            bool bMatch = bUseWildcards ? strCompare.WildcardCompare( strWork, bCaseSensitive )
                                        : ( strCompare.Compare( strWork, bCaseSensitive ) == 0 );
            if ( bMatch )
                return nTokenList[ nCount ];
        }
        return -9999;
    }

    void CString::Copy( const IASLibChar__ *strSource, int nLength )
    {
        if ( strSource == NULL || nLength == 0 )
        {
            Assign( "", 0 );
            return;
        }
        Assign( strSource, ( nLength < 0 ) ? strlen( strSource ) : BoundedLength( strSource, (size_t)nLength ) );
    }

    void CString::Clear( void )
    {
        Assign( "", 0 );
    }

    bool CString::IsDigits( void ) const
    {
        size_t nLength = GetLength();
        if ( nLength == 0 )
            return false;

        for ( size_t nCount = 0; nCount < nLength; nCount++ )
        {
            if ( ! isdigit( Byte( m_pStubData->m_strData[ nCount ] ) ) )
                return false;
        }
        return true;
    }

        // The same values the library has always given (each character as
        // a signed byte), computed without signed overflow.
    int CString::hashCode( void ) const
    {
        unsigned int nResult = 131;
        size_t nLength = GetLength();
        for ( size_t x = 0; x < nLength; x++ )
        {
            nResult = 37u * nResult + (unsigned int)(int)(signed char)m_pStubData->m_strData[x];
        }
        return (int)nResult;
    }

    bool CString::hasDelimitedTokens( const char *startDelimiter, const char *endDelimiter ) const
    {
        size_t nStart = IndexOf( startDelimiter );
        if ( nStart == NOT_FOUND )
            return false;
        return IndexOf( endDelimiter, nStart + TextLength( startDelimiter ) ) != NOT_FOUND;
    }

    CString &CString::replaceDelimitedToken( const char *startDelimiter, const char *endDelimiter, const char *name, const char *value )
    {
        std::string strToken;
        strToken += ( startDelimiter != NULL ) ? startDelimiter : "";
        strToken += ( name != NULL ) ? name : "";
        strToken += ( endDelimiter != NULL ) ? endDelimiter : "";
        Replace( strToken.c_str(), value );
        return *this;
    }

    const unsigned char *CString::getBytes( void ) const
    {
        return (const unsigned char *)c_str();
    }

} // namespace IASLib
