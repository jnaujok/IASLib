/**
 * SJSON bit writer and reader. See SjsonBits.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonBits.h"

namespace IASLib
{
    namespace
    {
            // (exclusive limit, prefix bits, prefix length, data bits)
        struct NybbleWidth { unsigned long long ullLimit; unsigned nPrefix; unsigned nPrefixBits; unsigned nDataBits; };
        const NybbleWidth g_aWidths[] =
        {
            { 1ULL << 4,  0x00, 1, 4 },
            { 1ULL << 8,  0x02, 2, 8 },
            { 1ULL << 12, 0x06, 3, 12 },
            { 1ULL << 16, 0x0E, 4, 16 },
            { 1ULL << 20, 0x1E, 5, 20 },
            { 1ULL << 24, 0x3E, 6, 24 },
            { 1ULL << 28, 0x7E, 7, 28 },
            { 1ULL << 32, 0x7F, 7, 32 },
        };
    }

    void CSjsonBitWriter::writeBits( unsigned long long ullValue, unsigned nCount )
    {
        while ( nCount > 0 )
        {
            size_t nBit = m_nBits & 7;
            if ( nBit == 0 )
                m_aBytes.push_back( 0 );
            unsigned nFree = (unsigned)( 8 - nBit );
            unsigned nTake = nCount < nFree ? nCount : nFree;
            unsigned long long ullChunk = ( ullValue >> ( nCount - nTake ) ) & ( ( 1ULL << nTake ) - 1 );
            m_aBytes.back() |= (unsigned char)( ullChunk << ( nFree - nTake ) );
            m_nBits += nTake;
            nCount -= nTake;
        }
    }

    void CSjsonBitWriter::writeBytes( const unsigned char *pData, size_t nLength )
    {
        if ( ( m_nBits & 7 ) == 0 )
        {
            m_aBytes.insert( m_aBytes.end(), pData, pData + nLength );
            m_nBits += nLength * 8;
            return;
        }
        for ( size_t n = 0; n < nLength; n++ )
            writeBits( pData[ n ], 8 );
    }

    void CSjsonBitWriter::writeBinary( const char *pszBits )
    {
        for ( ; *pszBits; pszBits++ )
            writeBool( *pszBits == '1' );
    }

    void CSjsonBitWriter::append( const CSjsonBitWriter &other )
    {
        size_t nWhole = other.m_nBits / 8;
        writeBytes( other.m_aBytes.data(), nWhole );
        unsigned nRest = (unsigned)( other.m_nBits & 7 );
        if ( nRest )
            writeBits( other.m_aBytes[ nWhole ] >> ( 8 - nRest ), nRest );
    }

    unsigned CSjsonBitWriter::nybbleBits( unsigned long long ullValue )
    {
        for ( size_t n = 0; n < sizeof( g_aWidths ) / sizeof( g_aWidths[ 0 ] ); n++ )
            if ( ullValue < g_aWidths[ n ].ullLimit )
                return g_aWidths[ n ].nPrefixBits + g_aWidths[ n ].nDataBits;
        throw CSjsonException( SJSON_ERROR_MALFORMED, "NybbleField values are limited to 32 bits" );
    }

    void CSjsonBitWriter::writeNybble( unsigned long long ullValue )
    {
        for ( size_t n = 0; n < sizeof( g_aWidths ) / sizeof( g_aWidths[ 0 ] ); n++ )
        {
            if ( ullValue < g_aWidths[ n ].ullLimit )
            {
                writeBits( g_aWidths[ n ].nPrefix, g_aWidths[ n ].nPrefixBits );
                writeBits( ullValue, g_aWidths[ n ].nDataBits );
                return;
            }
        }
        throw CSjsonException( SJSON_ERROR_MALFORMED, "NybbleField values are limited to 32 bits" );
    }

    std::string CSjsonBitWriter::binString( void ) const
    {
        std::string strOut;
        for ( size_t n = 0; n < m_nBits; n++ )
            strOut += ( m_aBytes[ n / 8 ] >> ( 7 - ( n & 7 ) ) ) & 1 ? '1' : '0';
        return strOut;
    }

    void CSjsonBitReader::require( size_t nCount ) const
    {
        if ( remaining() < nCount )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "unexpected end of bitstream" );
    }

    unsigned long long CSjsonBitReader::readBits( unsigned nCount )
    {
        require( nCount );
        unsigned long long ullValue = 0;
        while ( nCount > 0 )
        {
            unsigned nBit = (unsigned)( m_nPos & 7 );
            unsigned nAvail = 8 - nBit;
            unsigned nTake = nCount < nAvail ? nCount : nAvail;
            unsigned char byValue = m_pData[ m_nPos / 8 ];
            unsigned long long ullChunk = ( byValue >> ( nAvail - nTake ) ) & ( ( 1U << nTake ) - 1 );
            ullValue = ( nTake == 64 ? 0 : ( ullValue << nTake ) ) | ullChunk;
            m_nPos += nTake;
            nCount -= nTake;
        }
        return ullValue;
    }

    std::string CSjsonBitReader::readBytes( size_t nLength )
    {
        require( nLength * 8 );
        std::string strOut;
        strOut.resize( nLength );
        if ( ( m_nPos & 7 ) == 0 )
        {
            if ( nLength )
                strOut.assign( (const char *)m_pData + m_nPos / 8, nLength );
            m_nPos += nLength * 8;
            return strOut;
        }
        for ( size_t n = 0; n < nLength; n++ )
            strOut[ n ] = (char)readBits( 8 );
        return strOut;
    }

    unsigned long long CSjsonBitReader::readNybble( void )
    {
        unsigned nOnes = 0;
        while ( nOnes < 7 && readBool() )
            nOnes++;
        return readBits( g_aWidths[ nOnes ].nDataBits );
    }

    std::string CSjsonBitReader::peekBinary( unsigned nCount ) const
    {
        std::string strOut;
        for ( size_t n = m_nPos; n < m_nEnd && strOut.size() < nCount; n++ )
            strOut += ( m_pData[ n / 8 ] >> ( 7 - ( n & 7 ) ) ) & 1 ? '1' : '0';
        return strOut;
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
