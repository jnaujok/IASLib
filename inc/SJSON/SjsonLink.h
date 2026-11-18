/**
 * @file SjsonLink.h
 * @brief CSjsonLink: SJSON messages over a CStream, with ACK/NAK recovery.
 *
 *  A receiver cannot always decode an SJSON packet on its own: it may lack
 * field names the sender has learnt, have lost its place in a stream (it
 * restarted), or have received a damaged packet. CSjsonLink frames each
 * packet on a byte stream (a CSocketStream over TCP or WinSock, a serial
 * line, a pipe) and runs the protocol that recovers from each case, so the
 * application calls send() and receive() and sees only whole documents.
 *
 *  Frames are big-endian: 1 byte type, 2 bytes message number, 4 bytes
 * payload length, then the payload. The frame format is the same as the
 * Python library's examples/acknak.py, so the two can talk to each other.
 *
 * | frame | direction | payload | meaning |
 * |:------|:----------|:--------|:--------|
 * | DATA | sender to receiver | SJSON packet | a message |
 * | ACK | receiver to sender | none | decoded and delivered |
 * | NAK_DICT | receiver to sender | 16-bit sender id, 32-bit last tag, 32-bit last value | send the dictionary entries after these |
 * | DICT | sender to receiver | nakDictionary() bytes | the missing entries; the receiver retries the packet it kept |
 * | NAK_RESET | receiver to sender | none | stream gap: reset the stream and send the message again |
 * | NAK_RESEND | receiver to sender | none | damaged packet: send it again |
 * | NAK_FATAL | receiver to sender | UTF-8 reason | it can never be decoded (an unregistered schema) |
 *
 *  The link is stop-and-wait: send() returns once the message is ACKed.
 * Both ends may send; a DATA frame that arrives while send() waits is
 * decoded, ACKed and kept for receive(). CStream reads block, so the link
 * does not time out: use it on a reliable stream, which never loses frames.
 *
 * @code
 *     CSocketStream oStream( pSocket );
 *     CSjson oCodec( 1, true );                // stream mode
 *     CSjsonLink oLink( oStream, oCodec );
 *     oLink.send( *pDocument );                 // returns once it is ACKed
 *
 *     // and at the other end:
 *     CSjson oPeer( 2 );
 *     CSjsonLink oPeerLink( oPeerStream, oPeer );
 *     CJsonNode *pDocument = oPeerLink.receive(); // the caller owns it
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

#ifndef IASLIB_SJSONLINK_H__
#define IASLIB_SJSONLINK_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include "Sjson.h"

#include <deque>
#include <map>
#include <string>
#include <vector>

namespace IASLib
{
    class CJsonNode;
    class CStream;

        /// One end of an SJSON link over a CStream.
        ///
        /// The link uses, but does not own, the stream and the codec. The
        /// codec's settings (schema or dictionary, stream mode, sender id)
        /// decide what goes on the wire. Not thread-safe.
    class CSjsonLink
    {
        public:
                /// Frame types.
            enum FrameType
            {
                FRAME_DATA = 1,         ///< An SJSON packet.
                FRAME_ACK = 2,          ///< Decoded and delivered.
                FRAME_NAK_DICT = 3,     ///< Names are missing.
                FRAME_DICT = 4,         ///< The missing names.
                FRAME_NAK_RESET = 5,    ///< Stream gap: reset and send again.
                FRAME_NAK_RESEND = 6,   ///< Damaged: send again.
                FRAME_NAK_FATAL = 7     ///< Cannot be decoded.
            };

                /// Why send() or receive() failed.
            enum LinkError
            {
                LINK_OK = 0,            ///< No error.
                LINK_CLOSED,            ///< The stream ended or failed.
                LINK_ENCODE,            ///< The codec could not encode the document.
                LINK_REFUSED,           ///< The peer can never decode it (NAK_FATAL).
                LINK_TOO_MANY_NAKS      ///< The peer kept refusing the message.
            };

                /// Byte counts, for the curious.
            struct Stats
            {
                size_t              nSent;          ///< Packet bytes sent the first time.
                size_t              nResent;        ///< Packet bytes sent again.
                size_t              nNaks;          ///< NAKs received.
                size_t              nDictionaryBytes; ///< DICT payload bytes sent.
            };

                /// A link over stream using codec. Packets without a sender
                /// id (stream mode leaves it out by default) are attributed
                /// to nPeerId. send() gives up after nMaxNaks NAKs for one
                /// message.
                                    CSjsonLink( CStream &stream, CSjson &codec, unsigned nPeerId = 0, unsigned nMaxNaks = 4 );

                /// Sends a document and waits until the peer has decoded it.
                /// False on failure, with lastError() saying why.
            bool                    send( const CJsonNode &document );
                /// Waits for the next document from the peer; the caller
                /// owns it. NULL if the stream ended or failed.
            CJsonNode              *receive( void );
                /// Documents already received (while send() waited) and not
                /// yet taken by receive().
            size_t                  pending( void ) const { return m_aInbox.size(); }

            LinkError               lastError( void ) const { return m_eError; }
            const std::string      &lastErrorText( void ) const { return m_strError; }
            const Stats            &stats( void ) const { return m_stats; }
            CSjson                 &codec( void ) { return *m_pCodec; }
                /// Uses another codec from now on (a receiving process that
                /// restarted, say); the link's message numbering carries on.
            void                    setCodec( CSjson &codec ) { m_pCodec = &codec; }

                                   ~CSjsonLink( void );

        private:
                                    CSjsonLink( const CSjsonLink & );
            CSjsonLink             &operator =( const CSjsonLink & );

            bool                    writeFrame( unsigned nType, unsigned nNumber, const std::vector<unsigned char> &payload );
            bool                    readFrame( unsigned &nType, unsigned &nNumber, std::vector<unsigned char> &payload );
            void                    incoming( unsigned nType, unsigned nNumber, const std::vector<unsigned char> &payload );
            bool                    fail( LinkError eError, const std::string &strText );

            CStream                *m_pStream;      ///< The byte stream.
            CSjson                 *m_pCodec;       ///< Encodes and decodes.
            unsigned                m_nPeerId;      ///< Id for packets without one.
            unsigned                m_nMaxNaks;     ///< NAKs per message before giving up.
            unsigned                m_nNumber;      ///< The next message number.
            std::map<unsigned, std::vector<unsigned char> > m_mapHeld; ///< Packets waiting for names, by number.
            bool                    m_bDelivered;   ///< Whether m_nLastDelivered is set.
            unsigned                m_nLastDelivered; ///< The last message number delivered.
            std::deque<CJsonNode *> m_aInbox;       ///< Received, not yet taken.
            Stats                   m_stats;        ///< Byte counts.
            LinkError               m_eError;       ///< The last error.
            std::string             m_strError;     ///< Its description.
    };
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__

#endif // IASLIB_SJSONLINK_H__
