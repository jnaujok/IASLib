/**
 * @file SjsonSchemaCodec.h
 * @brief CSjsonSchemaCodec: SJSON's schema-driven encoding.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSONSCHEMACODEC_H__
#define IASLIB_SJSONSCHEMACODEC_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonBits.h"

namespace IASLib
{
    class CJsonNode;
    class CSjsonDictionary;
    class CSjsonSchemaModel;

        /// Encodes documents that fit a schema without field tags or type
        /// codes (see SjsonSchema.h).
    class CSjsonSchemaCodec
    {
        public:
                                    CSjsonSchemaCodec( const CSjsonSchemaModel &model, CSjsonDictionary &dictionary );
                                   ~CSjsonSchemaCodec( void );
                /// Encodes document; false (and nothing useful in out) if it
                /// does not fit the schema.
            bool                    encode( const CJsonNode &document, CSjsonBitWriter &out );
                /// Decodes a document written by encode(); the caller owns it.
            CJsonNode              *decode( CSjsonBitReader &in );
            CSjsonDictionary       &dictionary( void ) { return m_dictionary; }

        private:
            class Impl;
            Impl                   *m_pImpl;
            CSjsonDictionary       &m_dictionary;
    };
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSONSCHEMACODEC_H__
