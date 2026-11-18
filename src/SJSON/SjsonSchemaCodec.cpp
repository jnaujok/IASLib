/**
 * SJSON schema-driven encoding. See SjsonSchemaCodec.h.
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

#include "SjsonSchemaCodec.h"
#include "SjsonDictionary.h"

namespace IASLib
{
    class CSjsonSchemaCodec::Impl {};

    CSjsonSchemaCodec::CSjsonSchemaCodec( const CSjsonSchemaModel &model, CSjsonDictionary &dictionary )
        : m_pImpl( NULL ), m_dictionary( dictionary )
    {
        (void)model;
    }

    CSjsonSchemaCodec::~CSjsonSchemaCodec( void )
    {
        delete m_pImpl;
    }

    bool CSjsonSchemaCodec::encode( const CJsonNode &document, CSjsonBitWriter &out )
    {
        (void)document;
        (void)out;
        return false;
    }

    CJsonNode *CSjsonSchemaCodec::decode( CSjsonBitReader &in )
    {
        (void)in;
        throw CSjsonException( SJSON_ERROR_UNSUPPORTED, "schema-encoded packets are not supported yet" );
    }
} // namespace IASLib

#endif // IASLIB_SJSON_SUPPORT__
