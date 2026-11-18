/**
 * SJSON dictionary. See SjsonDictionary.h.
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

#include "SjsonDictionary.h"
#include "SjsonSchema.h"
#include "SjsonStream.h"
#include "SjsonUtil.h"

#include "ArrayNode.h"
#include "JsonNode.h"
#include "JsonParser.h"
#include "ObjectNode.h"

#include <fstream>
#ifdef IASLIB_SJSON_ZSTD__
#include <zstd.h>
#endif
#include <sstream>

namespace IASLib
{
    CSjsonDictionary::CSjsonDictionary( bool bWithValues )
        : m_nLastIndex( 0 ), m_nBaseSize( 0 ), m_pValues( bWithValues ? new CSjsonDictionary( false ) : NULL ),
          m_bFingerprint( false ), m_nFingerprint( 0 ), m_pStream( NULL ), m_pText( NULL ),
          m_pCDict( NULL ), m_nCDictLevel( 0 ), m_pDDict( NULL )
    {
    }

    CSjsonDictionary::~CSjsonDictionary( void )
    {
        releaseZstd();
        delete m_pValues;
    }

    void CSjsonDictionary::releaseZstd( void )
    {
#ifdef IASLIB_SJSON_ZSTD__
        ZSTD_freeCDict( (ZSTD_CDict *)m_pCDict );
        ZSTD_freeDDict( (ZSTD_DDict *)m_pDDict );
#endif
        m_pCDict = m_pDDict = NULL;
    }

    const void *CSjsonDictionary::zstdCompressDictionary( int nLevel ) const
    {
#ifdef IASLIB_SJSON_ZSTD__
        if ( m_strTextDictionary.empty() )
            return NULL;
        if ( m_pCDict == NULL || m_nCDictLevel != nLevel )
        {
            ZSTD_freeCDict( (ZSTD_CDict *)m_pCDict );
            m_pCDict = ZSTD_createCDict( m_strTextDictionary.data(), m_strTextDictionary.size(), nLevel );
            m_nCDictLevel = nLevel;
        }
        return m_pCDict;
#else
        (void)nLevel;
        return NULL;
#endif
    }

    const void *CSjsonDictionary::zstdDecompressDictionary( void ) const
    {
#ifdef IASLIB_SJSON_ZSTD__
        if ( m_strTextDictionary.empty() )
            return NULL;
        if ( m_pDDict == NULL )
            m_pDDict = ZSTD_createDDict( m_strTextDictionary.data(), m_strTextDictionary.size() );
        return m_pDDict;
#else
        return NULL;
#endif
    }

    CSjsonDictionary *CSjsonDictionary::fromNames( const std::vector<std::string> &aNames, const std::vector<std::string> &aValues,
                                                   const std::shared_ptr<const CSjsonSchemaModel> &pSchema, const std::string &strTextDictionary )
    {
        CSjsonDictionary *pDictionary = new CSjsonDictionary( true );
        for ( size_t n = 0; n < aNames.size(); n++ )
            pDictionary->add( aNames[ n ] );
        pDictionary->m_nBaseSize = pDictionary->m_nLastIndex;
        for ( size_t n = 0; n < aValues.size(); n++ )
            pDictionary->m_pValues->add( aValues[ n ] );
        pDictionary->m_pValues->m_nBaseSize = pDictionary->m_pValues->m_nLastIndex;
        pDictionary->m_pSchema = pSchema;
        pDictionary->m_strTextDictionary = strTextDictionary;
        pDictionary->setFingerprint( pDictionary->computeFingerprint() );
        return pDictionary;
    }

    CSjsonDictionary *CSjsonDictionary::fromSchema( const std::shared_ptr<const CSjsonSchemaModel> &pSchema )
    {
        return fromNames( pSchema->fieldNames(), pSchema->enumStrings(), pSchema );
    }

    unsigned CSjsonDictionary::computeFingerprint( void ) const
    {
        std::vector<std::string> aNames = baseNames(), aValues = m_pValues ? m_pValues->baseNames() : std::vector<std::string>();
        if ( m_pSchema && m_strTextDictionary.empty() && aNames == m_pSchema->fieldNames() && aValues == m_pSchema->enumStrings() )
            return m_pSchema->fingerprint();
        CObjectNode content;
        CArrayNode *pTags = content.putArray( "tags" );
        for ( size_t n = 0; n < aNames.size(); n++ )
            pTags->add( aNames[ n ] );
        CArrayNode *pValues = content.putArray( "values" );
        for ( size_t n = 0; n < aValues.size(); n++ )
            pValues->add( aValues[ n ] );
        if ( m_pSchema )
            content.set( "schema", m_pSchema->canonical() );
        else
            content.putNull( "schema" );
        if ( ! m_strTextDictionary.empty() )
            content.put( "text", (long long)SjsonUtil::crc32( (const unsigned char *)m_strTextDictionary.data(), m_strTextDictionary.size() ) );
        std::string strText = SjsonSchema::pythonJson( &content, true );
        return SjsonUtil::crc32( (const unsigned char *)strText.data(), strText.size() );
    }

    CJsonNode *CSjsonDictionary::toJson( void ) const
    {
        CObjectNode *pOut = new CObjectNode();
        pOut->put( "format", "sjson-dictionary" );
        pOut->put( "version", 1 );
        pOut->put( "fingerprint", (long long)computeFingerprint() );
        CArrayNode *pTags = pOut->putArray( "tags" );
        std::vector<std::string> aNames = baseNames();
        for ( size_t n = 0; n < aNames.size(); n++ )
            pTags->add( aNames[ n ] );
        CArrayNode *pValues = pOut->putArray( "values" );
        std::vector<std::string> aValues = m_pValues ? m_pValues->baseNames() : std::vector<std::string>();
        for ( size_t n = 0; n < aValues.size(); n++ )
            pValues->add( aValues[ n ] );
        if ( m_pSchema )
            pOut->set( "schema", m_pSchema->toJson() );
        else
            pOut->putNull( "schema" );
        if ( m_strTextDictionary.empty() )
            pOut->putNull( "text_dictionary" );
        else
            pOut->put( "text_dictionary", SjsonUtil::base64Encode( m_strTextDictionary ) );
        return pOut;
    }

    CSjsonDictionary *CSjsonDictionary::fromJson( const CJsonNode &data )
    {
        std::unique_ptr<CObjectNode> pWrapped;
        const CJsonNode *pData = &data;
        if ( data.isArray() )
        {
            pWrapped.reset( new CObjectNode() );
            pWrapped->set( "tags", data.clone() );
            pData = pWrapped.get();
        }
        if ( ! pData->isObject() )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "a compiled dictionary must be an object or a list" );
        const CJsonNode &format = pData->path( "format" );
        if ( ! format.isMissingNode() && ! ( format.isTextual() && format.textString() == "sjson-dictionary" ) )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "not an SJSON dictionary" );
        const CJsonNode &version = pData->path( "version" );
        if ( ! version.isMissingNode() && ! ( version.isIntegralNumber() && version.longValue() == 1 ) )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "unsupported dictionary version" );
        std::vector<std::string> aLists[ 2 ];
        const char *apszKeys[ 2 ] = { "tags", "values" };
        for ( int k = 0; k < 2; k++ )
        {
            const CJsonNode &list = pData->path( apszKeys[ k ] );
            if ( list.isMissingNode() )
                continue;
            if ( ! list.isArray() )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "dictionary tags and values must be string lists" );
            std::set<std::string> setSeen;
            for ( int n = 0; n < list.size(); n++ )
            {
                if ( ! list.path( n ).isTextual() )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "dictionary tags and values must be string lists" );
                if ( ! setSeen.insert( list.path( n ).textString() ).second )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "dictionary tags and values must be unique" );
                aLists[ k ].push_back( list.path( n ).textString() );
            }
        }
        std::shared_ptr<const CSjsonSchemaModel> pSchema;
        const CJsonNode &schema = pData->path( "schema" );
        if ( ! schema.isMissingNode() && ! schema.isNull() )
            pSchema.reset( CSjsonSchemaModel::fromJson( schema ) );
        std::string strText;
        const CJsonNode &text = pData->path( "text_dictionary" );
        if ( text.isTextual() && ! SjsonUtil::base64Decode( text.textString(), strText ) )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "malformed text dictionary" );
        std::unique_ptr<CSjsonDictionary> pDictionary( fromNames( aLists[ 0 ], aLists[ 1 ], pSchema, strText ) );
        const CJsonNode &recorded = pData->path( "fingerprint" );
        if ( recorded.isNumber() && ( ! recorded.isIntegralNumber() || (unsigned long long)recorded.longValue() != pDictionary->fingerprint() ) )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "dictionary fingerprint mismatch: the file was edited or corrupted" );
        return pDictionary.release();
    }

    CSjsonDictionary *CSjsonDictionary::load( const std::string &strPath )
    {
        std::ifstream file( strPath.c_str(), std::ios::binary );
        if ( ! file )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "cannot read dictionary " + strPath );
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::unique_ptr<CJsonNode> pData( CJsonParser::parse( buffer.str() ) );
        if ( ! pData )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "cannot parse dictionary " + strPath );
        return fromJson( *pData );
    }

    bool CSjsonDictionary::save( const std::string &strPath ) const
    {
        std::unique_ptr<CJsonNode> pData( toJson() );
        std::ofstream file( strPath.c_str(), std::ios::binary );
        file << pData->toJson( 1 );
        return (bool)file;
    }

    unsigned long long CSjsonDictionary::get( const std::string &strTag ) const
    {
        std::unordered_map<std::string, unsigned long long>::const_iterator it = m_mapIds.find( strTag );
        if ( it == m_mapIds.end() )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid tag name: " + strTag );
        return it->second;
    }

    const std::string &CSjsonDictionary::lookup( unsigned long long nId ) const
    {
        if ( ! hasId( nId ) )
            throw CSjsonException( SJSON_ERROR_UNKNOWN_TAG, "unknown tag id: " + std::to_string( nId ) );
        if ( nId <= m_aNames.size() )
            return m_aNames[ (size_t)nId - 1 ];
        return m_mapSparse.find( nId )->second;
    }

    unsigned long long CSjsonDictionary::add( const std::string &strTag )
    {
        std::unordered_map<std::string, unsigned long long>::const_iterator it = m_mapIds.find( strTag );
        if ( it != m_mapIds.end() )
            return it->second;
        set( strTag, m_nLastIndex + 1 );
        return m_nLastIndex;
    }

    void CSjsonDictionary::set( const std::string &strTag, unsigned long long nId )
    {
        if ( nId == 0 || hasTag( strTag ) || hasId( nId ) )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "tag/index collision: " + strTag );
        if ( nId > 0xFFFFFFFFULL )
            throw CSjsonException( SJSON_ERROR_MALFORMED, "tag id out of range" );
        const unsigned long long MAX_DENSE_GAP = 4096;
        if ( nId > m_aNames.size() + MAX_DENSE_GAP )
            m_mapSparse[ nId ] = strTag;    // a far-off id: do not allocate up to it
        else
        {
            if ( m_aNames.size() < nId )
            {
                m_aNames.resize( (size_t)nId );
                m_aAssigned.resize( (size_t)nId, false );
                    // Sparse ids the dense range now covers move into it.
                while ( ! m_mapSparse.empty() && m_mapSparse.begin()->first <= m_aNames.size() )
                {
                    m_aNames[ (size_t)m_mapSparse.begin()->first - 1 ] = m_mapSparse.begin()->second;
                    m_aAssigned[ (size_t)m_mapSparse.begin()->first - 1 ] = true;
                    m_mapSparse.erase( m_mapSparse.begin() );
                }
            }
            m_aNames[ (size_t)nId - 1 ] = strTag;
            m_aAssigned[ (size_t)nId - 1 ] = true;
        }
        m_mapIds[ strTag ] = nId;
        if ( nId > m_nLastIndex )
            m_nLastIndex = nId;
    }

    std::vector< std::pair<unsigned long long, std::string> > CSjsonDictionary::entriesAfter( unsigned long long nLastId ) const
    {
        std::vector< std::pair<unsigned long long, std::string> > aOut;
        for ( size_t n = (size_t)nLastId; n < m_aNames.size(); n++ )
            if ( m_aAssigned[ n ] )
                aOut.push_back( std::make_pair( (unsigned long long)n + 1, m_aNames[ n ] ) );
        for ( std::map<unsigned long long, std::string>::const_iterator it = m_mapSparse.upper_bound( nLastId ); it != m_mapSparse.end(); ++it )
            aOut.push_back( *it );
        return aOut;
    }

    std::vector<std::string> CSjsonDictionary::baseNames( void ) const
    {
        std::vector<std::string> aOut;
        for ( unsigned long long n = 1; n <= m_nBaseSize; n++ )
            aOut.push_back( lookup( n ) );
        return aOut;
    }

    std::pair<unsigned long long, unsigned long long> CSjsonDictionary::checkpoint( void ) const
    {
        return std::make_pair( m_nLastIndex, m_pValues ? m_pValues->m_nLastIndex : 0 );
    }

    void CSjsonDictionary::rollback( const std::pair<unsigned long long, unsigned long long> &checkpoint )
    {
        CSjsonDictionary *apDictionaries[ 2 ] = { this, m_pValues };
        unsigned long long anLast[ 2 ] = { checkpoint.first, checkpoint.second };
        for ( int k = 0; k < 2; k++ )
        {
            CSjsonDictionary *pDictionary = apDictionaries[ k ];
            if ( pDictionary == NULL )
                continue;
            for ( size_t n = (size_t)anLast[ k ]; n < pDictionary->m_aNames.size(); n++ )
                if ( pDictionary->m_aAssigned[ n ] )
                    pDictionary->m_mapIds.erase( pDictionary->m_aNames[ n ] );
            if ( pDictionary->m_aNames.size() > anLast[ k ] )
            {
                pDictionary->m_aNames.resize( (size_t)anLast[ k ] );
                pDictionary->m_aAssigned.resize( (size_t)anLast[ k ] );
            }
            std::map<unsigned long long, std::string> &sparse = pDictionary->m_mapSparse;
            for ( std::map<unsigned long long, std::string>::iterator it = sparse.upper_bound( anLast[ k ] ); it != sparse.end(); )
            {
                pDictionary->m_mapIds.erase( it->second );
                sparse.erase( it++ );
            }
            pDictionary->m_nLastIndex = anLast[ k ];
        }
    }

    void CSjsonDictionary::resetToBase( void )
    {
        rollback( std::make_pair( m_nBaseSize, m_pValues ? m_pValues->m_nBaseSize : 0 ) );
        m_setSeenUrls.clear();
    }

    void CSjsonDictionary::setStream( CSjsonStreamState *pStream )
    {
        m_pStream = pStream;
        if ( m_pValues )
            m_pValues->m_pStream = pStream;
    }

    void CSjsonDictionary::setText( CSjsonTextChannel *pText )
    {
        m_pText = pText;
        if ( m_pValues )
            m_pValues->m_pText = pText;
    }

    void CSjsonDictionary::encodeId( const std::string &strTag, CSjsonBitWriter &out )
    {
        if ( hasTag( strTag ) )
        {
            out.writeNybble( get( strTag ) );
            return;
        }
        unsigned long long nId = add( strTag );
        out.writeNybble( nId );
        if ( m_pStream != NULL )
        {
                // Stream mode: the new name is defined inline, in the text
                // channel when it has no NUL.
            if ( m_pText != NULL && strTag.find( '\0' ) == std::string::npos )
            {
                m_pText->writeTerminated( strTag );
                out.writeBool( true );
            }
            else
            {
                out.writeBool( false );
                out.writeNybble( strTag.size() );
                out.writeBytes( strTag );
            }
        }
    }

    const std::string &CSjsonDictionary::decodeId( CSjsonBitReader &in )
    {
        unsigned long long nId = in.readNybble();
        if ( m_pStream != NULL && ! hasId( nId ) && nId == m_nLastIndex + 1 )
        {
            std::string strText;
            if ( in.readBool() )
            {
                if ( m_pText == NULL )
                    throw CSjsonException( SJSON_ERROR_UNSUPPORTED, "an inline name in the text channel needs zstd" );
                strText = m_pText->readTerminated();
            }
            else
                strText = in.readBytes( (size_t)in.readNybble() );
            set( strText, nId );
        }
        return lookup( nId );
    }

    CSjsonDictionary *CSjsonDictionary::freshCopy( void ) const
    {
        CSjsonDictionary *pCopy = fromNames( baseNames(), m_pValues ? m_pValues->baseNames() : std::vector<std::string>(), m_pSchema, m_strTextDictionary );
        pCopy->m_bFingerprint = m_bFingerprint;
        pCopy->m_nFingerprint = m_nFingerprint;
        return pCopy;
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
