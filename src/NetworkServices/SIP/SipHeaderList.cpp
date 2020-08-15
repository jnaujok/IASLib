/**
 *  SIP Header List Class
 *
 *      This class provides a list of headers and is used by SIP clients
 * and servers to maintain the header list for requests and responses.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/22/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipHeaderList.h"

namespace IASLib
{

    IMPLEMENT_OBJECT( CSipHeaderList, CHeaderList );

    CHeader *CSipHeaderList::newHeader( const CString &name )
    {
        return new CSipHeader( name );
    }

    CSipHeaderList::CSipHeaderList( CStream &oInputStream ) : CHeaderList( oInputStream )
    {
    }

    CSipHeaderList::CSipHeaderList( void ) : CHeaderList()
    {

    }

    CSipHeaderList::~CSipHeaderList( void )
    {
        // Nothing to do, handled by the base class.
    }

    CIterator *CSipHeaderList::Enumerate( void )
    {
        // TODO: Fix this
        return NULL;
    }

} // Namespace IASLib

#endif // IASLIB_NETWORKING__
