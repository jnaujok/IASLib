/**
 * @file SjsonStream.h
 * @brief Stream-mode state for SJSON: recent-value caches, deltas and the
 *        text channel.
 *
 *  In stream mode the packets from one sender form an ordered stream, so
 * both sides remember recent values. Every cached value is filed under a key
 * (the path of field names leading to it) and each key keeps its most
 * recently used values. A value is then sent as one of:
 *
 * | bits | meaning |
 * |:-----|:--------|
 * | `1` + index | exactly the cached value at index |
 * | `01` + index + delta | a small change from the cached value at index |
 * | `00` + value | the value itself, encoded as usual |
 *
 *  Deltas exist for floats (difference or XOR of the IEEE-754 bit patterns),
 * integers (signed difference), ISO-8601 timestamps with the same layout
 * (difference in their smallest unit) and other strings (shared prefix plus
 * a new suffix). The encoder picks the shortest form.
 *
 *  The text channel carries longer strings and inline names as
 * NUL-terminated UTF-8 in one zstd block per packet.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSONSTREAM_H__
#define IASLIB_SJSONSTREAM_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonBits.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace IASLib
{
    class CJsonNode;
    class CSjsonDictionary;

        /// Per-packet text: strings routed here travel as UTF-8 bytes in one
        /// zstd-compressed block at the start of the packet data.
    class CSjsonTextChannel
    {
        public:
                /// The assumed compressed cost of a text byte, in bits, used
                /// when the encoder compares forms.
            static const unsigned   BITS_PER_BYTE = 2;

                                    CSjsonTextChannel( void ) : m_nPos( 0 ) {}
            void                    resetOut( void ) { m_aOut.clear(); }
            void                    load( const std::string &strData ) { m_strData = strData; m_nPos = 0; }
            void                    write( const std::string &strData ) { m_aOut.push_back( strData ); }
                /// Writes strData (which must not hold NUL) and a NUL.
            void                    writeTerminated( const std::string &strData ) { m_aOut.push_back( strData + std::string( 1, '\0' ) ); }
            std::string             read( size_t nLength );
            std::string             readTerminated( void );
            size_t                  mark( void ) const { return m_aOut.size(); }
            void                    truncate( size_t nMark ) { if ( m_aOut.size() > nMark ) m_aOut.resize( nMark ); }
            size_t                  size( void ) const;
            std::string             pending( void ) const;

        private:
            std::vector<std::string> m_aOut;    ///< Text written for this packet.
            std::string             m_strData;  ///< Text being read.
            size_t                  m_nPos;     ///< Read position.
    };

        /// Recent-value caches for one direction of one sender's stream.
    class CSjsonStreamState
    {
        public:
            typedef std::shared_ptr<const CJsonNode> Value;
            typedef std::function<void( CSjsonBitWriter & )> PlainEncoder;
            typedef std::function<CJsonNode *( CSjsonBitReader & )> PlainDecoder;

            static const unsigned   MAX_WIDTH = 7;

                /// Each key remembers 2^nWidth values (nWidth 0 to 7).
            explicit                CSjsonStreamState( unsigned nWidth = 3 );

            unsigned                width( void ) const { return m_nWidth; }

                /// @name Transactions
                ///@{
            void                    begin( void );
            void                    commit( void );
            void                    rollback( void );
                ///@}

                /// The path of names leading to the value being coded.
            std::vector<std::string> &path( void ) { return m_aPath; }
                /// The path joined with leaf, as a cache key.
            std::string             key( const std::string &strLeaf ) const;

            CSjsonTextChannel      &text( void ) { return m_text; }

                /// Encodes value under strKey. plain writes its ordinary
                /// encoding; it is only called when no cached value matches
                /// exactly. If a delta beats the plain form, anything plain
                /// added to pDictionary or the text channel is undone.
            void                    encode( const std::string &strKey, const CJsonNode &value, const PlainEncoder &plain,
                                            CSjsonDictionary *pDictionary, CSjsonBitWriter &out );
                /// Decodes a value written by encode(); the caller owns the result.
            CJsonNode              *decode( CSjsonBitReader &in, const std::string &strKey, const PlainDecoder &plain );

                /// Bits that turn reference into value, or false if no delta applies.
            static bool             encodeDelta( const CJsonNode &reference, const CJsonNode &value, CSjsonBitWriter &out );
                /// Applies a delta to reference; the caller owns the result.
            static CJsonNode       *decodeDelta( const CJsonNode &reference, CSjsonBitReader &in );

        private:
            void                    remember( const std::string &strKey, const CJsonNode &value );
            void                    writeIndex( size_t nIndex, CSjsonBitWriter &out ) const;
            const CJsonNode        &cached( const std::vector<Value> &aCache, unsigned long long nIndex ) const;

            unsigned                m_nWidth;   ///< Index width in bits.
            size_t                  m_nSize;    ///< 2^m_nWidth.
            std::map<std::string, std::vector<Value> > m_mapCaches;     ///< Recent values by key.
            std::vector<std::string> m_aPath;   ///< The current path.
            bool                    m_bJournal; ///< Whether a transaction is open.
            std::map<std::string, std::pair<bool, std::vector<Value> > > m_mapJournal;  ///< Caches as they were.
            CSjsonTextChannel       m_text;     ///< The text channel.
    };
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSONSTREAM_H__
