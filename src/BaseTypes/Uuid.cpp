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
            srand(time(NULL));
            bSeeded = true;
        }
        unsigned long *pLong = (unsigned long *)bytes;
        // First hextet of two random 32 bit values (64 bits). (8 bytes)
        pLong[0] = rand();
        pLong[1] = rand();

        // Octet 1-3 (96 bits) [12 bytes]
        pLong[2] = rand();
        pLong[3] = ((rand() & 0x0fff) | 0x4000);
        pLong[4] = (rand() % 0x3fff) + 0x8000;

        // Final 96 bit random value [12 bytes]
        pLong[5] = rand();
        pLong[6] = rand();
        pLong[7] = rand();
    }

    CString CUUID::bytesToString( unsigned char *byteArray )
    {
        unsigned long *pLong = (unsigned long *)byteArray;
        CString retVal = "";

        retVal.Format( "%04hhx%04hhx-%04hhx-%04hhx-%04hhx-%04hhx%04hhx%04hhx", 
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
        unsigned long *pLong = (unsigned long *)temp;
        unsigned long *pBytes = (unsigned long *)bytes;

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





