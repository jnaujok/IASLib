/**
 * Object Node Class
 *
 *  A JSON object. See ObjectNode.h.
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

#include "ObjectNode.h"
#include "ArrayNode.h"
#include "BinaryNode.h"
#include "BooleanNode.h"
#include "NullNode.h"
#include "NumberNode.h"
#include "TextNode.h"

#include <algorithm>

namespace IASLib
{
    IMPLEMENT_OBJECT( CCollectiveNode, CJsonNode );
    IMPLEMENT_OBJECT( CObjectNode, CCollectiveNode );

    CObjectNode::CObjectNode( void ) : m_nNextOrder( 0 )
    {
    }

    CObjectNode::~CObjectNode( void )
    {
        removeAll();
    }

    int CObjectNode::find( const std::string &strField ) const
    {
        std::unordered_map<std::string, unsigned long long>::const_iterator it = m_mapIndex.find( strField );
        if ( it == m_mapIndex.end() )
            return -1;
        std::vector<unsigned long long>::const_iterator itOrder = std::lower_bound( m_aOrder.begin(), m_aOrder.end(), it->second );
        return (int)( itOrder - m_aOrder.begin() );
    }

    CString CObjectNode::fieldName( int nIndex ) const
    {
        const std::string &strName = fieldNameString( nIndex );
        return CString( strName.c_str(), strName.size() );
    }

    const std::string &CObjectNode::fieldNameString( int nIndex ) const
    {
        if ( nIndex < 0 || nIndex >= size() )
            return CJsonNode::textString();     // empty
        return m_aFields[ nIndex ].first;
    }

    CJsonNode *CObjectNode::fieldValue( int nIndex ) const
    {
        if ( nIndex < 0 || nIndex >= size() )
            return NULL;
        return m_aFields[ nIndex ].second;
    }

    CStringArray CObjectNode::fieldNames( void ) const
    {
        CStringArray aNames;
        for ( size_t n = 0; n < m_aFields.size(); n++ )
            aNames.Push( CString( m_aFields[ n ].first.c_str(), m_aFields[ n ].first.size() ) );
        return aNames;
    }

    CJsonNode *CObjectNode::field( const std::string &strField )
    {
        int nIndex = find( strField );
        return ( nIndex < 0 ) ? NULL : m_aFields[ nIndex ].second;
    }

    CObjectNode *CObjectNode::set( const CJsonKey &key, CJsonNode *pValue )
    {
        pValue = storable( pValue );
        int nIndex = find( key.str() );
        if ( nIndex >= 0 )
        {
            if ( m_aFields[ nIndex ].second != pValue )
                delete m_aFields[ nIndex ].second;
            m_aFields[ nIndex ].second = pValue;
        }
        else
        {
            m_mapIndex[ key.str() ] = m_nNextOrder;
            m_aOrder.push_back( m_nNextOrder++ );
            m_aFields.push_back( std::make_pair( key.str(), pValue ) );
        }
        return this;
    }

    CObjectNode *CObjectNode::put( const CJsonKey &key, const char *pszValue )
    {
        return set( key, pszValue ? (CJsonNode *)new CTextNode( pszValue ) : (CJsonNode *)new CNullNode() );
    }

    CObjectNode *CObjectNode::put( const CJsonKey &key, const std::string &strValue ) { return set( key, new CTextNode( strValue ) ); }
    CObjectNode *CObjectNode::put( const CJsonKey &key, bool bValue ) { return set( key, new CBooleanNode( bValue ) ); }
    CObjectNode *CObjectNode::put( const CJsonKey &key, int nValue ) { return set( key, new CNumberNode( nValue ) ); }
    CObjectNode *CObjectNode::put( const CJsonKey &key, unsigned int nValue ) { return set( key, new CNumberNode( (long long)nValue ) ); }
    CObjectNode *CObjectNode::put( const CJsonKey &key, long lValue ) { return set( key, new CNumberNode( (long long)lValue ) ); }
    CObjectNode *CObjectNode::put( const CJsonKey &key, unsigned long lValue ) { return set( key, new CNumberNode( (unsigned long long)lValue ) ); }
    CObjectNode *CObjectNode::put( const CJsonKey &key, long long llValue ) { return set( key, new CNumberNode( llValue ) ); }
    CObjectNode *CObjectNode::put( const CJsonKey &key, unsigned long long llValue ) { return set( key, new CNumberNode( llValue ) ); }
    CObjectNode *CObjectNode::put( const CJsonKey &key, double dValue ) { return set( key, new CNumberNode( dValue ) ); }
    CObjectNode *CObjectNode::putNull( const CJsonKey &key ) { return set( key, new CNullNode() ); }
    CObjectNode *CObjectNode::putBinary( const CJsonKey &key, const std::vector<unsigned char> &aData ) { return set( key, new CBinaryNode( aData ) ); }

    CObjectNode *CObjectNode::putObject( const CJsonKey &key )
    {
        CObjectNode *pObject = new CObjectNode();
        set( key, pObject );
        return pObject;
    }

    CArrayNode *CObjectNode::putArray( const CJsonKey &key )
    {
        CArrayNode *pArray = new CArrayNode();
        set( key, pArray );
        return pArray;
    }

    CObjectNode *CObjectNode::with( const CJsonKey &key )
    {
        CJsonNode *pNode = field( key.str() );
        if ( pNode == NULL )
            return putObject( key );
        return pNode->isObject() ? static_cast<CObjectNode *>( pNode ) : NULL;
    }

    CArrayNode *CObjectNode::withArray( const CJsonKey &key )
    {
        CJsonNode *pNode = field( key.str() );
        if ( pNode == NULL )
            return putArray( key );
        return pNode->isArray() ? static_cast<CArrayNode *>( pNode ) : NULL;
    }

    bool CObjectNode::remove( const CJsonKey &key )
    {
        CJsonNode *pNode = release( key );
        delete pNode;
        return pNode != NULL;
    }

    CJsonNode *CObjectNode::release( const CJsonKey &key )
    {
        int nIndex = find( key.str() );
        if ( nIndex < 0 )
            return NULL;
        CJsonNode *pNode = m_aFields[ nIndex ].second;
            // The other fields keep their numbers, so the index needs no other
            // change. (The whole index used to be rebuilt, so removing each of
            // n fields took O(n log n), and all of them O(n^2 log n).)
        m_mapIndex.erase( key.str() );
        m_aFields.erase( m_aFields.begin() + nIndex );
        m_aOrder.erase( m_aOrder.begin() + nIndex );
        return pNode;
    }

    void CObjectNode::removeAll( void )
    {
        std::vector<CJsonNode *> aChildren;
        detachChildren( aChildren );
        deleteNodes( aChildren );
    }

    void CObjectNode::detachChildren( std::vector<CJsonNode *> &aChildren )
    {
        for ( size_t n = 0; n < m_aFields.size(); n++ )
            aChildren.push_back( m_aFields[ n ].second );
        m_aFields.clear();
        m_aOrder.clear();
        m_mapIndex.clear();
    }

    bool CObjectNode::equals( const CJsonNode *pOther ) const
    {
        return equalTrees( this, pOther );
    }

    CJsonNode *CObjectNode::clone( void ) const
    {
        return cloneTree( this );
    }

    void CObjectNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        writeTree( this, strOut, nIndent, nLevel, bAsciiOnly );
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
