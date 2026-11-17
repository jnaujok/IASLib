/**
 * Array Node Class
 *
 *  A JSON array. See ArrayNode.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/15/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_JSON_SUPPORT__

#include "ArrayNode.h"
#include "ObjectNode.h"
#include "BinaryNode.h"
#include "BooleanNode.h"
#include "NullNode.h"
#include "NumberNode.h"
#include "TextNode.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CArrayNode, CCollectiveNode );

    CArrayNode::CArrayNode( void )
    {
    }

    CArrayNode::~CArrayNode( void )
    {
        removeAll();
    }

    CJsonNode *CArrayNode::element( int nIndex )
    {
        if ( nIndex < 0 || nIndex >= size() )
            return NULL;
        return m_aElements[ nIndex ];
    }

    CArrayNode *CArrayNode::add( CJsonNode *pValue )
    {
        m_aElements.push_back( pValue ? pValue : new CNullNode() );
        return this;
    }

    CArrayNode *CArrayNode::add( const char *pszValue )
    {
        return add( pszValue ? (CJsonNode *)new CTextNode( pszValue ) : (CJsonNode *)new CNullNode() );
    }

    CArrayNode *CArrayNode::add( const std::string &strValue ) { return add( new CTextNode( strValue ) ); }
    CArrayNode *CArrayNode::add( bool bValue ) { return add( new CBooleanNode( bValue ) ); }
    CArrayNode *CArrayNode::add( int nValue ) { return add( new CNumberNode( nValue ) ); }
    CArrayNode *CArrayNode::add( unsigned int nValue ) { return add( new CNumberNode( (long long)nValue ) ); }
    CArrayNode *CArrayNode::add( long lValue ) { return add( new CNumberNode( (long long)lValue ) ); }
    CArrayNode *CArrayNode::add( unsigned long lValue ) { return add( new CNumberNode( (unsigned long long)lValue ) ); }
    CArrayNode *CArrayNode::add( long long llValue ) { return add( new CNumberNode( llValue ) ); }
    CArrayNode *CArrayNode::add( unsigned long long llValue ) { return add( new CNumberNode( llValue ) ); }
    CArrayNode *CArrayNode::add( double dValue ) { return add( new CNumberNode( dValue ) ); }
    CArrayNode *CArrayNode::addNull( void ) { return add( new CNullNode() ); }
    CArrayNode *CArrayNode::addBinary( const std::vector<unsigned char> &aData ) { return add( new CBinaryNode( aData ) ); }

    CObjectNode *CArrayNode::addObject( void )
    {
        CObjectNode *pObject = new CObjectNode();
        add( pObject );
        return pObject;
    }

    CArrayNode *CArrayNode::addArray( void )
    {
        CArrayNode *pArray = new CArrayNode();
        add( pArray );
        return pArray;
    }

    bool CArrayNode::insert( int nIndex, CJsonNode *pValue )
    {
        if ( nIndex < 0 || nIndex > size() )
        {
            delete pValue;
            return false;
        }
        m_aElements.insert( m_aElements.begin() + nIndex, pValue ? pValue : new CNullNode() );
        return true;
    }

    bool CArrayNode::set( int nIndex, CJsonNode *pValue )
    {
        if ( nIndex < 0 || nIndex >= size() )
        {
            delete pValue;
            return false;
        }
        if ( pValue == NULL )
            pValue = new CNullNode();
        if ( m_aElements[ nIndex ] != pValue )
            delete m_aElements[ nIndex ];
        m_aElements[ nIndex ] = pValue;
        return true;
    }

    bool CArrayNode::remove( int nIndex )
    {
        CJsonNode *pNode = release( nIndex );
        delete pNode;
        return pNode != NULL;
    }

    CJsonNode *CArrayNode::release( int nIndex )
    {
        if ( nIndex < 0 || nIndex >= size() )
            return NULL;
        CJsonNode *pNode = m_aElements[ nIndex ];
        m_aElements.erase( m_aElements.begin() + nIndex );
        return pNode;
    }

    void CArrayNode::removeAll( void )
    {
        for ( size_t n = 0; n < m_aElements.size(); n++ )
            delete m_aElements[ n ];
        m_aElements.clear();
    }

    bool CArrayNode::equals( const CJsonNode *pOther ) const
    {
        if ( pOther == this )
            return true;
        if ( pOther == NULL || ! pOther->isArray() || pOther->size() != size() )
            return false;
        const CArrayNode *pArray = static_cast<const CArrayNode *>( pOther );
        for ( size_t n = 0; n < m_aElements.size(); n++ )
        {
            if ( ! m_aElements[ n ]->equals( pArray->m_aElements[ n ] ) )
                return false;
        }
        return true;
    }

    CJsonNode *CArrayNode::clone( void ) const
    {
        CArrayNode *pCopy = new CArrayNode();
        pCopy->m_aElements.reserve( m_aElements.size() );
        for ( size_t n = 0; n < m_aElements.size(); n++ )
            pCopy->m_aElements.push_back( m_aElements[ n ]->clone() );
        return pCopy;
    }

    void CArrayNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        if ( m_aElements.empty() )
        {
            strOut += "[]";
            return;
        }
        strOut += '[';
        for ( size_t n = 0; n < m_aElements.size(); n++ )
        {
            if ( n > 0 )
                strOut += ',';
            newLine( strOut, nIndent, nLevel + 1 );
            m_aElements[ n ]->writeJson( strOut, nIndent, nLevel + 1, bAsciiOnly );
        }
        newLine( strOut, nIndent, nLevel );
        strOut += ']';
    }

    bool CArrayNode::collect( const std::string &strField, bool bParents, bool bFirst, std::vector<CJsonNode *> &aFound ) const
    {
        for ( size_t n = 0; n < m_aElements.size(); n++ )
        {
            if ( m_aElements[ n ]->collect( strField, bParents, bFirst, aFound ) && bFirst )
                return true;
        }
        return false;
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
