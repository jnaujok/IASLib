/**
 * SJSON schema-driven encoding. See SjsonSchemaCodec.h.
 *
 *  Documents that fit the schema are written without field tags or type
 * codes; only what the schema leaves open:
 *
 * | schema type | encoding |
 * |:------------|:---------|
 * | enum / const | index into the list, in ceil(log2 n) bits |
 * | boolean | 1 bit |
 * | null | nothing |
 * | integer with min and max | value - min in just enough bits |
 * | integer, min >= 0 | NybbleField of value - min |
 * | other integer / number | the number payload (no type code) |
 * | string | the string payload (no type code) |
 * | array with an item type | NybbleField count, then each item |
 * | object | a presence bit per optional member, then the members in |
 * |        | schema order; if extras are allowed, 1 bit and the tagged extras |
 * | anything else | the ordinary tagged encoding |
 *
 *  A type that also allows null gets a leading null bit. In stream mode,
 * strings, numbers, wide integers, wide enumerations, objects, arrays and
 * presence patterns go through the stream cache. This is a port of the
 * Python library's sjson.schema.codec, bit for bit.
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

#include "SjsonSchemaCodec.h"
#include "SjsonCodec.h"
#include "SjsonDictionary.h"
#include "SjsonSchema.h"
#include "SjsonSchemaNumber.h"
#include "SjsonStream.h"
#include "SjsonUtil.h"

#include "ArrayNode.h"
#include "BooleanNode.h"
#include "JsonNode.h"
#include "NullNode.h"
#include "NumberNode.h"
#include "ObjectNode.h"
#include "TextNode.h"

#include <map>
#include <memory>

namespace IASLib
{
    namespace
    {
            // The value does not fit the schema: fall back to the tagged encoding.
        struct NonConforming {};

        enum Plan { PLAN_GENERIC, PLAN_ENUM, PLAN_NULL, PLAN_BOOLEAN, PLAN_INTEGER, PLAN_NUMBER, PLAN_STRING, PLAN_ARRAY, PLAN_OBJECT };

        const size_t CACHED_ENUM_SIZE = 31;
        const size_t CACHED_PRESENCE_BITS = 4;

        std::string fieldName( const CObjectNode &object, int n )
        {
            CString strName = object.fieldName( n );
            return std::string( (const char *)strName, strName.GetLength() );
        }

            // Python's codec _same(): types must match (bool, int, float, str,
            // list, dict), values compare with == (so 0.0 equals -0.0) and
            // dictionaries compare as key sets.
        bool enumSame( const CJsonNode *a, const CJsonNode *b )
        {
            SjsonKind eKind = SjsonCodec::kindOf( a );
            if ( eKind != SjsonCodec::kindOf( b ) )
                return false;
            switch ( eKind )
            {
                case SJSON_KIND_NULL: return true;
                case SJSON_KIND_BOOL: return a->booleanValue() == b->booleanValue();
                case SJSON_KIND_INT: return SjsonCodec::integerText( *a ) == SjsonCodec::integerText( *b );
                case SJSON_KIND_FLOAT: return a->doubleValue() == b->doubleValue();
                case SJSON_KIND_STRING: return SjsonCodec::stringText( *a ) == SjsonCodec::stringText( *b );
                case SJSON_KIND_ARRAY:
                    if ( a->size() != b->size() )
                        return false;
                    for ( int n = 0; n < a->size(); n++ )
                        if ( ! enumSame( a->get( n ), b->get( n ) ) )
                            return false;
                    return true;
                case SJSON_KIND_OBJECT:
                {
                    const CObjectNode &x = static_cast<const CObjectNode &>( *a ), &y = static_cast<const CObjectNode &>( *b );
                    if ( x.size() != y.size() )
                        return false;
                    for ( int n = 0; n < x.size(); n++ )
                    {
                        const CJsonNode *pOther = y.get( fieldName( x, n ).c_str() );
                        bool bFound = false;
                        for ( int k = 0; ! bFound && k < y.size(); k++ )
                            if ( fieldName( y, k ) == fieldName( x, n ) )
                            {
                                pOther = y.fieldValue( k );
                                bFound = true;
                            }
                        if ( ! bFound || ! enumSame( x.fieldValue( n ), pOther ) )
                            return false;
                    }
                    return true;
                }
            }
            return false;
        }

        unsigned bitLength( unsigned long long u )
        {
            unsigned n = 0;
            for ( ; u; u >>= 1 )
                n++;
            return n;
        }
    }

    class CSjsonSchemaCodec::Impl
    {
        public:
            Impl( const CSjsonSchemaModel &model, CSjsonDictionary &dictionary ) : m_model( model ), m_dictionary( dictionary ) {}

            void encode( const CJsonNode *pValue, const CSjsonSchemaType &schema, CSjsonBitWriter &out, const std::string &strKey );
            CJsonNode *decode( CSjsonBitReader &in, const CSjsonSchemaType &schema, const std::string &strKey );
            const CSjsonSchemaModel &model( void ) const { return m_model; }

        private:
            std::pair<Plan, bool> plan( const CSjsonSchemaType &schema );
            bool cachedPlan( Plan ePlan, const CSjsonSchemaType &schema );
            void generic( const CJsonNode *pValue, const std::string &strKey, CSjsonBitWriter &out );
            CJsonNode *genericValue( CSjsonBitReader &in, const std::string &strKey );
            void encodeEnum( const CJsonNode *pValue, const CSjsonSchemaType &schema, CSjsonBitWriter &out );
            void checkScalar( const CJsonNode *pValue, const CSjsonSchemaType &schema, Plan ePlan );
            void plainScalar( const CJsonNode &value, const CSjsonSchemaType &schema, Plan ePlan, CSjsonBitWriter &out );
            CJsonNode *decodeScalar( CSjsonBitReader &in, const CSjsonSchemaType &schema, Plan ePlan );
            CJsonNode *decodeInteger( CSjsonBitReader &in, const CSjsonSchemaType &schema );
            CJsonNode *canonical( const CJsonNode *pValue, const CSjsonSchemaType &schema );
            void encodeArray( const CJsonNode &value, const CSjsonSchemaType &schema, CSjsonBitWriter &out, const std::string &strKey );
            void encodeObject( const CJsonNode &value, const CSjsonSchemaType &schema, CSjsonBitWriter &out, const std::string &strKey );
            CJsonNode *decodeArray( CSjsonBitReader &in, const CSjsonSchemaType &schema, const std::string &strKey );
            CJsonNode *decodeObject( CSjsonBitReader &in, const CSjsonSchemaType &schema, const std::string &strKey );

            const CSjsonSchemaModel &m_model;
            CSjsonDictionary &m_dictionary;
            std::map<const CSjsonSchemaType *, std::pair<Plan, bool> > m_mapPlans;
    };

    std::pair<Plan, bool> CSjsonSchemaCodec::Impl::plan( const CSjsonSchemaType &schema )
    {
        std::map<const CSjsonSchemaType *, std::pair<Plan, bool> >::iterator it = m_mapPlans.find( &schema );
        if ( it != m_mapPlans.end() )
            return it->second;
        std::pair<Plan, bool> result( PLAN_GENERIC, false );
        if ( schema.variants.empty() && schema.prefixItems.empty() )
        {
            if ( schema.bEnum && ! schema.enumValues.empty() )
                result = std::make_pair( PLAN_ENUM, false );
            else
            {
                std::set<std::string> kinds = schema.kinds;
                bool bNullable = kinds.erase( "null" ) > 0;
                if ( kinds.size() == 2 && kinds.count( "integer" ) && kinds.count( "number" ) )
                {
                    kinds.clear();
                    kinds.insert( "number" );
                }
                if ( kinds.size() == 1 )
                {
                    const std::string &strKind = *kinds.begin();
                    Plan ePlan = strKind == "boolean" ? PLAN_BOOLEAN : strKind == "integer" ? PLAN_INTEGER : strKind == "number" ? PLAN_NUMBER :
                                 strKind == "string" ? PLAN_STRING : strKind == "array" ? PLAN_ARRAY : PLAN_OBJECT;
                    result = std::make_pair( ePlan, bNullable );
                }
                else if ( kinds.empty() && bNullable )
                    result = std::make_pair( PLAN_NULL, false );
            }
        }
        m_mapPlans[ &schema ] = result;
        return result;
    }

    bool CSjsonSchemaCodec::Impl::cachedPlan( Plan ePlan, const CSjsonSchemaType &schema )
    {
        if ( m_dictionary.stream() == NULL )
            return false;
        if ( ePlan == PLAN_STRING || ePlan == PLAN_NUMBER )
            return true;
        if ( ePlan == PLAN_INTEGER )
        {
            SjsonNumber low, high;
            if ( ! schema.pMinimum || ! schema.pMaximum || ! SjsonNumber::from( *schema.pMinimum, low ) || ! SjsonNumber::from( *schema.pMaximum, high ) )
                return true;
            return SjsonNumber::spanBits( low, high ) > 6;
        }
        return false;
    }

    void CSjsonSchemaCodec::Impl::generic( const CJsonNode *pValue, const std::string &strKey, CSjsonBitWriter &out )
    {
        CSjsonStreamState *pStream = m_dictionary.stream();
        std::vector<std::string> aSaved;
        if ( pStream )
        {
            aSaved = pStream->path();
            pStream->path().assign( 1, strKey );
        }
        SjsonCodec::encodeTagged( pValue, m_dictionary, out );
        if ( pStream )
            pStream->path() = aSaved;
    }

    CJsonNode *CSjsonSchemaCodec::Impl::genericValue( CSjsonBitReader &in, const std::string &strKey )
    {
        CSjsonStreamState *pStream = m_dictionary.stream();
        std::vector<std::string> aSaved;
        if ( pStream )
        {
            aSaved = pStream->path();
            pStream->path().assign( 1, strKey );
        }
        CJsonNode *pValue = SjsonCodec::decodeTagged( in, m_dictionary );
        if ( pStream )
            pStream->path() = aSaved;
        return pValue;
    }

    void CSjsonSchemaCodec::Impl::encodeEnum( const CJsonNode *pValue, const CSjsonSchemaType &schema, CSjsonBitWriter &out )
    {
        const std::vector< std::shared_ptr<CJsonNode> > &options = schema.enumValues;
        for ( size_t n = 0; n < options.size(); n++ )
        {
            if ( enumSame( options[ n ].get(), pValue ) )
            {
                unsigned nWidth = bitLength( options.size() - 1 );
                if ( nWidth )
                    out.writeBits( n, nWidth );
                return;
            }
        }
        throw NonConforming();
    }

    void CSjsonSchemaCodec::Impl::checkScalar( const CJsonNode *pValue, const CSjsonSchemaType &schema, Plan ePlan )
    {
        SjsonKind eKind = SjsonCodec::kindOf( pValue );
        if ( ePlan == PLAN_STRING )
        {
            if ( eKind != SJSON_KIND_STRING )
                throw NonConforming();
            return;
        }
        if ( ePlan == PLAN_NUMBER )
        {
            if ( eKind != SJSON_KIND_INT && eKind != SJSON_KIND_FLOAT )
                throw NonConforming();
            return;
        }
        if ( eKind != SJSON_KIND_INT )
            throw NonConforming();
        SjsonNumber value = SjsonNumber::integer( SjsonCodec::integerText( *pValue ) ), low, high;
        bool bLow = schema.pMinimum && SjsonNumber::from( *schema.pMinimum, low );
        bool bHigh = schema.pMaximum && SjsonNumber::from( *schema.pMaximum, high );
        if ( ( bLow && SjsonNumber::compare( value, low ) < 0 ) || ( bHigh && SjsonNumber::compare( value, high ) > 0 ) )
            throw NonConforming();
        if ( bLow && SjsonNumber::compare( low, SjsonNumber::integer( "0" ) ) >= 0 && ! ( bHigh && SjsonNumber::spanBits( low, high ) <= 32 ) )
        {
            if ( SjsonNumber::compare( SjsonNumber::integer( SjsonUtil::decimalSubtract( value.strInt, low.bInt ? low.strInt : SjsonNumber::decimalOf( low.d ) ) ),
                                       SjsonNumber::integer( "4294967295" ) ) > 0 )
                throw NonConforming();
        }
    }

        // int( value - low ) as decimal text.
    static std::string offset( const SjsonNumber &value, const SjsonNumber &low )
    {
        if ( low.bInt )
            return SjsonUtil::decimalSubtract( value.strInt, low.strInt );
        return SjsonNumber::decimalOf( std::trunc( value.value() - low.d ) );
    }

    void CSjsonSchemaCodec::Impl::plainScalar( const CJsonNode &value, const CSjsonSchemaType &schema, Plan ePlan, CSjsonBitWriter &out )
    {
        if ( ePlan == PLAN_STRING )
        {
            SjsonCodec::encodeString( SjsonCodec::stringText( value ), &m_dictionary, out );
            return;
        }
        if ( ePlan == PLAN_NUMBER )
        {
            SjsonCodec::encodeNumber( value, out );
            return;
        }
        SjsonNumber number = SjsonNumber::integer( SjsonCodec::integerText( value ) ), low, high;
        bool bLow = schema.pMinimum && SjsonNumber::from( *schema.pMinimum, low );
        bool bHigh = schema.pMaximum && SjsonNumber::from( *schema.pMaximum, high );
        if ( bLow && bHigh )
        {
            unsigned nWidth = SjsonNumber::spanBits( low, high );
            if ( nWidth <= 32 )
            {
                if ( nWidth )
                    out.writeBits( strtoull( offset( number, low ).c_str(), NULL, 10 ), nWidth );
                return;
            }
        }
        if ( bLow && SjsonNumber::compare( low, SjsonNumber::integer( "0" ) ) >= 0 )
        {
            out.writeNybble( strtoull( offset( number, low ).c_str(), NULL, 10 ) );
            return;
        }
        SjsonCodec::encodeInteger( number.strInt, out );
    }

    CJsonNode *CSjsonSchemaCodec::Impl::decodeInteger( CSjsonBitReader &in, const CSjsonSchemaType &schema )
    {
        SjsonNumber low, high;
        bool bLow = schema.pMinimum && SjsonNumber::from( *schema.pMinimum, low );
        bool bHigh = schema.pMaximum && SjsonNumber::from( *schema.pMaximum, high );
        unsigned long long ullOffset = 0;
        bool bOffset = false;
        if ( bLow && bHigh )
        {
            unsigned nWidth = SjsonNumber::spanBits( low, high );
            if ( nWidth <= 32 )
            {
                ullOffset = in.readBits( nWidth );
                bOffset = true;
            }
        }
        if ( ! bOffset && bLow && SjsonNumber::compare( low, SjsonNumber::integer( "0" ) ) >= 0 )
        {
            ullOffset = in.readNybble();
            bOffset = true;
        }
        if ( bOffset )
        {
            if ( low.bInt )
                return SjsonCodec::makeInteger( SjsonUtil::decimalAdd( low.strInt, std::to_string( ullOffset ) ) );
            return SjsonCodec::makeInteger( SjsonNumber::decimalOf( std::trunc( low.d + (double)ullOffset ) ) );
        }
        std::unique_ptr<CJsonNode> pValue( SjsonCodec::decodeNumber( in ) );
        if ( ! pValue->isIntegralNumber() )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "expected an integer" );
        return pValue.release();
    }

    CJsonNode *CSjsonSchemaCodec::Impl::decodeScalar( CSjsonBitReader &in, const CSjsonSchemaType &schema, Plan ePlan )
    {
        if ( ePlan == PLAN_NUMBER )
            return SjsonCodec::decodeNumber( in );
        if ( ePlan == PLAN_STRING )
            return new CTextNode( SjsonCodec::decodeString( in, &m_dictionary ) );
        return decodeInteger( in, schema );
    }

    CJsonNode *CSjsonSchemaCodec::Impl::canonical( const CJsonNode *pValue, const CSjsonSchemaType &schema )
    {
        std::pair<Plan, bool> p = plan( schema );
        if ( p.first == PLAN_OBJECT && pValue && pValue->isObject() )
        {
            const CObjectNode &object = static_cast<const CObjectNode &>( *pValue );
            std::unique_ptr<CObjectNode> pResult( new CObjectNode() );
            for ( size_t n = 0; n < schema.properties.size(); n++ )
            {
                const CSjsonSchemaProperty &prop = schema.properties[ n ];
                for ( int k = 0; k < object.size(); k++ )
                    if ( fieldName( object, k ) == prop.strName )
                    {
                        pResult->set( prop.strName, canonical( object.fieldValue( k ), *prop.pType ) );
                        break;
                    }
            }
            for ( int k = 0; k < object.size(); k++ )
                if ( schema.property( fieldName( object, k ) ) == NULL )
                    pResult->set( fieldName( object, k ), object.fieldValue( k )->clone() );
            return pResult.release();
        }
        if ( p.first == PLAN_ARRAY && pValue && pValue->isArray() && schema.pItems )
        {
            std::unique_ptr<CArrayNode> pResult( new CArrayNode() );
            for ( int n = 0; n < pValue->size(); n++ )
                pResult->add( canonical( pValue->get( n ), *schema.pItems ) );
            return pResult.release();
        }
        return pValue ? pValue->clone() : new CNullNode();
    }

    void CSjsonSchemaCodec::Impl::encode( const CJsonNode *pValue, const CSjsonSchemaType &schema, CSjsonBitWriter &out, const std::string &strKey )
    {
        std::pair<Plan, bool> p = plan( schema );
        SjsonKind eKind = SjsonCodec::kindOf( pValue );
        if ( p.second )
        {
            out.writeBool( eKind == SJSON_KIND_NULL );
            if ( eKind == SJSON_KIND_NULL )
                return;
        }
        CSjsonStreamState *pStream = m_dictionary.stream();
        switch ( p.first )
        {
            case PLAN_GENERIC:
                generic( pValue, strKey, out );
                return;
            case PLAN_ENUM:
                if ( pStream && schema.enumValues.size() > CACHED_ENUM_SIZE )
                {
                    CSjsonBitWriter enumBits;
                    encodeEnum( pValue, schema, enumBits );
                    CNullNode null;
                    pStream->encode( strKey, pValue ? *pValue : null, [&]( CSjsonBitWriter &bits ) { bits.append( enumBits ); }, NULL, out );
                }
                else
                    encodeEnum( pValue, schema, out );
                return;
            case PLAN_NULL:
                if ( eKind != SJSON_KIND_NULL )
                    throw NonConforming();
                return;
            case PLAN_BOOLEAN:
                if ( eKind != SJSON_KIND_BOOL )
                    throw NonConforming();
                out.writeBool( pValue->booleanValue() );
                return;
            case PLAN_INTEGER:
            case PLAN_NUMBER:
            case PLAN_STRING:
                checkScalar( pValue, schema, p.first );
                if ( cachedPlan( p.first, schema ) )
                    pStream->encode( strKey, *pValue, [&]( CSjsonBitWriter &bits ) { plainScalar( *pValue, schema, p.first, bits ); }, &m_dictionary, out );
                else
                    plainScalar( *pValue, schema, p.first, out );
                return;
            case PLAN_ARRAY:
            case PLAN_OBJECT:
            {
                bool bArray = p.first == PLAN_ARRAY;
                if ( eKind != ( bArray ? SJSON_KIND_ARRAY : SJSON_KIND_OBJECT ) )
                    throw NonConforming();
                if ( pStream )
                {
                        // Cache the value as the receiver will decode it, so
                        // both sides' caches hold identical entries.
                    std::unique_ptr<CJsonNode> pCanonical( canonical( pValue, schema ) );
                    const CJsonNode &value = *pCanonical;
                    pStream->encode( strKey + "#", value, [&]( CSjsonBitWriter &bits )
                    {
                        if ( bArray ) encodeArray( value, schema, bits, strKey ); else encodeObject( value, schema, bits, strKey );
                    }, NULL, out );
                }
                else if ( bArray )
                    encodeArray( *pValue, schema, out, strKey );
                else
                    encodeObject( *pValue, schema, out, strKey );
                return;
            }
        }
    }

    void CSjsonSchemaCodec::Impl::encodeArray( const CJsonNode &value, const CSjsonSchemaType &schema, CSjsonBitWriter &out, const std::string &strKey )
    {
        if ( ! value.isArray() || (unsigned long long)value.size() > CSjsonBitWriter::NYBBLE_MAX )
            throw NonConforming();
        out.writeNybble( (unsigned long long)value.size() );
        std::string strItemKey = strKey + "[]";
        for ( int n = 0; n < value.size(); n++ )
        {
            if ( schema.pItems == NULL )
                generic( value.get( n ), strItemKey, out );
            else
                encode( value.get( n ), *schema.pItems, out, strItemKey );
        }
    }

    void CSjsonSchemaCodec::Impl::encodeObject( const CJsonNode &value, const CSjsonSchemaType &schema, CSjsonBitWriter &out, const std::string &strKey )
    {
        if ( ! value.isObject() )
            throw NonConforming();
        const CObjectNode &object = static_cast<const CObjectNode &>( value );
        std::map<std::string, int> mapMembers;
        for ( int n = 0; n < object.size(); n++ )
            mapMembers.insert( std::make_pair( fieldName( object, n ), n ) );
        CSjsonBitWriter presence;
        std::vector<bool> aPresence;
        for ( size_t n = 0; n < schema.properties.size(); n++ )
        {
            const CSjsonSchemaProperty &prop = schema.properties[ n ];
            bool bPresent = mapMembers.count( prop.strName ) != 0;
            if ( prop.bRequired && ! bPresent )
                throw NonConforming();
            if ( ! prop.bRequired )
            {
                presence.writeBool( bPresent );
                aPresence.push_back( bPresent );
            }
        }
        CSjsonStreamState *pStream = m_dictionary.stream();
        if ( pStream && aPresence.size() > CACHED_PRESENCE_BITS )
        {
            std::unique_ptr<CJsonNode> pMask( SjsonCodec::makeInteger( SjsonUtil::decimalFromBits( aPresence ) ) );
            pStream->encode( strKey + "?", *pMask, [&]( CSjsonBitWriter &bits ) { bits.append( presence ); }, NULL, out );
        }
        else
            out.append( presence );
        for ( size_t n = 0; n < schema.properties.size(); n++ )
        {
            const CSjsonSchemaProperty &prop = schema.properties[ n ];
            std::map<std::string, int>::const_iterator it = mapMembers.find( prop.strName );
            if ( it != mapMembers.end() )
                encode( object.fieldValue( it->second ), *prop.pType, out, strKey + "/" + prop.strName );
        }
        std::vector<int> aExtras;
        for ( int n = 0; n < object.size(); n++ )
            if ( schema.property( fieldName( object, n ) ) == NULL )
                aExtras.push_back( n );
        if ( schema.bAdditionalProperties )
        {
            out.writeBool( ! aExtras.empty() );
            if ( ! aExtras.empty() )
            {
                out.writeNybble( aExtras.size() );
                for ( size_t n = 0; n < aExtras.size(); n++ )
                {
                    std::string strName = fieldName( object, aExtras[ n ] );
                    m_dictionary.encodeId( strName, out );
                    generic( object.fieldValue( aExtras[ n ] ), strKey + "/" + strName, out );
                }
            }
        }
        else if ( ! aExtras.empty() )
            throw NonConforming();
    }

    CJsonNode *CSjsonSchemaCodec::Impl::decode( CSjsonBitReader &in, const CSjsonSchemaType &schema, const std::string &strKey )
    {
        std::pair<Plan, bool> p = plan( schema );
        if ( p.second && in.readBool() )
            return new CNullNode();
        CSjsonStreamState *pStream = m_dictionary.stream();
        switch ( p.first )
        {
            case PLAN_GENERIC:
                return genericValue( in, strKey );
            case PLAN_ENUM:
            {
                const std::vector< std::shared_ptr<CJsonNode> > &options = schema.enumValues;
                CSjsonStreamState::PlainDecoder readEnum = [&]( CSjsonBitReader &bits ) -> CJsonNode *
                {
                    unsigned long long nIndex = bits.readBits( bitLength( options.size() - 1 ) );
                    if ( nIndex >= options.size() )
                        throw CSjsonException( SJSON_ERROR_MALFORMED, "enumeration index out of range" );
                    return options[ (size_t)nIndex ] ? options[ (size_t)nIndex ]->clone() : new CNullNode();
                };
                if ( pStream && options.size() > CACHED_ENUM_SIZE )
                    return pStream->decode( in, strKey, readEnum );
                return readEnum( in );
            }
            case PLAN_NULL:
                return new CNullNode();
            case PLAN_BOOLEAN:
                return new CBooleanNode( in.readBool() );
            case PLAN_INTEGER:
            case PLAN_NUMBER:
            case PLAN_STRING:
                if ( cachedPlan( p.first, schema ) )
                    return pStream->decode( in, strKey, [&]( CSjsonBitReader &bits ) { return decodeScalar( bits, schema, p.first ); } );
                return decodeScalar( in, schema, p.first );
            case PLAN_ARRAY:
            case PLAN_OBJECT:
            {
                bool bArray = p.first == PLAN_ARRAY;
                if ( pStream )
                    return pStream->decode( in, strKey + "#", [&]( CSjsonBitReader &bits )
                    {
                        return bArray ? decodeArray( bits, schema, strKey ) : decodeObject( bits, schema, strKey );
                    } );
                return bArray ? decodeArray( in, schema, strKey ) : decodeObject( in, schema, strKey );
            }
        }
        throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid schema plan" );
    }

    CJsonNode *CSjsonSchemaCodec::Impl::decodeArray( CSjsonBitReader &in, const CSjsonSchemaType &schema, const std::string &strKey )
    {
        unsigned long long nCount = in.readNybble();
            // Items usually take at least a bit each, but null, a one-value
            // enum or a fixed integer take none, so allow some more.
        const unsigned long long MAX_ZERO_BIT_ITEMS = 1ULL << 20;
        if ( nCount > in.remaining() && nCount > MAX_ZERO_BIT_ITEMS )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "array longer than the message" );
        std::string strItemKey = strKey + "[]";
        std::unique_ptr<CArrayNode> pArray( new CArrayNode() );
        for ( unsigned long long n = 0; n < nCount; n++ )
            pArray->add( schema.pItems == NULL ? genericValue( in, strItemKey ) : decode( in, *schema.pItems, strItemKey ) );
        return pArray.release();
    }

    CJsonNode *CSjsonSchemaCodec::Impl::decodeObject( CSjsonBitReader &in, const CSjsonSchemaType &schema, const std::string &strKey )
    {
        std::vector<size_t> aOptional;
        for ( size_t n = 0; n < schema.properties.size(); n++ )
            if ( ! schema.properties[ n ].bRequired )
                aOptional.push_back( n );
        std::vector<bool> aPresent( schema.properties.size(), true );
        CSjsonStreamState *pStream = m_dictionary.stream();
        if ( pStream && aOptional.size() > CACHED_PRESENCE_BITS )
        {
            size_t nCount = aOptional.size();
            std::unique_ptr<CJsonNode> pMask( pStream->decode( in, strKey + "?", [&]( CSjsonBitReader &bits ) -> CJsonNode *
            {
                std::vector<bool> aBits;
                for ( size_t n = 0; n < nCount; n++ )
                    aBits.push_back( bits.readBool() );
                return SjsonCodec::makeInteger( SjsonUtil::decimalFromBits( aBits ) );
            } ) );
            std::vector<bool> aBits;
            if ( ! pMask->isIntegralNumber() || ! SjsonUtil::decimalToBits( SjsonCodec::integerText( *pMask ), nCount, aBits ) )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid presence mask" );
            for ( size_t n = 0; n < nCount; n++ )
                aPresent[ aOptional[ n ] ] = aBits[ n ];
        }
        else
            for ( size_t n = 0; n < aOptional.size(); n++ )
                aPresent[ aOptional[ n ] ] = in.readBool();
        std::unique_ptr<CObjectNode> pObject( new CObjectNode() );
        for ( size_t n = 0; n < schema.properties.size(); n++ )
            if ( aPresent[ n ] )
                pObject->set( schema.properties[ n ].strName, decode( in, *schema.properties[ n ].pType, strKey + "/" + schema.properties[ n ].strName ) );
        if ( schema.bAdditionalProperties && in.readBool() )
        {
            unsigned long long nCount = in.readNybble();
            for ( unsigned long long n = 0; n < nCount; n++ )
            {
                std::string strName = m_dictionary.decodeId( in );
                pObject->set( strName, genericValue( in, strKey + "/" + strName ) );
            }
        }
        return pObject.release();
    }

    // ---- public interface --------------------------------------------------

    CSjsonSchemaCodec::CSjsonSchemaCodec( const CSjsonSchemaModel &model, CSjsonDictionary &dictionary )
        : m_pImpl( new Impl( model, dictionary ) ), m_dictionary( dictionary )
    {
    }

    CSjsonSchemaCodec::~CSjsonSchemaCodec( void )
    {
        delete m_pImpl;
    }

    bool CSjsonSchemaCodec::encode( const CJsonNode &document, CSjsonBitWriter &out )
    {
        try
        {
            CSjsonBitWriter bits;
            m_pImpl->encode( &document, *m_pImpl->model().pRoot, bits, "" );
            out.append( bits );
            return true;
        }
        catch ( const NonConforming & )
        {
            return false;
        }
    }

    CJsonNode *CSjsonSchemaCodec::decode( CSjsonBitReader &in )
    {
        return m_pImpl->decode( in, *m_pImpl->model().pRoot, "" );
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
