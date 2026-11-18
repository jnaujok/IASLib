/**
 * @file SjsonBits.h
 * @brief Bit-level writer and reader, and NybbleField integers, for SJSON.
 *
 * SJSON (streaming JSON) packs JSON values into a bitstream. These classes
 * write and read that stream most-significant bit first, the same layout
 * as the reference Python implementation (https://github.com/jnaujok/sjson).
 *
 * A NybbleField is SJSON's variable-length unsigned integer: a unary prefix
 * that says how many nybbles follow.
 *
 * | prefix  | data bits | values              |
 * |:--------|:---------:|:--------------------|
 * | 0       | 4         | 0 - 15              |
 * | 10      | 8         | 16 - 255            |
 * | 110     | 12        | 256 - 4,095         |
 * | 1110    | 16        | up to 65,535        |
 * | 11110   | 20        | up to 1,048,575     |
 * | 111110  | 24        | up to 16,777,215    |
 * | 1111110 | 28        | up to 268,435,455   |
 * | 1111111 | 32        | up to 4,294,967,295 |
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSONBITS_H__
#define IASLIB_SJSONBITS_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace IASLib
{
        /// What went wrong while decoding SJSON.
    enum SjsonError
    {
        SJSON_OK = 0,               ///< No error.
        SJSON_ERROR_MALFORMED,      ///< The bits do not form a valid packet or value.
        SJSON_ERROR_CRC,            ///< The packet's CRC does not match.
        SJSON_ERROR_VERSION,        ///< The packet version is not supported.
        SJSON_ERROR_UNKNOWN_TAG,    ///< A tag or interned value is not known yet: send a NAK.
        SJSON_ERROR_UNKNOWN_SCHEMA, ///< The packet names an unregistered schema or dictionary.
        SJSON_ERROR_STREAM_GAP,     ///< A stream-mode packet arrived out of sequence.
        SJSON_ERROR_UNSUPPORTED     ///< The packet needs a feature this build lacks.
    };

        /// Thrown inside the SJSON code; CSjson turns it into an SjsonError.
    class CSjsonException : public std::runtime_error
    {
        public:
                                    CSjsonException( SjsonError eError, const std::string &strWhat )
                                        : std::runtime_error( strWhat ), m_eError( eError ) {}
            SjsonError              error( void ) const { return m_eError; }

        private:
            SjsonError              m_eError;   ///< The error code.
    };

        /// Appends bits, most significant first, to a growing byte buffer.
    class CSjsonBitWriter
    {
        public:
                                    CSjsonBitWriter( void ) : m_nBits( 0 ) {}

                /// Appends the low nCount bits of ullValue (nCount up to 64).
            void                    writeBits( unsigned long long ullValue, unsigned nCount );
                /// Appends one bit.
            void                    writeBool( bool bValue ) { writeBits( bValue ? 1 : 0, 1 ); }
                /// Appends whole bytes (8 bits each).
            void                    writeBytes( const unsigned char *pData, size_t nLength );
                /// Appends the bytes of a string.
            void                    writeBytes( const std::string &strData ) { writeBytes( (const unsigned char *)strData.data(), strData.size() ); }
                /// Appends a bit pattern given as text, such as "0110".
            void                    writeBinary( const char *pszBits );
                /// Appends another writer's bits.
            void                    append( const CSjsonBitWriter &other );
                /// Appends a NybbleField (values up to 2^32 - 1).
            void                    writeNybble( unsigned long long ullValue );
                /// The number of bits written.
            size_t                  bitLength( void ) const { return m_nBits; }
                /// The bytes, the last one padded with zero bits.
            const std::vector<unsigned char> &bytes( void ) const { return m_aBytes; }
                /// Removes everything.
            void                    clear( void ) { m_aBytes.clear(); m_nBits = 0; }
                /// The bits as text, for tests and debugging.
            std::string             binString( void ) const;

                /// Bits a NybbleField takes for ullValue.
            static unsigned         nybbleBits( unsigned long long ullValue );
                /// The largest NybbleField value.
            static const unsigned long long NYBBLE_MAX = 0xFFFFFFFFULL;

        private:
            std::vector<unsigned char> m_aBytes;    ///< The bits, packed.
            size_t                  m_nBits;        ///< How many bits are used.
    };

        /// Reads bits, most significant first, from a byte buffer.
        ///
        /// Reading past the end throws CSjsonException (SJSON_ERROR_MALFORMED).
    class CSjsonBitReader
    {
        public:
                /// Reads nBitLength bits of pData, starting at bit nStart.
                                    CSjsonBitReader( const unsigned char *pData, size_t nBitLength, size_t nStart = 0 )
                                        : m_pData( pData ), m_nEnd( nBitLength ), m_nPos( nStart ) {}

                /// The next nCount bits (up to 64) as an unsigned number.
            unsigned long long      readBits( unsigned nCount );
                /// The next bit.
            bool                    readBool( void ) { return readBits( 1 ) != 0; }
                /// The next nLength whole bytes.
            std::string             readBytes( size_t nLength );
                /// The next NybbleField.
            unsigned long long      readNybble( void );
                /// The next nCount bits as text, without consuming them (fewer at the end).
            std::string             peekBinary( unsigned nCount ) const;
                /// Bits left.
            size_t                  remaining( void ) const { return m_nEnd > m_nPos ? m_nEnd - m_nPos : 0; }
                /// The current bit position.
            size_t                  position( void ) const { return m_nPos; }

        private:
            void                    require( size_t nCount ) const;

            const unsigned char    *m_pData;    ///< The bytes.
            size_t                  m_nEnd;     ///< The bit length.
            size_t                  m_nPos;     ///< The next bit.
    };
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSONBITS_H__
