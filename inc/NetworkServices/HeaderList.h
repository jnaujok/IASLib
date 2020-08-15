/**
 * Header List Class 
 *
 * This class encapsulates a a full set of headers for an HTTP or SIP style
 * request. The header list is used on both requests and responses.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 * 
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_HEADERLIST_H__
#define IASLIB_HEADERLIST_H__

#ifdef IASLIB_NETWORKING__

#include "BaseTypes/String_.h"
#include "Collections/Hash.h"
#include "Collections/StringArray.h"
#include "Streams/Stream.h"
#include "NetworkServices/Header.h"

namespace IASLib
{
    class CHeaderList : public CObject
    {
        private:
            CHash   m_hashHeaders;
            bool    m_bModifiable;
        public:
            DEFINE_OBJECT( CHeaderList );

            CHeaderList( CStream &headerStream );
            CHeaderList( CHeaderList &oSource );
            CHeaderList( void );

            virtual ~CHeaderList( void );

            // Returns the header object that matches the given key, or NULL if there
            // is no matching key.
            virtual CHeader *getHeader( const char *key );

            // Add a header to the set of headers. Note that this is only available
            // for a SipHeader object created empty or as a copy of a stream-generated
            // header set.
            virtual void addHeader( const char *key, const char *value );
            virtual void addHeader( const char *key, CStringArray &values );

            virtual void removeHeader( const char *key );

            virtual bool hasHeader( const char *key );

            // Returns an unmodifiable List of all of the header string values
            // of the given named header.
            virtual const CStringArray allValues( const char *name );

            // Returns the first header string value of the given named (and
            // possibly multi-valued) header.
            virtual CString     firstValue( const char *name );

            // Returns the first header string value of the given named (and
            // possibly multi-valued) header.
            virtual long        firstValueAsLong( const char *name );

            // Returns an unmodifiable multi Map view of the entire set of
            // headers in the form Key: CString Value: CStringArray. Note that
            // this hash map must be deleted by the caller.
            virtual CHash      *map( void );

            // Turns the header array into a string with all lines and values,
            // appropriate for sending to a stream.
            virtual CString     toString( void );

            virtual CHeader    *newHeader( CString &name );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__

#endif // IASLIB_HEADERLIST_H__
