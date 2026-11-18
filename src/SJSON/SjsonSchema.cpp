/**
 * SJSON schema model, Python-compatible JSON text, and the JSON Schema
 * loader. See SjsonSchema.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonSchema.h"
#include "SjsonBits.h"
#include "SjsonCodec.h"
#include "SjsonSchemaNumber.h"
#include "SjsonUtil.h"

#include "ArrayNode.h"
#include "BooleanNode.h"
#include "JsonNode.h"
#include "JsonParser.h"
#include "NullNode.h"
#include "NumberNode.h"
#include "ObjectNode.h"
#include "TextNode.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <stdlib.h>
#else
#include <climits>
#include <limits.h>
#endif

namespace IASLib
{
    static const char *const g_apszKinds[] = { "null", "boolean", "integer", "number", "string", "array", "object" };

    static bool knownKind( const std::string &strKind )
    {
        for ( size_t n = 0; n < sizeof( g_apszKinds ) / sizeof( g_apszKinds[ 0 ] ); n++ )
            if ( strKind == g_apszKinds[ n ] )
                return true;
        return false;
    }

    // ---- types and model -------------------------------------------------

    void CSjsonSchemaType::addProperty( const std::string &strName, CSjsonSchemaType *pType, bool bRequired )
    {
        std::map<std::string, size_t>::iterator it = m_mapIndex.find( strName );
        if ( it == m_mapIndex.end() )
        {
            m_mapIndex[ strName ] = properties.size();
            CSjsonSchemaProperty property = { strName, pType, bRequired };
            properties.push_back( property );
            return;
        }
        properties[ it->second ].pType = pType;
        properties[ it->second ].bRequired = properties[ it->second ].bRequired || bRequired;
    }

    const CSjsonSchemaProperty *CSjsonSchemaType::property( const std::string &strName ) const
    {
        std::map<std::string, size_t>::const_iterator it = m_mapIndex.find( strName );
        return it == m_mapIndex.end() ? NULL : &properties[ it->second ];
    }

    void CSjsonSchemaType::children( std::vector<CSjsonSchemaType *> &aChildren ) const
    {
        for ( size_t n = 0; n < properties.size(); n++ )
            aChildren.push_back( properties[ n ].pType );
        if ( pItems )
            aChildren.push_back( pItems );
        aChildren.insert( aChildren.end(), prefixItems.begin(), prefixItems.end() );
        aChildren.insert( aChildren.end(), variants.begin(), variants.end() );
    }

    CSjsonSchemaType *CSjsonSchemaModel::newType( void )
    {
        m_aPool.push_back( std::unique_ptr<CSjsonSchemaType>( new CSjsonSchemaType() ) );
        m_bFingerprint = false;
        return m_aPool.back().get();
    }

    std::vector<const CSjsonSchemaType *> CSjsonSchemaModel::walk( void ) const
    {
        std::vector<const CSjsonSchemaType *> aOut;
        std::set<const CSjsonSchemaType *> setSeen;
        std::vector<CSjsonSchemaType *> aStarts( 1, pRoot );
        aStarts.insert( aStarts.end(), extras.begin(), extras.end() );
        for ( size_t s = 0; s < aStarts.size(); s++ )
        {
            std::deque<const CSjsonSchemaType *> queue( 1, aStarts[ s ] );
            while ( ! queue.empty() )
            {
                const CSjsonSchemaType *pNode = queue.front();
                queue.pop_front();
                if ( pNode == NULL || setSeen.count( pNode ) )
                    continue;
                setSeen.insert( pNode );
                aOut.push_back( pNode );
                std::vector<CSjsonSchemaType *> aChildren;
                pNode->children( aChildren );
                queue.insert( queue.end(), aChildren.begin(), aChildren.end() );
            }
        }
        return aOut;
    }

    std::vector<std::string> CSjsonSchemaModel::fieldNames( void ) const
    {
        std::vector<std::string> aOut;
        std::set<std::string> setSeen;
        std::vector<const CSjsonSchemaType *> aTypes = walk();
        for ( size_t t = 0; t < aTypes.size(); t++ )
        {
            for ( int bRequired = 1; bRequired >= 0; bRequired-- )
            {
                for ( size_t n = 0; n < aTypes[ t ]->properties.size(); n++ )
                {
                    const CSjsonSchemaProperty &prop = aTypes[ t ]->properties[ n ];
                    if ( prop.bRequired == ( bRequired == 1 ) && setSeen.insert( prop.strName ).second )
                        aOut.push_back( prop.strName );
                }
            }
        }
        return aOut;
    }

    std::vector<std::string> CSjsonSchemaModel::enumStrings( void ) const
    {
        std::vector<std::string> aOut;
        std::set<std::string> setSeen;
        std::vector<const CSjsonSchemaType *> aTypes = walk();
        for ( size_t t = 0; t < aTypes.size(); t++ )
            for ( size_t n = 0; n < aTypes[ t ]->enumValues.size(); n++ )
            {
                const CJsonNode *pValue = aTypes[ t ]->enumValues[ n ].get();
                if ( pValue && pValue->isTextual() && setSeen.insert( pValue->textString() ).second )
                    aOut.push_back( pValue->textString() );
            }
        return aOut;
    }

    CJsonNode *CSjsonSchemaModel::canonical( void ) const
    {
        std::vector<const CSjsonSchemaType *> aTypes = walk();
        std::map<const CSjsonSchemaType *, long long> mapNumbers;
        for ( size_t n = 0; n < aTypes.size(); n++ )
            mapNumbers[ aTypes[ n ] ] = (long long)n;
        CArrayNode *pOut = new CArrayNode();
        for ( size_t t = 0; t < aTypes.size(); t++ )
        {
            const CSjsonSchemaType &node = *aTypes[ t ];
            CObjectNode *pEntry = pOut->addObject();
            CArrayNode *pKinds = pEntry->putArray( "kinds" );
            for ( std::set<std::string>::const_iterator it = node.kinds.begin(); it != node.kinds.end(); ++it )
                pKinds->add( *it );
            CArrayNode *pProperties = pEntry->putArray( "properties" );
            for ( size_t n = 0; n < node.properties.size(); n++ )
                pProperties->addArray()->add( node.properties[ n ].strName )->add( mapNumbers[ node.properties[ n ].pType ] )->add( node.properties[ n ].bRequired );
            pEntry->put( "additional", node.bAdditionalProperties );
            if ( node.pItems )
                pEntry->put( "items", mapNumbers[ node.pItems ] );
            else
                pEntry->putNull( "items" );
            CArrayNode *pPrefix = pEntry->putArray( "prefix_items" );
            for ( size_t n = 0; n < node.prefixItems.size(); n++ )
                pPrefix->add( mapNumbers[ node.prefixItems[ n ] ] );
            if ( node.bEnum )
            {
                CArrayNode *pEnum = pEntry->putArray( "enum" );
                for ( size_t n = 0; n < node.enumValues.size(); n++ )
                    pEnum->add( node.enumValues[ n ] ? node.enumValues[ n ]->clone() : new CNullNode() );
            }
            else
                pEntry->putNull( "enum" );
            if ( node.pMinimum ) pEntry->set( "minimum", node.pMinimum->clone() ); else pEntry->putNull( "minimum" );
            if ( node.pMaximum ) pEntry->set( "maximum", node.pMaximum->clone() ); else pEntry->putNull( "maximum" );
            if ( node.bFormat ) pEntry->put( "format", node.strFormat ); else pEntry->putNull( "format" );
            CArrayNode *pVariants = pEntry->putArray( "variants" );
            for ( size_t n = 0; n < node.variants.size(); n++ )
                pVariants->add( mapNumbers[ node.variants[ n ] ] );
        }
        return pOut;
    }

    unsigned CSjsonSchemaModel::fingerprint( void ) const
    {
        if ( ! m_bFingerprint )
        {
            std::unique_ptr<CJsonNode> pCanonical( canonical() );
            std::string strText = SjsonSchema::pythonJson( pCanonical.get(), true );
            m_nFingerprint = SjsonUtil::crc32( (const unsigned char *)strText.data(), strText.size() );
            m_bFingerprint = true;
        }
        return m_nFingerprint;
    }

    CJsonNode *CSjsonSchemaModel::toJson( void ) const
    {
        std::vector<const CSjsonSchemaType *> aTypes = walk();
        std::map<const CSjsonSchemaType *, long long> mapNumbers;
        for ( size_t n = 0; n < aTypes.size(); n++ )
            mapNumbers[ aTypes[ n ] ] = (long long)n;
        CObjectNode *pOut = new CObjectNode();
        pOut->set( "types", canonical() );
        CArrayNode *pExtras = pOut->putArray( "extras" );
        for ( size_t n = 0; n < extras.size(); n++ )
            pExtras->add( mapNumbers[ extras[ n ] ] );
        pOut->put( "source", strSource );
        return pOut;
    }

    CSjsonSchemaModel *CSjsonSchemaModel::fromJson( const CJsonNode &data )
    {
        const CJsonNode &entries = data.path( "types" );
        if ( ! entries.isArray() || entries.size() == 0 )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "a compiled schema needs at least one type" );
        std::unique_ptr<CSjsonSchemaModel> pModel( new CSjsonSchemaModel() );
        std::vector<CSjsonSchemaType *> aNodes;
        for ( int n = 0; n < entries.size(); n++ )
            aNodes.push_back( pModel->newType() );
        struct Ref
        {
            static CSjsonSchemaType *node( const std::vector<CSjsonSchemaType *> &aNodes, const CJsonNode &index )
            {
                if ( ! index.isIntegralNumber() || index.longValue() < 0 || index.longValue() >= (long long)aNodes.size() )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid type reference" );
                return aNodes[ (size_t)index.longValue() ];
            }
        };
        for ( int n = 0; n < entries.size(); n++ )
        {
            const CJsonNode &entry = entries.path( n );
            CSjsonSchemaType &target = *aNodes[ (size_t)n ];
            if ( ! entry.isObject() )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "malformed compiled schema" );
            const CJsonNode &kinds = entry.path( "kinds" );
            for ( int k = 0; k < kinds.size(); k++ )
                if ( knownKind( kinds.path( k ).textString() ) )
                    target.kinds.insert( kinds.path( k ).textString() );
            const CJsonNode &properties = entry.path( "properties" );
            for ( int k = 0; k < properties.size(); k++ )
            {
                const CJsonNode &triple = properties.path( k );
                if ( triple.size() != 3 )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "malformed compiled schema property" );
                std::string strName = triple.path( 0 ).textString();
                CSjsonSchemaType *pType = Ref::node( aNodes, triple.path( 1 ) );
                const CSjsonSchemaProperty *pExisting = target.property( strName );
                bool bRequired = triple.path( 2 ).asBoolean();
                if ( pExisting )
                {
                    const_cast<CSjsonSchemaProperty *>( pExisting )->pType = pType;
                    const_cast<CSjsonSchemaProperty *>( pExisting )->bRequired = bRequired;
                }
                else
                    target.addProperty( strName, pType, bRequired );
            }
            if ( ! entry.path( "additional" ).isBoolean() && ! entry.path( "additional" ).isNumber() )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "malformed compiled schema" );
            target.bAdditionalProperties = entry.path( "additional" ).asBoolean();
            if ( ! entry.path( "items" ).isNull() && ! entry.path( "items" ).isMissingNode() )
                target.pItems = Ref::node( aNodes, entry.path( "items" ) );
            const CJsonNode &prefix = entry.path( "prefix_items" );
            for ( int k = 0; k < prefix.size(); k++ )
                target.prefixItems.push_back( Ref::node( aNodes, prefix.path( k ) ) );
            const CJsonNode &values = entry.path( "enum" );
            if ( values.isArray() )
            {
                target.bEnum = true;
                for ( int k = 0; k < values.size(); k++ )
                    target.enumValues.push_back( std::shared_ptr<CJsonNode>( values.path( k ).clone() ) );
            }
            if ( entry.path( "minimum" ).isNumber() )
                target.pMinimum.reset( entry.path( "minimum" ).clone() );
            if ( entry.path( "maximum" ).isNumber() )
                target.pMaximum.reset( entry.path( "maximum" ).clone() );
            if ( entry.path( "format" ).isTextual() )
            {
                target.bFormat = true;
                target.strFormat = entry.path( "format" ).textString();
            }
            const CJsonNode &variants = entry.path( "variants" );
            for ( int k = 0; k < variants.size(); k++ )
                target.variants.push_back( Ref::node( aNodes, variants.path( k ) ) );
        }
        pModel->pRoot = aNodes[ 0 ];
        const CJsonNode &extras = data.path( "extras" );
        for ( int k = 0; k < extras.size(); k++ )
            pModel->extras.push_back( Ref::node( aNodes, extras.path( k ) ) );
        pModel->strSource = data.path( "source" ).isTextual() ? data.path( "source" ).textString() : std::string();
        return pModel.release();
    }

    // ---- Python-compatible JSON text --------------------------------------

    namespace SjsonSchema
    {
        static void writeString( const std::string &strText, std::string &strOut )
        {
            static const char s_szHex[] = "0123456789abcdef";
            strOut += '"';
            std::vector<unsigned> aCodes = SjsonUtil::codePoints( strText );
            for ( size_t n = 0; n < aCodes.size(); n++ )
            {
                unsigned c = aCodes[ n ];
                switch ( c )
                {
                    case '"':  strOut += "\\\""; continue;
                    case '\\': strOut += "\\\\"; continue;
                    case '\n': strOut += "\\n"; continue;
                    case '\r': strOut += "\\r"; continue;
                    case '\t': strOut += "\\t"; continue;
                    case '\b': strOut += "\\b"; continue;
                    case '\f': strOut += "\\f"; continue;
                    default: break;
                }
                if ( c >= 0x20 && c <= 0x7E )
                {
                    strOut += (char)c;
                    continue;
                }
                unsigned aUnits[ 2 ];
                int nUnits = 1;
                if ( c > 0xFFFF )
                {
                    c -= 0x10000;
                    aUnits[ 0 ] = 0xD800 | ( c >> 10 );
                    aUnits[ 1 ] = 0xDC00 | ( c & 0x3FF );
                    nUnits = 2;
                }
                else
                    aUnits[ 0 ] = c;
                for ( int u = 0; u < nUnits; u++ )
                {
                    strOut += "\\u";
                    for ( int shift = 12; shift >= 0; shift -= 4 )
                        strOut += s_szHex[ ( aUnits[ u ] >> shift ) & 15 ];
                }
            }
            strOut += '"';
        }

        static void write( const CJsonNode *pNode, bool bSortKeys, std::string &strOut )
        {
            switch ( SjsonCodec::kindOf( pNode ) )
            {
                case SJSON_KIND_NULL:   strOut += "null"; return;
                case SJSON_KIND_BOOL:   strOut += pNode->booleanValue() ? "true" : "false"; return;
                case SJSON_KIND_INT:    strOut += SjsonCodec::integerText( *pNode ); return;
                case SJSON_KIND_FLOAT:
                {
                    double d = pNode->doubleValue();
                    if ( std::isnan( d ) ) strOut += "NaN";
                    else if ( std::isinf( d ) ) strOut += d > 0 ? "Infinity" : "-Infinity";
                    else strOut += SjsonUtil::floatRepr( d );
                    return;
                }
                case SJSON_KIND_STRING: writeString( SjsonCodec::stringText( *pNode ), strOut ); return;
                case SJSON_KIND_ARRAY:
                    strOut += '[';
                    for ( int n = 0; n < pNode->size(); n++ )
                    {
                        if ( n ) strOut += ',';
                        write( pNode->get( n ), bSortKeys, strOut );
                    }
                    strOut += ']';
                    return;
                case SJSON_KIND_OBJECT:
                {
                    const CObjectNode &object = static_cast<const CObjectNode &>( *pNode );
                    std::vector< std::pair<std::string, int> > aFields;
                    for ( int n = 0; n < object.size(); n++ )
                    {
                        CString strName = object.fieldName( n );
                        aFields.push_back( std::make_pair( std::string( (const char *)strName, strName.GetLength() ), n ) );
                    }
                    if ( bSortKeys )
                        std::stable_sort( aFields.begin(), aFields.end(),
                                          []( const std::pair<std::string, int> &a, const std::pair<std::string, int> &b ) { return a.first < b.first; } );
                    strOut += '{';
                    for ( size_t n = 0; n < aFields.size(); n++ )
                    {
                        if ( n ) strOut += ',';
                        writeString( aFields[ n ].first, strOut );
                        strOut += ':';
                        write( object.fieldValue( aFields[ n ].second ), bSortKeys, strOut );
                    }
                    strOut += '}';
                    return;
                }
            }
        }

        std::string pythonJson( const CJsonNode *pNode, bool bSortKeys )
        {
            std::string strOut;
            write( pNode, bSortKeys, strOut );
            return strOut;
        }
    }

    // ---- JSON Schema loader ------------------------------------------------

    namespace
    {
        std::string readFile( const std::string &strPath )
        {
            std::ifstream file( strPath.c_str(), std::ios::binary );
            if ( ! file )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "cannot read " + strPath );
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        std::string absolutePath( const std::string &strPath )
        {
#ifdef _WIN32
            char szBuffer[ 4096 ];
            if ( _fullpath( szBuffer, strPath.c_str(), sizeof( szBuffer ) ) )
                return szBuffer;
#else
            char szBuffer[ PATH_MAX ];
            if ( realpath( strPath.c_str(), szBuffer ) )
                return szBuffer;
#endif
            return strPath;
        }

        std::string parentDirectory( const std::string &strPath )
        {
            size_t n = strPath.find_last_of( "/\\" );
            return n == std::string::npos ? std::string( "." ) : strPath.substr( 0, n );
        }

        std::string percentDecode( const std::string &strText )
        {
            std::string strOut;
            for ( size_t n = 0; n < strText.size(); n++ )
            {
                if ( strText[ n ] == '%' && n + 2 < strText.size() && isxdigit( (unsigned char)strText[ n + 1 ] ) && isxdigit( (unsigned char)strText[ n + 2 ] ) )
                {
                    strOut += (char)strtol( strText.substr( n + 1, 2 ).c_str(), NULL, 16 );
                    n += 2;
                }
                else
                    strOut += strText[ n ];
            }
            return strOut;
        }

        std::string replaceAll( std::string strText, const std::string &strFrom, const std::string &strTo )
        {
            for ( size_t n = strText.find( strFrom ); n != std::string::npos; n = strText.find( strFrom, n + strTo.size() ) )
                strText.replace( n, strFrom.size(), strTo );
            return strText;
        }

        bool isEmptyObject( const CJsonNode &node ) { return node.isObject() && node.size() == 0; }

        class CJsonSchemaLoader
        {
            public:
                CJsonSchemaLoader( CSjsonSchemaModel &model, const CJsonNode *pDocument, const std::string &strRootKey )
                    : m_model( model ), m_strRootKey( strRootKey )
                {
                    m_mapDocuments[ strRootKey ] = std::shared_ptr<CJsonNode>( pDocument->clone() );
                }

                void load( void )
                {
                    m_model.pRoot = resolve( m_strRootKey, "" );
                    const CJsonNode &document = *m_mapDocuments[ m_strRootKey ];
                    if ( document.isObject() )
                    {
                        const char *apszKeys[] = { "$defs", "definitions" };
                        for ( int k = 0; k < 2; k++ )
                        {
                            const CJsonNode &defs = document.path( apszKeys[ k ] );
                            if ( ! defs.isObject() )
                                continue;
                            const CObjectNode &object = static_cast<const CObjectNode &>( defs );
                            for ( int n = 0; n < object.size(); n++ )
                            {
                                CString strName = object.fieldName( n );
                                    // JSON Pointer escapes (RFC 6901), plus "%"
                                    // since pointers are percent-decoded.
                                std::string strEscaped;
                                for ( size_t c = 0; c < (size_t)strName.GetLength(); c++ )
                                {
                                    char ch = ( (const char *)strName )[ c ];
                                    if ( ch == '%' ) strEscaped += "%25";
                                    else if ( ch == '~' ) strEscaped += "~0";
                                    else if ( ch == '/' ) strEscaped += "~1";
                                    else strEscaped += ch;
                                }
                                m_model.extras.push_back( resolve( m_strRootKey, std::string( "/" ) + apszKeys[ k ] + "/" + strEscaped ) );
                            }
                        }
                    }
                    m_model.strSource = m_strRootKey;
                }

            private:
                CSjsonSchemaType *resolve( const std::string &strDocKey, const std::string &strPointer )
                {
                    std::pair<std::string, std::string> key( strDocKey, strPointer );
                    std::map< std::pair<std::string, std::string>, CSjsonSchemaType * >::iterator it = m_mapCache.find( key );
                    if ( it != m_mapCache.end() )
                        return it->second;
                    const CJsonNode &node = followPointer( *m_mapDocuments[ strDocKey ], strPointer, strDocKey );
                    CSjsonSchemaType *pResult = m_model.newType();
                    m_mapCache[ key ] = pResult;        // placed first so cycles terminate
                    fill( *pResult, node, strDocKey );
                    return pResult;
                }

                const CJsonNode &followPointer( const CJsonNode &document, const std::string &strPointer, const std::string &strDocKey )
                {
                    if ( strPointer.empty() || strPointer == "/" )
                        return document;
                    size_t nStart = strPointer.find_first_not_of( '/' );
                    std::string strRest = nStart == std::string::npos ? std::string() : strPointer.substr( nStart );
                    const CJsonNode *pNode = &document;
                    size_t nPos = 0;
                    for ( ;; )
                    {
                        size_t nSlash = strRest.find( '/', nPos );
                        std::string strRaw = strRest.substr( nPos, nSlash == std::string::npos ? std::string::npos : nSlash - nPos );
                        std::string strPart = replaceAll( replaceAll( percentDecode( strRaw ), "~1", "/" ), "~0", "~" );
                        bool bFound = false;
                        if ( pNode->isArray() )
                        {
                            char *pEnd = NULL;
                            long lIndex = strtol( strPart.c_str(), &pEnd, 10 );
                            if ( ! strPart.empty() && *pEnd == '\0' && lIndex >= 0 && lIndex < pNode->size() )
                            {
                                pNode = pNode->get( (int)lIndex );
                                bFound = true;
                            }
                        }
                        else if ( pNode->isObject() )
                        {
                            const CJsonNode *pChild = pNode->get( strPart.c_str() );
                            const CObjectNode &object = static_cast<const CObjectNode &>( *pNode );
                            for ( int n = 0; pChild == NULL && n < object.size(); n++ )
                            {
                                CString strName = object.fieldName( n );
                                if ( std::string( (const char *)strName, strName.GetLength() ) == strPart )
                                    pChild = object.fieldValue( n );
                            }
                            if ( pChild )
                            {
                                pNode = pChild;
                                bFound = true;
                            }
                        }
                        if ( ! bFound )
                            throw CSjsonException( SJSON_ERROR_MALFORMED, "unresolvable reference #" + strPointer + " in " + strDocKey );
                        if ( nSlash == std::string::npos )
                            break;
                        nPos = nSlash + 1;
                    }
                    return *pNode;
                }

                CSjsonSchemaType *ref( const std::string &strRef, const std::string &strDocKey )
                {
                    size_t nHash = strRef.find( '#' );
                    std::string strLocation = strRef.substr( 0, nHash );
                    std::string strPointer = nHash == std::string::npos ? std::string() : strRef.substr( nHash + 1 );
                    if ( strLocation.empty() )
                        return resolve( strDocKey, strPointer );
                    if ( strLocation.find( "://" ) != std::string::npos )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "remote references are not supported: " + strRef );
                    if ( strDocKey == "<inline>" )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "relative reference " + strRef + " needs a schema loaded from a file" );
                    std::string strPath = absolutePath( parentDirectory( strDocKey ) + "/" + strLocation );
                    if ( m_mapDocuments.find( strPath ) == m_mapDocuments.end() )
                    {
                        CJsonNode *pDocument = CJsonParser::parse( readFile( strPath ) );
                        if ( pDocument == NULL )
                            throw CSjsonException( SJSON_ERROR_MALFORMED, "cannot read referenced schema " + strPath );
                        m_mapDocuments[ strPath ] = std::shared_ptr<CJsonNode>( pDocument );
                    }
                    return resolve( strPath, strPointer );
                }

                CSjsonSchemaType *typeOf( const CJsonNode &node, const std::string &strDocKey )
                {
                    if ( node.isObject() && node.size() == 1 && node.path( "$ref" ).isTextual() )
                        return ref( node.path( "$ref" ).textString(), strDocKey );
                    CSjsonSchemaType *pResult = m_model.newType();
                    fill( *pResult, node, strDocKey );
                    return pResult;
                }

                static void merge( CSjsonSchemaType &target, const CSjsonSchemaType &source )
                {
                    if ( ! source.kinds.empty() )
                    {
                        if ( target.kinds.empty() )
                            target.kinds = source.kinds;
                        else
                        {
                            std::set<std::string> setBoth;
                            for ( std::set<std::string>::const_iterator it = target.kinds.begin(); it != target.kinds.end(); ++it )
                                if ( source.kinds.count( *it ) )
                                    setBoth.insert( *it );
                            target.kinds = setBoth;
                        }
                    }
                    std::vector<CSjsonSchemaProperty> aProperties = source.properties;
                    for ( size_t n = 0; n < aProperties.size(); n++ )
                        target.addProperty( aProperties[ n ].strName, aProperties[ n ].pType, aProperties[ n ].bRequired );
                    target.bAdditionalProperties = target.bAdditionalProperties && source.bAdditionalProperties;
                    if ( target.pItems == NULL )
                        target.pItems = source.pItems;
                    if ( target.prefixItems.empty() )
                        target.prefixItems = source.prefixItems;
                    if ( source.bEnum )
                    {
                        target.bEnum = true;
                        target.enumValues = source.enumValues;
                    }
                    if ( source.pMinimum )
                        target.pMinimum = source.pMinimum;
                    if ( source.pMaximum )
                        target.pMaximum = source.pMaximum;
                    if ( ! ( target.bFormat && ! target.strFormat.empty() ) )     // Python: target.format or source.format
                    {
                        target.bFormat = source.bFormat;
                        target.strFormat = source.strFormat;
                    }
                    std::vector<CSjsonSchemaType *> aVariants = source.variants;
                    target.variants.insert( target.variants.end(), aVariants.begin(), aVariants.end() );
                }

                static bool has( const CJsonNode &node, const char *pszKey ) { return ! node.path( pszKey ).isMissingNode(); }

                void fill( CSjsonSchemaType &result, const CJsonNode &node, const std::string &strDocKey )
                {
                    if ( ( node.isBoolean() && node.booleanValue() ) || isEmptyObject( node ) )
                        return;
                    if ( node.isBoolean() )
                    {
                        result.kinds.clear();
                        return;
                    }
                    if ( ! node.isObject() )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid schema node: " + node.toJson() );

                    if ( node.path( "$ref" ).isTextual() )
                        merge( result, *ref( node.path( "$ref" ).textString(), strDocKey ) );

                    const CJsonNode &declared = node.path( "type" );
                    if ( declared.isTextual() )
                    {
                        result.kinds.clear();
                        result.kinds.insert( declared.textString() );
                    }
                    else if ( declared.isArray() )
                    {
                        result.kinds.clear();
                        for ( int n = 0; n < declared.size(); n++ )
                            result.kinds.insert( declared.path( n ).isTextual() ? declared.path( n ).textString() : declared.path( n ).toJson() );
                    }
                    for ( std::set<std::string>::const_iterator it = result.kinds.begin(); it != result.kinds.end(); ++it )
                        if ( ! knownKind( *it ) )
                            throw CSjsonException( SJSON_ERROR_MALFORMED, "unknown type: " + *it );
                    if ( node.path( "nullable" ).isBoolean() && node.path( "nullable" ).booleanValue() && ! result.kinds.empty() )
                        result.kinds.insert( "null" );

                    const CJsonNode &allOf = node.path( "allOf" );
                    for ( int n = 0; n < allOf.size(); n++ )
                        merge( result, *typeOf( allOf.path( n ), strDocKey ) );

                    const char *apszVariants[] = { "anyOf", "oneOf" };
                    for ( int k = 0; k < 2; k++ )
                    {
                        const CJsonNode &list = node.path( apszVariants[ k ] );
                        for ( int n = 0; n < list.size(); n++ )
                            result.variants.push_back( typeOf( list.path( n ), strDocKey ) );
                    }

                    std::set<std::string> setRequired;
                    const CJsonNode &required = node.path( "required" );
                    for ( int n = 0; required.isArray() && n < required.size(); n++ )
                        setRequired.insert( required.path( n ).textString() );
                    const CJsonNode &properties = node.path( "properties" );
                    if ( properties.isObject() )
                    {
                        const CObjectNode &object = static_cast<const CObjectNode &>( properties );
                        for ( int n = 0; n < object.size(); n++ )
                        {
                            CString strName = object.fieldName( n );
                            std::string strKey( (const char *)strName, strName.GetLength() );
                            result.addProperty( strKey, typeOf( *object.fieldValue( n ), strDocKey ), setRequired.count( strKey ) != 0 );
                        }
                    }

                    std::vector<const CJsonNode *> aOptional;
                    aOptional.push_back( &node.path( "then" ) );
                    aOptional.push_back( &node.path( "else" ) );
                    const char *apszMaps[] = { "dependentSchemas", "dependencies" };
                    for ( int k = 0; k < 2; k++ )
                    {
                        const CJsonNode &map = node.path( apszMaps[ k ] );
                        if ( ! map.isObject() )
                            continue;
                        const CObjectNode &object = static_cast<const CObjectNode &>( map );
                        for ( int n = 0; n < object.size(); n++ )
                            aOptional.push_back( object.fieldValue( n ) );
                    }
                    for ( size_t n = 0; n < aOptional.size(); n++ )
                    {
                        if ( ! aOptional[ n ]->isObject() )
                            continue;
                        CSjsonSchemaType *pOther = typeOf( *aOptional[ n ], strDocKey );
                        for ( size_t p = 0; p < pOther->properties.size(); p++ )
                            if ( result.property( pOther->properties[ p ].strName ) == NULL )
                                result.addProperty( pOther->properties[ p ].strName, pOther->properties[ p ].pType, false );
                    }

                    const CJsonNode &patterns = node.path( "patternProperties" );
                    if ( patterns.isObject() && patterns.size() > 0 )
                    {
                        result.bAdditionalProperties = true;
                        const CObjectNode &object = static_cast<const CObjectNode &>( patterns );
                        for ( int n = 0; n < object.size(); n++ )
                            result.variants.push_back( typeOf( *object.fieldValue( n ), strDocKey ) );
                    }
                    if ( has( node, "additionalProperties" ) )
                    {
                        const CJsonNode &extra = node.path( "additionalProperties" );
                        result.bAdditionalProperties = ! ( extra.isBoolean() && ! extra.booleanValue() );
                        if ( extra.isObject() && extra.size() > 0 )
                            result.variants.push_back( typeOf( extra, strDocKey ) );
                    }
                    if ( node.path( "unevaluatedProperties" ).isBoolean() && ! node.path( "unevaluatedProperties" ).booleanValue() )
                        result.bAdditionalProperties = false;

                    const CJsonNode &items = node.path( "items" );
                    if ( items.isArray() )
                    {
                        result.prefixItems.clear();
                        for ( int n = 0; n < items.size(); n++ )
                            result.prefixItems.push_back( typeOf( items.path( n ), strDocKey ) );
                        if ( node.path( "additionalItems" ).isObject() )
                            result.pItems = typeOf( node.path( "additionalItems" ), strDocKey );
                    }
                    else
                    {
                        const CJsonNode &prefix = node.path( "prefixItems" );
                        if ( has( node, "prefixItems" ) )
                        {
                            result.prefixItems.clear();
                            for ( int n = 0; n < prefix.size(); n++ )
                                result.prefixItems.push_back( typeOf( prefix.path( n ), strDocKey ) );
                        }
                        if ( items.isObject() || ( items.isBoolean() && ! items.booleanValue() ) )
                            result.pItems = typeOf( items, strDocKey );
                    }

                    if ( has( node, "const" ) )
                    {
                        result.bEnum = true;
                        result.enumValues.assign( 1, std::shared_ptr<CJsonNode>( node.path( "const" ).clone() ) );
                    }
                    else if ( node.path( "enum" ).isArray() )
                    {
                        result.bEnum = true;
                        result.enumValues.clear();
                        const CJsonNode &values = node.path( "enum" );
                        for ( int n = 0; n < values.size(); n++ )
                            result.enumValues.push_back( std::shared_ptr<CJsonNode>( values.path( n ).clone() ) );
                    }

                    bounds( result, node );
                    if ( node.path( "format" ).isTextual() )
                    {
                        result.bFormat = true;
                        result.strFormat = node.path( "format" ).textString();
                    }

                    if ( result.kinds.empty() && result.variants.empty() )
                    {
                        if ( has( node, "properties" ) || has( node, "required" ) )
                            result.kinds.insert( "object" );
                        else if ( has( node, "items" ) || has( node, "prefixItems" ) )
                            result.kinds.insert( "array" );
                    }
                }

                static void bounds( CSjsonSchemaType &result, const CJsonNode &node )
                {
                    bool bInteger = ! result.kinds.empty();
                    for ( std::set<std::string>::const_iterator it = result.kinds.begin(); it != result.kinds.end(); ++it )
                        if ( *it != "integer" && *it != "null" )
                            bInteger = false;
                    SjsonNumber low, high, exLow, exHigh;
                    bool bLow = SjsonNumber::from( node.path( "minimum" ), low );
                    bool bHigh = SjsonNumber::from( node.path( "maximum" ), high );
                    if ( bInteger && node.path( "exclusiveMinimum" ).isBoolean() && node.path( "exclusiveMinimum" ).booleanValue() && bLow )
                        low = low.plus( 1 );
                    if ( bInteger && node.path( "exclusiveMaximum" ).isBoolean() && node.path( "exclusiveMaximum" ).booleanValue() && bHigh )
                        high = high.plus( -1 );
                    if ( bInteger && SjsonNumber::from( node.path( "exclusiveMinimum" ), exLow ) )
                    {
                        SjsonNumber candidate = exLow.floor().plus( 1 );
                        low = bLow ? SjsonNumber::max( low, candidate ) : candidate;
                        bLow = true;
                    }
                    if ( bInteger && SjsonNumber::from( node.path( "exclusiveMaximum" ), exHigh ) )
                    {
                        SjsonNumber candidate = exHigh.ceil().plus( -1 );
                        high = bHigh ? SjsonNumber::min( high, candidate ) : candidate;
                        bHigh = true;
                    }
                    if ( bInteger )
                    {
                        if ( bLow ) low = low.ceil();
                        if ( bHigh ) high = high.floor();
                    }
                    if ( bLow )
                        result.pMinimum.reset( low.node() );
                    if ( bHigh )
                        result.pMaximum.reset( high.node() );
                }

                CSjsonSchemaModel  &m_model;
                std::string         m_strRootKey;
                std::map<std::string, std::shared_ptr<CJsonNode> > m_mapDocuments;
                std::map< std::pair<std::string, std::string>, CSjsonSchemaType * > m_mapCache;
        };
    }

    namespace SjsonSchema
    {
        CSjsonSchemaModel *loadJsonSchemaDocument( const CJsonNode &document )
        {
            std::unique_ptr<CSjsonSchemaModel> pModel( new CSjsonSchemaModel() );
            CJsonSchemaLoader loader( *pModel, &document, "<inline>" );
            loader.load();
            return pModel.release();
        }

        CSjsonSchemaModel *loadJsonSchema( const std::string &strPath )
        {
            std::unique_ptr<CJsonNode> pDocument( CJsonParser::parse( readFile( strPath ) ) );
            if ( ! pDocument )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "cannot read JSON schema " + strPath );
            std::unique_ptr<CSjsonSchemaModel> pModel( new CSjsonSchemaModel() );
            CJsonSchemaLoader loader( *pModel, pDocument.get(), absolutePath( strPath ) );
            loader.load();
            return pModel.release();
        }

        CSjsonSchemaModel *loadSchema( const std::string &strPath, const CSjsonXsdOptions &options )
        {
            std::string strLower;
            for ( size_t n = 0; n < strPath.size(); n++ )
                strLower += (char)tolower( (unsigned char)strPath[ n ] );
            if ( strLower.size() >= 4 && strLower.compare( strLower.size() - 4, 4, ".xsd" ) == 0 )
                return loadXsd( strPath, options );
            return loadJsonSchema( strPath );
        }
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
