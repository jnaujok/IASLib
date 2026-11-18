/*
 * Stream Abstract Class
 *
 *      The defaults every stream shares. See Stream.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Stream.h"
#include "StreamException.h"

#include <ctype.h>
#include <errno.h>
#include <locale>
#include <sstream>
#include <string.h>
#include <string>

namespace IASLib
{
    IMPLEMENT_OBJECT( CStream, CObject );

        // Characters

    char CStream::GetChar( void )
    {
        char chRead;
        if ( GetBuffer( &chRead, 1 ) != 1 )
            IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
        return chRead;
    }

    unsigned char CStream::GetUChar( void )
    {
        return (unsigned char)GetChar();
    }

    void CStream::PutChar( const char chPut )
    {
        PutBuffer( &chPut, 1 );
    }

    void CStream::PutChar( const unsigned char chPut )
    {
        char chOut = (char)chPut;
        PutBuffer( &chOut, 1 );
    }

    CString CStream::GetLine( void )
    {
        std::string strLine;
        while ( ! IsEOS() )
        {
            char ch = GetChar();
            if ( ch == '\n' )
                break;
            if ( ch != '\0' )
                strLine += ch;
        }
        if ( ! strLine.empty() && strLine[ strLine.size() - 1 ] == '\r' )
            strLine.erase( strLine.size() - 1 );
        return CString( strLine.data(), strLine.size() );
    }

    void CStream::PutLine( const CString &strOutput )
    {
        PutBuffer( strOutput, (int)strOutput.GetLength() );
        PutChar( '\n' );
    }

        // Binary

    void CStream::GetExact( char *achBuffer, int nLength )
    {
        int nRead = 0;
        while ( nRead < nLength )
        {
            int nGot = GetBuffer( achBuffer + nRead, nLength - nRead );
            if ( nGot <= 0 )
                IASLIB_THROW_STREAM_EXCEPTION( "Attempt to read past the end of the stream." );
            nRead += nGot;
        }
    }

    void CStream::PutUInt8( uint8_t nValue )
    {
        PutChar( (unsigned char)nValue );
    }

    void CStream::PutUInt16( uint16_t nValue )
    {
        char ach[2] = { (char)( nValue >> 8 ), (char)nValue };
        PutBuffer( ach, 2 );
    }

    void CStream::PutUInt32( uint32_t nValue )
    {
        char ach[4];
        for ( int n = 0; n < 4; n++ )
            ach[n] = (char)( nValue >> ( 24 - 8 * n ) );
        PutBuffer( ach, 4 );
    }

    void CStream::PutUInt64( uint64_t nValue )
    {
        char ach[8];
        for ( int n = 0; n < 8; n++ )
            ach[n] = (char)( nValue >> ( 56 - 8 * n ) );
        PutBuffer( ach, 8 );
    }

    uint8_t CStream::GetUInt8( void )
    {
        return (uint8_t)GetUChar();
    }

    uint16_t CStream::GetUInt16( void )
    {
        unsigned char ach[2];
        GetExact( (char *)ach, 2 );
        return (uint16_t)( ( ach[0] << 8 ) | ach[1] );
    }

    uint32_t CStream::GetUInt32( void )
    {
        unsigned char ach[4];
        GetExact( (char *)ach, 4 );
        uint32_t nValue = 0;
        for ( int n = 0; n < 4; n++ )
            nValue = ( nValue << 8 ) | ach[n];
        return nValue;
    }

    uint64_t CStream::GetUInt64( void )
    {
        unsigned char ach[8];
        GetExact( (char *)ach, 8 );
        uint64_t nValue = 0;
        for ( int n = 0; n < 8; n++ )
            nValue = ( nValue << 8 ) | ach[n];
        return nValue;
    }

        // Text out

    CStream &CStream::operator << ( const CString &strPut )
    {
        PutBuffer( strPut, (int)strPut.GetLength() );
        return *this;
    }

    CStream &CStream::operator << ( const char *strPut )
    {
        if ( strPut != NULL )
            PutBuffer( strPut, (int)strlen( strPut ) );
        return *this;
    }

#define IASLIB_STREAM_PUT_NUMBER__( TYPE ) \
    CStream &CStream::operator << ( const TYPE nPut ) \
    { \
        CString strValue; \
        strValue = nPut; \
        PutBuffer( strValue, (int)strValue.GetLength() ); \
        return *this; \
    }

    IASLIB_STREAM_PUT_NUMBER__( int )
    IASLIB_STREAM_PUT_NUMBER__( unsigned int )
    IASLIB_STREAM_PUT_NUMBER__( long )
    IASLIB_STREAM_PUT_NUMBER__( unsigned long )
    IASLIB_STREAM_PUT_NUMBER__( long long )
    IASLIB_STREAM_PUT_NUMBER__( unsigned long long )
    IASLIB_STREAM_PUT_NUMBER__( double )

#undef IASLIB_STREAM_PUT_NUMBER__

    CStream &CStream::operator << ( const short nPut )
    {
        return *this << (int)nPut;
    }

    CStream &CStream::operator << ( const unsigned short nPut )
    {
        return *this << (unsigned int)nPut;     // (it used to print "%ud": "5d")
    }

        // Text in

    CString CStream::ReadNumberText( bool bFloat )
    {
        while ( ! IsEOS() && isspace( (unsigned char)PeekChar() ) )
            GetChar();

        std::string strText;
        while ( ! IsEOS() )
        {
            char ch = PeekChar();
            bool bTake = isdigit( (unsigned char)ch ) ||
                         ( ( ch == '-' || ch == '+' ) && ( strText.empty() || ( bFloat && ( strText[ strText.size() - 1 ] | 0x20 ) == 'e' ) ) ) ||
                         ( bFloat && ( ch == '.' || ch == 'e' || ch == 'E' ) );
            if ( ! bTake )
                break;
            strText += GetChar();
        }
        if ( strText.empty() )
            IASLIB_THROW_STREAM_EXCEPTION( "Expected a number in the stream." );
        return CString( strText.data(), strText.size() );
    }

    long long CStream::ReadSigned( long long nMin, long long nMax )
    {
        CString strText = ReadNumberText( false );
        char *pEnd = NULL;
        errno = 0;
        long long nValue = strtoll( strText, &pEnd, 10 );
        if ( pEnd == NULL || *pEnd != '\0' || errno == ERANGE || nValue < nMin || nValue > nMax )
            IASLIB_THROW_STREAM_EXCEPTION( "Number in the stream is malformed or out of range." );
        return nValue;
    }

    unsigned long long CStream::ReadUnsigned( unsigned long long nMax )
    {
        CString strText = ReadNumberText( false );
        char *pEnd = NULL;
        errno = 0;
        unsigned long long nValue = strtoull( strText, &pEnd, 10 );
        if ( strText[0] == '-' || pEnd == NULL || *pEnd != '\0' || errno == ERANGE || nValue > nMax )
            IASLIB_THROW_STREAM_EXCEPTION( "Number in the stream is malformed or out of range." );
        return nValue;
    }

    CStream &CStream::operator >> ( short &nGet ) { nGet = (short)ReadSigned( -32768, 32767 ); return *this; }
    CStream &CStream::operator >> ( unsigned short &nGet ) { nGet = (unsigned short)ReadUnsigned( 65535 ); return *this; }
    CStream &CStream::operator >> ( int &nGet ) { nGet = (int)ReadSigned( -2147483647LL - 1, 2147483647LL ); return *this; }
    CStream &CStream::operator >> ( unsigned int &nGet ) { nGet = (unsigned int)ReadUnsigned( 4294967295ULL ); return *this; }
    CStream &CStream::operator >> ( long &nGet ) { nGet = (long)ReadSigned( ( sizeof( long ) == 8 ) ? ( -9223372036854775807LL - 1 ) : ( -2147483647LL - 1 ), ( sizeof( long ) == 8 ) ? 9223372036854775807LL : 2147483647LL ); return *this; }
    CStream &CStream::operator >> ( unsigned long &nGet ) { nGet = (unsigned long)ReadUnsigned( ( sizeof( long ) == 8 ) ? 18446744073709551615ULL : 4294967295ULL ); return *this; }
    CStream &CStream::operator >> ( long long &nGet ) { nGet = ReadSigned( -9223372036854775807LL - 1, 9223372036854775807LL ); return *this; }
    CStream &CStream::operator >> ( unsigned long long &nGet ) { nGet = ReadUnsigned( 18446744073709551615ULL ); return *this; }

    CStream &CStream::operator >> ( double &dGet )
    {
        CString strText = ReadNumberText( true );
            // In the C locale, whatever the program's ('.' is the point).
        std::istringstream oIn( std::string( strText.c_str(), strText.GetLength() ) );
        oIn.imbue( std::locale::classic() );
        double dValue = 0.0;
        oIn >> dValue;
        if ( oIn.fail() || ! oIn.eof() )
            IASLIB_THROW_STREAM_EXCEPTION( "Number in the stream is malformed." );
        dGet = dValue;
        return *this;
    }

    CStream &CStream::operator >> ( CString &strGet )
    {
        while ( ! IsEOS() && isspace( (unsigned char)PeekChar() ) )
            GetChar();
        std::string strWord;
        while ( ! IsEOS() && ! isspace( (unsigned char)PeekChar() ) )
            strWord += GetChar();
        strGet = CString( strWord.data(), strWord.size() );
        return *this;
    }
} // namespace IASLib
