/**
 * @file SjsonDictionary.h
 * @brief CSjsonDictionary: SJSON's field-name ids and interned values.
 *
 *  Field names are sent as small integer ids. The companion value dictionary
 * does the same for interned string values (URLs, and strings a schema
 * enumerates). Ids start at 1; id 0 means "unnamed". Entries that both sides
 * load up front (from a schema or a compiled dictionary) are the "base";
 * entries a sender adds later reach the receiver through a NAK.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSONDICTIONARY_H__
#define IASLIB_SJSONDICTIONARY_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonBits.h"

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace IASLib
{
    class CJsonNode;
    class CSjsonStreamState;
    class CSjsonTextChannel;
    class CSjsonSchemaModel;

        /// SJSON's dictionary of field names (and, through values(), of
        /// interned string values).
    class CSjsonDictionary
    {
        public:
                /// An empty dictionary; with bWithValues it has a value
                /// dictionary as well (the value dictionary itself has none).
            explicit                CSjsonDictionary( bool bWithValues = true );
                                   ~CSjsonDictionary( void );

                /// A new dictionary holding names (ids 1, 2, ...) and values, with
                /// an optional schema and text dictionary; its fingerprint is
                /// computed (see computeFingerprint()).
            static CSjsonDictionary *fromNames( const std::vector<std::string> &aNames,
                                                const std::vector<std::string> &aValues,
                                                const std::shared_ptr<const CSjsonSchemaModel> &pSchema = std::shared_ptr<const CSjsonSchemaModel>(),
                                                const std::string &strTextDictionary = std::string() );
                /// A dictionary pre-loaded with every field name and enumerated
                /// string of a schema (same ids and fingerprint as Python's
                /// TagDictionary.from_schema()).
            static CSjsonDictionary *fromSchema( const std::shared_ptr<const CSjsonSchemaModel> &pSchema );
                /// Loads a compiled dictionary (as written by Python's
                /// TagDictionary.save() or toJson()): the full document, a
                /// {"tags": [...], "values": [...]} object, or a list of names.
                /// Throws CSjsonException if it is malformed or its recorded
                /// fingerprint does not match its content.
            static CSjsonDictionary *fromJson( const CJsonNode &data );
                /// Reads a compiled dictionary file.
            static CSjsonDictionary *load( const std::string &strPath );
                /// The pre-loaded part as a compiled dictionary document (the
                /// caller owns it).
            CJsonNode              *toJson( void ) const;
                /// Writes toJson() to a file; false on failure.
            bool                    save( const std::string &strPath ) const;
                /// The fingerprint for the pre-loaded content: the schema's own
                /// fingerprint if this holds exactly what the schema produces,
                /// otherwise a CRC of names, values, schema and text dictionary.
            unsigned                computeFingerprint( void ) const;

                /// True if strTag has an id.
            bool                    hasTag( const std::string &strTag ) const { return m_mapIds.find( strTag ) != m_mapIds.end(); }
                /// True if nId is assigned.
            bool                    hasId( unsigned long long nId ) const { return nId >= 1 && nId <= m_aNames.size() && m_aAssigned[ nId - 1 ]; }
                /// strTag's id; throws SJSON_ERROR_MALFORMED if it has none.
            unsigned long long      get( const std::string &strTag ) const;
                /// The text for nId; throws SJSON_ERROR_UNKNOWN_TAG if unknown.
            const std::string      &lookup( unsigned long long nId ) const;
                /// Adds strTag (if new) and returns its id.
            unsigned long long      add( const std::string &strTag );
                /// Assigns strTag the id nId (as loaded from a NAK response).
            void                    set( const std::string &strTag, unsigned long long nId );
                /// Every entry with an id above nLastId, in id order.
            std::vector< std::pair<unsigned long long, std::string> > entriesAfter( unsigned long long nLastId ) const;

                /// The highest id in use.
            unsigned long long      lastIndex( void ) const { return m_nLastIndex; }
                /// How many ids were loaded up front.
            unsigned long long      baseSize( void ) const { return m_nBaseSize; }
            void                    setBaseSize( unsigned long long nSize ) { m_nBaseSize = nSize; }
                /// The pre-loaded names, in id order.
            std::vector<std::string> baseNames( void ) const;

                /// The value dictionary (NULL for a value dictionary).
            CSjsonDictionary       *values( void ) const { return m_pValues; }

                /// The fingerprint sent in packet headers, if the dictionary has one.
            bool                    hasFingerprint( void ) const { return m_bFingerprint; }
            unsigned                fingerprint( void ) const { return m_nFingerprint; }
            void                    setFingerprint( unsigned nFingerprint ) { m_nFingerprint = nFingerprint; m_bFingerprint = true; }

                /// URLs seen once (a URL is interned the second time it is seen).
            std::set<std::string>  &seenUrls( void ) { return m_setSeenUrls; }

                /// @name Transactions: an encode or decode that fails leaves no trace.
                ///@{
            std::pair<unsigned long long, unsigned long long> checkpoint( void ) const;
            void                    rollback( const std::pair<unsigned long long, unsigned long long> &checkpoint );
                /// Forgets every entry learnt after the pre-loaded ones.
            void                    resetToBase( void );
                ///@}

                /// @name Stream mode (set by CSjson while encoding or decoding)
                ///@{
            void                    setStream( CSjsonStreamState *pStream );
            CSjsonStreamState      *stream( void ) const { return m_pStream; }
            void                    setText( CSjsonTextChannel *pText );
            CSjsonTextChannel      *text( void ) const { return m_pText; }
                ///@}

                /// Writes strTag's id; in stream mode a new tag is defined inline.
            void                    encodeId( const std::string &strTag, CSjsonBitWriter &out );
                /// Reads an id written by encodeId() and returns its text.
            const std::string      &decodeId( CSjsonBitReader &in );

                /// The schema this dictionary was built from (NULL if none);
                /// copies share it.
            const CSjsonSchemaModel *schema( void ) const { return m_pSchema.get(); }
            void                    setSchema( const std::shared_ptr<const CSjsonSchemaModel> &pSchema ) { m_pSchema = pSchema; }
            const std::shared_ptr<const CSjsonSchemaModel> &sharedSchema( void ) const { return m_pSchema; }

                /// A zstd dictionary for the text channel (empty if none).
            const std::string      &textDictionary( void ) const { return m_strTextDictionary; }
            void                    setTextDictionary( const std::string &strData ) { m_strTextDictionary = strData; }

                /// A new dictionary holding only the pre-loaded entries (and the
                /// same schema, fingerprint and text dictionary).
            CSjsonDictionary       *freshCopy( void ) const;

        private:
                                    CSjsonDictionary( const CSjsonDictionary & );
            CSjsonDictionary       &operator=( const CSjsonDictionary & );

            std::unordered_map<std::string, unsigned long long> m_mapIds;  ///< Text to id.
            std::vector<std::string> m_aNames;          ///< Text by id - 1.
            std::vector<bool>       m_aAssigned;        ///< Whether id - 1 is in use.
            unsigned long long      m_nLastIndex;       ///< The highest id.
            unsigned long long      m_nBaseSize;        ///< Pre-loaded ids.
            CSjsonDictionary       *m_pValues;          ///< The value dictionary (owned).
            bool                    m_bFingerprint;     ///< Whether m_nFingerprint is set.
            unsigned                m_nFingerprint;     ///< The fingerprint.
            std::set<std::string>   m_setSeenUrls;      ///< URLs seen once.
            CSjsonStreamState      *m_pStream;          ///< Stream state while coding (not owned).
            CSjsonTextChannel      *m_pText;            ///< Text channel while coding (not owned).
            std::shared_ptr<const CSjsonSchemaModel> m_pSchema; ///< The schema, shared by copies.
            std::string             m_strTextDictionary; ///< zstd dictionary bytes.
    };
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSONDICTIONARY_H__
