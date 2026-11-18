/**
 * @file SjsonSchemaNumber.h
 * @brief SjsonNumber: an integer of any size or a double, with the
 *        arithmetic SJSON's schema bounds need (as Python's int and float).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSONSCHEMANUMBER_H__
#define IASLIB_SJSONSCHEMANUMBER_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonCodec.h"
#include "SjsonUtil.h"
#include "JsonNode.h"
#include "NumberNode.h"

#include <cmath>
#include <cstdlib>
#include <string>

namespace IASLib
{
        /// An exact integer (decimal text) or a double.
    struct SjsonNumber
    {
        bool        bInt;       ///< Whether this is an integer.
        std::string strInt;     ///< The integer, as decimal text.
        double      d;          ///< The double (when not bInt).

                    SjsonNumber( void ) : bInt( true ), strInt( "0" ), d( 0 ) {}
        static SjsonNumber integer( const std::string &strDecimal ) { SjsonNumber n; n.bInt = true; n.strInt = strDecimal; return n; }
        static SjsonNumber real( double dValue ) { SjsonNumber n; n.bInt = false; n.d = dValue; return n; }

            /// From a JSON number node (not a boolean); false otherwise.
        static bool from( const CJsonNode &node, SjsonNumber &out )
        {
            if ( ! node.isNumber() )
                return false;
            out = node.isIntegralNumber() ? integer( SjsonCodec::integerText( node ) ) : real( node.doubleValue() );
            return true;
        }

            /// The exact decimal text of an integral double.
        static std::string decimalOf( double d )
        {
            if ( std::fabs( d ) < 9.0e18 )
                return std::to_string( (long long)d );
            int nExponent;
            double dMantissa = std::frexp( std::fabs( d ), &nExponent );
            unsigned long long ullMantissa = (unsigned long long)std::ldexp( dMantissa, 53 );
            std::string strOut = std::to_string( ullMantissa );
            for ( int n = 0; n < nExponent - 53; n++ )
                strOut = SjsonUtil::decimalAdd( strOut, strOut );
            return d < 0 ? "-" + strOut : strOut;
        }

        double value( void ) const { return bInt ? strtod( strInt.c_str(), NULL ) : d; }

        SjsonNumber plus( long lDelta ) const
        {
            return bInt ? integer( SjsonUtil::decimalAdd( strInt, std::to_string( lDelta ) ) ) : real( d + (double)lDelta );
        }
        SjsonNumber floor( void ) const { return bInt ? *this : integer( decimalOf( std::floor( d ) ) ); }
        SjsonNumber ceil( void ) const { return bInt ? *this : integer( decimalOf( std::ceil( d ) ) ); }

            /// -1, 0 or 1, as Python compares int and float.
        static int compare( const SjsonNumber &a, const SjsonNumber &b )
        {
            if ( a.bInt && b.bInt )
            {
                std::string strDiff = SjsonUtil::decimalSubtract( a.strInt, b.strInt );
                return strDiff == "0" ? 0 : ( strDiff[ 0 ] == '-' ? -1 : 1 );
            }
            double x = a.value(), y = b.value();
            return x < y ? -1 : ( x > y ? 1 : 0 );
        }
            /// Python's max( a, b ) and min( a, b ): the first wins a tie.
        static SjsonNumber max( const SjsonNumber &a, const SjsonNumber &b ) { return compare( b, a ) > 0 ? b : a; }
        static SjsonNumber min( const SjsonNumber &a, const SjsonNumber &b ) { return compare( b, a ) < 0 ? b : a; }

            /// int( b - a ).bit_length() as Python computes it.
        static unsigned spanBits( const SjsonNumber &a, const SjsonNumber &b )
        {
            std::string strSpan;
            if ( a.bInt && b.bInt )
                strSpan = SjsonUtil::decimalSubtract( b.strInt, a.strInt );
            else
                strSpan = decimalOf( std::trunc( b.value() - a.value() ) );
            unsigned long long ullMagnitude;
            bool bNegative;
            if ( ! SjsonUtil::decimalMagnitude( strSpan, ullMagnitude, bNegative ) )
                return 65;
            unsigned nBits = 0;
            for ( ; ullMagnitude; ullMagnitude >>= 1 )
                nBits++;
            return nBits;
        }

            /// A new number node.
        CJsonNode *node( void ) const { return bInt ? SjsonCodec::makeInteger( strInt ) : new CNumberNode( d ); }
    };
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSONSCHEMANUMBER_H__
