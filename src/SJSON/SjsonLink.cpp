/**
 * SJSON messages over a CStream with ACK/NAK recovery. See SjsonLink.h.
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

#include "SjsonLink.h"
#include "JsonNode.h"
#include "Stream.h"

#include <random>

namespace IASLib
{
    namespace
    {
        void putUInt( std::vector<unsigned char> &out, unsigned long long nValue, int nBytes )
        {
            for ( int n = nBytes - 1; n >= 0; n-- )
                out.push_back( (unsigned char)( nValue >> ( 8 * n ) ) );
        }

        unsigned long long getUInt( const std::vector<unsigned char> &in, size_t nAt, int nBytes )
        {
            unsigned long long nValue = 0;
            for ( int n = 0; n < nBytes; n++ )
                nValue = ( nValue << 8 ) | in[ nAt + (size_t)n ];
            return nValue;
        }
    }

    CSjsonLink::CSjsonLink( CStream &stream, CSjson &codec, unsigned nPeerId, unsigned nMaxNaks, size_t nMaxFrame )
        : m_pStream( &stream ), m_pCodec( &codec ), m_nPeerId( nPeerId ), m_nMaxNaks( nMaxNaks ), m_nMaxFrame( nMaxFrame ),
          m_nNumber( 0 ), m_bDelivered( false ), m_nLastDelivered( 0 ), m_eError( LINK_OK )
    {
        m_stats.nSent = m_stats.nResent = m_stats.nNaks = m_stats.nDictionaryBytes = 0;
            // Start numbering at random, so a restarted sender's first
            // message is not mistaken for a resend of the last one delivered.
        std::random_device random;
        m_nNumber = (unsigned)random() & 0xFFFF;
    }

    CSjsonLink::~CSjsonLink( void )
    {
        for ( size_t n = 0; n < m_aInbox.size(); n++ )
            delete m_aInbox[ n ];
    }

    bool CSjsonLink::fail( LinkError eError, const std::string &strText )
    {
        m_eError = eError;
        m_strError = strText;
        return false;
    }

    bool CSjsonLink::writeFrame( unsigned nType, unsigned nNumber, const std::vector<unsigned char> &payload )
    {
        std::vector<unsigned char> frame;
        frame.reserve( 7 + payload.size() );
        putUInt( frame, nType, 1 );
        putUInt( frame, nNumber & 0xFFFF, 2 );
        putUInt( frame, payload.size(), 4 );
        frame.insert( frame.end(), payload.begin(), payload.end() );
        if ( m_pStream->PutBuffer( (const char *)frame.data(), (int)frame.size() ) != (int)frame.size() )
            return fail( LINK_CLOSED, "the stream would not take the frame" );
        m_pStream->Flush();
        return true;
    }

    bool CSjsonLink::readFrame( unsigned &nType, unsigned &nNumber, std::vector<unsigned char> &payload )
    {
        unsigned char achHeader[ 7 ];
        int nGot = 0;
        while ( nGot < 7 )
        {
            int nRead = m_pStream->GetBuffer( (char *)achHeader + nGot, 7 - nGot );
            if ( nRead <= 0 )
                return fail( LINK_CLOSED, "the stream ended" );
            nGot += nRead;
        }
        std::vector<unsigned char> header( achHeader, achHeader + 7 );
        nType = (unsigned)getUInt( header, 0, 1 );
        nNumber = (unsigned)getUInt( header, 1, 2 );
        size_t nLength = (size_t)getUInt( header, 3, 4 );
        if ( nType < FRAME_DATA || nType > FRAME_NAK_FATAL )
            return fail( LINK_PROTOCOL, "unknown frame type " + std::to_string( nType ) );
            // Check before allocating: a peer must not make us set aside
            // memory for a frame it never sends.
        if ( nLength > m_nMaxFrame )
            return fail( LINK_PROTOCOL, "frame too long: " + std::to_string( nLength ) + " bytes" );
        bool bEmpty = nType == FRAME_ACK || nType == FRAME_NAK_RESET || nType == FRAME_NAK_RESEND;
        if ( ( bEmpty && nLength != 0 ) || ( nType == FRAME_NAK_DICT && nLength != 10 ) )
            return fail( LINK_PROTOCOL, "control frame of the wrong size" );
        payload.resize( nLength );
        size_t nAt = 0;
        while ( nAt < nLength )
        {
            int nRead = m_pStream->GetBuffer( (char *)payload.data() + nAt, (int)( nLength - nAt ) );
            if ( nRead <= 0 )
                return fail( LINK_CLOSED, "the stream ended inside a frame" );
            nAt += (size_t)nRead;
        }
        return true;
    }

    bool CSjsonLink::send( const CJsonNode &document )
    {
        m_eError = LINK_OK;
        m_strError.clear();
        unsigned nNumber = m_nNumber;
        m_nNumber = ( m_nNumber + 1 ) & 0xFFFF;
        std::vector<unsigned char> packet;
        if ( ! m_pCodec->encode( document, packet ) )
            return fail( LINK_ENCODE, m_pCodec->lastErrorText() );
        if ( ! writeFrame( FRAME_DATA, nNumber, packet ) )
            return false;
        m_stats.nSent += packet.size();
        unsigned nNaks = 0;
        for ( ;; )
        {
            unsigned nType = 0, nAnswered = 0;
            std::vector<unsigned char> payload;
            if ( ! readFrame( nType, nAnswered, payload ) )
                return false;
            if ( nType == FRAME_DATA || nType == FRAME_DICT )
            {
                incoming( nType, nAnswered, payload );     // the peer sends meanwhile
                if ( m_eError != LINK_OK )
                    return false;
                continue;
            }
            if ( nAnswered != nNumber )
                continue;                                   // a late answer
            if ( nType == FRAME_ACK )
                return true;
            m_stats.nNaks++;
            if ( nType == FRAME_NAK_FATAL )
                return fail( LINK_REFUSED, std::string( payload.begin(), payload.end() ) );
            if ( ++nNaks > m_nMaxNaks )
                return fail( LINK_TOO_MANY_NAKS, "the peer kept refusing the message" );
            if ( nType == FRAME_NAK_DICT && payload.size() >= 10 )
            {
                unsigned nSender = (unsigned)getUInt( payload, 0, 2 );
                std::vector<unsigned char> update = m_pCodec->nakDictionary( nSender, getUInt( payload, 2, 4 ), getUInt( payload, 6, 4 ) );
                m_stats.nDictionaryBytes += update.size();
                if ( ! writeFrame( FRAME_DICT, nNumber, update ) )
                    return false;
            }
            else if ( nType == FRAME_NAK_RESET && m_pCodec->stateful() )
            {
                    // The peer lost the stream: start again from sequence 0.
                m_pCodec->resetStream();
                if ( ! m_pCodec->encode( document, packet ) )
                    return fail( LINK_ENCODE, m_pCodec->lastErrorText() );
                if ( ! writeFrame( FRAME_DATA, nNumber, packet ) )
                    return false;
                m_stats.nResent += packet.size();
            }
            else    // NAK_RESEND (and NAK_RESET to a stateless sender, which has no stream to reset)
            {
                if ( ! writeFrame( FRAME_DATA, nNumber, packet ) )
                    return false;
                m_stats.nResent += packet.size();
            }
        }
    }

    CJsonNode *CSjsonLink::receive( void )
    {
        m_eError = LINK_OK;
        m_strError.clear();
        while ( m_aInbox.empty() )
        {
            unsigned nType = 0, nNumber = 0;
            std::vector<unsigned char> payload;
            if ( ! readFrame( nType, nNumber, payload ) )
                return NULL;
            incoming( nType, nNumber, payload );
            if ( m_eError != LINK_OK )
                return NULL;
        }
        CJsonNode *pDocument = m_aInbox.front();
        m_aInbox.pop_front();
        return pDocument;
    }

    void CSjsonLink::incoming( unsigned nType, unsigned nNumber, const std::vector<unsigned char> &payload )
    {
        if ( nType == FRAME_DATA )
        {
                // Stop-and-wait: a new message means the sender gave up on
                // any earlier one, so only this packet is kept.
            m_mapHeld.clear();
            m_mapHeld[ nNumber ] = payload;
        }
        else if ( nType == FRAME_DICT )
        {
            if ( ! m_pCodec->loadDictionary( payload.data(), payload.size() ) && payload.size() >= 2 )
            {
                    // The names contradict what we learnt (the sender
                    // restarted) or the DICT is damaged: forget what we
                    // learnt from that sender, and the retry below NAKs for
                    // everything after the pre-loaded entries.
                m_pCodec->dictionary( (unsigned)getUInt( payload, 0, 2 ) ).resetToBase();
            }
        }
        else
            return;                                         // an answer nobody waits for
        std::map<unsigned, std::vector<unsigned char> >::iterator it = m_mapHeld.find( nNumber );
        if ( it == m_mapHeld.end() )
            return;
        std::vector<unsigned char> packet;
        packet.swap( it->second );
        m_mapHeld.erase( it );
        std::vector<unsigned char> none;
        if ( m_bDelivered && nNumber == m_nLastDelivered )
        {
                // Our ACK was lost and the sender tried again: ACK it again
                // without delivering the message twice.
            writeFrame( FRAME_ACK, nNumber, none );
            return;
        }
        CJsonNode *pDocument = m_pCodec->decode( packet.data(), packet.size(), m_nPeerId );
        if ( pDocument != NULL )
        {
            m_bDelivered = true;
            m_nLastDelivered = nNumber;
            m_aInbox.push_back( pDocument );
            writeFrame( FRAME_ACK, nNumber, none );
            return;
        }
        switch ( m_pCodec->lastError() )
        {
            case SJSON_ERROR_UNKNOWN_TAG:
            {
                    // Keep the packet: the DICT answer makes it decodable.
                CSjson::Header header;
                unsigned nSender = m_nPeerId;
                if ( CSjson::readHeader( packet.data(), packet.size(), header, m_nPeerId ) )
                    nSender = header.nSenderId;
                std::vector<unsigned char> request;
                putUInt( request, nSender, 2 );
                putUInt( request, m_pCodec->lastKnownTag( nSender ) & 0xFFFFFFFF, 4 );
                putUInt( request, m_pCodec->lastKnownValue( nSender ) & 0xFFFFFFFF, 4 );
                m_mapHeld[ nNumber ].swap( packet );
                writeFrame( FRAME_NAK_DICT, nNumber, request );
                break;
            }
            case SJSON_ERROR_STREAM_GAP:
                writeFrame( FRAME_NAK_RESET, nNumber, none );
                break;
            case SJSON_ERROR_UNKNOWN_SCHEMA:
            case SJSON_ERROR_UNSUPPORTED:       // e.g. needs zstd, or our own id
            case SJSON_ERROR_VERSION:
            {
                const std::string &strText = m_pCodec->lastErrorText();
                writeFrame( FRAME_NAK_FATAL, nNumber, std::vector<unsigned char>( strText.begin(), strText.end() ) );
                break;
            }
            case SJSON_ERROR_MALFORMED:
            {
                    // An intact stream packet our stream state does not fit
                    // (out of sync): start the stream again.
                CSjson::Header header;
                bool bStream = CSjson::readHeader( packet.data(), packet.size(), header, m_nPeerId ) && header.bSequence;
                writeFrame( bStream ? FRAME_NAK_RESET : FRAME_NAK_RESEND, nNumber, none );
                break;
            }
            default:                                        // CRC, truncation, a mangled header
                writeFrame( FRAME_NAK_RESEND, nNumber, none );
                break;
        }
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
