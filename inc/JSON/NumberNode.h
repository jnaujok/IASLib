/**
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
    class CNumberNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CNumberNode )

                                    CNumberNode( int nValue );
                                    CNumberNode( long long llValue );
                                    CNumberNode( unsigned long long llValue );
                                    CNumberNode( double dValue );

                // A number from JSON number text, which must already be valid
                // (as the parser checks). NULL if it is not.
            static CNumberNode     *fromText( const std::string &strText );

            virtual JsonNodeType    getNodeType( void ) const { return JSON_NUMBER; }
            virtual bool            isIntegralNumber( void ) const { return m_eKind != KIND_DOUBLE; }
            virtual bool            isFloatingPointNumber( void ) const { return m_eKind == KIND_DOUBLE; }
            virtual bool            isInt( void ) const;
            virtual bool            isLong( void ) const { return m_eKind == KIND_LONG; }
            virtual bool            isUnsignedLong( void ) const { return m_eKind == KIND_ULONG; }
            virtual bool            isBigInteger( void ) const { return m_eKind == KIND_BIG; }
            virtual bool            isDouble( void ) const { return m_eKind == KIND_DOUBLE; }
            virtual bool            canConvertToInt( void ) const;
            virtual bool            canConvertToLong( void ) const;
            virtual bool            canConvertToUnsignedLong( void ) const;

            virtual int             intValue( void ) const;
            virtual long long       longValue( void ) const;
                // Negative numbers give 0, and numbers past 2^64 - 1 that.
            virtual unsigned long long unsignedLongValue( void ) const;
            virtual double          doubleValue( void ) const;

                // The number as JSON writes it.
            virtual CString         asText( void ) const;
            virtual bool            asBoolean( bool bDefault = false ) const { (void)bDefault; return doubleValue() != 0.0; }
            virtual int             asInt( int nDefault = 0 ) const { (void)nDefault; return intValue(); }
            virtual long long       asLong( long long llDefault = 0 ) const { (void)llDefault; return longValue(); }
            virtual unsigned long long asUnsignedLong( unsigned long long ullDefault = 0 ) const { (void)ullDefault; return unsignedLongValue(); }
            virtual double          asDouble( double dDefault = 0.0 ) const { (void)dDefault; return doubleValue(); }

            virtual bool            equals( const CJsonNode *pOther ) const;
            virtual CJsonNode      *clone( void ) const;
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        private:
            enum Kind { KIND_LONG, KIND_ULONG, KIND_BIG, KIND_DOUBLE };

            Kind                    m_eKind;
            long long               m_llValue;      // KIND_LONG
            unsigned long long      m_ullValue;     // KIND_ULONG: above LLONG_MAX
            double                  m_dValue;       // KIND_BIG and KIND_DOUBLE
            std::string             m_strText;      // as parsed, if not KIND_LONG

            std::string             text( void ) const;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_NUMBERNODE_H__
