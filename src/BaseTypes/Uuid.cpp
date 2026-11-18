/**
 * UUID class
 *
 * This class generates and stores UUIDs (RFC 9562).
 *
 * Author: Jeffrey R. Naujok
 * Created: August 25, 2019
 *
 * Copyright (c) 2019, Irene Adler Software, All Rights Reserved.
 */

#include "Uuid.h"

#include <random>

namespace IASLib
{
    IMPLEMENT_OBJECT( CUUID, CObject );

    namespace
    {
        int HexValue( char ch )
        {
            if ( ch >= '0' && ch <= '9' ) return ch - '0';
            if ( ch >= 'a' && ch <= 'f' ) return 10 + ( ch - 'a' );
            if ( ch >= 'A' && ch <= 'F' ) return 10 + ( ch - 'A' );
            return -1;
        }
    }

    CUUID::CUUID( void )
    {
        generateRandomUUID();
        m_IsValid = true;
    }

    CUUID::CUUID( const char *uuid )
    {
        memset( m_achBytes, 0, SIZE );
        m_IsValid = stringToBytes( uuid, m_achBytes );
    }

    CUUID::CUUID( const unsigned char achBytes[ SIZE ] )
    {
        m_IsValid = ( achBytes != NULL );
        if ( m_IsValid )
            memcpy( m_achBytes, achBytes, SIZE );
        else
            memset( m_achBytes, 0, SIZE );
    }

    CUUID::CUUID( const CUUID &oSource ) : CObject( oSource )
    {
        m_IsValid = oSource.m_IsValid;
        memcpy( m_achBytes, oSource.m_achBytes, SIZE );
    }

    CUUID::~CUUID( void )
    {
    }

    const unsigned char *CUUID::getBytes( void ) const
    {
        return m_IsValid ? m_achBytes : NULL;
    }

    CString CUUID::toString( void ) const
    {
        return m_IsValid ? bytesToString( m_achBytes ) : CString();
    }

    int CUUID::getVersion( void ) const
    {
        return m_IsValid ? ( m_achBytes[6] >> 4 ) : -1;
    }

    bool CUUID::isNil( void ) const
    {
        if ( ! m_IsValid )
            return false;
        for ( size_t n = 0; n < SIZE; n++ )
            if ( m_achBytes[n] != 0 )
                return false;
        return true;
    }

    CUUID CUUID::Nil( void )
    {
        unsigned char achZero[ SIZE ] = { 0 };
        return CUUID( achZero );
    }

    CUUID CUUID::fromString( const char *uuid )
    {
        return CUUID( uuid );
    }

    CUUID &CUUID::operator =( const CUUID &oSource )
    {
        if ( this != &oSource )
        {
            m_IsValid = oSource.m_IsValid;
            memcpy( m_achBytes, oSource.m_achBytes, SIZE );
        }
        return *this;
    }

    bool CUUID::operator ==( const CUUID &oOther ) const
    {
        if ( m_IsValid != oOther.m_IsValid )
            return false;
        return ! m_IsValid || memcmp( m_achBytes, oOther.m_achBytes, SIZE ) == 0;
    }

    bool CUUID::operator <( const CUUID &oOther ) const
    {
        if ( m_IsValid != oOther.m_IsValid )
            return ! m_IsValid;
        return m_IsValid && memcmp( m_achBytes, oOther.m_achBytes, SIZE ) < 0;
    }

    int CUUID::hashCode( void ) const
    {
        unsigned int nHash = 2166136261u;           // FNV-1a
        for ( size_t n = 0; n < SIZE; n++ )
            nHash = ( nHash ^ m_achBytes[n] ) * 16777619u;
        return (int)nHash;
    }

    void CUUID::generateRandomUUID( void )
    {
            // One random_device per thread: it reads the operating system's
            // random source (getrandom, arc4random or BCryptGenRandom).
        static thread_local std::random_device device;
        for ( size_t n = 0; n < SIZE; n += 4 )
        {
            uint32_t nRandom = (uint32_t)device();
            m_achBytes[n]     = (unsigned char)( nRandom >> 24 );
            m_achBytes[n + 1] = (unsigned char)( nRandom >> 16 );
            m_achBytes[n + 2] = (unsigned char)( nRandom >> 8 );
            m_achBytes[n + 3] = (unsigned char)nRandom;
        }
        m_achBytes[6] = (unsigned char)( ( m_achBytes[6] & 0x0F ) | 0x40 );   // version 4
        m_achBytes[8] = (unsigned char)( ( m_achBytes[8] & 0x3F ) | 0x80 );   // RFC 9562 variant
    }

    CString CUUID::bytesToString( const unsigned char *bytes )
    {
        if ( bytes == NULL )
            return CString();

        static const char *HEX = "0123456789abcdef";
        char achText[ 37 ];
        size_t nOut = 0;
        for ( size_t n = 0; n < SIZE; n++ )
        {
            if ( n == 4 || n == 6 || n == 8 || n == 10 )
                achText[ nOut++ ] = '-';
            achText[ nOut++ ] = HEX[ bytes[n] >> 4 ];
            achText[ nOut++ ] = HEX[ bytes[n] & 0x0F ];
        }
        achText[ nOut ] = 0;
        return CString( achText );
    }

    bool CUUID::stringToBytes( const char *uuid, unsigned char *bytes )
    {
        if ( uuid == NULL || bytes == NULL )
            return false;

        size_t nLength = strlen( uuid );
        if ( nLength >= 2 && uuid[0] == '{' && uuid[ nLength - 1 ] == '}' )
        {
            uuid++;
            nLength -= 2;
        }

        bool bDashed = ( nLength == 36 );
        if ( ! bDashed && nLength != 32 )
            return false;

        unsigned char achParsed[ SIZE ];
        size_t nIn = 0;
        for ( size_t n = 0; n < SIZE; n++ )
        {
            if ( bDashed && ( n == 4 || n == 6 || n == 8 || n == 10 ) )
            {
                if ( uuid[ nIn++ ] != '-' )
                    return false;
            }
            int nHigh = HexValue( uuid[ nIn++ ] );
            int nLow = HexValue( uuid[ nIn++ ] );
            if ( nHigh < 0 || nLow < 0 )
                return false;
            achParsed[n] = (unsigned char)( nHigh * 16 + nLow );
        }

        memcpy( bytes, achParsed, SIZE );
        return true;
    }

} // Namespace IASLib
