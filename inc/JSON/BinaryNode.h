/**
 * Binary Node Class
 *
 *  Bytes in a JSON tree, written as a base64 string. Parsed JSON has no
 * binary nodes (base64 text is a text node; use binaryValue() to decode it).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/16/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BINARYNODE_H__
#define IASLIB_BINARYNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"

namespace IASLib
{
    class CBinaryNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CBinaryNode )

                                    CBinaryNode( const std::vector<unsigned char> &aData ) : m_aData( aData ) {}
                                    CBinaryNode( const unsigned char *pData, size_t nLength ) : m_aData( pData, pData + nLength ) {}

            virtual JsonNodeType    getNodeType( void ) const { return BINARY; }
            const std::vector<unsigned char> &data( void ) const { return m_aData; }
            virtual bool            binaryValue( std::vector<unsigned char> &aData ) const { aData = m_aData; return true; }
                // The base64 text.
            virtual CString         asText( void ) const;

            virtual bool            equals( const CJsonNode *pOther ) const;
            virtual CJsonNode      *clone( void ) const { return new CBinaryNode( m_aData ); }
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        private:
            std::vector<unsigned char>  m_aData;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_BINARYNODE_H__
