/**
 * @file NumberNode.h
 * @brief CNumberNode, a JSON number.
 *
 * Number Node Class
 *
 *  A JSON number. Integers that fit in 64 bits are held exactly; larger
 * integers and numbers with a fraction or exponent are held as doubles. A
 * parsed number that is not a 64-bit integer keeps the text it was read from,
 * so it is written back exactly as it was (for example 12345678901234567890,
 * or 1.50).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_NUMBERNODE_H__
#define IASLIB_NUMBERNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"

namespace IASLib
{
        /// A JSON number.
        ///
        /// Integers that fit in 64 bits (signed, or unsigned up to 2^64 - 1)
        /// are held exactly; larger integers and numbers with a fraction or
        /// exponent are held as doubles. A parsed number that is not a 64-bit
        /// integer keeps the text it was read from, so it is written back
        /// exactly as it was (for example 12345678901234567890000, or 1.50).
        /// Conversions to other types truncate fractions and clamp to the
        /// type's range. See CJsonNode for the kinds of number (isInt() and
        /// the rest).
        ///
        /// @code
        ///     CJsonNode *pNode = CJsonParser::parse( "18446744073709551615" );
        ///     pNode->isUnsignedLong();     // true
        ///     pNode->unsignedLongValue();  // 18446744073709551615, exactly
        ///     pNode->longValue();          // LLONG_MAX (clamped)
        ///     delete pNode;
        /// @endcode
    class CNumberNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CNumberNode )

                /// An integer.
                                    CNumberNode( int nValue );
                /// A signed 64-bit integer.
                                    CNumberNode( long long llValue );
                /// An unsigned 64-bit integer, held exactly. Values past
                /// LLONG_MAX make an isUnsignedLong() number.
                /// @note They used to be held as a double, as big integers, and
                /// so could not be read back as the same 64-bit value.
                                    CNumberNode( unsigned long long llValue );
                /// A floating point number (isDouble(), even with no fraction).
                /// Infinities and NaN are written as null.
                                    CNumberNode( double dValue );

                /// A number from JSON number text, which must already be valid
                /// (as the parser checks). Integers that fit in 64 bits are held
                /// exactly; others keep the text, to be written back as read.
                /// @return A new node, owned by the caller; NULL if the text is
                ///         not a number.
                /// @note Integers from 2^63 to 2^64 - 1 used to be read as big
                /// integers, held as doubles.
            static CNumberNode     *fromText( const std::string &strText );

                /// #JSON_NUMBER.
            virtual JsonNodeType    getNodeType( void ) const { return JSON_NUMBER; }
                /// True unless the number has a fraction or exponent.
            virtual bool            isIntegralNumber( void ) const { return m_eKind != KIND_DOUBLE; }
                /// True if the number has a fraction or exponent.
            virtual bool            isFloatingPointNumber( void ) const { return m_eKind == KIND_DOUBLE; }
                /// True for an integer that fits in an int.
            virtual bool            isInt( void ) const;
                /// True for an integer held as a signed 64-bit value.
            virtual bool            isLong( void ) const { return m_eKind == KIND_LONG; }
                /// True for an integer from 2^63 to 2^64 - 1, held exactly.
            virtual bool            isUnsignedLong( void ) const { return m_eKind == KIND_ULONG; }
                /// True for an integer too large for 64 bits, held as a double.
            virtual bool            isBigInteger( void ) const { return m_eKind == KIND_BIG; }
                /// True if the number has a fraction or exponent.
            virtual bool            isDouble( void ) const { return m_eKind == KIND_DOUBLE; }
                /// True if the value, dropping any fraction, fits in an int.
            virtual bool            canConvertToInt( void ) const;
                /// True if the value, dropping any fraction, fits in a signed
                /// 64-bit integer.
            virtual bool            canConvertToLong( void ) const;
                /// True if the value, dropping any fraction, fits in an unsigned
                /// 64-bit integer.
            virtual bool            canConvertToUnsignedLong( void ) const;

                /// The value truncated and clamped to an int.
            virtual int             intValue( void ) const;
                /// The value truncated and clamped to a signed 64-bit integer
                /// (NaN gives 0).
            virtual long long       longValue( void ) const;
                /// The value truncated to an unsigned 64-bit integer. Negative
                /// numbers give 0, and numbers past 2^64 - 1 that.
            virtual unsigned long long unsignedLongValue( void ) const;
                /// The value as a double (rounded, for a large integer).
            virtual double          doubleValue( void ) const;

                /// The number as JSON writes it.
            virtual CString         asText( void ) const;
                /// True if the number is not zero; bDefault is not used.
            virtual bool            asBoolean( bool bDefault = false ) const { (void)bDefault; return doubleValue() != 0.0; }
                /// intValue(); nDefault is not used.
            virtual int             asInt( int nDefault = 0 ) const { (void)nDefault; return intValue(); }
                /// longValue(); llDefault is not used.
            virtual long long       asLong( long long llDefault = 0 ) const { (void)llDefault; return longValue(); }
                /// unsignedLongValue(); ullDefault is not used.
            virtual unsigned long long asUnsignedLong( unsigned long long ullDefault = 0 ) const { (void)ullDefault; return unsignedLongValue(); }
                /// doubleValue(); dDefault is not used.
            virtual double          asDouble( double dDefault = 0.0 ) const { (void)dDefault; return doubleValue(); }

                /// True for a number of equal value. Integers compare exactly; an
                /// integer equals a double only if the double is exactly that
                /// integer; two big integers compare by their text.
            virtual bool            equals( const CJsonNode *pOther ) const;
                /// A copy, owned by the caller, including any text kept from parsing.
            virtual CJsonNode      *clone( void ) const;
                /// Appends the number as asText() gives it.
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        private:
                /// How the value is held.
            enum Kind
            {
                KIND_LONG,      ///< Exactly, in m_llValue.
                KIND_ULONG,     ///< Exactly, in m_ullValue (above LLONG_MAX).
                KIND_BIG,       ///< An integer too large for 64 bits, in m_dValue.
                KIND_DOUBLE     ///< A number with a fraction or exponent, in m_dValue.
            };

            Kind                    m_eKind;        ///< How the value is held.
            long long               m_llValue;      ///< KIND_LONG
            unsigned long long      m_ullValue;     ///< KIND_ULONG: above LLONG_MAX
            double                  m_dValue;       ///< KIND_BIG and KIND_DOUBLE
            std::string             m_strText;      ///< As parsed, if not KIND_LONG or KIND_ULONG (empty if not parsed).

                /// The JSON text: the integer, the parsed text, or the shortest
                /// form of the double.
            std::string             text( void ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_NUMBERNODE_H__
