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

namespace IASLib
{
    IMPLEMENT_OBJECT( CCollectiveNode, CJsonNode );
    IMPLEMENT_OBJECT( CObjectNode, CCollectiveNode );

    CObjectNode::CObjectNode( void )
    {
    }

    CObjectNode::~CObjectNode( void )
    {
        removeAll();
    }

    int CObjectNode::find( const std::string &strField ) const
    {
        std::map<std::string, size_t>::const_iterator it = m_mapIndex.find( strField );
        return ( it == m_mapIndex.end() ) ? -1 : (int)it->second;
    }

    void CObjectNode::reindex( void )
    {
        m_mapIndex.clear();
        for ( size_t n = 0; n < m_aFields.size(); n++ )
            m_mapIndex[ m_aFields[ n ].first ] = n;
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
        if ( pValue == NULL )
            pValue = new CNullNode();
        int nIndex = find( key.str() );
        if ( nIndex >= 0 )
        {
            if ( m_aFields[ nIndex ].second != pValue )
                delete m_aFields[ nIndex ].second;
            m_aFields[ nIndex ].second = pValue;
        }
        else
        {
            m_mapIndex[ key.str() ] = m_aFields.size();
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
        m_aFields.erase( m_aFields.begin() + nIndex );
        reindex();
        return pNode;
    }

    void CObjectNode::removeAll( void )
    {
        for ( size_t n = 0; n < m_aFields.size(); n++ )
            delete m_aFields[ n ].second;
        m_aFields.clear();
        m_mapIndex.clear();
    }

    bool CObjectNode::equals( const CJsonNode *pOther ) const
    {
        if ( pOther == this )
            return true;
        if ( pOther == NULL || ! pOther->isObject() || pOther->size() != size() )
            return false;
        const CObjectNode *pObject = static_cast<const CObjectNode *>( pOther );
        for ( size_t n = 0; n < m_aFields.size(); n++ )
        {
            int nIndex = pObject->find( m_aFields[ n ].first );
            if ( nIndex < 0 || ! m_aFields[ n ].second->equals( pObject->m_aFields[ nIndex ].second ) )
                return false;
        }
        return true;
    }

    CJsonNode *CObjectNode::clone( void ) const
    {
        CObjectNode *pCopy = new CObjectNode();
        pCopy->m_aFields.reserve( m_aFields.size() );
        for ( size_t n = 0; n < m_aFields.size(); n++ )
            pCopy->m_aFields.push_back( std::make_pair( m_aFields[ n ].first, m_aFields[ n ].second->clone() ) );
        pCopy->m_mapIndex = m_mapIndex;
        return pCopy;
    }

    void CObjectNode::writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const
    {
        if ( m_aFields.empty() )
        {
            strOut += "{}";
            return;
        }
        strOut += '{';
        for ( size_t n = 0; n < m_aFields.size(); n++ )
        {
            if ( n > 0 )
                strOut += ',';
            newLine( strOut, nIndent, nLevel + 1 );
            writeString( strOut, m_aFields[ n ].first, bAsciiOnly );
            strOut += ( nIndent >= 0 ) ? ": " : ":";
            m_aFields[ n ].second->writeJson( strOut, nIndent, nLevel + 1, bAsciiOnly );
        }
        newLine( strOut, nIndent, nLevel );
        strOut += '}';
    }

    bool CObjectNode::collect( const std::string &strField, bool bParents, bool bFirst, std::vector<CJsonNode *> &aFound ) const
    {
        for ( size_t n = 0; n < m_aFields.size(); n++ )
        {
            if ( m_aFields[ n ].first == strField )
            {
                    // Names are unique, so an object is listed once as a parent.
                aFound.push_back( bParents ? const_cast<CObjectNode *>( this ) : m_aFields[ n ].second );
                if ( bFirst )
                    return true;
            }
            else if ( m_aFields[ n ].second->collect( strField, bParents, bFirst, aFound ) && bFirst )
            {
                return true;
            }
        }
        return false;
    }
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
