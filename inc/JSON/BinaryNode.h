/**
 * @file BinaryNode.h
 * @brief CBinaryNode, bytes in a JSON tree.
 *
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
        /// Bytes in a JSON tree, written as a base64 string.
        ///
        /// JSON has no binary type, so parsed JSON has no binary nodes: base64
        /// text is read as a CTextNode, and CJsonNode::binaryValue() decodes it.
        /// A binary node equals only another binary node with the same bytes.
    class CBinaryNode : public CValueNode
    {
        public:
            DEFINE_OBJECT( CBinaryNode )

                /// A node holding a copy of these bytes.
                                    CBinaryNode( const std::vector<unsigned char> &aData ) : m_aData( aData ) {}
                /// A node holding a copy of nLength bytes from pData.
                                    CBinaryNode( const unsigned char *pData, size_t nLength ) : m_aData( pData, pData + nLength ) {}

                /// #JSON_BINARY.
            virtual JsonNodeType    getNodeType( void ) const { return JSON_BINARY; }
                /// The bytes.
            const std::vector<unsigned char> &data( void ) const { return m_aData; }
                /// Copies the bytes into aData. Always true.
            virtual bool            binaryValue( std::vector<unsigned char> &aData ) const { aData = m_aData; return true; }
                /// The bytes as standard base64 text, with padding.
            virtual CString         asText( void ) const;

                /// True for a binary node with the same bytes.
            virtual bool            equals( const CJsonNode *pOther ) const;
                /// A copy, owned by the caller.
            virtual CJsonNode      *clone( void ) const { return new CBinaryNode( m_aData ); }
                /// Appends the base64 text, quoted.
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const;

        private:
            std::vector<unsigned char>  m_aData;    ///< The bytes.
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_BINARYNODE_H__
