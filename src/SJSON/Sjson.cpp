/**
 * SJSON packets. See Sjson.h.
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

#include "Sjson.h"
#include "SjsonCodec.h"
#include "SjsonSchemaCodec.h"
#include "SjsonUtil.h"
#include "JsonNode.h"

#include <cstring>

#ifdef IASLIB_SJSON_ZSTD__
#include <zstd.h>
#endif

namespace IASLib
{
    namespace
    {
        const unsigned VERSION_BITS = 4, FLAG_BITS = 4, SEQUENCE_BITS = 8, CACHE_WIDTH_BITS = 3;

            // The CRC covers the zero-padded bytes of the first nBits bits.
        unsigned packetCrc( const unsigned char *pData, size_t nBits )
        {
            std::vector<unsigned char> aBytes( pData, pData + ( nBits + 7 ) / 8 );
            if ( nBits & 7 )
                aBytes.back() &= (unsigned char)( 0xFF << ( 8 - ( nBits & 7 ) ) );
            return SjsonUtil::crc32( aBytes.data(), aBytes.size() );
        }

            // Sets a dictionary's stream state and text channel for one packet.
        struct Coding
        {
            CSjsonDictionary &dictionary;
            Coding( CSjsonDictionary &d, CSjsonStreamState *pStream, CSjsonTextChannel *pText ) : dictionary( d )
            {
                dictionary.setStream( pStream );
                dictionary.setText( pText );
            }
            ~Coding( void )
            {
                dictionary.setStream( NULL );
                dictionary.setText( NULL );
            }
        };

            // Packets carry a text block in stream mode, or when the dictionary
            // has a text dictionary.
        bool hasTextBlock( const CSjsonDictionary &dictionary, const CSjsonStreamState *pStream )
        {
            return pStream != NULL || ! dictionary.textDictionary().empty();
        }

            // The channel strings may be routed to (none without zstd: then
            // only empty text blocks are written).
        CSjsonTextChannel *textChannel( CSjsonDictionary &dictionary, CSjsonStreamState *pStream, CSjsonTextChannel &perPacket )
        {
            if ( ! CSjson::zstdAvailable() || ! hasTextBlock( dictionary, pStream ) )
                return NULL;
            return pStream != NULL ? &pStream->text() : &perPacket;
        }
    }

    CSjson::CSjson( unsigned nSenderId, bool bStateful, unsigned nCacheWidth )
        : m_nSenderId( nSenderId & 0xFFFF ), m_bStateful( bStateful ), m_bIncludeSenderId( ! bStateful ), m_nCacheWidth( nCacheWidth ),
          m_bSchemaEncoding( true ), m_bLz4( true ), m_nTextLevel( TEXT_LEVEL ), m_nTextWindowLog( TEXT_WINDOW_LOG ), m_nSequence( 0 ), m_pTextOut( NULL ), m_pFrameOut( NULL ), m_pFrameIn( NULL ), m_pOwn( new CSjsonDictionary( true ) ), m_eError( SJSON_OK )
    {
        if ( bStateful )
            m_pStream.reset( new CSjsonStreamState( nCacheWidth ) );
    }

    CSjson::~CSjson( void )
    {
        delete m_pOwn;
        for ( std::map<unsigned, CSjsonDictionary *>::iterator it = m_mapDictionaries.begin(); it != m_mapDictionaries.end(); ++it )
            delete it->second;
        for ( std::map<unsigned, CSjsonDictionary *>::iterator it = m_mapKnown.begin(); it != m_mapKnown.end(); ++it )
            delete it->second;
        for ( std::map<const CSjsonDictionary *, CSjsonSchemaCodec *>::iterator it = m_mapCodecs.begin(); it != m_mapCodecs.end(); ++it )
            delete it->second;
#ifdef IASLIB_SJSON_ZSTD__
        ZSTD_freeCCtx( (ZSTD_CCtx *)m_pTextOut );
        ZSTD_freeCCtx( (ZSTD_CCtx *)m_pFrameOut );
        ZSTD_freeDCtx( (ZSTD_DCtx *)m_pFrameIn );
        for ( std::map<unsigned, void *>::iterator it = m_mapTextIn.begin(); it != m_mapTextIn.end(); ++it )
            ZSTD_freeDCtx( (ZSTD_DCtx *)it->second );
#endif
    }

    bool CSjson::zstdAvailable( void )
    {
#ifdef IASLIB_SJSON_ZSTD__
        return true;
#else
        return false;
#endif
    }

    void CSjson::fail( SjsonError eError, const std::string &strText )
    {
        m_eError = eError;
        m_strError = strText;
    }

    void CSjson::setDictionary( const CSjsonDictionary &dictionary )
    {
            // Copy first: dictionary may be the one being replaced.
        CSjsonDictionary *pCopy = dictionary.freshCopy();
        registerDictionary( *pCopy );
        dropCodec( m_pOwn );
        delete m_pOwn;
        m_pOwn = pCopy;
            // A stream started with the old dictionary: start again, so the
            // next packet names the new one.
        if ( m_bStateful )
            resetStream();
    }

    unsigned CSjson::registerDictionary( const CSjsonDictionary &dictionary )
    {
        unsigned nFingerprint = dictionary.fingerprint();
        std::map<unsigned, CSjsonDictionary *>::iterator it = m_mapKnown.find( nFingerprint );
        if ( it != m_mapKnown.end() )
            delete it->second;
        m_mapKnown[ nFingerprint ] = dictionary.freshCopy();
        return nFingerprint;
    }

    CSjsonDictionary &CSjson::dictionary( unsigned nSenderId )
    {
        std::map<unsigned, CSjsonDictionary *>::iterator it = m_mapDictionaries.find( nSenderId );
        if ( it != m_mapDictionaries.end() )
            return *it->second;
        CSjsonDictionary *pDictionary = new CSjsonDictionary( true );
        m_mapDictionaries[ nSenderId ] = pDictionary;
        return *pDictionary;
    }

    unsigned long long CSjson::lastKnownTag( unsigned nSenderId ) const
    {
        std::map<unsigned, CSjsonDictionary *>::const_iterator it = m_mapDictionaries.find( nSenderId );
        return it == m_mapDictionaries.end() ? 0 : it->second->lastIndex();
    }

    unsigned long long CSjson::lastKnownValue( unsigned nSenderId ) const
    {
        std::map<unsigned, CSjsonDictionary *>::const_iterator it = m_mapDictionaries.find( nSenderId );
        return it == m_mapDictionaries.end() || it->second->values() == NULL ? 0 : it->second->values()->lastIndex();
    }

    void CSjson::resetStream( void )
    {
        m_pOwn->resetToBase();
        if ( m_bStateful )
            m_pStream.reset( new CSjsonStreamState( m_nCacheWidth ) );
        m_nSequence = 0;
#ifdef IASLIB_SJSON_ZSTD__
        ZSTD_freeCCtx( (ZSTD_CCtx *)m_pTextOut );
#endif
        m_pTextOut = NULL;
    }

    void CSjson::dropCodec( const CSjsonDictionary *pDictionary )
    {
        std::map<const CSjsonDictionary *, CSjsonSchemaCodec *>::iterator it = m_mapCodecs.find( pDictionary );
        if ( it != m_mapCodecs.end() )
        {
            delete it->second;
            m_mapCodecs.erase( it );
        }
    }

    CSjsonSchemaCodec &CSjson::codec( CSjsonDictionary &dictionary )
    {
        std::map<const CSjsonDictionary *, CSjsonSchemaCodec *>::iterator it = m_mapCodecs.find( &dictionary );
        if ( it != m_mapCodecs.end() )
            return *it->second;
        CSjsonSchemaCodec *pCodec = new CSjsonSchemaCodec( *dictionary.schema(), dictionary );
        m_mapCodecs[ &dictionary ] = pCodec;
        return *pCodec;
    }

    std::string CSjson::compressText( CSjsonDictionary &dictionary, const std::string &strText, bool bStream )
    {
        if ( strText.empty() )
            return std::string();
#ifdef IASLIB_SJSON_ZSTD__
        const std::string &strDict = dictionary.textDictionary();
        if ( ! bStream )
        {
                // One self-contained frame per packet. The context and the
                // digested text dictionary are kept between packets, so a
                // frame does not rebuild the dictionary's level-19 tables.
            if ( m_pFrameOut == NULL )
                m_pFrameOut = ZSTD_createCCtx();
            ZSTD_CCtx *pContext = (ZSTD_CCtx *)m_pFrameOut;
            ZSTD_CCtx_reset( pContext, ZSTD_reset_session_and_parameters );
            ZSTD_CCtx_setParameter( pContext, ZSTD_c_compressionLevel, m_nTextLevel );
            ZSTD_CCtx_setParameter( pContext, ZSTD_c_checksumFlag, 0 );
            if ( ! strDict.empty() )
                ZSTD_CCtx_refCDict( pContext, (const ZSTD_CDict *)dictionary.zstdCompressDictionary( m_nTextLevel ) );
            std::string strOut( ZSTD_compressBound( strText.size() ), '\0' );
            size_t nSize = ZSTD_compress2( pContext, &strOut[ 0 ], strOut.size(), strText.data(), strText.size() );
            ZSTD_CCtx_refCDict( pContext, NULL );       // the dictionary may be replaced before the next frame
            if ( ZSTD_isError( nSize ) )
                throw CSjsonException( SJSON_ERROR_MALFORMED, ZSTD_getErrorName( nSize ) );
            strOut.resize( nSize );
            return strOut;
        }
        if ( m_pTextOut == NULL )
        {
            ZSTD_CCtx *pContext = ZSTD_createCCtx();
            ZSTD_CCtx_setParameter( pContext, ZSTD_c_compressionLevel, m_nTextLevel );
            if ( m_nTextWindowLog > 0 && m_nTextWindowLog < 23 )
            {
                    // A smaller window, with the match tables to suit it.
                ZSTD_CCtx_setParameter( pContext, ZSTD_c_windowLog, m_nTextWindowLog );
                ZSTD_CCtx_setParameter( pContext, ZSTD_c_hashLog, m_nTextWindowLog + 1 < 22 ? m_nTextWindowLog + 1 : 22 );
                ZSTD_CCtx_setParameter( pContext, ZSTD_c_chainLog, m_nTextWindowLog + 1 < 24 ? m_nTextWindowLog + 1 : 24 );
            }
            if ( ! strDict.empty() )
                ZSTD_CCtx_loadDictionary( pContext, strDict.data(), strDict.size() );
            m_pTextOut = pContext;
        }
        ZSTD_CCtx *pContext = (ZSTD_CCtx *)m_pTextOut;
        std::string strOut;
        std::vector<char> aBuffer( ZSTD_CStreamOutSize() );
        ZSTD_inBuffer input = { strText.data(), strText.size(), 0 };
        for ( ;; )
        {
            ZSTD_outBuffer output = { aBuffer.data(), aBuffer.size(), 0 };
            size_t nRemaining = ZSTD_compressStream2( pContext, &output, &input, ZSTD_e_flush );
            if ( ZSTD_isError( nRemaining ) )
                throw CSjsonException( SJSON_ERROR_MALFORMED, ZSTD_getErrorName( nRemaining ) );
            strOut.append( aBuffer.data(), output.pos );
            if ( nRemaining == 0 && input.pos == input.size )
                break;
        }
        return strOut;
#else
        (void)dictionary;
        (void)bStream;
        throw CSjsonException( SJSON_ERROR_UNSUPPORTED, "the text channel needs zstd" );
#endif
    }

    std::string CSjson::readText( const Header &header, CSjsonBitReader &in, CSjsonDictionary &dictionary )
    {
        std::string strBlob = in.readBytes( (size_t)in.readNybble() );
        if ( ! header.bSequence )
        {
            if ( strBlob.empty() )
                return std::string();
#ifdef IASLIB_SJSON_ZSTD__
            unsigned long long ullSize = ZSTD_getFrameContentSize( strBlob.data(), strBlob.size() );
            if ( ullSize == ZSTD_CONTENTSIZE_ERROR || ullSize == ZSTD_CONTENTSIZE_UNKNOWN || ullSize > 64ULL * 1024 * 1024 )
                throw CSjsonException( SJSON_ERROR_MALFORMED, "invalid text block" );
            if ( m_pFrameIn == NULL )
                m_pFrameIn = ZSTD_createDCtx();
            ZSTD_DCtx *pContext = (ZSTD_DCtx *)m_pFrameIn;
            std::string strOut( (size_t)ullSize, '\0' );
            const ZSTD_DDict *pDictionary = (const ZSTD_DDict *)dictionary.zstdDecompressDictionary();
            size_t nSize = pDictionary ? ZSTD_decompress_usingDDict( pContext, ullSize ? &strOut[ 0 ] : NULL, strOut.size(), strBlob.data(), strBlob.size(), pDictionary )
                                       : ZSTD_decompressDCtx( pContext, ullSize ? &strOut[ 0 ] : NULL, strOut.size(), strBlob.data(), strBlob.size() );
            if ( ZSTD_isError( nSize ) )
                throw CSjsonException( SJSON_ERROR_MALFORMED, ZSTD_getErrorName( nSize ) );
            strOut.resize( nSize );
            return strOut;
#else
            throw CSjsonException( SJSON_ERROR_UNSUPPORTED, "the text channel needs zstd" );
#endif
        }
        std::map<unsigned, std::pair<std::pair<unsigned, std::string>, std::string> >::iterator itRetry = m_mapTextRetry.find( header.nSenderId );
        if ( itRetry != m_mapTextRetry.end() && itRetry->second.first.first == header.nSequence && itRetry->second.first.second == strBlob )
            return itRetry->second.second;
        std::string strOut;
#ifdef IASLIB_SJSON_ZSTD__
        std::map<unsigned, void *>::iterator it = m_mapTextIn.find( header.nSenderId );
        if ( header.nSequence == 0 || it == m_mapTextIn.end() )
        {
            if ( it != m_mapTextIn.end() )
                ZSTD_freeDCtx( (ZSTD_DCtx *)it->second );
            ZSTD_DCtx *pContext = ZSTD_createDCtx();
            const std::string &strDict = dictionary.textDictionary();
            if ( ! strDict.empty() )
                ZSTD_DCtx_loadDictionary( pContext, strDict.data(), strDict.size() );
            m_mapTextIn[ header.nSenderId ] = pContext;
        }
        if ( ! strBlob.empty() )
        {
            ZSTD_DCtx *pContext = (ZSTD_DCtx *)m_mapTextIn[ header.nSenderId ];
            std::vector<char> aBuffer( ZSTD_DStreamOutSize() );
            ZSTD_inBuffer input = { strBlob.data(), strBlob.size(), 0 };
            while ( input.pos < input.size )
            {
                ZSTD_outBuffer output = { aBuffer.data(), aBuffer.size(), 0 };
                size_t nResult = ZSTD_decompressStream( pContext, &output, &input );
                if ( ZSTD_isError( nResult ) )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, ZSTD_getErrorName( nResult ) );
                strOut.append( aBuffer.data(), output.pos );
                if ( strOut.size() > 64ULL * 1024 * 1024 )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, "text block too large" );
            }
                // Drain anything still buffered.
            for ( ;; )
            {
                ZSTD_outBuffer output = { aBuffer.data(), aBuffer.size(), 0 };
                size_t nResult = ZSTD_decompressStream( pContext, &output, &input );
                if ( ZSTD_isError( nResult ) )
                    throw CSjsonException( SJSON_ERROR_MALFORMED, ZSTD_getErrorName( nResult ) );
                strOut.append( aBuffer.data(), output.pos );
                if ( output.pos < output.size )
                    break;
            }
        }
#else
        (void)dictionary;
        if ( ! strBlob.empty() )
            throw CSjsonException( SJSON_ERROR_UNSUPPORTED, "the text channel needs zstd" );
#endif
        m_mapTextRetry[ header.nSenderId ] = std::make_pair( std::make_pair( header.nSequence, strBlob ), strOut );
        return strOut;
    }

    bool CSjson::encode( const CJsonNode &document, std::vector<unsigned char> &packet, size_t *pBits )
    {
        m_eError = SJSON_OK;
        m_strError.clear();
        CSjsonDictionary &dict = *m_pOwn;
        CSjsonStreamState *pStream = m_pStream.get();
        unsigned nFlags = pStream ? FLAG_STATEFUL : 0;
        CSjsonTextChannel perPacket;
        CSjsonTextChannel *pChannel = textChannel( dict, pStream, perPacket );
        std::pair<unsigned long long, unsigned long long> checkpoint = dict.checkpoint();
        SjsonCodec::Options saved = SjsonCodec::options();
        SjsonCodec::options().bLz4 = m_bLz4;
        CSjsonBitWriter data;
        try
        {
            Coding coding( dict, pStream, pChannel );
            bool bDone = false;
            if ( m_bSchemaEncoding && dict.schema() != NULL )
            {
                if ( pStream )
                    pStream->begin();
                CSjsonBitWriter bits;
                if ( codec( dict ).encode( document, bits ) )
                {
                    data.append( bits );
                    nFlags |= FLAG_SCHEMA_ENCODED;
                    bDone = true;
                }
                else
                {
                        // Undo whatever the attempt cached or defined.
                    dict.rollback( checkpoint );
                    if ( pStream )
                        pStream->rollback();
                    if ( pChannel )
                        pChannel->resetOut();
                }
            }
            if ( ! bDone )
            {
                if ( pStream )
                    pStream->begin();
                SjsonCodec::encodeTagged( &document, dict, data );
            }
            if ( hasTextBlock( dict, pStream ) )
            {
                    // Stream packets (and packets with a text dictionary) start
                    // with the text block (possibly empty).
                std::string strBlob = pChannel ? compressText( dict, pChannel->pending(), pStream != NULL ) : std::string();
                CSjsonBitWriter withText;
                withText.writeNybble( strBlob.size() );
                withText.writeBytes( strBlob );
                withText.append( data );
                data = withText;
            }
            if ( pStream )
                pStream->commit();
        }
        catch ( const CSjsonException &e )
        {
            dict.rollback( checkpoint );
            if ( pStream )
                pStream->rollback();
            SjsonCodec::options() = saved;
            fail( e.error(), e.what() );
            return false;
        }
        SjsonCodec::options() = saved;

        bool bFingerprint = dict.hasFingerprint() && ( pStream == NULL || m_nSequence == 0 );
        if ( bFingerprint )
            nFlags |= FLAG_SCHEMA;
        if ( m_bIncludeSenderId )
            nFlags |= FLAG_SENDER;
        CSjsonBitWriter out;
        out.writeBits( VERSION, VERSION_BITS );
        out.writeBits( nFlags, FLAG_BITS );
        if ( m_bIncludeSenderId )
            out.writeBits( m_nSenderId, 16 );
        if ( pStream )
        {
            out.writeBits( m_nSequence, SEQUENCE_BITS );
            out.writeBits( m_nCacheWidth, CACHE_WIDTH_BITS );
            m_nSequence = m_nSequence % 255 + 1;    // 1..255; 0 means "reset"
        }
        if ( bFingerprint )
            out.writeBits( dict.fingerprint(), 32 );
        out.writeNybble( data.bitLength() );
        out.append( data );
        out.writeBits( SjsonUtil::crc32( out.bytes().data(), out.bytes().size() ), 32 );
        packet = out.bytes();
        if ( pBits )
            *pBits = out.bitLength();
        return true;
    }

    bool CSjson::readHeader( const unsigned char *pData, size_t nLength, Header &header, unsigned nSenderId )
    {
        try
        {
            CSjsonBitReader in( pData, nLength * 8 );
            header.nVersion = (unsigned)in.readBits( VERSION_BITS );
            if ( header.nVersion != VERSION )
                return false;
            header.nFlags = (unsigned)in.readBits( FLAG_BITS );
            header.bSender = ( header.nFlags & FLAG_SENDER ) != 0;
            header.nSenderId = header.bSender ? (unsigned)in.readBits( 16 ) : ( nSenderId & 0xFFFF );
            header.bSequence = ( header.nFlags & FLAG_STATEFUL ) != 0;
            header.nSequence = header.nCacheWidth = 0;
            if ( header.bSequence )
            {
                header.nSequence = (unsigned)in.readBits( SEQUENCE_BITS );
                header.nCacheWidth = (unsigned)in.readBits( CACHE_WIDTH_BITS );
            }
            header.bFingerprint = ( header.nFlags & FLAG_SCHEMA ) != 0;
            header.nFingerprint = header.bFingerprint ? (unsigned)in.readBits( 32 ) : 0;
            header.nDataLength = (size_t)in.readNybble();
            header.nHeaderLength = in.position();
            return true;
        }
        catch ( const CSjsonException & )
        {
            return false;
        }
    }

    CSjsonDictionary &CSjson::dictionaryFor( const Header &header )
    {
        if ( ! header.bFingerprint )
            return dictionary( header.nSenderId );
        std::map<unsigned, CSjsonDictionary *>::iterator it = m_mapDictionaries.find( header.nSenderId );
        if ( it != m_mapDictionaries.end() && it->second->hasFingerprint() && it->second->fingerprint() == header.nFingerprint )
            return *it->second;
        std::map<unsigned, CSjsonDictionary *>::iterator itKnown = m_mapKnown.find( header.nFingerprint );
        if ( itKnown == m_mapKnown.end() )
            throw CSjsonException( SJSON_ERROR_UNKNOWN_SCHEMA, "the packet names an unregistered schema or dictionary" );
            // A new sender, or one that switched dictionary: start from a copy.
        if ( it != m_mapDictionaries.end() )
        {
            dropCodec( it->second );
            delete it->second;
        }
        CSjsonDictionary *pCopy = itKnown->second->freshCopy();
        m_mapDictionaries[ header.nSenderId ] = pCopy;
        return *pCopy;
    }

    CSjsonStreamState *CSjson::streamFor( const Header &header, std::unique_ptr<CSjsonStreamState> &pFresh )
    {
        if ( ! header.bSequence )
            return NULL;
        if ( header.nSequence == 0 )
        {
            dictionary( header.nSenderId ).resetToBase();
            pFresh.reset( new CSjsonStreamState( header.nCacheWidth ) );
            return pFresh.get();
        }
        std::map<unsigned, ReceiveStream>::iterator it = m_mapStreams.find( header.nSenderId );
        if ( it == m_mapStreams.end() || it->second.nNext != header.nSequence )
            throw CSjsonException( SJSON_ERROR_STREAM_GAP, "stream packet out of sequence" );
        return it->second.pState.get();
    }

    CJsonNode *CSjson::decode( const unsigned char *pData, size_t nLength, unsigned nSenderId )
    {
        m_eError = SJSON_OK;
        m_strError.clear();
        Header header;
        if ( ! readHeader( pData, nLength, header, nSenderId ) )
        {
            fail( SJSON_ERROR_VERSION, "unreadable header or unsupported packet version" );
            return NULL;
        }
        size_t nMessage = header.messageLength();
        if ( nLength * 8 < nMessage + 32 )
        {
            fail( SJSON_ERROR_MALFORMED, "truncated message" );
            return NULL;
        }
        CSjsonBitReader crcReader( pData, nLength * 8, nMessage );
        if ( (unsigned)crcReader.readBits( 32 ) != packetCrc( pData, nMessage ) )
        {
            fail( SJSON_ERROR_CRC, "CRC mismatch" );
            return NULL;
        }
        CSjsonDictionary *pDictionary = NULL;
        std::unique_ptr<CSjsonStreamState> pFresh;
        CSjsonStreamState *pStream = NULL;
        std::pair<unsigned long long, unsigned long long> checkpoint;
        CJsonNode *pResult = NULL;
        bool bCheckpoint = false;
        try
        {
            pDictionary = &dictionaryFor( header );
            pStream = streamFor( header, pFresh );      // may throw a stream gap
            checkpoint = pDictionary->checkpoint();
            bCheckpoint = true;
            CSjsonTextChannel perPacket;
            bool bTextBlock = hasTextBlock( *pDictionary, pStream );
            CSjsonTextChannel *pChannel = bTextBlock ? ( pStream ? &pStream->text() : &perPacket ) : NULL;
            if ( pStream )
                pStream->begin();
            Coding coding( *pDictionary, pStream, pChannel );
            CSjsonBitReader in( pData, nMessage, header.nHeaderLength );
            if ( bTextBlock )
                pChannel->load( readText( header, in, *pDictionary ) );     // needs zstd unless empty
            if ( header.nFlags & FLAG_SCHEMA_ENCODED )
            {
                if ( pDictionary->schema() == NULL )
                    throw CSjsonException( SJSON_ERROR_UNKNOWN_SCHEMA, "schema-encoded packet without a schema" );
                pResult = codec( *pDictionary ).decode( in );
            }
            else
                pResult = SjsonCodec::decodeTagged( in, *pDictionary );
        }
        catch ( const CSjsonException &e )
        {
                // Leave no trace, so the same packet can be decoded again (for
                // example after a NAK brought the missing names).
            if ( bCheckpoint )
            {
                pDictionary->rollback( checkpoint );
                if ( pStream )
                    pStream->rollback();
            }
            fail( e.error(), e.what() );
            return NULL;
        }
        if ( pStream )
        {
            pStream->commit();
            ReceiveStream &entry = m_mapStreams[ header.nSenderId ];
            if ( pFresh )
                entry.pState.reset( pFresh.release() );
            entry.nNext = header.nSequence % 255 + 1;
        }
        return pResult;
    }

    std::vector<unsigned char> CSjson::nakDictionary( unsigned nSenderId, unsigned long long nLastTag, unsigned long long nLastValue )
    {
        CSjsonBitWriter out;
        out.writeBits( nSenderId & 0xFFFF, 16 );
            // A NAK asks the sender for its own names, under whatever id the
            // peer knows it by (which, without ids on the wire, is the id the
            // peer gave the link).
        std::vector< std::pair<unsigned long long, std::string> > aTags = m_pOwn->entriesAfter( nLastTag ), aValues;
        if ( m_pOwn->values() )
            aValues = m_pOwn->values()->entriesAfter( nLastValue );
        const std::vector< std::pair<unsigned long long, std::string> > *apLists[ 2 ] = { &aTags, &aValues };
        for ( int k = 0; k < 2; k++ )
        {
            out.writeNybble( apLists[ k ]->size() );
            for ( size_t n = 0; n < apLists[ k ]->size(); n++ )
            {
                out.writeNybble( ( *apLists[ k ] )[ n ].first );
                out.writeNybble( ( *apLists[ k ] )[ n ].second.size() );
                out.writeBytes( ( *apLists[ k ] )[ n ].second );
            }
        }
        return out.bytes();
    }

    bool CSjson::loadDictionary( const unsigned char *pData, size_t nLength )
    {
        m_eError = SJSON_OK;
        try
        {
            CSjsonBitReader in( pData, nLength * 8 );
            unsigned nSenderId = (unsigned)in.readBits( 16 );
            CSjsonDictionary &dict = dictionary( nSenderId );
            CSjsonDictionary *apTargets[ 2 ] = { &dict, dict.values() };
            for ( int k = 0; k < 2; k++ )
            {
                unsigned long long nCount = in.readNybble();
                for ( unsigned long long n = 0; n < nCount; n++ )
                {
                    unsigned long long nId = in.readNybble();
                    std::string strText = in.readBytes( (size_t)in.readNybble() );
                    CSjsonDictionary *pTarget = apTargets[ k ];
                    if ( pTarget == NULL )
                        continue;
                    if ( pTarget->hasId( nId ) )
                    {
                        if ( pTarget->lookup( nId ) != strText )
                            throw CSjsonException( SJSON_ERROR_MALFORMED, "tag/index collision: " + strText );
                        continue;
                    }
                    pTarget->set( strText, nId );
                }
            }
            return true;
        }
        catch ( const CSjsonException &e )
        {
            fail( e.error(), e.what() );
            return false;
        }
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
