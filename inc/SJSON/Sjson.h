/**
 * @file Sjson.h
 * @brief CSjson: compact binary encoding of JSON (SJSON packets).
 *
 *  SJSON packs a JSON document into a bitstream using a dictionary of field
 * names, tight number and string encodings and, optionally, a schema and a
 * stream mode that remembers recent values. It is wire-compatible with the
 * reference Python library (https://github.com/jnaujok/sjson): packets from
 * either decode in the other.
 *
 *  A packet:
 *
 * | field | description |
 * |:------|:------------|
 * | 4 bits | packet version (5) |
 * | 4 bits | flags: schema fingerprint follows, schema-encoded data, stream mode, sender id follows |
 * | 16 bits | sender id, only when flagged (stateless packets by default) |
 * | 8 + 3 bits | stream mode only: sequence number and cache width |
 * | 32 bits | schema or dictionary fingerprint, only when flagged |
 * | NybbleField | data length in bits |
 * | ... | data |
 * | 32 bits | CRC-32 of the zero-padded bytes of everything above |
 *
 *  Each receiver keeps a dictionary per sender. A receiver that meets a tag
 * it does not know gets SJSON_ERROR_UNKNOWN_TAG from decode(); it asks the
 * sender for the missing entries (a NAK, answered by nakDictionary()) and
 * decodes the same packet again after loadDictionary().
 *
 * @code
 *     CSjson sender( 1 ), receiver( 2 );
 *     std::vector<unsigned char> packet;
 *     sender.encode( *pDocument, packet );
 *     CJsonNode *pCopy = receiver.decode( packet.data(), packet.size() );
 *     if ( pCopy == NULL && receiver.lastError() == SJSON_ERROR_UNKNOWN_TAG )
 *     {
 *         std::vector<unsigned char> update = sender.nakDictionary( 1,
 *             receiver.lastKnownTag( 1 ), receiver.lastKnownValue( 1 ) );
 *         receiver.loadDictionary( update.data(), update.size() );
 *         pCopy = receiver.decode( packet.data(), packet.size() );
 *     }
 * @endcode
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSON_H__
#define IASLIB_SJSON_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonBits.h"
#include "SjsonDictionary.h"
#include "SjsonStream.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace IASLib
{
    class CJsonNode;
    class CSjsonSchemaCodec;

        /// Encodes JSON trees into SJSON packets and decodes them.
        ///
        /// One CSjson is one endpoint: it sends with its own sender id and
        /// receives from any number of senders. Not thread-safe: use one per
        /// thread, or lock around it.
    class CSjson
    {
        public:
                /// A parsed packet header.
            struct Header
            {
                unsigned            nSenderId;      ///< The sender (the caller's default when bSender is false).
                bool                bSender;        ///< Whether the packet carries a sender id.
                unsigned            nVersion;       ///< The packet version.
                unsigned            nFlags;         ///< FLAG_ bits.
                bool                bFingerprint;   ///< Whether nFingerprint is present.
                unsigned            nFingerprint;   ///< The schema or dictionary fingerprint.
                bool                bSequence;      ///< Stream mode: whether nSequence is present.
                unsigned            nSequence;      ///< Stream mode: the sequence number (0 resets).
                unsigned            nCacheWidth;    ///< Stream mode: the cache width.
                size_t              nDataLength;    ///< Data bits.
                size_t              nHeaderLength;  ///< Header bits.
                    /// Header plus data, in bits (the CRC is not included).
                size_t              messageLength( void ) const { return nHeaderLength + nDataLength; }
            };

            static const unsigned   VERSION = 5;
            static const unsigned   FLAG_SCHEMA = 1;            ///< A fingerprint follows.
            static const unsigned   FLAG_SCHEMA_ENCODED = 2;    ///< The data is schema-encoded.
            static const unsigned   FLAG_STATEFUL = 4;          ///< Stream mode.
            static const unsigned   FLAG_SENDER = 8;            ///< A sender id follows.
            static const int        TEXT_LEVEL = 19;            ///< Default zstd level for the text channel.
            static const int        TEXT_WINDOW_LOG = 17;       ///< Default zstd window (2^17 = 128 KiB) for stream mode.

                /// An endpoint sending as nSenderId (16 bits). bStateful turns
                /// on stream mode for links that deliver packets in order; each
                /// field then remembers 2^nCacheWidth recent values. Stateless
                /// packets carry the sender id; stream packets leave it out,
                /// since a connection has one sender (see setIncludeSenderId).
            explicit                CSjson( unsigned nSenderId, bool bStateful = false, unsigned nCacheWidth = 3 );
                                   ~CSjson( void );

            unsigned                senderId( void ) const { return m_nSenderId; }
            bool                    stateful( void ) const { return m_bStateful; }
                /// Put the sender id in every packet (needed when receivers
                /// hear several senders, as on a broadcast network). The
                /// default is on for stateless packets and off in stream mode.
            void                    setIncludeSenderId( bool bOn ) { m_bIncludeSenderId = bOn; }
            bool                    includeSenderId( void ) const { return m_bIncludeSenderId; }

                /// @name Pre-loaded dictionaries
                ///@{
                /// Sends with a copy of dictionary's pre-loaded entries (from a
                /// schema or a compiled dictionary), and registers it.
            void                    setDictionary( const CSjsonDictionary &dictionary );
                /// Lets this endpoint decode packets that name dictionary's
                /// fingerprint; returns the fingerprint.
            unsigned                registerDictionary( const CSjsonDictionary &dictionary );
                ///@}

                /// Use the schema-driven encoding when the dictionary has a
                /// schema and a document fits it (the default).
            void                    setSchemaEncoding( bool bOn ) { m_bSchemaEncoding = bOn; }
                /// Allow the LZ4 string form (only used if liblz4 was found).
            void                    setLz4( bool bOn ) { m_bLz4 = bOn; }
                /// The zstd level for the text channel (1 to 22; default 19).
            void                    setTextLevel( int nLevel ) { m_nTextLevel = nLevel; }
                /// The zstd window for stream mode, as a power of two (10 to
                /// 27; default 17). A level-19 stream compressor needs about
                /// 3 MB at the default, 19 MB at 20 and 90 MB at 23 (zstd's own
                /// default); on typical message streams 17 compresses as well
                /// as 23. Decoders accept any window either side uses.
            void                    setTextWindowLog( int nWindowLog ) { m_nTextWindowLog = nWindowLog; }

                /// Encodes document into packet (zero-padded bytes). Returns
                /// false (see lastError()) if it cannot be encoded. pBits, if
                /// given, receives the packet length in bits.
            bool                    encode( const CJsonNode &document, std::vector<unsigned char> &packet, size_t *pBits = NULL );
                /// Decodes a packet; the caller owns the result. NULL on error,
                /// with lastError() saying why. The data may carry padding.
                /// A packet without a sender id is attributed to nSenderId
                /// (for example one id per connection), which must not be
                /// this endpoint's own id.
            CJsonNode              *decode( const unsigned char *pData, size_t nLength, unsigned nSenderId = 0 );

            SjsonError              lastError( void ) const { return m_eError; }
            const std::string      &lastErrorText( void ) const { return m_strError; }

                /// Parses a packet header; false if the data is too short or
                /// the version is not supported. A packet without a sender id
                /// is attributed to nSenderId.
            static bool             readHeader( const unsigned char *pData, size_t nLength, Header &header, unsigned nSenderId = 0 );

                /// @name Dictionary exchange
                ///@{
                /// This endpoint's own names after the given ids: the answer
                /// to a NAK. It is labelled with nSenderId, the id the peer
                /// knows this endpoint by (the id it gave the link when
                /// packets carry no sender id).
            std::vector<unsigned char> nakDictionary( unsigned nSenderId, unsigned long long nLastTag, unsigned long long nLastValue = 0 );
                /// Loads an answer to a NAK into the dictionary learnt from
                /// that sender.
            bool                    loadDictionary( const unsigned char *pData, size_t nLength );
                /// The highest tag id known for a sender: what to NAK with.
            unsigned long long      lastKnownTag( unsigned nSenderId ) const;
                /// The highest interned-value id known for a sender.
            unsigned long long      lastKnownValue( unsigned nSenderId ) const;
                /// The dictionary learnt from a peer (created empty if new).
                /// This endpoint's own sending dictionary is ownDictionary(),
                /// kept apart, so no packet or NAK reply from a peer can reach
                /// it, whatever id it carries.
            CSjsonDictionary       &dictionary( unsigned nSenderId );
                /// The dictionary this endpoint sends with.
            CSjsonDictionary       &ownDictionary( void ) { return *m_pOwn; }
                ///@}

                /// Stream mode: forgets all cached values and everything learnt
                /// since the pre-loaded dictionary. The next packet carries
                /// sequence 0, which tells receivers to do the same.
            void                    resetStream( void );

                /// True if this build can compress the text channel (libzstd).
            static bool             zstdAvailable( void );

        private:
                                    CSjson( const CSjson & );
            CSjson                 &operator=( const CSjson & );

            struct ReceiveStream
            {
                std::unique_ptr<CSjsonStreamState> pState;  ///< The receiver's caches.
                unsigned            nNext;                  ///< The next sequence number.
            };

            CSjsonDictionary       &dictionaryFor( const Header &header );
            CSjsonStreamState      *streamFor( const Header &header, std::unique_ptr<CSjsonStreamState> &pFresh );
            CSjsonSchemaCodec      &codec( CSjsonDictionary &dictionary );
            void                    dropCodec( const CSjsonDictionary *pDictionary );
            std::string             compressText( CSjsonDictionary &dictionary, const std::string &strText, bool bStream );
            std::string             readText( const Header &header, CSjsonBitReader &in, CSjsonDictionary &dictionary );
            void                    fail( SjsonError eError, const std::string &strText );

            unsigned                m_nSenderId;    ///< This endpoint's sender id.
            bool                    m_bStateful;    ///< Stream mode.
            bool                    m_bIncludeSenderId; ///< Packets carry the sender id.
            unsigned                m_nCacheWidth;  ///< Stream mode cache width.
            bool                    m_bSchemaEncoding;  ///< Use the schema codec when possible.
            bool                    m_bLz4;         ///< Allow LZ4.
            int                     m_nTextLevel;   ///< zstd level.
            int                     m_nTextWindowLog;   ///< zstd window log in stream mode.
            std::unique_ptr<CSjsonStreamState> m_pStream;   ///< The sending stream state.
            unsigned                m_nSequence;    ///< The next sequence number to send.
            CSjsonDictionary       *m_pOwn;         ///< The dictionary this endpoint sends with (owned).
            std::map<unsigned, CSjsonDictionary *> m_mapDictionaries;  ///< Learnt from each peer, by sender id (owned).
            std::map<unsigned, CSjsonDictionary *> m_mapKnown;         ///< By fingerprint (owned).
            std::map<unsigned, ReceiveStream> m_mapStreams;            ///< Receiving stream state per sender.
            std::map<const CSjsonDictionary *, CSjsonSchemaCodec *> m_mapCodecs;  ///< Schema codecs (owned).
            void                   *m_pTextOut;     ///< zstd compression stream (ZSTD_CCtx).
            void                   *m_pFrameOut;    ///< Reused for stateless text frames (ZSTD_CCtx).
            void                   *m_pFrameIn;     ///< Reused for stateless text frames (ZSTD_DCtx).
            std::map<unsigned, void *> m_mapTextIn; ///< zstd decompression streams (ZSTD_DCtx) per sender.
            std::map<unsigned, std::pair<std::pair<unsigned, std::string>, std::string> > m_mapTextRetry;  ///< Last text per sender.
            SjsonError              m_eError;       ///< The last error.
            std::string             m_strError;     ///< Its description.
    };
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSON_H__
