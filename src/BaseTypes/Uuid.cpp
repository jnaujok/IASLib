/**
 * UUID class
 * 
 * This class generates and stores UUIDs that meet requirements for uniqueness.
 * 
 * Author: Jeffrey R. Naujok
 * Created: August 25, 2019
 * 
 * Copyright (c) 2019, Irene Adler Software, All Rights Reserved.
 */

#include "Uuid.h"
#include <arpa/inet.h>

namespace IASLib 
{
    IMPLEMENT_OBJECT( CUUID, CObject );

    bool CUUID::bSeeded = false;
#ifdef IASLIB_MULTI_THREADED__
    CMutex CUUID::randomMutex;
#endif
    TRandom< (8), __uint32_t > *CUUID::m_pRandomGenerator = NULL;

    CUUID::CUUID( void ) 
    {
        m_IsValid = true;
        generateRandomUUID();
        strUUID = bytesToString( bytes );
    }

    CUUID::CUUID( const char *uuid )
    {
        m_IsValid = false;

        if ( stringToBytes( uuid, bytes ) )
        {
            m_IsValid = true;
            strUUID = bytesToString( bytes );
        }
    }

    CUUID::CUUID( const CUUID &oSource )
    {
        m_IsValid = oSource.m_IsValid;
        for ( size_t nX = 0; nX < 32; nX++ )
        {
            bytes[nX] = oSource.bytes[nX];
        }
        if ( m_IsValid )
        {
            strUUID = bytesToString( bytes );
        }
    }

    CUUID::~CUUID( void )
    {
        // Nothing to do or clean-up here.
    }

    unsigned char *CUUID::getBytes( void )
    {
        if ( m_IsValid )
        {
            return bytes;
        }
        return NULL;
    }

    CString CUUID::toString( void )
    {
        if ( m_IsValid )
        {
            return strUUID;
        }
        return CString( "" );
    }

    CUUID CUUID::fromString( const char *uuid )
    {
        CUUID temp = CUUID( uuid );

        return temp;
    }

    CUUID &CUUID::operator =( const CUUID &oSource )
    {
        if ( this != &oSource )
        {
            m_IsValid = oSource.m_IsValid;
            for ( size_t nX = 0; nX < 32; nX++ )
            {
                bytes[nX] = oSource.bytes[nX];
            }
            if ( m_IsValid )
            {
                strUUID = bytesToString( bytes );
            }
        }

        return *this;
    }

    void CUUID::generateRandomUUID( void )
    {
        if ( ! bSeeded )
        {
#ifdef IASLIB_MULTI_THREADED__
            randomMutex.Lock();
            if ( ! bSeeded )
            {
#endif
                if ( m_pRandomGenerator == NULL )
                {
                    m_pRandomGenerator = new TRandom< (8), __uint32_t>( time(NULL), time(NULL) * time(NULL), (time(NULL) * 183601463UL) % 2147430101UL );
                }
                bSeeded = true;
#ifdef IASLIB_MULTI_THREADED__
            }
            randomMutex.Unlock();
#endif
        }

#ifdef IASLIB_MULTI_THREADED__
        randomMutex.Lock();
#endif

        __uint32_t *pLong = (__uint32_t *)bytes;
        // First hextet of two random 32 bit values (64 bits). (8 bytes)
        pLong[0] = m_pRandomGenerator->rand();
        pLong[1] = m_pRandomGenerator->rand();

        // Octet 1-3 (96 bits) [12 bytes]
        pLong[2] = m_pRandomGenerator->rand();
        pLong[3] = ((m_pRandomGenerator->rand() & 0x0fff) | 0x4000);
        pLong[4] = (m_pRandomGenerator->rand() % 0x3fff) + 0x8000;

        // Final 96 bit random value [12 bytes]
        pLong[5] = m_pRandomGenerator->rand();
        pLong[6] = m_pRandomGenerator->rand();
        pLong[7] = m_pRandomGenerator->rand();

#ifdef IASLIB_MULTI_THREADED__
        randomMutex.Unlock();
#endif
    }

    CString CUUID::bytesToString( unsigned char *byteArray )
    {
        __uint32_t *pLong = (__uint32_t *)byteArray;
        CString retVal = "";

        retVal.Format( "%04x%04x-%04x-%04x-%04x-%04x%04x%04x", 
            pLong[0], pLong[1],             // 64-bit Hex number
            pLong[2],                       // 32-bit Hex number
            pLong[3],                       // 32-bit Hex number of the form 4xxx (4 indicates the UUID version)
            pLong[4],                       // 32-bit Hex number in the range [0x8000, 0xbfff]
            pLong[5],                       // 96-bit Hex number
            pLong[6], 
            pLong[7] );
        return retVal;
    }

    bool CUUID::stringToBytes( const char *uuid, unsigned char *bytes )
    {
        unsigned char temp[32];
        size_t nX = 0;
        while ( *uuid && nX < 32 )
        {
            if ( *uuid != '-' )
            {
                // We have a hex byte pair (hopefully)
                temp[nX] = ( hexToByte( *uuid ) * 16 ) + ( hexToByte( *uuid + 1 ) );
                uuid++;
                nX++;
            }
            uuid++;
        }
        if ( nX < 32 || *uuid )
        {
            return false;
        }

        // We need to make sure the bytes are in the correct order for the machine.
        __uint32_t *pLong = (__uint32_t *)temp;
        __uint32_t *pBytes = (__uint32_t *)bytes;

        for ( nX = 0; nX < 8; nX++ )
        {
            pBytes[nX] = ntohl( pLong[nX] );
        }

        return true;
    }

    unsigned char CUUID::hexToByte( char val )
    {
        const char *translate = "0123456789abcdef";
        char low = tolower( val );
        unsigned char nX = 0;
        while ( translate[ nX ] )
        {
            if ( translate[nX] == low )
            {
                return nX;
            }
            nX++;
        }
        return 0;
    }

}; // Namespace IASLib





