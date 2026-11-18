/**
 * @file SjsonSchema.h
 * @brief SJSON schema support: a format-neutral schema model, loaders for
 *        JSON Schema and XSD, and Python-identical canonical form and
 *        fingerprint.
 *
 *  Both loaders produce a CSjsonSchemaModel holding what SJSON needs: field
 * names (to pre-build the dictionary), enumerated values, and enough type
 * information for the schema-driven encoding. A model loaded here from a
 * given schema has the same field order, enumerations and 32-bit
 * fingerprint as one the Python sjson library loads from it, so C++ and
 * Python peers agree on the dictionary without exchanging it.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSONSCHEMA_H__
#define IASLIB_SJSONSCHEMA_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace IASLib
{
    class CJsonNode;
    class CSjsonSchemaType;

        /// A named member of an object type.
    struct CSjsonSchemaProperty
    {
        std::string         strName;    ///< The member name.
        CSjsonSchemaType   *pType;      ///< Its type (owned by the model).
        bool                bRequired;  ///< Whether it must be present.
    };

        /// One node of a schema (a JSON Schema sub-schema or an XSD type).
    class CSjsonSchemaType
    {
        public:
                                    CSjsonSchemaType( void ) : bAdditionalProperties( true ), pItems( NULL ), bEnum( false ), bFormat( false ) {}

                /// The JSON types a value may have ("null", "boolean", "integer",
                /// "number", "string", "array", "object"); empty means anything.
            std::set<std::string>   kinds;
                /// Members of an object, in declaration order.
            std::vector<CSjsonSchemaProperty> properties;
                /// Whether names not in properties may appear.
            bool                    bAdditionalProperties;
                /// The type of every array item, or NULL.
            CSjsonSchemaType       *pItems;
                /// Positional array item types (tuple validation).
            std::vector<CSjsonSchemaType *> prefixItems;
                /// Whether the schema enumerates the allowed values (enumValues).
            bool                    bEnum;
            std::vector< std::shared_ptr<CJsonNode> > enumValues;
                /// Inclusive numeric bounds (number nodes), or NULL.
            std::shared_ptr<CJsonNode> pMinimum, pMaximum;
                /// A string format hint such as "date-time", if bFormat.
            bool                    bFormat;
            std::string             strFormat;
                /// Alternative types (anyOf / oneOf / XSD unions).
            std::vector<CSjsonSchemaType *> variants;

                /// Adds a member, or replaces an existing one's type (it keeps
                /// its place and becomes required if either is required).
            void                    addProperty( const std::string &strName, CSjsonSchemaType *pType, bool bRequired );
                /// The member named strName, or NULL.
            const CSjsonSchemaProperty *property( const std::string &strName ) const;
                /// Every directly nested type, in a fixed order.
            void                    children( std::vector<CSjsonSchemaType *> &aChildren ) const;

        private:
            std::map<std::string, size_t> m_mapIndex;  ///< Member name to position.
    };

        /// A loaded schema: types owned by the model, a root and unreferenced extras.
    class CSjsonSchemaModel
    {
        public:
                                    CSjsonSchemaModel( void ) : pRoot( NULL ), m_bFingerprint( false ), m_nFingerprint( 0 ) {}

                /// A new type owned by the model.
            CSjsonSchemaType       *newType( void );

            CSjsonSchemaType       *pRoot;          ///< The type of the whole document.
            std::vector<CSjsonSchemaType *> extras; ///< Defined but not reachable from the root.
            std::string             strSource;      ///< Where the schema came from.

                /// Every type once, breadth first from the root, then the extras.
            std::vector<const CSjsonSchemaType *> walk( void ) const;
                /// All property names, without duplicates: breadth first, and in
                /// an object required members before optional ones.
            std::vector<std::string> fieldNames( void ) const;
                /// All string values listed in enumerations, without duplicates.
            std::vector<std::string> enumStrings( void ) const;
                /// The canonical description (the caller owns it).
            CJsonNode              *canonical( void ) const;
                /// CRC-32 of the canonical description written as Python's
                /// json.dumps(sort_keys=True, separators=(",", ":")).
            unsigned                fingerprint( void ) const;

                /// The compiled form (as Python's SchemaModel.to_dict()); the
                /// caller owns it.
            CJsonNode              *toJson( void ) const;
                /// Rebuilds a model from toJson(); throws CSjsonException if malformed.
            static CSjsonSchemaModel *fromJson( const CJsonNode &data );

        private:
            std::vector< std::unique_ptr<CSjsonSchemaType> > m_aPool;  ///< The types.
            mutable bool            m_bFingerprint; ///< Whether m_nFingerprint is computed.
            mutable unsigned        m_nFingerprint; ///< The cached fingerprint.
    };

        /// Options for reading an XSD (see SjsonSchema::loadXsd()).
    struct CSjsonXsdOptions
    {
        std::string         strAttributePrefix; ///< Prefix for attribute members ("@").
        std::string         strTextKey;         ///< Member for element text ("#text").
        bool                bTypedValues;       ///< Simple types map to JSON numbers and booleans.
                            CSjsonXsdOptions( void ) : strAttributePrefix( "@" ), strTextKey( "#text" ), bTypedValues( true ) {}
    };

    namespace SjsonSchema
    {
            /// JSON text exactly as Python's json.dumps(value, sort_keys=bSortKeys,
            /// separators=(",", ":")) (ASCII only, \\uXXXX escapes).
        std::string         pythonJson( const CJsonNode *pNode, bool bSortKeys = true );

            /// Loads a JSON Schema file (relative $refs to other files are followed).
            /// Throws CSjsonException on error.
        CSjsonSchemaModel  *loadJsonSchema( const std::string &strPath );
            /// Loads a parsed JSON Schema (no relative file references).
        CSjsonSchemaModel  *loadJsonSchemaDocument( const CJsonNode &document );
            /// Loads an XML Schema, mapped to JSON with the xmltodict convention.
        CSjsonSchemaModel  *loadXsd( const std::string &strPath, const CSjsonXsdOptions &options = CSjsonXsdOptions() );
            /// Loads a schema file: ".xsd" as XML Schema, anything else as JSON Schema.
        CSjsonSchemaModel  *loadSchema( const std::string &strPath, const CSjsonXsdOptions &options = CSjsonXsdOptions() );
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSONSCHEMA_H__
