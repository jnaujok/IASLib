/**
 * JSON Node Class
 *
 *  This abstract base class is the root type of all JSON nodes in a JSON
 * tree, modelled on Jackson's JsonNode. Every node answers the whole
 * interface: asking a node for something it does not have gives a neutral
 * answer (NULL, 0, false, "" or the missing node) rather than failing, so
 * paths through a document can be followed without checking each step:
 *
 *      root->path( "users" ).path( 0 ).path( "name" ).asText()
 *
 *  Ownership: a container owns its children, and deleting the root deletes
 * the tree. Pointers returned by get(), path(), at() and the find methods
 * belong to the tree; do not delete them. The missing node is a shared
 * singleton. Nodes may be read from several threads at once, but a tree
 * being changed must not be read or changed elsewhere at the same time.
 *
 *  Text is UTF-8. Text values may hold any character, including U+0000;
 * textValue() and asText() return a CString, which stops at a NUL, so use
 * textString() where that matters.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_JSONNODE_H__
#define IASLIB_JSONNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../Collections/StringArray.h"
#include "JsonNodeType.h"

#include <string>
#include <vector>

namespace IASLib
{
    class CObjectNode;
    class CArrayNode;

        // A field name, from any of the library's string types.
    class CJsonKey
    {
        public:
                                    CJsonKey( const char *pszKey ) : m_strKey( pszKey ? pszKey : "" ) {}
                                    CJsonKey( const std::string &strKey ) : m_strKey( strKey ) {}
                                    CJsonKey( const CString &strKey ) : m_strKey( (const char *)strKey, strKey.GetLength() ) {}
            const std::string      &str( void ) const { return m_strKey; }

        private:
            std::string             m_strKey;
    };

    class CJsonNode : public CObject
    {
        public:
            DEFINE_OBJECT( CJsonNode )

            virtual                ~CJsonNode( void ) {}

                // Type.
            virtual JsonNodeType    getNodeType( void ) const = 0;
            bool                    isArray( void ) const { return getNodeType() == ARRAY; }
            bool                    isObject( void ) const { return getNodeType() == OBJECT; }
            bool                    isTextual( void ) const { return getNodeType() == STRING; }
            bool                    isNumber( void ) const { return getNodeType() == NUMBER; }
            bool                    isBoolean( void ) const { return getNodeType() == BOOLEAN; }
            bool                    isNull( void ) const { return getNodeType() == NULLVAL; }
            bool                    isBinary( void ) const { return getNodeType() == BINARY; }
            bool                    isMissingNode( void ) const { return getNodeType() == MISSING; }
            bool                    isContainerNode( void ) const { return isArray() || isObject(); }
            bool                    isValueNode( void ) const { return ! isContainerNode() && ! isMissingNode(); }

                // Numbers: an integer that fits in 32 bits (isInt) or 64 bits
                // (isLong); an integer too large for 64 bits (isBigInteger,
                // held as a double but written as it was read); or a number
                // with a fraction or exponent (isDouble).
            virtual bool            isIntegralNumber( void ) const { return false; }
            virtual bool            isFloatingPointNumber( void ) const { return false; }
            virtual bool            isInt( void ) const { return false; }
            virtual bool            isLong( void ) const { return false; }
            virtual bool            isBigInteger( void ) const { return false; }
            virtual bool            isDouble( void ) const { return false; }
                // A number whose value (dropping any fraction) fits the type.
            virtual bool            canConvertToInt( void ) const { return false; }
            virtual bool            canConvertToLong( void ) const { return false; }

                // The node's own value, for a node of that type: 0, false or ""
                // otherwise. Numbers convert between types, truncating fractions
                // and clamping to the type's range.
            virtual bool            booleanValue( void ) const { return false; }
            virtual int             intValue( void ) const { return 0; }
            virtual long long       longValue( void ) const { return 0; }
            short                   shortValue( void ) const;
            virtual double          doubleValue( void ) const { return 0.0; }
            float                   floatValue( void ) const { return (float)doubleValue(); }
            virtual CString         textValue( void ) const { return CString( "" ); }
            virtual const std::string &textString( void ) const;
                // Bytes: a binary node's, or a text node's base64 decoded. False
                // if there are none.
            virtual bool            binaryValue( std::vector<unsigned char> &aData ) const;

                // Values converted from other types where that makes sense: text
                // to numbers ("12") or booleans ("true"), numbers to booleans
                // (non-zero) and text. Otherwise the default given.
            virtual CString         asText( void ) const { return CString( "" ); }
            virtual bool            asBoolean( bool bDefault = false ) const { return bDefault; }
            virtual int             asInt( int nDefault = 0 ) const { return nDefault; }
            virtual long long       asLong( long long llDefault = 0 ) const { return llDefault; }
            virtual double          asDouble( double dDefault = 0.0 ) const { return dDefault; }

                // Children: the elements of an array, the fields of an object.
            virtual int             size( void ) const { return 0; }

                // An array's element, or an object's field; NULL if there is none.
            CJsonNode              *get( int nIndex ) { return element( nIndex ); }
            const CJsonNode        *get( int nIndex ) const { return const_cast<CJsonNode *>( this )->element( nIndex ); }
            CJsonNode              *get( const char *pszField ) { return pszField ? field( std::string( pszField ) ) : NULL; }
            const CJsonNode        *get( const char *pszField ) const { return const_cast<CJsonNode *>( this )->get( pszField ); }
            CJsonNode              *get( const std::string &strField ) { return field( strField ); }
            const CJsonNode        *get( const std::string &strField ) const { return const_cast<CJsonNode *>( this )->field( strField ); }

                // The same, but the missing node rather than NULL, so calls can
                // be chained.
            CJsonNode              &path( int nIndex ) const;
            CJsonNode              &path( const char *pszField ) const;
            CJsonNode              &path( const std::string &strField ) const;

                // True if there is such an element or field (has), and it is not
                // null (hasNonNull).
            bool                    has( int nIndex ) const { return get( nIndex ) != NULL; }
            bool                    has( const char *pszField ) const { return get( pszField ) != NULL; }
            bool                    has( const std::string &strField ) const { return get( strField ) != NULL; }
            bool                    hasNonNull( int nIndex ) const;
            bool                    hasNonNull( const char *pszField ) const;
            bool                    hasNonNull( const std::string &strField ) const;

                // A JSON Pointer (RFC 6901), such as "/users/0/name"; "" is this
                // node. The missing node if there is no such value.
            CJsonNode              &at( const char *pszPointer ) const;
            CJsonNode              &at( const std::string &strPointer ) const { return at( strPointer.c_str() ); }

                // Searches this node and its descendants, depth first, for
                // object fields with this name. A field that matches is not
                // searched inside.
                //  findValue:  the first such field's value, or NULL.
                //  findPath:   the same, or the missing node.
                //  findValues: every such value; findValuesAsText their asText().
                //  findParent(s): the object(s) holding such fields.
            CJsonNode              *findValue( const char *pszField ) const;
            CJsonNode              &findPath( const char *pszField ) const;
            std::vector<CJsonNode *> findValues( const char *pszField ) const;
            CStringArray            findValuesAsText( const char *pszField ) const;
            CObjectNode            *findParent( const char *pszField ) const;
            std::vector<CObjectNode *> findParents( const char *pszField ) const;

                // Deep equality. Numbers compare by value (1 equals 1.0); object
                // fields compare regardless of order.
            virtual bool            equals( const CJsonNode *pOther ) const = 0;
                // A deep copy, owned by the caller.
            virtual CJsonNode      *clone( void ) const = 0;

                // JSON text: compact (toString), indented by two spaces
                // (toPrettyString), or with a chosen indent (negative for
                // compact) and, optionally, only ASCII (other characters as \u
                // escapes). Text that is not valid UTF-8 is written with U+FFFD
                // in place of the bad bytes, so the output is always valid JSON.
                // The missing node writes nothing.
            CString                 toString( void ) const;
            CString                 toPrettyString( void ) const;
            std::string             toJson( int nIndent = -1, bool bAsciiOnly = false ) const;

                // Appends this node's JSON to strOut (used by toJson).
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const = 0;

                // Helpers shared by the writer and parser.
            static void             writeString( std::string &strOut, const std::string &strText, bool bAsciiOnly );
            static std::string      formatDouble( double dValue );
            static std::string      encodeBase64( const std::vector<unsigned char> &aData );
            static bool             decodeBase64( const std::string &strText, std::vector<unsigned char> &aData );
                // Reads a whole string as a double, with '.' as the decimal point
                // whatever the locale. False if it is not entirely a number.
            static bool             parseDouble( const std::string &strText, double &dValue );
                // The length of the valid UTF-8 sequence at p, and its code
                // point; 0 if there is none (overlong forms, surrogates and
                // values past U+10FFFF are not valid).
            static size_t           decodeUtf8( const unsigned char *p, const unsigned char *pEnd, unsigned int &nCode );

        protected:
                // What get() uses: an array element or object field, or NULL.
            virtual CJsonNode      *element( int nIndex ) { (void)nIndex; return NULL; }
            virtual CJsonNode      *field( const std::string &strField ) { (void)strField; return NULL; }

                // The search behind the find methods: values (or, with bParents,
                // the objects holding them) of fields named strField. Stops after
                // the first if bFirst. Returns true once something is found and
                // bFirst is set.
            virtual bool            collect( const std::string &strField, bool bParents, bool bFirst, std::vector<CJsonNode *> &aFound ) const;

            static void             newLine( std::string &strOut, int nIndent, int nLevel );

            friend class CObjectNode;
            friend class CArrayNode;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_JSONNODE_H__
