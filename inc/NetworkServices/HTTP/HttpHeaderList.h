/**
 * @file HttpHeaderList.h
 * @brief The header fields of an HTTP message (CHttpHeaderList).
 *
 *  HTTP Header List Class
 *
 *      This class provides a list of headers and is used by HTTP clients
 * and servers to maintain the header list for requests and responses.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/31/2000
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPHEADERLIST_H__
#define IASLIB_HTTPHEADERLIST_H__

#include "Collections/Hash.h"
#include "Streams/Stream.h"
#include "HttpHeader.h"
#include "NetworkServices/HeaderList.h"

namespace IASLib
{
    /// The header fields of an HTTP request or response.
    ///
    /// It uses the generic CHeaderList rules (case-insensitive names, order
    /// kept, injection-safe setters) and stores each header as a CHttpHeader.
    class CHttpHeaderList : public CHeaderList
    {
        public:
                                /// Parses header lines from the stream, up to and
                                /// including the blank line that ends them (see
                                /// CHeaderList::parse()).
                                CHttpHeaderList( CStream &oInputStream );
                                /// Creates an empty list.
                                CHttpHeaderList( void );
                                /// Deletes the headers.
            virtual            ~CHttpHeaderList( void );

                                DEFINE_OBJECT( CHttpHeaderList );

        protected:
                /// Creates a CHttpHeader for a new header.
            virtual CHeader    *newHeader( const CString &name );
    }; // Class CHttpHeaderList
}  // Namespace IASLib

#endif // IASLIB_HTTPHEADERLIST_H__

#endif // IASLIB_NETWORKING__
