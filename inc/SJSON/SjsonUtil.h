/**
 * @file SjsonUtil.h
 * @brief Helpers for SJSON: UTF-8, CRC-32, base64, Python-compatible float
 *        text, LZ4 blocks and proleptic Gregorian day numbers.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSONUTIL_H__
#define IASLIB_SJSONUTIL_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include <string>
#include <vector>

namespace IASLib
{
    namespace SjsonUtil
    {
            /// The code points of UTF-8 text (invalid bytes are taken as Latin-1).
        std::vector<unsigned> codePoints( const std::string &strUtf8 );
            /// UTF-8 text from code points.
        std::string         utf8( const std::vector<unsigned> &aCodePoints );
            /// Appends one code point as UTF-8.
        void                appendUtf8( std::string &strOut, unsigned nCodePoint );

            /// The zlib / IEEE 802.3 CRC-32.
        unsigned            crc32( const unsigned char *pData, size_t nLength );

            /// Standard base64 with padding.
        std::string         base64Encode( const std::string &strData );
            /// Strict standard base64 decoding; false if the text is not valid.
        bool                base64Decode( const std::string &strText, std::string &strData );

            /// The shortest text that reads back as dValue, formatted exactly as
            /// Python's repr(float) ("1.5", "1e-07", "1e+16", "-0.0", "inf", "nan").
        std::string         floatRepr( double dValue );
            /// strtod for text with a '.' decimal point, whatever the C
            /// locale says. pnUsed, if given, receives the characters parsed.
        double              parseDouble( const std::string &strText, size_t *pnUsed = NULL );

            /// Decompresses an LZ4 block (no frame, no stored size) into exactly
            /// nOutLength bytes; false if the block is malformed.
        bool                lz4Decompress( const std::string &strBlock, size_t nOutLength, std::string &strOut );
            /// True if this build can write LZ4 blocks (liblz4 was found).
        bool                lz4Available( void );
            /// Compresses into an LZ4 block like LZ4_compress_default; false if
            /// LZ4 compression is unavailable.
        bool                lz4Compress( const std::string &strData, std::string &strBlock );

            /// @name Decimal integers of any size ("-123"), for values beyond 64 bits.
            ///@{
            /// a - b.
        std::string         decimalSubtract( const std::string &strA, const std::string &strB );
            /// a + b.
        std::string         decimalAdd( const std::string &strA, const std::string &strB );
            /// The decimal text of an unsigned number given as bits, most
            /// significant first ("1011" gives "11").
        std::string         decimalFromBits( const std::vector<bool> &aBits );
            /// The low nCount bits of a non-negative decimal number, most
            /// significant first; false if it is negative or needs more bits.
        bool                decimalToBits( const std::string &strDecimal, size_t nCount, std::vector<bool> &aBits );
            /// The magnitude of a decimal number if it fits in 64 bits.
        bool                decimalMagnitude( const std::string &strDecimal, unsigned long long &ullMagnitude, bool &bNegative );
            ///@}

            /// Days from 0001-01-01 (day 1), proleptic Gregorian, as Python's
            /// date.toordinal().
        long                dateOrdinal( int nYear, int nMonth, int nDay );
            /// The date of a day number from dateOrdinal(); false if out of range.
        bool                dateFromOrdinal( long lOrdinal, int &nYear, int &nMonth, int &nDay );
            /// True for a valid proleptic Gregorian date in years 1 to 9999.
        bool                validDate( int nYear, int nMonth, int nDay );
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSONUTIL_H__
