/**
 * @file BooleanNode.h
 * @brief CBooleanNode, a JSON true or false.
 *
 * Boolean Node Class
 *
 *  A JSON true or false.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BOOLEANNODE_H__
#define IASLIB_BOOLEANNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"

namespace IASLib
{
        /// A JSON true or false.
        ///
        /// As a number it is 1 or 0 (asInt() and the like); its own numeric
        /// values (intValue() and the like) stay 0, since it is not a number.
    class CBooleanNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CBooleanNode )

                /// A node holding bValue.
                                    CBooleanNode( bool bValue ) : m_bValue( bValue ) {}

                /// #JSON_BOOLEAN.
            virtual JsonNodeType    getNodeType( void ) const { return JSON_BOOLEAN; }
                /// The value.
            virtual bool            booleanValue( void ) const { return m_bValue; }
                /// "true" or "false".
            virtual CString         asText( void ) const { return CString( m_bValue ? "true" : "false" ); }
                /// The value; bDefault is not used.
            virtual bool            asBoolean( bool bDefault = false ) const { (void)bDefault; return m_bValue; }
                /// 1 for true, 0 for false; nDefault is not used.
            virtual int             asInt( int nDefault = 0 ) const { (void)nDefault; return m_bValue ? 1 : 0; }
                /// 1 for true, 0 for false; llDefault is not used.
            virtual long long       asLong( long long llDefault = 0 ) const { (void)llDefault; return m_bValue ? 1 : 0; }
                /// 1 for true, 0 for false; ullDefault is not used.
            virtual unsigned long long asUnsignedLong( unsigned long long ullDefault = 0 ) const { (void)ullDefault; return m_bValue ? 1 : 0; }
                /// 1.0 for true, 0.0 for false; dDefault is not used.
            virtual double          asDouble( double dDefault = 0.0 ) const { (void)dDefault; return m_bValue ? 1.0 : 0.0; }

                /// True for a boolean node with the same value.
            virtual bool            equals( const CJsonNode *pOther ) const;
                /// A copy, owned by the caller.
            virtual CJsonNode      *clone( void ) const { return new CBooleanNode( m_bValue ); }
                /// Appends true or false.
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        private:
            bool                    m_bValue;   ///< The value.
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_BOOLEANNODE_H__
