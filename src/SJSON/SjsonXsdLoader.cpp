/**
 * SJSON XSD loader: builds a CSjsonSchemaModel from an XML Schema, mapped
 * to JSON with the xmltodict convention. See SjsonSchema.h.
 *
 *      This is a rule-for-rule port of the Python sjson library's XsdLoader
 * (sjson/schema/xsd_loader.py), so that the field names, enumerations,
 * canonical form and fingerprint match what Python builds from the same
 * schema:
 *
 *      - the document is an object whose members are the global elements;
 *      - child elements become members named after the element;
 *      - attributes become members named prefix + attribute name ("@");
 *      - the text of simple or mixed content is a member ("#text");
 *      - an element that may repeat (maxOccurs > 1) is an array.
 *
 *      xs:include, xs:import and xs:redefine of local files are followed.
 * Namespace prefixes in attribute values are dropped (local names are
 * used). XSD elements are recognised by namespace, whatever prefix the
 * file binds to it; as in Python, elements in no namespace are matched by
 * name too, and elements in any other namespace are ignored.
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
#include "SjsonUtil.h"

#include "BooleanNode.h"
#include "JsonNode.h"
#include "NumberNode.h"
#include "TextNode.h"

#include "XMLParser.h"
#include "XMLIndex.h"
#include "XMLElement.h"
#include "XMLException.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <stdlib.h>
#else
#include <climits>
#include <limits.h>
#endif

namespace IASLib
{
    namespace
    {
        const char *const XS_NAMESPACE = "http://www.w3.org/2001/XMLSchema";
        const char *const XML_NAMESPACE = "http://www.w3.org/XML/1998/namespace";

        // ---- files -----------------------------------------------------------

        std::string readFile( const std::string &strPath )
        {
            std::ifstream file( strPath.c_str(), std::ios::binary );
            if ( ! file )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "Cannot read XSD " + strPath + ": the file cannot be opened" );
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

            /// The path with symbolic links and "." / ".." resolved (as
            /// Python's Path.resolve()), or the path itself if it does not exist.
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
            if ( n == std::string::npos )
                return ".";
            return n == 0 ? std::string( "/" ) : strPath.substr( 0, n );
        }

            /// Python's path.parent / location.
        std::string joinPath( const std::string &strDirectory, const std::string &strLocation )
        {
            if ( ! strLocation.empty() && ( strLocation[ 0 ] == '/' || strLocation[ 0 ] == '\\' ) )
                return strLocation;
#ifdef _WIN32
            if ( strLocation.size() > 1 && strLocation[ 1 ] == ':' )
                return strLocation;
#endif
            if ( ! strDirectory.empty() && ( strDirectory[ strDirectory.size() - 1 ] == '/' || strDirectory[ strDirectory.size() - 1 ] == '\\' ) )
                return strDirectory + strLocation;
            return strDirectory + "/" + strLocation;
        }

        // ---- Python string semantics -------------------------------------------

            /// The length of the Python white space character (str.isspace())
            /// at n in UTF-8 text, or 0 if there is none.
        size_t spaceAt( const std::string &strText, size_t n )
        {
            unsigned char c = (unsigned char)strText[ n ];
            if ( c == ' ' || ( c >= 0x09 && c <= 0x0D ) || ( c >= 0x1C && c <= 0x1F ) )
                return 1;
            if ( c < 0x80 )
                return 0;
            unsigned nCode = 0;
            size_t nLength = 0;
            if ( ( c & 0xE0 ) == 0xC0 ) { nCode = c & 0x1F; nLength = 2; }
            else if ( ( c & 0xF0 ) == 0xE0 ) { nCode = c & 0x0F; nLength = 3; }
            else return 0;
            if ( n + nLength > strText.size() )
                return 0;
            for ( size_t k = 1; k < nLength; k++ )
            {
                unsigned char cc = (unsigned char)strText[ n + k ];
                if ( ( cc & 0xC0 ) != 0x80 )
                    return 0;
                nCode = ( nCode << 6 ) | ( cc & 0x3F );
            }
            if ( nCode == 0x85 || nCode == 0xA0 || nCode == 0x1680 || ( nCode >= 0x2000 && nCode <= 0x200A ) ||
                 nCode == 0x2028 || nCode == 0x2029 || nCode == 0x202F || nCode == 0x205F || nCode == 0x3000 )
                return nLength;
            return 0;
        }

            /// Whether the Python white space character ending at nEnd is
            /// nLength bytes long (for stripping from the right).
        size_t spaceBefore( const std::string &strText, size_t nEnd )
        {
            for ( size_t nLength = 1; nLength <= 3 && nLength <= nEnd; nLength++ )
                if ( spaceAt( strText, nEnd - nLength ) == nLength )
                    return nLength;
            return 0;
        }

            /// Python's str.strip().
        std::string pyStrip( const std::string &strText )
        {
            size_t nStart = 0, nEnd = strText.size();
            size_t nStep;
            while ( nStart < nEnd && ( nStep = spaceAt( strText, nStart ) ) != 0 )
                nStart += nStep;
            while ( nEnd > nStart && ( nStep = spaceBefore( strText, nEnd ) ) != 0 )
                nEnd -= nStep;
            return strText.substr( nStart, nEnd - nStart );
        }

            /// Python's str.split() (on runs of white space).
        std::vector<std::string> pySplit( const std::string &strText )
        {
            std::vector<std::string> aOut;
            std::string strWord;
            size_t n = 0;
            while ( n < strText.size() )
            {
                size_t nStep = spaceAt( strText, n );
                if ( nStep )
                {
                    if ( ! strWord.empty() )
                        aOut.push_back( strWord );
                    strWord.clear();
                    n += nStep;
                }
                else
                    strWord += strText[ n++ ];
            }
            if ( ! strWord.empty() )
                aOut.push_back( strWord );
            return aOut;
        }

            /// Python's _local(): the text after the last colon.
        std::string localName( const std::string &strName )
        {
            size_t n = strName.rfind( ':' );
            return n == std::string::npos ? strName : strName.substr( n + 1 );
        }

            /// Digits with single underscores between them (Python's
            /// digitpart), from n; returns the position after them, or n if
            /// there are none. Digits (without underscores) go to strDigits.
        size_t digitPart( const std::string &strText, size_t n, std::string &strDigits )
        {
            size_t nStart = n;
            if ( n >= strText.size() || ! isdigit( (unsigned char)strText[ n ] ) )
                return nStart;
            strDigits += strText[ n++ ];
            while ( n < strText.size() )
            {
                if ( isdigit( (unsigned char)strText[ n ] ) )
                    strDigits += strText[ n++ ];
                else if ( strText[ n ] == '_' && n + 1 < strText.size() && isdigit( (unsigned char)strText[ n + 1 ] ) )
                    n++;
                else
                    break;
            }
            return n;
        }

            /// Python's int( text ), as decimal text; throws as int() raises.
        std::string pyInt( const std::string &strValue )
        {
            std::string strText = pyStrip( strValue );
            size_t n = 0;
            bool bNegative = false;
            if ( n < strText.size() && ( strText[ n ] == '+' || strText[ n ] == '-' ) )
                bNegative = strText[ n++ ] == '-';
            std::string strDigits;
            size_t nEnd = digitPart( strText, n, strDigits );
            if ( nEnd == n || nEnd != strText.size() )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid literal for int() with base 10: '" + strValue + "'" );
            if ( strDigits.size() > 4300 )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "Exceeds the limit (4300 digits) for integer string conversion" );
            size_t nFirst = strDigits.find_first_not_of( '0' );
            if ( nFirst == std::string::npos )
                return "0";
            return ( bNegative ? "-" : "" ) + strDigits.substr( nFirst );
        }

            /// Python's float( text ); throws as float() raises.
        double pyFloat( const std::string &strValue )
        {
            std::string strText = pyStrip( strValue );
            size_t n = 0;
            bool bNegative = false;
            if ( n < strText.size() && ( strText[ n ] == '+' || strText[ n ] == '-' ) )
                bNegative = strText[ n++ ] == '-';
            std::string strRest;
            for ( size_t k = n; k < strText.size(); k++ )
                strRest += (char)tolower( (unsigned char)strText[ k ] );
            if ( strRest == "inf" || strRest == "infinity" )
                return bNegative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
            if ( strRest == "nan" )
                return std::numeric_limits<double>::quiet_NaN();
            std::string strNumber = bNegative ? "-" : "";
            std::string strDigits;
            size_t nPos = digitPart( strText, n, strDigits );
            bool bAny = nPos != n;
            strNumber += strDigits;
            if ( nPos < strText.size() && strText[ nPos ] == '.' )
            {
                strNumber += '.';
                nPos++;
                std::string strFraction;
                size_t nAfter = digitPart( strText, nPos, strFraction );
                bAny = bAny || nAfter != nPos;
                nPos = nAfter;
                strNumber += strFraction;
            }
            if ( bAny && nPos < strText.size() && ( strText[ nPos ] == 'e' || strText[ nPos ] == 'E' ) )
            {
                strNumber += 'e';
                nPos++;
                if ( nPos < strText.size() && ( strText[ nPos ] == '+' || strText[ nPos ] == '-' ) )
                    strNumber += strText[ nPos++ ];
                std::string strExponent;
                size_t nAfter = digitPart( strText, nPos, strExponent );
                if ( nAfter == nPos )
                    bAny = false;
                nPos = nAfter;
                strNumber += strExponent;
            }
            if ( ! bAny || nPos != strText.size() )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "could not convert string to float: '" + strValue + "'" );
            return strtod( strNumber.c_str(), NULL );
        }

        // ---- the parsed schema documents ----------------------------------------

            /// An element of an XSD file, with its namespace resolved.
        struct XsNode
        {
                /// The tag as Python's tag.replace( XS, "" ) gives it: the local
                /// name for XSD and no-namespace elements, "{uri}name" otherwise.
            std::string             strTag;
                /// The local name, and whether the element is in the XSD
                /// namespace (Python's find( XS + name ) needs that).
            std::string             strLocal;
            bool                    bXs;
                /// The attributes in no namespace, in order.
            std::vector< std::pair<std::string, std::string> > attributes;
            std::vector<const XsNode *> children;

                /// Python's node.get( name ): the value, or NULL.
            const std::string *get( const char *pszName ) const
            {
                for ( size_t n = 0; n < attributes.size(); n++ )
                    if ( attributes[ n ].first == pszName )
                        return &attributes[ n ].second;
                return NULL;
            }
                /// The value, or "" (Python's node.get( name, "" )).
            std::string value( const char *pszName ) const
            {
                const std::string *pValue = get( pszName );
                return pValue ? *pValue : std::string();
            }
                /// Whether the attribute is there and not empty (truthy).
            bool has( const char *pszName ) const
            {
                const std::string *pValue = get( pszName );
                return pValue && ! pValue->empty();
            }
                /// Python's node.find( XS + name ): the first such child, or NULL.
            const XsNode *find( const char *pszLocal ) const
            {
                for ( size_t n = 0; n < children.size(); n++ )
                    if ( children[ n ]->bXs && children[ n ]->strLocal == pszLocal )
                        return children[ n ];
                return NULL;
            }
            bool is( const char *pszTag ) const { return strTag == pszTag; }
        };

            /// Namespace prefixes in scope.
        struct NsScope
        {
            std::map<std::string, std::string> prefixes;
            std::string strDefault;
        };

        bool isParticle( const XsNode &node )
        {
            return node.is( "sequence" ) || node.is( "choice" ) || node.is( "all" ) || node.is( "group" );
        }

        bool isAttributeLike( const XsNode &node )
        {
            return node.is( "attribute" ) || node.is( "attributeGroup" ) || node.is( "anyAttribute" );
        }

        // ---- the loader ----------------------------------------------------------

        struct IntegerBounds
        {
            const char *pszName;
            const char *pszLow;     ///< NULL for none.
            const char *pszHigh;    ///< NULL for none.
        };

        const IntegerBounds g_aIntegerBounds[] =
        {
            { "integer", NULL, NULL },
            { "long", "-9223372036854775808", "9223372036854775807" },
            { "int", "-2147483648", "2147483647" },
            { "short", "-32768", "32767" },
            { "byte", "-128", "127" },
            { "nonNegativeInteger", "0", NULL },
            { "positiveInteger", "1", NULL },
            { "nonPositiveInteger", NULL, "0" },
            { "negativeInteger", NULL, "-1" },
            { "unsignedLong", "0", "18446744073709551615" },
            { "unsignedInt", "0", "4294967295" },
            { "unsignedShort", "0", "65535" },
            { "unsignedByte", "0", "255" },
        };

        const char *const g_apszStringFormats[][ 2 ] =
        {
            { "date", "date" },
            { "dateTime", "date-time" },
            { "time", "time" },
            { "anyURI", "uri" },
            { "base64Binary", "base64" },
            { "hexBinary", "hex" },
        };

            /// How facet and enumeration values are converted (Python's _converter()).
        enum Converter { CONVERT_INT, CONVERT_FLOAT, CONVERT_BOOL, CONVERT_STR };

            /// An ordered table (Python dict): first insertion fixes the place,
            /// a later one replaces the value.
        struct Table
        {
            std::unordered_map<std::string, const XsNode *> map;
            std::vector<std::string> order;

            void put( const std::string &strName, const XsNode *pNode )
            {
                if ( map.find( strName ) == map.end() )
                    order.push_back( strName );
                map[ strName ] = pNode;
            }
            const XsNode *get( const std::string &strName ) const
            {
                std::unordered_map<std::string, const XsNode *>::const_iterator it = map.find( strName );
                return it == map.end() ? NULL : it->second;
            }
            bool contains( const std::string &strName ) const { return map.find( strName ) != map.end(); }
        };

        class CXsdLoader
        {
            public:
                CXsdLoader( CSjsonSchemaModel &model, const CSjsonXsdOptions &options )
                    : m_model( model ), m_options( options ) {}

                void load( const std::string &strPath )
                {
                    read( strPath );
                    CSjsonSchemaType *pRoot = m_model.newType();
                    pRoot->kinds.insert( "object" );
                    pRoot->bAdditionalProperties = false;
                    for ( size_t n = 0; n < m_elements.order.size(); n++ )
                    {
                        const std::string &strName = m_elements.order[ n ];
                        pRoot->addProperty( strName, elementType( *m_elements.get( strName ), 1, 1 ), false );
                    }
                    for ( size_t n = 0; n < m_complex.order.size(); n++ )
                        m_model.extras.push_back( namedType( m_complex.order[ n ] ) );
                    m_model.pRoot = pRoot;
                    m_model.strSource = strPath;
                }

            private:
                CSjsonSchemaModel          &m_model;
                const CSjsonXsdOptions     &m_options;
                std::deque<XsNode>          m_nodes;            ///< Every element of every file read.
                Table                       m_elements, m_complex, m_simple, m_groups, m_attributeGroups, m_attributes;
                std::set<std::string>       m_setLoaded;
                std::unordered_map<std::string, CSjsonSchemaType *> m_cacheComplex, m_cacheSimple;

                // -- reading ----------------------------------------------------

                static std::string text( const char *psz ) { return psz ? std::string( psz ) : std::string(); }

                const XsNode *convert( const CXMLElement &element, const NsScope &parent, const std::string &strPath )
                {
                    NsScope scope;
                    const NsScope *pScope = &parent;
                    size_t nProperties = element.GetPropertyCount();
                    for ( size_t n = 0; n < nProperties; n++ )
                    {
                        const CXMLProperty *pProperty = element.GetProperty( n );
                        std::string strName = text( pProperty->GetName() );
                        if ( strName == "xmlns" || strName.compare( 0, 6, "xmlns:" ) == 0 )
                        {
                            if ( pScope == &parent )
                            {
                                scope = parent;
                                pScope = &scope;
                            }
                            if ( strName == "xmlns" )
                                scope.strDefault = text( pProperty->GetValue() );
                            else
                                scope.prefixes[ strName.substr( 6 ) ] = text( pProperty->GetValue() );
                        }
                    }

                    m_nodes.push_back( XsNode() );
                    XsNode &node = m_nodes.back();
                    std::string strName = text( element.GetName() );
                    std::string strNamespace;
                    size_t nColon = strName.find( ':' );
                    if ( nColon == std::string::npos )
                    {
                        strNamespace = pScope->strDefault;
                        node.strLocal = strName;
                    }
                    else
                    {
                        std::map<std::string, std::string>::const_iterator it = pScope->prefixes.find( strName.substr( 0, nColon ) );
                        if ( it == pScope->prefixes.end() )
                            throw CSjsonException( SJSON_ERROR_MALFORMED, "Cannot read XSD " + strPath + ": unbound prefix in <" + strName + ">" );
                        strNamespace = it->second;
                        node.strLocal = strName.substr( nColon + 1 );
                    }
                    node.bXs = strNamespace == XS_NAMESPACE;
                    node.strTag = ( node.bXs || strNamespace.empty() ) ? node.strLocal : "{" + strNamespace + "}" + node.strLocal;

                    for ( size_t n = 0; n < nProperties; n++ )
                    {
                        const CXMLProperty *pProperty = element.GetProperty( n );
                        std::string strAttribute = text( pProperty->GetName() );
                        if ( strAttribute == "xmlns" || strAttribute.compare( 0, 6, "xmlns:" ) == 0 )
                            continue;
                        size_t nAttributeColon = strAttribute.find( ':' );
                        if ( nAttributeColon != std::string::npos )
                        {
                                // A namespaced attribute: never what get( name ) finds.
                            std::string strPrefix = strAttribute.substr( 0, nAttributeColon );
                            if ( strPrefix != "xml" && pScope->prefixes.find( strPrefix ) == pScope->prefixes.end() )
                                throw CSjsonException( SJSON_ERROR_MALFORMED, "Cannot read XSD " + strPath + ": unbound prefix in attribute " + strAttribute );
                            continue;
                        }
                        node.attributes.push_back( std::make_pair( strAttribute, text( pProperty->GetValue() ) ) );
                    }

                    size_t nChunks = element.GetChunkCount();
                    for ( size_t n = 0; n < nChunks; n++ )
                    {
                        const CXMLChunk *pChunk = element.GetChunk( n );
                        if ( pChunk == NULL || ! pChunk->IsElement() )
                            continue;
                        const CXMLElement *pChild = static_cast<const CXMLElement *>( pChunk );
                        if ( ! pChild->IsPlainElement() )
                            continue;
                        const XsNode *pConverted = convert( *pChild, *pScope, strPath );
                        node.children.push_back( pConverted );
                    }
                    return &node;
                }

                const XsNode *parse( const std::string &strPath )
                {
                    std::string strData = readFile( strPath );
                    CXMLIndex index;
                    try
                    {
                        CXMLParser parser;
                        parser.SetSingleRoot( true );
                        parser.Parse( strData.data(), strData.size(), index );
                    }
                    catch ( CException &e )
                    {
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "Cannot read XSD " + strPath + ": " + std::string( (const char *)e.GetMessage() ) );
                    }
                    const CXMLElement *pRoot = index.GetRoot();
                    if ( pRoot == NULL )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "Cannot read XSD " + strPath + ": no element found" );
                    NsScope scope;
                    scope.prefixes[ "xml" ] = XML_NAMESPACE;
                    return convert( *pRoot, scope, strPath );
                }

                void read( const std::string &strGiven )
                {
                    std::string strPath = absolutePath( strGiven );
                    if ( m_setLoaded.count( strPath ) )
                        return;
                    m_setLoaded.insert( strPath );
                    const XsNode *pSchema = parse( strPath );
                    if ( ! ( pSchema->bXs && pSchema->strLocal == "schema" ) )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, strPath + " is not an XML Schema document" );
                    for ( size_t n = 0; n < pSchema->children.size(); n++ )
                    {
                        const XsNode &child = *pSchema->children[ n ];
                        if ( child.is( "include" ) || child.is( "import" ) || child.is( "redefine" ) )
                        {
                            std::string strLocation = child.value( "schemaLocation" );
                            if ( ! strLocation.empty() && strLocation.find( "://" ) == std::string::npos )
                                read( joinPath( parentDirectory( strPath ), strLocation ) );
                            if ( child.is( "redefine" ) )
                                for ( size_t k = 0; k < child.children.size(); k++ )
                                    registerGlobal( *child.children[ k ] );
                        }
                        else if ( child.get( "name" ) != NULL )
                            registerGlobal( child );
                    }
                }

                void registerGlobal( const XsNode &child )
                {
                    const std::string *pName = child.get( "name" );
                    if ( pName == NULL )
                        return;
                    Table *pTable = NULL;
                    if ( child.is( "element" ) ) pTable = &m_elements;
                    else if ( child.is( "complexType" ) ) pTable = &m_complex;
                    else if ( child.is( "simpleType" ) ) pTable = &m_simple;
                    else if ( child.is( "group" ) ) pTable = &m_groups;
                    else if ( child.is( "attributeGroup" ) ) pTable = &m_attributeGroups;
                    else if ( child.is( "attribute" ) ) pTable = &m_attributes;
                    if ( pTable )
                        pTable->put( *pName, &child );
                }

                // -- types ------------------------------------------------------

                CSjsonSchemaType *newType( const char *pszKind = NULL )
                {
                    CSjsonSchemaType *pType = m_model.newType();
                    if ( pszKind )
                        pType->kinds.insert( pszKind );
                    return pType;
                }

                CSjsonSchemaType *namedType( const std::string &strQName )
                {
                    std::string strName = localName( strQName );
                    if ( m_complex.contains( strName ) )
                    {
                        std::unordered_map<std::string, CSjsonSchemaType *>::iterator it = m_cacheComplex.find( strName );
                        if ( it != m_cacheComplex.end() )
                            return it->second;
                        CSjsonSchemaType *pResult = newType();
                        m_cacheComplex[ strName ] = pResult;    // placed first so recursive types terminate
                        complexType( *pResult, *m_complex.get( strName ) );
                        return pResult;
                    }
                    if ( m_simple.contains( strName ) )
                    {
                        std::unordered_map<std::string, CSjsonSchemaType *>::iterator it = m_cacheSimple.find( strName );
                        if ( it != m_cacheSimple.end() )
                            return it->second;
                        CSjsonSchemaType *pResult = newType();
                        m_cacheSimple[ strName ] = pResult;     // placed first so recursive types terminate
                        simpleType( *pResult, *m_simple.get( strName ) );
                        return pResult;
                    }
                    return builtin( strName );
                }

                static const char *stringFormat( const std::string &strName )
                {
                    for ( size_t n = 0; n < sizeof( g_apszStringFormats ) / sizeof( g_apszStringFormats[ 0 ] ); n++ )
                        if ( strName == g_apszStringFormats[ n ][ 0 ] )
                            return g_apszStringFormats[ n ][ 1 ];
                    return NULL;
                }

                CSjsonSchemaType *stringType( const std::string &strName )
                {
                    CSjsonSchemaType *pResult = newType( "string" );
                    const char *pszFormat = stringFormat( strName );
                    if ( pszFormat )
                    {
                        pResult->bFormat = true;
                        pResult->strFormat = pszFormat;
                    }
                    return pResult;
                }

                CSjsonSchemaType *builtin( const std::string &strName )
                {
                    if ( strName == "anyType" || strName == "anySimpleType" )
                        return newType();
                    if ( ! m_options.bTypedValues )
                        return stringType( strName );
                    for ( size_t n = 0; n < sizeof( g_aIntegerBounds ) / sizeof( g_aIntegerBounds[ 0 ] ); n++ )
                    {
                        if ( strName != g_aIntegerBounds[ n ].pszName )
                            continue;
                        CSjsonSchemaType *pResult = newType( "integer" );
                        if ( g_aIntegerBounds[ n ].pszLow )
                            pResult->pMinimum.reset( SjsonCodec::makeInteger( g_aIntegerBounds[ n ].pszLow ) );
                        if ( g_aIntegerBounds[ n ].pszHigh )
                            pResult->pMaximum.reset( SjsonCodec::makeInteger( g_aIntegerBounds[ n ].pszHigh ) );
                        return pResult;
                    }
                    if ( strName == "decimal" || strName == "float" || strName == "double" )
                        return newType( "number" );
                    if ( strName == "boolean" )
                        return newType( "boolean" );
                    return stringType( strName );
                }

                    /// The JSON type of an element, wrapped in an array if it repeats.
                CSjsonSchemaType *elementType( const XsNode &element, double dMinOccurs, double dMaxOccurs )
                {
                    (void)dMinOccurs;
                    const XsNode *pTarget = &element;
                    if ( element.has( "ref" ) )
                    {
                        std::string strRef = localName( element.value( "ref" ) );
                        pTarget = m_elements.get( strRef );
                        if ( pTarget == NULL )
                            throw CSjsonException( SJSON_ERROR_MALFORMED, "Unknown element reference: " + strRef );
                    }

                    const XsNode *pInlineComplex = pTarget->find( "complexType" );
                    const XsNode *pInlineSimple = pTarget->find( "simpleType" );
                    CSjsonSchemaType *pValue;
                    if ( pTarget->has( "type" ) )
                        pValue = namedType( pTarget->value( "type" ) );
                    else if ( pInlineComplex )
                    {
                        pValue = newType();
                        complexType( *pValue, *pInlineComplex );
                    }
                    else if ( pInlineSimple )
                    {
                        pValue = newType();
                        simpleType( *pValue, *pInlineSimple );
                    }
                    else
                        pValue = newType();

                    const std::string *pNillable = pTarget->get( "nillable" );
                    if ( pNillable && *pNillable == "true" && ! pValue->kinds.empty() )
                    {
                        CSjsonSchemaType *pNullable = newType( "null" );
                        CSjsonSchemaType *pWrapper = newType();
                        pWrapper->variants.push_back( pValue );
                        pWrapper->variants.push_back( pNullable );
                        pValue = pWrapper;
                    }

                    if ( dMaxOccurs > 1 )
                    {
                        CSjsonSchemaType *pArray = newType( "array" );
                        pArray->pItems = pValue;
                        return pArray;
                    }
                    return pValue;
                }

                void addText( CSjsonSchemaType &result )
                {
                    result.addProperty( m_options.strTextKey, newType( "string" ), false );
                }

                void complexType( CSjsonSchemaType &result, const XsNode &node )
                {
                    result.kinds.clear();
                    result.kinds.insert( "object" );
                    result.bAdditionalProperties = false;
                    const std::string *pMixed = node.get( "mixed" );
                    if ( pMixed && *pMixed == "true" )
                        addText( result );
                    for ( size_t n = 0; n < node.children.size(); n++ )
                    {
                        const XsNode &child = *node.children[ n ];
                        if ( isParticle( child ) )
                            particle( result, child, 1, 1 );
                        else if ( isAttributeLike( child ) )
                            attribute( result, child );
                        else if ( child.is( "simpleContent" ) )
                            simpleContent( result, child );
                        else if ( child.is( "complexContent" ) )
                            complexContent( result, child );
                    }
                }

                static void copyProperties( CSjsonSchemaType &result, const CSjsonSchemaType &source )
                {
                    std::vector<CSjsonSchemaProperty> aProperties = source.properties;
                    for ( size_t n = 0; n < aProperties.size(); n++ )
                        result.addProperty( aProperties[ n ].strName, aProperties[ n ].pType, aProperties[ n ].bRequired );
                }

                void complexContent( CSjsonSchemaType &result, const XsNode &node )
                {
                    const std::string *pMixed = node.get( "mixed" );
                    if ( pMixed && *pMixed == "true" )
                        addText( result );
                    for ( size_t n = 0; n < node.children.size(); n++ )
                    {
                        const XsNode &derivation = *node.children[ n ];
                        if ( ! derivation.is( "extension" ) && ! derivation.is( "restriction" ) )
                            continue;
                        if ( derivation.is( "extension" ) && derivation.has( "base" ) && localName( derivation.value( "base" ) ) != "anyType" )
                        {
                            CSjsonSchemaType *pBase = namedType( derivation.value( "base" ) );
                            copyProperties( result, *pBase );
                            result.bAdditionalProperties = pBase->bAdditionalProperties;
                        }
                        for ( size_t k = 0; k < derivation.children.size(); k++ )
                        {
                            const XsNode &child = *derivation.children[ k ];
                            if ( isParticle( child ) )
                                particle( result, child, 1, 1 );
                            else if ( isAttributeLike( child ) )
                                attribute( result, child );
                        }
                    }
                }

                void simpleContent( CSjsonSchemaType &result, const XsNode &node )
                {
                    for ( size_t n = 0; n < node.children.size(); n++ )
                    {
                        const XsNode &derivation = *node.children[ n ];
                        if ( ! derivation.is( "extension" ) && ! derivation.is( "restriction" ) )
                            continue;
                        bool bBase = derivation.has( "base" );
                        std::string strBase = derivation.value( "base" );
                        CSjsonSchemaType *pText = bBase ? namedType( strBase ) : newType();
                        if ( bBase && m_complex.contains( localName( strBase ) ) )
                        {
                                // Deriving from a complex type with simple content.
                            copyProperties( result, *pText );
                        }
                        else
                        {
                            if ( derivation.is( "restriction" ) )
                            {
                                CSjsonSchemaType *pRestricted = newType();
                                copySimple( *pRestricted, *pText );
                                facets( *pRestricted, derivation );
                                pText = pRestricted;
                            }
                            result.addProperty( m_options.strTextKey, pText, true );
                        }
                        for ( size_t k = 0; k < derivation.children.size(); k++ )
                            if ( isAttributeLike( *derivation.children[ k ] ) )
                                attribute( result, *derivation.children[ k ] );
                    }
                }

                    /// Python's _occurs().
                static double occurs( const XsNode &node, const char *pszAttribute, double dDefault )
                {
                    const std::string *pValue = node.get( pszAttribute );
                    if ( pValue == NULL )
                        return dDefault;
                    if ( *pValue == "unbounded" )
                        return std::numeric_limits<double>::infinity();
                    return strtod( pyInt( *pValue ).c_str(), NULL );
                }

                    /// A sequence / choice / all / group and the elements in it.
                void particle( CSjsonSchemaType &result, const XsNode &start, double dMinOccurs, double dMaxOccurs )
                {
                    dMinOccurs = dMinOccurs * occurs( start, "minOccurs", 1 );
                    dMaxOccurs = dMaxOccurs * occurs( start, "maxOccurs", 1 );
                    const XsNode *pNode = &start;
                    if ( start.is( "group" ) )
                    {
                        const std::string *pRef = start.get( "ref" );
                        if ( pRef != NULL )
                        {
                            pNode = m_groups.get( localName( *pRef ) );
                            if ( pNode == NULL )
                                throw CSjsonException( SJSON_ERROR_MALFORMED, "Unknown group reference: " + *pRef );
                        }
                        for ( size_t n = 0; n < pNode->children.size(); n++ )
                        {
                            const XsNode &child = *pNode->children[ n ];
                            if ( child.is( "sequence" ) || child.is( "choice" ) || child.is( "all" ) )
                                particle( result, child, dMinOccurs, dMaxOccurs );
                        }
                        return;
                    }
                    if ( start.is( "choice" ) )
                        dMinOccurs = 0;
                    for ( size_t n = 0; n < start.children.size(); n++ )
                    {
                        const XsNode &child = *start.children[ n ];
                        if ( child.is( "element" ) )
                        {
                            double dChildMin = dMinOccurs * occurs( child, "minOccurs", 1 );
                            double dChildMax = dMaxOccurs * occurs( child, "maxOccurs", 1 );
                            std::string strName = child.has( "name" ) ? child.value( "name" ) : localName( child.value( "ref" ) );
                            CSjsonSchemaType *pType = elementType( child, dChildMin, dChildMax );
                            result.addProperty( strName, pType, dChildMin > 0 );
                        }
                        else if ( isParticle( child ) )
                            particle( result, child, dMinOccurs, dMaxOccurs );
                        else if ( child.is( "any" ) )
                            result.bAdditionalProperties = true;
                    }
                }

                void attribute( CSjsonSchemaType &result, const XsNode &node )
                {
                    if ( node.is( "anyAttribute" ) )
                    {
                        result.bAdditionalProperties = true;
                        return;
                    }
                    if ( node.is( "attributeGroup" ) )
                    {
                        const XsNode *pGroup = m_attributeGroups.get( localName( node.value( "ref" ) ) );
                        if ( pGroup == NULL )
                            throw CSjsonException( SJSON_ERROR_MALFORMED, "Unknown attributeGroup: " + ( node.get( "ref" ) ? node.value( "ref" ) : std::string( "None" ) ) );
                        for ( size_t n = 0; n < pGroup->children.size(); n++ )
                            if ( isAttributeLike( *pGroup->children[ n ] ) )
                                attribute( result, *pGroup->children[ n ] );
                        return;
                    }
                    const std::string *pUse = node.get( "use" );
                    if ( pUse && *pUse == "prohibited" )
                        return;
                    const XsNode *pTarget = &node;
                    if ( node.has( "ref" ) )
                    {
                        const XsNode *pGlobal = m_attributes.get( localName( node.value( "ref" ) ) );
                        if ( pGlobal )
                            pTarget = pGlobal;
                    }
                    std::string strName = pTarget->has( "name" ) ? pTarget->value( "name" ) : localName( node.value( "ref" ) );
                    const XsNode *pInline = pTarget->find( "simpleType" );
                    CSjsonSchemaType *pType;
                    if ( pTarget->has( "type" ) )
                        pType = namedType( pTarget->value( "type" ) );
                    else if ( pInline )
                    {
                        pType = newType();
                        simpleType( *pType, *pInline );
                    }
                    else
                        pType = newType( "string" );
                    result.addProperty( m_options.strAttributePrefix + strName, pType, pUse && *pUse == "required" );
                }

                void simpleType( CSjsonSchemaType &result, const XsNode &node )
                {
                    for ( size_t n = 0; n < node.children.size(); n++ )
                    {
                        const XsNode &child = *node.children[ n ];
                        if ( child.is( "restriction" ) )
                        {
                            CSjsonSchemaType *pBase;
                            if ( child.has( "base" ) )
                                pBase = namedType( child.value( "base" ) );
                            else
                            {
                                pBase = newType();
                                const XsNode *pInline = child.find( "simpleType" );
                                if ( pInline )
                                    simpleType( *pBase, *pInline );
                            }
                            copySimple( result, *pBase );
                            facets( result, child );
                        }
                        else if ( child.is( "list" ) )
                        {
                            result.kinds.clear();
                            result.kinds.insert( "string" );
                        }
                        else if ( child.is( "union" ) )
                        {
                            std::vector<CSjsonSchemaType *> aMembers;
                            std::vector<std::string> aNames = pySplit( child.value( "memberTypes" ) );
                            for ( size_t k = 0; k < aNames.size(); k++ )
                                aMembers.push_back( namedType( aNames[ k ] ) );
                            for ( size_t k = 0; k < child.children.size(); k++ )
                            {
                                const XsNode &inlineType = *child.children[ k ];
                                if ( ! ( inlineType.bXs && inlineType.strLocal == "simpleType" ) )
                                    continue;
                                CSjsonSchemaType *pMember = newType();
                                simpleType( *pMember, inlineType );
                                aMembers.push_back( pMember );
                            }
                            result.variants = aMembers;
                        }
                    }
                }

                static void copySimple( CSjsonSchemaType &target, const CSjsonSchemaType &source )
                {
                    std::set<std::string> kinds = source.kinds;
                    target.kinds = kinds;
                    target.bEnum = source.bEnum;
                    std::vector< std::shared_ptr<CJsonNode> > values = source.enumValues;
                    target.enumValues = values;
                    target.pMinimum = source.pMinimum;
                    target.pMaximum = source.pMaximum;
                    target.bFormat = source.bFormat;
                    std::string strFormat = source.strFormat;
                    target.strFormat = strFormat;
                    std::vector<CSjsonSchemaType *> variants = source.variants;
                    target.variants = variants;
                }

                static bool onlyKind( const CSjsonSchemaType &type, const char *pszKind )
                {
                    return type.kinds.size() == 1 && *type.kinds.begin() == pszKind;
                }

                static Converter converter( const CSjsonSchemaType &result )
                {
                    if ( onlyKind( result, "integer" ) )
                        return CONVERT_INT;
                    if ( onlyKind( result, "number" ) )
                        return CONVERT_FLOAT;
                    if ( onlyKind( result, "boolean" ) )
                        return CONVERT_BOOL;
                    return CONVERT_STR;
                }

                static CJsonNode *convertValue( Converter eConvert, const std::string &strValue )
                {
                    switch ( eConvert )
                    {
                        case CONVERT_INT:   return SjsonCodec::makeInteger( pyInt( strValue ) );
                        case CONVERT_FLOAT: return new CNumberNode( pyFloat( strValue ) );
                        case CONVERT_BOOL:
                        {
                            std::string strText = pyStrip( strValue );
                            return new CBooleanNode( strText == "true" || strText == "1" );
                        }
                        default:            return new CTextNode( strValue );
                    }
                }

                void facets( CSjsonSchemaType &result, const XsNode &restriction )
                {
                    Converter eConvert = converter( result );
                    std::vector<std::string> aValues;
                    for ( size_t n = 0; n < restriction.children.size(); n++ )
                    {
                        const XsNode &facet = *restriction.children[ n ];
                        if ( facet.bXs && facet.strLocal == "enumeration" )
                            aValues.push_back( facet.value( "value" ) );
                    }
                    if ( ! aValues.empty() )
                    {
                        std::vector< std::shared_ptr<CJsonNode> > aEnum;
                        for ( size_t n = 0; n < aValues.size(); n++ )
                            aEnum.push_back( std::shared_ptr<CJsonNode>( convertValue( eConvert, aValues[ n ] ) ) );
                        result.bEnum = true;
                        result.enumValues = aEnum;
                    }
                    for ( size_t n = 0; n < restriction.children.size(); n++ )
                    {
                        const XsNode &facet = *restriction.children[ n ];
                        const std::string *pValue = facet.get( "value" );
                        if ( pValue == NULL )
                            continue;
                        bool bOther = false;
                        for ( std::set<std::string>::const_iterator it = result.kinds.begin(); it != result.kinds.end(); ++it )
                            if ( *it != "integer" && *it != "number" )
                                bOther = true;
                        if ( bOther )
                            continue;
                        std::shared_ptr<CJsonNode> pNumber( convertValue( eConvert, *pValue ) );
                        bool bInteger = onlyKind( result, "integer" );
                        if ( facet.is( "minInclusive" ) )
                            result.pMinimum = pNumber;
                        else if ( facet.is( "maxInclusive" ) )
                            result.pMaximum = pNumber;
                        else if ( facet.is( "minExclusive" ) && bInteger )
                            result.pMinimum.reset( SjsonCodec::makeInteger( SjsonUtil::decimalAdd( pyInt( *pValue ), "1" ) ) );
                        else if ( facet.is( "maxExclusive" ) && bInteger )
                            result.pMaximum.reset( SjsonCodec::makeInteger( SjsonUtil::decimalSubtract( pyInt( *pValue ), "1" ) ) );
                    }
                }
        };
    }

    namespace SjsonSchema
    {
        CSjsonSchemaModel *loadXsd( const std::string &strPath, const CSjsonXsdOptions &options )
        {
            std::unique_ptr<CSjsonSchemaModel> pModel( new CSjsonSchemaModel() );
            CXsdLoader loader( *pModel, options );
            loader.load( strPath );
            return pModel.release();
        }
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
