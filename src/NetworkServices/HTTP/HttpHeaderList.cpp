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
#include "HttpHeaderList.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{

    IMPLEMENT_OBJECT( CHttpHeaderList, CHeaderList );

    CHeader *CHttpHeaderList::newHeader( const CString &name )
    {
        return new CHttpHeader( name );
    }

    CHttpHeaderList::CHttpHeaderList( CStream &oInputStream ) : CHeaderList( oInputStream )
    {
    }

    CHttpHeaderList::CHttpHeaderList( void ) : CHeaderList()
    {

    }

    CHttpHeaderList::~CHttpHeaderList( void )
    {
        // Nothing to do, handled by the base class.
    }

    CIterator *CHttpHeaderList::Enumerate( void )
    {
        // TODO: Fix this
        return NULL;
    }

} // Namespace IASLib

#endif // IASLIB_NETWORKING__
