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
#include "SjsonStream.h"

namespace IASLib
{
    CSjsonDictionary::CSjsonDictionary( bool bWithValues )
        : m_nLastIndex( 0 ), m_nBaseSize( 0 ), m_pValues( bWithValues ? new CSjsonDictionary( false ) : NULL ),
          m_bFingerprint( false ), m_nFingerprint( 0 ), m_pStream( NULL ), m_pText( NULL )
    {
    }

    CSjsonDictionary::~CSjsonDictionary( void )
    {
        delete m_pValues;
    }

    CSjsonDictionary *CSjsonDictionary::fromNames( const std::vector<std::string> &aNames, const std::vector<std::string> &aValues )
    {
        CSjsonDictionary *pDictionary = new CSjsonDictionary( true );
        for ( size_t n = 0; n < aNames.size(); n++ )
            pDictionary->add( aNames[ n ] );
        pDictionary->m_nBaseSize = pDictionary->m_nLastIndex;
        for ( size_t n = 0; n < aValues.size(); n++ )
            pDictionary->m_pValues->add( aValues[ n ] );
        pDictionary->m_pValues->m_nBaseSize = pDictionary->m_pValues->m_nLastIndex;
        return pDictionary;
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
        return m_aNames[ (size_t)nId - 1 ];
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
        if ( m_aNames.size() < nId )
        {
            m_aNames.resize( (size_t)nId );
            m_aAssigned.resize( (size_t)nId, false );
        }
        m_aNames[ (size_t)nId - 1 ] = strTag;
        m_aAssigned[ (size_t)nId - 1 ] = true;
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
        CSjsonDictionary *pCopy = fromNames( baseNames(), m_pValues ? m_pValues->baseNames() : std::vector<std::string>() );
        pCopy->m_pSchema = m_pSchema;
        pCopy->m_strTextDictionary = m_strTextDictionary;
        pCopy->m_bFingerprint = m_bFingerprint;
        pCopy->m_nFingerprint = m_nFingerprint;
        return pCopy;
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
