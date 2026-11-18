/**
 * SJSON value encodings. See SjsonCodec.h.
 *
 *  The choices here mirror the reference Python library exactly (candidate
 * order, tie-breaking, Python's float formatting), so that the same document
 * gives the same bytes in both.
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

#include "SjsonCodec.h"
#include "SjsonStream.h"
#include "SjsonUtil.h"

#include "ArrayNode.h"
#include "BinaryNode.h"
#include "BooleanNode.h"
#include "JsonNode.h"
#include "NullNode.h"
#include "NumberNode.h"
#include "ObjectNode.h"
#include "TextNode.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace IASLib
{
    namespace SjsonCodec
    {
        const char *const NODE_NULL = "000";
        const char *const NODE_BOOLEAN = "001";
        const char *const NODE_NUMBER = "010";
        const char *const NODE_STRING = "011";
        const char *const NODE_ARRAY = "100";
        const char *const NODE_OBJECT = "101";
        const char *const END_OF_ARRAY = "110";
        const char *const END_OF_MEMBERS = "00000";

        Options &options( void )
        {
            static thread_local Options s_options;
            return s_options;
        }

        // ---- value helpers ----------------------------------------------

        SjsonKind kindOf( const CJsonNode *pNode )
        {
            if ( pNode == NULL )
                return SJSON_KIND_NULL;
            switch ( pNode->getNodeType() )
            {
                case JSON_BOOLEAN:  return SJSON_KIND_BOOL;
                case JSON_NUMBER:   return pNode->isIntegralNumber() ? SJSON_KIND_INT : SJSON_KIND_FLOAT;
                case JSON_STRING:   return SJSON_KIND_STRING;
                case JSON_BINARY:   return SJSON_KIND_STRING;
                case JSON_ARRAY:    return SJSON_KIND_ARRAY;
                case JSON_OBJECT:   return SJSON_KIND_OBJECT;
                default:            return SJSON_KIND_NULL;
            }
        }

        std::string integerText( const CJsonNode &node )
        {
            if ( node.isUnsignedLong() )
                return std::to_string( node.unsignedLongValue() );
            if ( node.isBigInteger() )
            {
                CString strText = node.asText();
                return std::string( (const char *)strText, strText.GetLength() );
            }
            return std::to_string( node.longValue() );
        }

        std::string stringText( const CJsonNode &node )
        {
            if ( node.isBinary() )
            {
                std::vector<unsigned char> aData;
                node.binaryValue( aData );
                return SjsonUtil::base64Encode( std::string( aData.begin(), aData.end() ) );
            }
            return node.textString();
        }

        CJsonNode *makeInteger( const std::string &strDecimal )
        {
            unsigned long long ullMagnitude;
            bool bNegative;
            if ( SjsonUtil::decimalMagnitude( strDecimal, ullMagnitude, bNegative ) )
            {
                if ( ! bNegative && ullMagnitude <= 9223372036854775807ULL )
                    return new CNumberNode( (long long)ullMagnitude );
                if ( bNegative && ullMagnitude <= 9223372036854775808ULL )
                    return new CNumberNode( (long long)( 0 - ullMagnitude ) );
                if ( ! bNegative )
                    return new CNumberNode( ullMagnitude );
            }
            CNumberNode *pNode = CNumberNode::fromText( strDecimal );
            if ( pNode == NULL )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid integer: " + strDecimal );
            return pNode;
        }

        static std::string fieldName( const CObjectNode &object, int nIndex )
        {
            CString strName = object.fieldName( nIndex );
            return std::string( (const char *)strName, strName.GetLength() );
        }

        bool sameValue( const CJsonNode *pA, const CJsonNode *pB )
        {
            SjsonKind eKind = kindOf( pA );
            if ( eKind != kindOf( pB ) )
                return false;
            switch ( eKind )
            {
                case SJSON_KIND_NULL:   return true;
                case SJSON_KIND_BOOL:   return pA->booleanValue() == pB->booleanValue();
                case SJSON_KIND_INT:    return integerText( *pA ) == integerText( *pB );
                case SJSON_KIND_FLOAT:
                {
                    double a = pA->doubleValue(), b = pB->doubleValue();
                    return a == b && std::signbit( a ) == std::signbit( b );
                }
                case SJSON_KIND_STRING: return stringText( *pA ) == stringText( *pB );
                case SJSON_KIND_ARRAY:
                {
                    if ( pA->size() != pB->size() )
                        return false;
                    for ( int n = 0; n < pA->size(); n++ )
                        if ( ! sameValue( pA->get( n ), pB->get( n ) ) )
                            return false;
                    return true;
                }
                case SJSON_KIND_OBJECT:
                {
                    const CObjectNode &a = static_cast<const CObjectNode &>( *pA ), &b = static_cast<const CObjectNode &>( *pB );
                    if ( a.size() != b.size() )
                        return false;
                    for ( int n = 0; n < a.size(); n++ )
                        if ( fieldName( a, n ) != fieldName( b, n ) || ! sameValue( a.fieldValue( n ), b.fieldValue( n ) ) )
                            return false;
                    return true;
                }
            }
            return false;
        }

        // ---- numbers ----------------------------------------------------

        static void writeDecimal( const std::string &strRepr, bool bFloat, CSjsonBitWriter &out )
        {
            std::vector<unsigned> aNybbles;
            if ( strRepr == "nan" )
                aNybbles.push_back( 15 );
            else if ( strRepr == "inf" )
                aNybbles.push_back( 14 );
            else if ( strRepr == "-inf" )
            {
                aNybbles.push_back( 13 );
                aNybbles.push_back( 14 );
            }
            else
            {
                for ( size_t n = 0; n < strRepr.size(); n++ )
                {
                    char c = strRepr[ n ];
                    if ( c >= '0' && c <= '9' ) aNybbles.push_back( (unsigned)( c - '0' ) );
                    else if ( c == '.' ) aNybbles.push_back( 10 );
                    else if ( c == 'e' || c == 'E' ) aNybbles.push_back( 11 );
                    else if ( c == '+' ) aNybbles.push_back( 12 );
                    else if ( c == '-' ) aNybbles.push_back( 13 );
                    else throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid character in number: " + strRepr );
                }
            }
            out.writeBinary( "11" );
            out.writeBool( bFloat );
            out.writeNybble( aNybbles.size() );
            for ( size_t n = 0; n < aNybbles.size(); n++ )
                out.writeBits( aNybbles[ n ], 4 );
        }

        void encodeInteger( const std::string &strDecimal, CSjsonBitWriter &out )
        {
            unsigned long long ullMagnitude;
            bool bNegative;
            if ( SjsonUtil::decimalMagnitude( strDecimal, ullMagnitude, bNegative ) && ullMagnitude <= CSjsonBitWriter::NYBBLE_MAX )
            {
                out.writeBinary( "00" );
                out.writeBool( bNegative );
                out.writeNybble( ullMagnitude );
                return;
            }
            writeDecimal( strDecimal, false, out );
        }

        void encodeFloat( double dValue, CSjsonBitWriter &out )
        {
            CSjsonBitWriter decimal, float64, float32;
            writeDecimal( SjsonUtil::floatRepr( dValue ), true, decimal );
            unsigned long long ullBits;
            memcpy( &ullBits, &dValue, sizeof( ullBits ) );
            float64.writeBinary( "10" );
            float64.writeBits( ullBits, 64 );
            const CSjsonBitWriter *pBest = &decimal;
            if ( float64.bitLength() < pBest->bitLength() )
                pBest = &float64;
            if ( std::isfinite( dValue ) )
            {
                float fValue = (float)dValue;
                if ( ! std::isinf( fValue ) && (double)fValue == dValue )
                {
                    unsigned nBits;
                    memcpy( &nBits, &fValue, sizeof( nBits ) );
                    float32.writeBinary( "01" );
                    float32.writeBits( nBits, 32 );
                    if ( float32.bitLength() < pBest->bitLength() )
                        pBest = &float32;
                }
            }
            out.append( *pBest );
        }

        void encodeNumber( const CJsonNode &node, CSjsonBitWriter &out )
        {
            if ( node.isIntegralNumber() )
                encodeInteger( integerText( node ), out );
            else
                encodeFloat( node.doubleValue(), out );
        }

        CJsonNode *decodeNumber( CSjsonBitReader &in )
        {
            unsigned nSubtype = (unsigned)in.readBits( 2 );
            if ( nSubtype == 0 )
            {
                bool bNegative = in.readBool();
                long long llMagnitude = (long long)in.readNybble();
                return new CNumberNode( bNegative ? -llMagnitude : llMagnitude );
            }
            if ( nSubtype == 1 )
            {
                unsigned nBits = (unsigned)in.readBits( 32 );
                float fValue;
                memcpy( &fValue, &nBits, sizeof( fValue ) );
                return new CNumberNode( (double)fValue );
            }
            if ( nSubtype == 2 )
            {
                unsigned long long ullBits = in.readBits( 64 );
                double dValue;
                memcpy( &dValue, &ullBits, sizeof( dValue ) );
                return new CNumberNode( dValue );
            }
            bool bFloat = in.readBool();
            unsigned long long nLength = in.readNybble();
            if ( nLength * 4 > in.remaining() )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "number longer than the message" );
            std::string strText;
            for ( unsigned long long n = 0; n < nLength; n++ )
            {
                unsigned nNybble = (unsigned)in.readBits( 4 );
                if ( nNybble < 10 ) strText += (char)( '0' + nNybble );
                else if ( nNybble == 10 ) strText += '.';
                else if ( nNybble == 11 ) strText += 'E';
                else if ( nNybble == 12 ) strText += '+';
                else if ( nNybble == 13 ) strText += '-';
                else if ( nNybble == 14 ) strText += "inf";
                else strText += "nan";
            }
            if ( bFloat )
            {
                char *pEnd = NULL;
                double dValue = strtod( strText.c_str(), &pEnd );
                if ( strText.empty() || *pEnd != '\0' )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid float: " + strText );
                return new CNumberNode( dValue );
            }
            size_t nStart = ( ! strText.empty() && strText[ 0 ] == '-' ) ? 1 : 0;
            if ( strText.size() == nStart || strText.find_first_not_of( "0123456789", nStart ) != std::string::npos )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid integer: " + strText );
            return makeInteger( strText );
        }

        // ---- strings ----------------------------------------------------

        namespace
        {
            const int SPECIAL_UUID = 0x0, SPECIAL_INTERNED = 0x1, SPECIAL_LZ4 = 0x2, SPECIAL_4BIT = 0x3,
                      SPECIAL_5BIT = 0x4, SPECIAL_6BIT = 0x5, SPECIAL_7BIT = 0x6, SPECIAL_EMPTY = 0x7,
                      SPECIAL_RANGE_N = 0x8, SPECIAL_HEX = 0x9, SPECIAL_DATETIME = 0xA, SPECIAL_BASE64 = 0xB,
                      SPECIAL_TEXT = 0xC;
            const size_t LZ4_MIN_BYTES = 24;
            const size_t TEXT_MIN_BYTES = 8;
            const unsigned DAY_BITS = 22, SECOND_BITS = 17;

            void special( int nCode, CSjsonBitWriter &out )
            {
                out.writeBool( true );
                out.writeBits( (unsigned)nCode, 4 );
            }

            bool isHex( char c ) { return ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ); }

            bool isUuid( const std::string &s )
            {
                if ( s.size() == 32 )
                {
                    for ( size_t n = 0; n < 32; n++ )
                        if ( ! isHex( s[ n ] ) )
                            return false;
                    return true;
                }
                if ( s.size() != 36 )
                    return false;
                for ( size_t n = 0; n < 36; n++ )
                {
                    bool bHyphen = n == 8 || n == 13 || n == 18 || n == 23;
                    if ( bHyphen ? s[ n ] != '-' : ! isHex( s[ n ] ) )
                        return false;
                }
                return true;
            }

            bool isUrl( const std::string &s )
            {
                std::string strLower;
                for ( size_t n = 0; n < s.size() && n < 8; n++ )
                    strLower += (char)( s[ n ] >= 'A' && s[ n ] <= 'Z' ? s[ n ] + 32 : s[ n ] );
                return strLower.compare( 0, 8, "https://" ) == 0 || strLower.compare( 0, 7, "http://" ) == 0;
            }

            int hexValue( char c )
            {
                if ( c >= '0' && c <= '9' ) return c - '0';
                if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
                return c - 'A' + 10;
            }

            void encodeRaw( const std::string &s, CSjsonBitWriter &out )
            {
                out.writeBool( false );
                out.writeNybble( s.size() );
                out.writeBytes( s );
            }

            void encodeRange( const std::vector<unsigned> &aCodes, CSjsonBitWriter &out )
            {
                unsigned nBase = aCodes[ 0 ], nMax = aCodes[ 0 ];
                for ( size_t n = 1; n < aCodes.size(); n++ )
                {
                    if ( aCodes[ n ] < nBase ) nBase = aCodes[ n ];
                    if ( aCodes[ n ] > nMax ) nMax = aCodes[ n ];
                }
                unsigned nWidth = 0;
                for ( unsigned v = nMax - nBase; v; v >>= 1 )
                    nWidth++;
                if ( nWidth < 4 )
                    nWidth = 4;
                if ( nWidth <= 7 )
                    special( SPECIAL_4BIT + (int)nWidth - 4, out );
                else
                {
                    special( SPECIAL_RANGE_N, out );
                    out.writeNybble( nWidth );
                }
                out.writeNybble( aCodes.size() );
                out.writeNybble( nBase );
                for ( size_t n = 0; n < aCodes.size(); n++ )
                    out.writeBits( aCodes[ n ] - nBase, nWidth );
            }

            bool encodeHex( const std::string &s, CSjsonBitWriter &out )
            {
                bool bLower = true, bUpper = true;
                for ( size_t n = 0; n < s.size(); n++ )
                {
                    char c = s[ n ];
                    bool bDigit = c >= '0' && c <= '9';
                    bLower = bLower && ( bDigit || ( c >= 'a' && c <= 'f' ) );
                    bUpper = bUpper && ( bDigit || ( c >= 'A' && c <= 'F' ) );
                }
                if ( ! bLower && ! bUpper )
                    return false;
                special( SPECIAL_HEX, out );
                out.writeBool( ! bLower );
                out.writeNybble( s.size() );
                for ( size_t n = 0; n < s.size(); n++ )
                    out.writeBits( (unsigned)hexValue( s[ n ] ), 4 );
                return true;
            }

            bool encodeBase64( const std::string &s, CSjsonBitWriter &out )
            {
                if ( s.size() < 8 || s.size() % 4 != 0 )
                    return false;
                size_t nPadStart = s.find( '=' );
                size_t nBody = nPadStart == std::string::npos ? s.size() : nPadStart;
                if ( nBody == 0 || s.size() - nBody > 2 )
                    return false;
                for ( size_t n = 0; n < s.size(); n++ )
                {
                    char c = s[ n ];
                    bool bAlpha = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || c == '+' || c == '/';
                    if ( n < nBody ? ! bAlpha : c != '=' )
                        return false;
                }
                std::string strData;
                if ( ! SjsonUtil::base64Decode( s, strData ) || SjsonUtil::base64Encode( strData ) != s )
                    return false;
                special( SPECIAL_BASE64, out );
                out.writeNybble( strData.size() );
                out.writeBytes( strData );
                return true;
            }

            bool allDigits( const std::string &s, size_t nPos, size_t nCount )
            {
                if ( nPos + nCount > s.size() )
                    return false;
                for ( size_t n = nPos; n < nPos + nCount; n++ )
                    if ( s[ n ] < '0' || s[ n ] > '9' )
                        return false;
                return true;
            }

            int digits( const std::string &s, size_t nPos, size_t nCount )
            {
                return atoi( s.substr( nPos, nCount ).c_str() );
            }

            bool encodeDatetime( const std::string &s, CSjsonBitWriter &out )
            {
                    // ^(\d{4})-(\d{2})-(\d{2})(?:T(\d{2}):(\d{2}):(\d{2})(?:\.(\d{1,9}))?(Z|[+-]\d{2}:\d{2})?)?$
                if ( s.size() < 10 || ! allDigits( s, 0, 4 ) || s[ 4 ] != '-' || ! allDigits( s, 5, 2 ) || s[ 7 ] != '-' || ! allDigits( s, 8, 2 ) )
                    return false;
                int nYear = digits( s, 0, 4 ), nMonth = digits( s, 5, 2 ), nDay = digits( s, 8, 2 );
                bool bTime = s.size() > 10;
                int nHour = 0, nMinute = 0, nSecond = 0;
                size_t nDigits = 0;
                long lFraction = 0;
                std::string strZone;
                if ( bTime )
                {
                    if ( s.size() < 19 || s[ 10 ] != 'T' || ! allDigits( s, 11, 2 ) || s[ 13 ] != ':' || ! allDigits( s, 14, 2 ) || s[ 16 ] != ':' || ! allDigits( s, 17, 2 ) )
                        return false;
                    nHour = digits( s, 11, 2 );
                    nMinute = digits( s, 14, 2 );
                    nSecond = digits( s, 17, 2 );
                    size_t n = 19;
                    if ( n < s.size() && s[ n ] == '.' )
                    {
                        size_t nStart = ++n;
                        while ( n < s.size() && s[ n ] >= '0' && s[ n ] <= '9' )
                            n++;
                        nDigits = n - nStart;
                        if ( nDigits < 1 || nDigits > 9 )
                            return false;
                        lFraction = atol( s.substr( nStart, nDigits ).c_str() );
                    }
                    if ( n < s.size() )
                    {
                        if ( s[ n ] == 'Z' && n + 1 == s.size() )
                            strZone = "Z";
                        else if ( ( s[ n ] == '+' || s[ n ] == '-' ) && n + 6 == s.size() && allDigits( s, n + 1, 2 ) && s[ n + 3 ] == ':' && allDigits( s, n + 4, 2 ) )
                            strZone = s.substr( n );
                        else
                            return false;
                    }
                }
                if ( ! SjsonUtil::validDate( nYear, nMonth, nDay ) )
                    return false;
                if ( bTime && ( nHour > 23 || nMinute > 59 || nSecond > 59 ) )
                    return false;
                CSjsonBitWriter bits;
                special( SPECIAL_DATETIME, bits );
                bits.writeBool( bTime );
                bits.writeBits( (unsigned long long)( SjsonUtil::dateOrdinal( nYear, nMonth, nDay ) - 1 ), DAY_BITS );
                if ( bTime )
                {
                    bits.writeBits( (unsigned long long)( nHour * 3600 + nMinute * 60 + nSecond ), SECOND_BITS );
                    bits.writeBits( nDigits, 4 );
                    if ( nDigits )
                    {
                        unsigned long long ullMax = 1;
                        for ( size_t k = 0; k < nDigits; k++ )
                            ullMax *= 10;
                        unsigned nWidth = 0;
                        for ( unsigned long long v = ullMax - 1; v; v >>= 1 )
                            nWidth++;
                        bits.writeBits( (unsigned long long)lFraction, nWidth );
                    }
                    if ( strZone.empty() )
                        bits.writeBinary( "00" );
                    else if ( strZone == "Z" )
                        bits.writeBinary( "01" );
                    else
                    {
                        int nZoneHours = digits( strZone, 1, 2 ), nZoneMinutes = digits( strZone, 4, 2 );
                        if ( nZoneHours > 23 || nZoneMinutes > 59 )
                            return false;
                        bits.writeBinary( "10" );
                        bits.writeBool( strZone[ 0 ] == '-' );
                        bits.writeBits( (unsigned)nZoneHours, 5 );
                        bits.writeBits( (unsigned)nZoneMinutes, 6 );
                    }
                }
                out.append( bits );
                return true;
            }

            bool encodeLz4( const std::string &s, CSjsonBitWriter &out )
            {
                if ( s.size() < LZ4_MIN_BYTES || ! options().bLz4 )
                    return false;
                std::string strPacked;
                if ( ! SjsonUtil::lz4Compress( s, strPacked ) )
                    return false;
                special( SPECIAL_LZ4, out );
                out.writeNybble( s.size() );
                out.writeNybble( strPacked.size() );
                out.writeBytes( strPacked );
                return true;
            }

            void encodeUuid( const std::string &s, CSjsonBitWriter &out )
            {
                special( SPECIAL_UUID, out );
                bool bUpper = false;
                std::string strHex;
                for ( size_t n = 0; n < s.size(); n++ )
                {
                    if ( s[ n ] == '-' )
                        continue;
                    bUpper = bUpper || ( s[ n ] >= 'A' && s[ n ] <= 'F' );
                    strHex += s[ n ];
                }
                out.writeBool( s.size() == 36 );
                out.writeBool( bUpper );
                for ( size_t n = 0; n < 32; n += 2 )
                    out.writeBits( (unsigned)( hexValue( strHex[ n ] ) * 16 + hexValue( strHex[ n + 1 ] ) ), 8 );
            }
        }

        void encodeString( const std::string &s, CSjsonDictionary *pDictionary, CSjsonBitWriter &out )
        {
            if ( s.empty() )
            {
                special( SPECIAL_EMPTY, out );
                return;
            }
            CSjsonDictionary *pValues = pDictionary ? pDictionary->values() : NULL;
            if ( pValues != NULL && pValues->hasTag( s ) )
            {
                special( SPECIAL_INTERNED, out );
                out.writeNybble( pValues->get( s ) );
                return;
            }
            if ( isUuid( s ) )
            {
                bool bUpper = false, bLower = false;
                for ( size_t n = 0; n < s.size(); n++ )
                {
                    bUpper = bUpper || ( s[ n ] >= 'A' && s[ n ] <= 'F' );
                    bLower = bLower || ( s[ n ] >= 'a' && s[ n ] <= 'f' );
                }
                if ( ! ( bUpper && bLower ) )
                {
                    encodeUuid( s, out );
                    return;
                }
            }
            if ( isUrl( s ) && pValues != NULL )
            {
                    // Interned the second time it is seen: most URLs never repeat.
                if ( pDictionary->seenUrls().count( s ) )
                {
                    special( SPECIAL_INTERNED, out );
                    pValues->encodeId( s, out );
                    return;
                }
                pDictionary->seenUrls().insert( s );
            }

                // Every form that applies; the first shortest wins.
            CSjsonBitWriter best;
            encodeRaw( s, best );
            CSjsonBitWriter candidate;
            std::vector<unsigned> aCodes = SjsonUtil::codePoints( s );
            encodeRange( aCodes, candidate );
            if ( candidate.bitLength() < best.bitLength() )
                best = candidate;
            candidate.clear();
            if ( encodeHex( s, candidate ) && candidate.bitLength() < best.bitLength() )
                best = candidate;
            candidate.clear();
            if ( encodeDatetime( s, candidate ) && candidate.bitLength() < best.bitLength() )
                best = candidate;
            candidate.clear();
            if ( encodeBase64( s, candidate ) && candidate.bitLength() < best.bitLength() )
                best = candidate;
            candidate.clear();
            if ( encodeLz4( s, candidate ) && candidate.bitLength() < best.bitLength() )
                best = candidate;

            CSjsonTextChannel *pChannel = pDictionary ? pDictionary->text() : NULL;
            if ( pChannel != NULL && s.size() >= TEXT_MIN_BYTES && s.find( '\0' ) == std::string::npos &&
                 5 + CSjsonTextChannel::BITS_PER_BYTE * ( s.size() + 1 ) < best.bitLength() )
            {
                pChannel->writeTerminated( s );
                special( SPECIAL_TEXT, out );
                return;
            }
            out.append( best );
        }

        static std::string decodeRange( CSjsonBitReader &in, unsigned nWidth )
        {
            if ( nWidth < 4 || nWidth > 21 )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid range width" );
            unsigned long long nLength = in.readNybble();
            unsigned long long nBase = in.readNybble();
            if ( nLength * nWidth > in.remaining() )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "range-coded string is longer than the message" );
            std::string strOut;
            for ( unsigned long long n = 0; n < nLength; n++ )
            {
                unsigned long long nCode = in.readBits( nWidth ) + nBase;
                if ( nCode > 0x10FFFF )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "code point out of range" );
                SjsonUtil::appendUtf8( strOut, (unsigned)nCode );
            }
            return strOut;
        }

        static std::string decodeDatetime( CSjsonBitReader &in )
        {
            bool bTime = in.readBool();
            int nYear, nMonth, nDay;
            if ( ! SjsonUtil::dateFromOrdinal( (long)in.readBits( DAY_BITS ) + 1, nYear, nMonth, nDay ) )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid date" );
            char szBuffer[ 64 ];
            snprintf( szBuffer, sizeof( szBuffer ), "%04d-%02d-%02d", nYear, nMonth, nDay );
            std::string strOut = szBuffer;
            if ( ! bTime )
                return strOut;
            unsigned long long nSeconds = in.readBits( SECOND_BITS );
            snprintf( szBuffer, sizeof( szBuffer ), "T%02llu:%02llu:%02llu", nSeconds / 3600, nSeconds / 60 % 60, nSeconds % 60 );
            strOut += szBuffer;
            unsigned nDigits = (unsigned)in.readBits( 4 );
            if ( nDigits )
            {
                unsigned long long ullMax = 1;
                for ( unsigned k = 0; k < nDigits; k++ )
                    ullMax *= 10;
                unsigned nWidth = 0;
                for ( unsigned long long v = ullMax - 1; v; v >>= 1 )
                    nWidth++;
                snprintf( szBuffer, sizeof( szBuffer ), ".%0*llu", (int)nDigits, in.readBits( nWidth ) );
                strOut += szBuffer;
            }
            unsigned nZone = (unsigned)in.readBits( 2 );
            if ( nZone == 1 )
                strOut += "Z";
            else if ( nZone == 2 )
            {
                bool bNegative = in.readBool();
                unsigned nHours = (unsigned)in.readBits( 5 ), nMinutes = (unsigned)in.readBits( 6 );
                snprintf( szBuffer, sizeof( szBuffer ), "%c%02u:%02u", bNegative ? '-' : '+', nHours, nMinutes );
                strOut += szBuffer;
            }
            return strOut;
        }

        std::string decodeString( CSjsonBitReader &in, CSjsonDictionary *pDictionary )
        {
            if ( ! in.readBool() )
                return in.readBytes( (size_t)in.readNybble() );
            int nCode = (int)in.readBits( 4 );
            switch ( nCode )
            {
                case SPECIAL_UUID:
                {
                    bool bHyphens = in.readBool(), bUpper = in.readBool();
                    std::string strBytes = in.readBytes( 16 );
                    static const char s_szLower[] = "0123456789abcdef", s_szUpper[] = "0123456789ABCDEF";
                    const char *pszDigits = bUpper ? s_szUpper : s_szLower;
                    std::string strOut;
                    for ( size_t n = 0; n < 16; n++ )
                    {
                        if ( bHyphens && ( n == 4 || n == 6 || n == 8 || n == 10 ) )
                            strOut += '-';
                        strOut += pszDigits[ (unsigned char)strBytes[ n ] >> 4 ];
                        strOut += pszDigits[ (unsigned char)strBytes[ n ] & 15 ];
                    }
                    return strOut;
                }
                case SPECIAL_INTERNED:
                    if ( pDictionary == NULL || pDictionary->values() == NULL )
                        throw CSjsonException( SJSON_ERROR_UNKNOWN_TAG, "a dictionary is needed for interned strings" );
                    return pDictionary->values()->decodeId( in );
                case SPECIAL_LZ4:
                {
                    unsigned long long nRaw = in.readNybble(), nPacked = in.readNybble();
                    if ( nRaw > ( nPacked ? nPacked : 1 ) * 255 )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid LZ4 length" );
                    std::string strPacked = in.readBytes( (size_t)nPacked ), strOut;
                    if ( ! SjsonUtil::lz4Decompress( strPacked, (size_t)nRaw, strOut ) )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid LZ4 block" );
                    return strOut;
                }
                case SPECIAL_4BIT: return decodeRange( in, 4 );
                case SPECIAL_5BIT: return decodeRange( in, 5 );
                case SPECIAL_6BIT: return decodeRange( in, 6 );
                case SPECIAL_7BIT: return decodeRange( in, 7 );
                case SPECIAL_EMPTY: return std::string();
                case SPECIAL_RANGE_N: return decodeRange( in, (unsigned)in.readNybble() );
                case SPECIAL_HEX:
                {
                    bool bUpper = in.readBool();
                    unsigned long long nLength = in.readNybble();
                    if ( nLength * 4 > in.remaining() )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "hex string is longer than the message" );
                    const char *pszDigits = bUpper ? "0123456789ABCDEF" : "0123456789abcdef";
                    std::string strOut;
                    for ( unsigned long long n = 0; n < nLength; n++ )
                        strOut += pszDigits[ in.readBits( 4 ) ];
                    return strOut;
                }
                case SPECIAL_DATETIME: return decodeDatetime( in );
                case SPECIAL_BASE64: return SjsonUtil::base64Encode( in.readBytes( (size_t)in.readNybble() ) );
                case SPECIAL_TEXT:
                {
                    CSjsonTextChannel *pChannel = pDictionary ? pDictionary->text() : NULL;
                    if ( pChannel == NULL )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "text-channel string without a text channel" );
                    return pChannel->readTerminated();
                }
                default:
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid special handling type" );
            }
        }

        // ---- tagged trees -------------------------------------------------

        void encodeTagged( const CJsonNode *pNode, CSjsonDictionary &dictionary, CSjsonBitWriter &out )
        {
            CSjsonStreamState *pStream = dictionary.stream();
            switch ( kindOf( pNode ) )
            {
                case SJSON_KIND_NULL:
                    out.writeBinary( NODE_NULL );
                    return;
                case SJSON_KIND_BOOL:
                    out.writeBinary( NODE_BOOLEAN );
                    out.writeBool( pNode->booleanValue() );
                    return;
                case SJSON_KIND_INT:
                case SJSON_KIND_FLOAT:
                    out.writeBinary( NODE_NUMBER );
                    encodeNumber( *pNode, out );
                    return;
                case SJSON_KIND_STRING:
                    out.writeBinary( NODE_STRING );
                    encodeString( stringText( *pNode ), &dictionary, out );
                    return;
                case SJSON_KIND_ARRAY:
                {
                    out.writeBinary( NODE_ARRAY );
                    if ( pStream )
                        pStream->path().push_back( "[]" );
                    CJsonNode *pArray = const_cast<CJsonNode *>( pNode );
                    for ( int n = 0; n < pArray->size(); n++ )
                        encodeTagged( pArray->get( n ), dictionary, out );
                    if ( pStream )
                        pStream->path().pop_back();
                    out.writeBinary( END_OF_ARRAY );
                    return;
                }
                case SJSON_KIND_OBJECT:
                {
                    const CObjectNode &object = static_cast<const CObjectNode &>( *pNode );
                    out.writeBinary( NODE_OBJECT );
                    if ( pStream == NULL )
                    {
                        for ( int n = 0; n < object.size(); n++ )
                        {
                            dictionary.encodeId( fieldName( object, n ), out );
                            encodeTagged( object.fieldValue( n ), dictionary, out );
                        }
                        out.writeBinary( END_OF_MEMBERS );
                        return;
                    }
                        // Stream mode: the shape (member names) is cached, then
                        // each member value goes through the cache.
                    std::vector<std::string> aNames;
                    CArrayNode shape;
                    for ( int n = 0; n < object.size(); n++ )
                    {
                        aNames.push_back( fieldName( object, n ) );
                        shape.add( aNames.back() );
                    }
                    pStream->encode( pStream->key( "{}" ), shape, [&]( CSjsonBitWriter &bits )
                    {
                        bits.writeNybble( aNames.size() );
                        for ( size_t n = 0; n < aNames.size(); n++ )
                            dictionary.encodeId( aNames[ n ], bits );
                    }, NULL, out );
                    for ( int n = 0; n < object.size(); n++ )
                    {
                        const std::string &strName = aNames[ (size_t)n ];
                        const CJsonNode *pValue = object.fieldValue( n );
                        CNullNode null;
                        pStream->encode( pStream->key( strName ), pValue ? *pValue : null, [&]( CSjsonBitWriter &bits )
                        {
                            pStream->path().push_back( strName );
                            encodeTagged( pValue, dictionary, bits );
                            pStream->path().pop_back();
                        }, &dictionary, out );
                    }
                    return;
                }
            }
        }

        CJsonNode *decodeTagged( CSjsonBitReader &in, CSjsonDictionary &dictionary )
        {
            CSjsonStreamState *pStream = dictionary.stream();
            std::string strType = in.peekBinary( 3 );
            if ( strType.size() < 3 )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "unexpected end of bitstream" );
            in.readBits( 3 );
            if ( strType == NODE_NULL )
                return new CNullNode();
            if ( strType == NODE_BOOLEAN )
                return new CBooleanNode( in.readBool() );
            if ( strType == NODE_NUMBER )
                return decodeNumber( in );
            if ( strType == NODE_STRING )
                return new CTextNode( decodeString( in, &dictionary ) );
            if ( strType == NODE_ARRAY )
            {
                std::unique_ptr<CArrayNode> pArray( new CArrayNode() );
                if ( pStream )
                    pStream->path().push_back( "[]" );
                while ( in.peekBinary( 3 ) != END_OF_ARRAY )
                    pArray->add( decodeTagged( in, dictionary ) );
                in.readBits( 3 );
                if ( pStream )
                    pStream->path().pop_back();
                return pArray.release();
            }
            if ( strType == NODE_OBJECT )
            {
                std::unique_ptr<CObjectNode> pObject( new CObjectNode() );
                if ( pStream == NULL )
                {
                    while ( in.peekBinary( 5 ) != END_OF_MEMBERS )
                    {
                        std::string strName = dictionary.decodeId( in );
                        pObject->set( strName, decodeTagged( in, dictionary ) );
                    }
                    in.readBits( 5 );
                    return pObject.release();
                }
                std::unique_ptr<CJsonNode> pShape( pStream->decode( in, pStream->key( "{}" ), [&]( CSjsonBitReader &bits ) -> CJsonNode *
                {
                    std::unique_ptr<CArrayNode> pNames( new CArrayNode() );
                    unsigned long long nCount = bits.readNybble();
                    for ( unsigned long long n = 0; n < nCount; n++ )
                        pNames->add( dictionary.decodeId( bits ) );
                    return pNames.release();
                } ) );
                if ( ! pShape->isArray() )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "object shape is not a list of names" );
                for ( int n = 0; n < pShape->size(); n++ )
                {
                    std::string strName = stringText( *pShape->get( n ) );
                    CJsonNode *pValue = pStream->decode( in, pStream->key( strName ), [&]( CSjsonBitReader &bits ) -> CJsonNode *
                    {
                        pStream->path().push_back( strName );
                        CJsonNode *pResult = decodeTagged( bits, dictionary );
                        pStream->path().pop_back();
                        return pResult;
                    } );
                    pObject->set( strName, pValue );
                }
                return pObject.release();
            }
            throw CSjsonException( SJSON_ERROR_MALFORMED, "unknown node type " + strType );
        }
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
