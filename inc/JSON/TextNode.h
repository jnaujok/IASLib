/**
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
    class CTextNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CTextNode )

                                    CTextNode( const std::string &strValue ) : m_strValue( strValue ) {}
                                    CTextNode( const char *pszValue ) : m_strValue( pszValue ? pszValue : "" ) {}

            virtual JsonNodeType    getNodeType( void ) const { return STRING; }
            virtual CString         textValue( void ) const;
            virtual const std::string &textString( void ) const { return m_strValue; }

                // "true" and "false" (ignoring surrounding white space) are
                // booleans; text that is a number converts to that number.
            virtual CString         asText( void ) const { return textValue(); }
            virtual bool            asBoolean( bool bDefault = false ) const;
            virtual int             asInt( int nDefault = 0 ) const;
            virtual long long       asLong( long long llDefault = 0 ) const;
            virtual double          asDouble( double dDefault = 0.0 ) const;

            virtual bool            equals( const CJsonNode *pOther ) const;
            virtual CJsonNode      *clone( void ) const { return new CTextNode( m_strValue ); }
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        private:
            std::string             m_strValue;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_TEXTNODE_H__
