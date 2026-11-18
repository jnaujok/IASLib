/**
 * @file SjsonCodec.h
 * @brief SJSON value encodings: numbers, strings, and the tagged encoding of
 *        whole JSON trees.
 *
 *  Every value in the tagged encoding starts with a 3-bit type code:
 *
 * | code | JSON type | payload |
 * |:----:|:----------|:--------|
 * | 000 | null | none |
 * | 001 | boolean | 1 bit |
 * | 010 | number | 2-bit sub-type: integer, float32, float64 or decimal |
 * | 011 | string | plain UTF-8 or one of the special forms |
 * | 100 | array | the items, then `110` |
 * | 101 | object | (NybbleField tag id + value) per member, then tag id 0 |
 *
 *  The encoder computes every form that applies to a value and keeps the
 * shortest, making the same choices as the reference Python library, so the
 * two write identical bytes for the same document.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SJSONCODEC_H__
#define IASLIB_SJSONCODEC_H__

#ifdef IASLIB_SJSON_SUPPORT__

#include "SjsonBits.h"
#include "SjsonDictionary.h"

#include <string>

namespace IASLib
{
    class CJsonNode;

        /// The JSON value kinds SJSON distinguishes (as Python does: an
        /// integer and a float are different even when equal).
    enum SjsonKind
    {
        SJSON_KIND_NULL,
        SJSON_KIND_BOOL,
        SJSON_KIND_INT,
        SJSON_KIND_FLOAT,
        SJSON_KIND_STRING,
        SJSON_KIND_OBJECT,
        SJSON_KIND_ARRAY
    };

    namespace SjsonCodec
    {
            /// @name Type codes
            ///@{
        extern const char *const NODE_NULL;
        extern const char *const NODE_BOOLEAN;
        extern const char *const NODE_NUMBER;
        extern const char *const NODE_STRING;
        extern const char *const NODE_ARRAY;
        extern const char *const NODE_OBJECT;
        extern const char *const END_OF_ARRAY;
        extern const char *const END_OF_MEMBERS;
            ///@}

            /// The kind of a node (NULL and the missing node are null; binary
            /// nodes are strings holding their base64 text).
        SjsonKind           kindOf( const CJsonNode *pNode );
            /// An integer node's value as decimal text.
        std::string         integerText( const CJsonNode &node );
            /// A string node's text (a binary node's base64 text).
        std::string         stringText( const CJsonNode &node );
            /// A new number node for a decimal integer of any size.
        CJsonNode          *makeInteger( const std::string &strDecimal );
            /// Strict deep equality: kinds must match (1 is not 1.0, 0.0 is
            /// not -0.0, true is not 1) and object members must be in the
            /// same order.
        bool                sameValue( const CJsonNode *pA, const CJsonNode *pB );

            /// Options that steer encoder choices.
        struct Options
        {
            bool            bLz4;           ///< Try the LZ4 string form (needs liblz4).
            Options( void ) : bLz4( true ) {}
        };
            /// The options in effect (thread-local defaults; set by CSjson).
        Options            &options( void );

            /// Writes a number's payload (no type code).
        void                encodeNumber( const CJsonNode &node, CSjsonBitWriter &out );
            /// Writes a float's payload.
        void                encodeFloat( double dValue, CSjsonBitWriter &out );
            /// Writes an integer's payload from decimal text.
        void                encodeInteger( const std::string &strDecimal, CSjsonBitWriter &out );
            /// Reads a number payload into a new node.
        CJsonNode          *decodeNumber( CSjsonBitReader &in );

            /// Writes a string payload (no type code), choosing the shortest
            /// form. pDictionary may be NULL (no interning).
        void                encodeString( const std::string &strText, CSjsonDictionary *pDictionary, CSjsonBitWriter &out );
            /// Reads a string payload.
        std::string         decodeString( CSjsonBitReader &in, CSjsonDictionary *pDictionary );

            /// Writes a whole tree in the tagged encoding.
        void                encodeTagged( const CJsonNode *pNode, CSjsonDictionary &dictionary, CSjsonBitWriter &out );
            /// Reads a tree written by encodeTagged().
        CJsonNode          *decodeTagged( CSjsonBitReader &in, CSjsonDictionary &dictionary );
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
#endif // IASLIB_SJSONCODEC_H__
