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
        m_aElements.push_back( storable( pValue ) );
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
            discard( pValue );
            return false;
        }
        m_aElements.insert( m_aElements.begin() + nIndex, storable( pValue ) );
        return true;
    }

    bool CArrayNode::set( int nIndex, CJsonNode *pValue )
    {
        if ( nIndex < 0 || nIndex >= size() )
        {
            discard( pValue );
            return false;
        }
        pValue = storable( pValue );
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
        std::vector<CJsonNode *> aChildren;
        aChildren.swap( m_aElements );
        deleteNodes( aChildren );
    }

    void CArrayNode::detachChildren( std::vector<CJsonNode *> &aChildren )
    {
        aChildren.insert( aChildren.end(), m_aElements.begin(), m_aElements.end() );
        m_aElements.clear();
    }

    bool CArrayNode::equals( const CJsonNode *pOther ) const
    {
        return equalTrees( this, pOther );
    }

    CJsonNode *CArrayNode::clone( void ) const
    {
        return cloneTree( this );
    }

    void CArrayNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        writeTree( this, strOut, nIndent, nLevel, bAsciiOnly );
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
