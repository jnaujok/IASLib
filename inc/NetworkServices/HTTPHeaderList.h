/**
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

#ifndef IASLIB_HTTPHEADERLIST_H__
#define IASLIB_HTTPHEADERLIST_H__

#include "../Collections/Hash.h"
#include "../Streams/Stream.h"
#include "HTTPHeader.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{
    class CHTTPHeaderList : public CHash
    {
        public:
                                CHTTPHeaderList( CStream &oInputStream );
                                CHTTPHeaderList( void );
            virtual            ~CHTTPHeaderList( void );

                                DEFINE_OBJECT( CHTTPHeaderList );
            CString             GetValue( const char *strHeaderName );
            CHTTPHeader        *GetHeader( const char *strHeaderName );
            virtual CIterator  *Enumerate( void );
    }; // Class CHTTPHeaderList
}  // Namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_HTTPHEADERLIST_H__

