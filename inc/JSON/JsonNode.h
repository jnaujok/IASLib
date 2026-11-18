/**
 * @file JsonNode.h
 * @brief CJsonNode, the root type of every JSON node, and CJsonKey.
 *
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
 * belong to the tree; do not delete them. path(), at() and findPath() give a
 * const reference: to change a node found by a path, use get() (which gives
 * NULL rather than the missing node). The missing node is one shared,
 * read-only object; do not delete it or add it to a tree (containers store a
 * null in its place). Nodes may be read from several threads at once, but a
 * tree being changed must not be read or changed elsewhere at the same time.
 *
 *  Trees of any depth can be written, copied, compared, searched and
 * deleted: those walk the tree with a list of their own rather than by
 * recursion, so a deep tree cannot overflow the stack.
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

        /// A field name, from any of the library's string types.
        ///
        /// CObjectNode's methods take a CJsonKey, so a field can be named by a
        /// `const char *`, a `std::string` or a CString without overloads for
        /// each. The name is copied; a CString key keeps any embedded NUL.
    class CJsonKey
    {
        public:
                /// A key from C text; NULL gives the empty name.
                                    CJsonKey( const char *pszKey ) : m_strKey( pszKey ? pszKey : "" ) {}
                /// A key from a std::string.
                                    CJsonKey( const std::string &strKey ) : m_strKey( strKey ) {}
                /// A key from a CString, all GetLength() bytes of it.
                                    CJsonKey( const CString &strKey ) : m_strKey( (const char *)strKey, strKey.GetLength() ) {}
                /// The name.
            const std::string      &str( void ) const { return m_strKey; }

        private:
            std::string             m_strKey;   ///< The name, as UTF-8 bytes.
    };

        /// The abstract root type of every node in a JSON tree.
        ///
        /// Modelled on Jackson's JsonNode. Every node answers the whole
        /// interface: asking a node for something it does not have gives a
        /// neutral answer (NULL, 0, false, "" or the missing node) rather than
        /// failing, so paths through a document can be followed without
        /// checking each step.
        ///
        /// Ownership: a container owns its children, and deleting the root
        /// deletes the tree. Pointers returned by get(), path(), at() and the
        /// find methods belong to the tree; do not delete them. The missing
        /// node (CMissingNode::getInstance()) is shared and read-only.
        ///
        /// Threading: nodes may be read from several threads at once, but a
        /// tree being changed must not be read or changed elsewhere at the
        /// same time.
        ///
        /// Text is UTF-8 and may hold U+0000; textValue() and asText() return
        /// a CString, which stops at a NUL, so use textString() where that
        /// matters.
        ///
        /// @code
        ///     CJsonNode *pRoot = CJsonParser::parse( strText );
        ///     if ( pRoot != NULL )
        ///     {
        ///         CString strName = pRoot->path( "users" ).path( 0 ).path( "name" ).asText();
        ///         int nAge = pRoot->at( "/users/0/age" ).asInt( -1 );
        ///         delete pRoot;
        ///     }
        /// @endcode
    class CJsonNode : public CObject
    {
        public:
            DEFINE_OBJECT( CJsonNode )

                /// Deletes the node (and, for a container, its whole tree).
            virtual                ~CJsonNode( void ) {}

                /// @name Type
                ///@{

                /// What kind of node this is.
            virtual JsonNodeType    getNodeType( void ) const = 0;
                /// True for an array.
            bool                    isArray( void ) const { return getNodeType() == JSON_ARRAY; }
                /// True for an object.
            bool                    isObject( void ) const { return getNodeType() == JSON_OBJECT; }
                /// True for a string.
            bool                    isTextual( void ) const { return getNodeType() == JSON_STRING; }
                /// True for a number.
            bool                    isNumber( void ) const { return getNodeType() == JSON_NUMBER; }
                /// True for true or false.
            bool                    isBoolean( void ) const { return getNodeType() == JSON_BOOLEAN; }
                /// True for null.
            bool                    isNull( void ) const { return getNodeType() == JSON_NULL; }
                /// True for a binary node.
            bool                    isBinary( void ) const { return getNodeType() == JSON_BINARY; }
                /// True for the missing node.
            bool                    isMissingNode( void ) const { return getNodeType() == JSON_MISSING; }
                /// True for an array or object.
            bool                    isContainerNode( void ) const { return isArray() || isObject(); }
                /// True for a single value: not a container and not missing.
            bool                    isValueNode( void ) const { return ! isContainerNode() && ! isMissingNode(); }
                ///@}

                /// @name Kinds of number
                /// Numbers are one of: an integer that fits in 32 bits (isInt)
                /// or 64 bits (isLong); a positive integer above the signed
                /// 64-bit range that fits in 64 bits unsigned (isUnsignedLong,
                /// held exactly); an integer too large for either
                /// (isBigInteger, held as a double but written as it was read);
                /// or a number with a fraction or exponent (isDouble). All are
                /// false for a node that is not a number.
                /// @note Integers from 2^63 to 2^64 - 1 used to be big
                /// integers, so they could not be read back exactly.
                ///@{

                /// True for a number without a fraction or exponent.
            virtual bool            isIntegralNumber( void ) const { return false; }
                /// True for a number with a fraction or exponent.
            virtual bool            isFloatingPointNumber( void ) const { return false; }
                /// True for an integer that fits in a signed 32-bit int.
            virtual bool            isInt( void ) const { return false; }
                /// True for an integer held as a signed 64-bit value (which
                /// includes those isInt() accepts).
            virtual bool            isLong( void ) const { return false; }
                /// True for an integer above the signed 64-bit range that fits in
                /// 64 bits unsigned (2^63 to 2^64 - 1).
            virtual bool            isUnsignedLong( void ) const { return false; }
                /// True for an integer too large for 64 bits, held as a double.
            virtual bool            isBigInteger( void ) const { return false; }
                /// True for a number with a fraction or exponent (the same as
                /// isFloatingPointNumber()).
            virtual bool            isDouble( void ) const { return false; }
                /// True for a number whose value, dropping any fraction, fits in
                /// an int.
            virtual bool            canConvertToInt( void ) const { return false; }
                /// True for a number whose value, dropping any fraction, fits in
                /// a signed 64-bit integer.
            virtual bool            canConvertToLong( void ) const { return false; }
                /// True for a number whose value, dropping any fraction, fits in
                /// an unsigned 64-bit integer.
            virtual bool            canConvertToUnsignedLong( void ) const { return false; }
                ///@}

                /// @name Own values
                /// The node's own value, for a node of that type: 0, false or ""
                /// otherwise. Numbers convert between types, truncating
                /// fractions and clamping to the type's range.
                ///@{

                /// A boolean node's value; false for any other node.
            virtual bool            booleanValue( void ) const { return false; }
                /// A number's value as an int (truncated and clamped); 0 for any other node.
            virtual int             intValue( void ) const { return 0; }
                /// A number's value as a signed 64-bit integer (truncated and
                /// clamped); 0 for any other node.
            virtual long long       longValue( void ) const { return 0; }
                /// A number's value as an unsigned 64-bit integer (truncated and
                /// clamped, so negative numbers give 0); 0 for any other node.
            virtual unsigned long long unsignedLongValue( void ) const { return 0; }
                /// intValue() clamped to the range of a short.
            short                   shortValue( void ) const;
                /// A number's value as a double; 0.0 for any other node.
            virtual double          doubleValue( void ) const { return 0.0; }
                /// doubleValue() as a float.
            float                   floatValue( void ) const { return (float)doubleValue(); }
                /// A text node's text; "" for any other node. The CString stops
                /// at the first U+0000: see textString().
            virtual CString         textValue( void ) const { return CString( "" ); }
                /// A text node's text, whole; a reference to an empty string for
                /// any other node. The reference stays valid while the node does.
            virtual const std::string &textString( void ) const;
                /// The node's bytes: a binary node's, or a text node's text
                /// decoded as base64 (standard or URL-safe, padding optional).
                /// @param aData Receives the bytes.
                /// @return False if there are none: the node is neither binary
                ///         nor text, or the text is not base64.
            virtual bool            binaryValue( std::vector<unsigned char> &aData ) const;
                ///@}

                /// @name Converted values
                /// Values converted from other types where that makes sense:
                /// text to numbers ("12") or booleans ("true"), numbers to
                /// booleans (non-zero) and text. Otherwise the default given.
                ///@{

                /// The value as text: a string's text, a number as JSON writes
                /// it, "true", "false" or "null", or a binary node's base64.
                /// "" for a container or the missing node.
            virtual CString         asText( void ) const { return CString( "" ); }
                /// The value as a boolean, or bDefault if it has none.
            virtual bool            asBoolean( bool bDefault = false ) const { return bDefault; }
                /// The value as an int, or nDefault if it has none.
            virtual int             asInt( int nDefault = 0 ) const { return nDefault; }
                /// The value as a signed 64-bit integer, or llDefault if it has none.
            virtual long long       asLong( long long llDefault = 0 ) const { return llDefault; }
                /// The value as an unsigned 64-bit integer, or ullDefault if it has none.
            virtual unsigned long long asUnsignedLong( unsigned long long ullDefault = 0 ) const { return ullDefault; }
                /// The value as a double, or dDefault if it has none.
            virtual double          asDouble( double dDefault = 0.0 ) const { return dDefault; }
                ///@}

                /// @name Children
                ///@{

                /// The number of children: an array's elements or an object's
                /// fields; 0 for any other node.
            virtual int             size( void ) const { return 0; }

                /// An array's element; NULL if there is none (out of range, or
                /// not an array). The node belongs to the tree.
            CJsonNode              *get( int nIndex ) { return element( nIndex ); }
                /// @copydoc get(int)
            const CJsonNode        *get( int nIndex ) const { return const_cast<CJsonNode *>( this )->element( nIndex ); }
                /// An object's field; NULL if there is none (or pszField is NULL,
                /// or this is not an object). The node belongs to the tree.
            CJsonNode              *get( const char *pszField ) { return pszField ? field( std::string( pszField ) ) : NULL; }
                /// @copydoc get(const char *)
            const CJsonNode        *get( const char *pszField ) const { return const_cast<CJsonNode *>( this )->get( pszField ); }
                /// An object's field; NULL if there is none. The node belongs to
                /// the tree.
            CJsonNode              *get( const std::string &strField ) { return field( strField ); }
                /// @copydoc get(const std::string &)
            const CJsonNode        *get( const std::string &strField ) const { return const_cast<CJsonNode *>( this )->field( strField ); }

                /// The same as get(), but the missing node rather than NULL, so
                /// calls can be chained. The reference is const: the missing
                /// node is shared, so it must not be changed.
                /// @note These used to give a non-const reference to it.
            const CJsonNode        &path( int nIndex ) const;
                /// @copydoc path(int) const
            const CJsonNode        &path( const char *pszField ) const;
                /// @copydoc path(int) const
            const CJsonNode        &path( const std::string &strField ) const;

                /// True if there is such an element.
            bool                    has( int nIndex ) const { return get( nIndex ) != NULL; }
                /// True if there is such a field.
            bool                    has( const char *pszField ) const { return get( pszField ) != NULL; }
                /// @copydoc has(const char *) const
            bool                    has( const std::string &strField ) const { return get( strField ) != NULL; }
                /// True if there is such an element and it is not null.
            bool                    hasNonNull( int nIndex ) const;
                /// True if there is such a field and it is not null.
            bool                    hasNonNull( const char *pszField ) const;
                /// @copydoc hasNonNull(const char *) const
            bool                    hasNonNull( const std::string &strField ) const;

                /// The value at a JSON Pointer (RFC 6901), such as
                /// "/users/0/name".
                /// @param pszPointer "" for this node; otherwise "/"-separated
                ///        tokens, with "~1" for "/" and "~0" for "~". An array
                ///        index is digits without leading zeros ("-" never
                ///        matches).
                /// @return The node, or the missing node if there is no such
                ///         value (or pszPointer is NULL or malformed).
            const CJsonNode        &at( const char *pszPointer ) const;
                /// The same as at(const char *) const, for a std::string pointer.
            const CJsonNode        &at( const std::string &strPointer ) const { return at( strPointer.c_str() ); }
                ///@}

                /// @name Searching
                /// These search this node and its descendants, depth first and
                /// in order, for object fields with this name. A field that
                /// matches is not searched inside. The nodes found belong to the
                /// tree. A NULL name searches for the empty name.
                ///@{

                /// The first matching field's value, or NULL.
            CJsonNode              *findValue( const char *pszField ) const;
                /// The first matching field's value, or the missing node.
            const CJsonNode        &findPath( const char *pszField ) const;
                /// Every matching field's value, in the order found.
            std::vector<CJsonNode *> findValues( const char *pszField ) const;
                /// The asText() of every matching field's value.
            CStringArray            findValuesAsText( const char *pszField ) const;
                /// The first object holding a matching field, or NULL.
            CObjectNode            *findParent( const char *pszField ) const;
                /// Every object holding a matching field, each once.
            std::vector<CObjectNode *> findParents( const char *pszField ) const;
                ///@}

                /// Deep equality. Numbers compare by value (1 equals 1.0);
                /// object fields compare regardless of order.
                /// @param pOther The node to compare with; NULL is never equal.
            virtual bool            equals( const CJsonNode *pOther ) const = 0;
                /// A deep copy, owned by the caller.
            virtual CJsonNode      *clone( void ) const = 0;

                /// @name Writing JSON
                /// Text that is not valid UTF-8 is written with U+FFFD in place
                /// of the bad bytes, so the output is always valid JSON. The
                /// missing node by itself writes nothing; inside an array or
                /// object (or as a NULL child) it is written as null.
                /// @note It used to write nothing there too, leaving "[,1]" or
                /// "{"a":}".
                ///@{

                /// Compact JSON text.
            CString                 toString( void ) const;
                /// JSON text indented by two spaces.
            CString                 toPrettyString( void ) const;
                /// JSON text.
                /// @param nIndent    Spaces per level of nesting; negative for
                ///                   compact output on one line.
                /// @param bAsciiOnly Write characters past U+007F as `\uXXXX`
                ///                   escapes (surrogate pairs past U+FFFF).
            std::string             toJson( int nIndent = -1, bool bAsciiOnly = false ) const;

                /// Appends this node's JSON to strOut (used by toJson()).
                /// @param strOut     The text to append to.
                /// @param nIndent    Spaces per level; negative for compact.
                /// @param nLevel     The nesting level this node is written at.
                /// @param bAsciiOnly As for toJson().
            virtual void            writeJson( std::string &strOut, int nIndent, int nLevel, bool bAsciiOnly ) const = 0;
                ///@}

                /// @name Helpers shared by the writer and parser
                ///@{

                /// Appends strText as a quoted, escaped JSON string. Bytes that
                /// are not valid UTF-8 each become U+FFFD.
                /// @param strOut     The text to append to.
                /// @param strText    The UTF-8 text.
                /// @param bAsciiOnly As for toJson().
            static void             writeString( std::string &strOut, const std::string &strText, bool bAsciiOnly );
                /// The shortest text that reads back as the same double, always
                /// with a fraction or exponent, and with '.' whatever the
                /// locale. "null" for infinities and NaN, which JSON lacks.
            static std::string      formatDouble( double dValue );
                /// Standard base64, with padding.
            static std::string      encodeBase64( const std::vector<unsigned char> &aData );
                /// Decodes standard or URL-safe base64, with or without padding.
                /// @param strText The base64 text.
                /// @param aData   Receives the bytes (emptied on failure).
                /// @return False for anything else, including white space.
            static bool             decodeBase64( const std::string &strText, std::vector<unsigned char> &aData );
                /// Reads a whole string as a double, with '.' as the decimal
                /// point whatever the locale.
                /// @return False if it is not entirely a number.
            static bool             parseDouble( const std::string &strText, double &dValue );
                /// The length of the valid UTF-8 sequence at p, and its code point.
                /// @param p     The first byte.
                /// @param pEnd  Just past the last byte that may be read.
                /// @param nCode Receives the code point.
                /// @return The sequence's length in bytes, or 0 if there is none
                ///         (overlong forms, surrogates and values past U+10FFFF
                ///         are not valid).
            static size_t           decodeUtf8( const unsigned char *p, const unsigned char *pEnd, unsigned int &nCode );
                ///@}

        protected:
                /// What get() uses for an index: an array's element, or NULL.
            virtual CJsonNode      *element( int nIndex ) { (void)nIndex; return NULL; }
                /// What get() uses for a name: an object's field, or NULL.
            virtual CJsonNode      *field( const std::string &strField ) { (void)strField; return NULL; }

                /// The search behind the find methods: values (or, with
                /// bParents, the objects holding them) of fields named
                /// strField, depth first, without recursion.
                /// @param strField The field name.
                /// @param bParents Collect the objects holding the fields, not their values.
                /// @param bFirst   Stop after the first found.
                /// @param aFound   Receives what is found (appended).
                /// @return True once something is found and bFirst is set.
            bool                    collect( const std::string &strField, bool bParents, bool bFirst, std::vector<CJsonNode *> &aFound ) const;

                /// Starts a new line indented to nLevel (nothing when nIndent
                /// is negative, for compact output).
            static void             newLine( std::string &strOut, int nIndent, int nLevel );

            friend class CObjectNode;
            friend class CArrayNode;
    };
} // namespace IASLib

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_JSONNODE_H__
