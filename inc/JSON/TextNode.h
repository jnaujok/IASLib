/**
 * @file TextNode.h
 * @brief CTextNode, a JSON string.
 *
 * Text Node Class
 *
 *  A JSON string. The text is UTF-8 and may hold any character, including
 * U+0000 (see textString()).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TEXTNODE_H__
#define IASLIB_TEXTNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"

namespace IASLib
{
        /// A JSON string.
        ///
        /// The text is UTF-8 and may hold any character, including U+0000;
        /// textString() gives all of it, while textValue() and asText() give a
        /// CString, which stops at the first NUL.
        ///
        /// The as...() conversions read the text: "true" and "false" (ignoring
        /// surrounding white space) are booleans, and text that is a number
        /// (an integer, or a finite decimal number with '.' as the point)
        /// converts to that number, clamped to the type's range. Other text
        /// gives the default.
    class CTextNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CTextNode )

                /// A node holding a copy of the text, which may hold NULs.
                                    CTextNode( const std::string &strValue ) : m_strValue( strValue ) {}
                /// A node holding a copy of NUL-terminated text; NULL gives "".
                                    CTextNode( const char *pszValue ) : m_strValue( pszValue ? pszValue : "" ) {}

                /// #JSON_STRING.
            virtual JsonNodeType    getNodeType( void ) const { return JSON_STRING; }
                /// The text as a CString (stopping at the first NUL).
            virtual CString         textValue( void ) const;
                /// The whole text.
            virtual const std::string &textString( void ) const { return m_strValue; }

                /// The same as textValue().
            virtual CString         asText( void ) const { return textValue(); }
                /// true for "true", false for "false" (ignoring surrounding white
                /// space), otherwise bDefault.
            virtual bool            asBoolean( bool bDefault = false ) const;
                /// The number the text holds, truncated and clamped to an int;
                /// nDefault if it holds none.
            virtual int             asInt( int nDefault = 0 ) const;
                /// The number the text holds, truncated and clamped to 64 bits;
                /// llDefault if it holds none.
            virtual long long       asLong( long long llDefault = 0 ) const;
                /// The number the text holds as an unsigned 64-bit integer:
                /// digits alone are read exactly up to 2^64 - 1 (larger gives
                /// that), negative numbers give 0; ullDefault if it holds none.
            virtual unsigned long long asUnsignedLong( unsigned long long ullDefault = 0 ) const;
                /// The number the text holds; dDefault if it holds none.
            virtual double          asDouble( double dDefault = 0.0 ) const;

                /// True for a text node with the same bytes.
            virtual bool            equals( const CJsonNode *pOther ) const;
                /// A copy, owned by the caller.
            virtual CJsonNode      *clone( void ) const { return new CTextNode( m_strValue ); }
                /// Appends the text, quoted and escaped (see CJsonNode::writeString()).
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        private:
            std::string             m_strValue;     ///< The text, as UTF-8 bytes.
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_TEXTNODE_H__
