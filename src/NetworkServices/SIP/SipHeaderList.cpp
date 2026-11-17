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

    CSipHeaderList::CSipHeaderList( CStream &oInputStream ) : CHeaderList()
    {
        m_bModifiable = false;
            // Parse here rather than in the base constructor so that our
            // canonicalName() override is the one that gets used.
        parse( oInputStream );
    }

    CSipHeaderList::CSipHeaderList( void ) : CHeaderList()
    {
    }

    CSipHeaderList::~CSipHeaderList( void )
    {
        // Nothing to do, handled by the base class.
    }

    CString CSipHeaderList::canonicalName( const CString &name )
    {
        return expandCompactForm( name );
    }

    CString CSipHeaderList::expandCompactForm( const CString &name )
    {
        if ( name.GetLength() != 1 )
        {
            return name;
        }

        switch ( name[0] )
        {
            case 'a': case 'A': return CString( "Accept-Contact" );
            case 'b': case 'B': return CString( "Referred-By" );
            case 'c': case 'C': return CString( "Content-Type" );
            case 'd': case 'D': return CString( "Request-Disposition" );
            case 'e': case 'E': return CString( "Content-Encoding" );
            case 'f': case 'F': return CString( "From" );
            case 'i': case 'I': return CString( "Call-ID" );
            case 'j': case 'J': return CString( "Reject-Contact" );
            case 'k': case 'K': return CString( "Supported" );
            case 'l': case 'L': return CString( "Content-Length" );
            case 'm': case 'M': return CString( "Contact" );
            case 'n': case 'N': return CString( "Identity-Info" );
            case 'o': case 'O': return CString( "Event" );
            case 'r': case 'R': return CString( "Refer-To" );
            case 's': case 'S': return CString( "Subject" );
            case 't': case 'T': return CString( "To" );
            case 'u': case 'U': return CString( "Allow-Events" );
            case 'v': case 'V': return CString( "Via" );
            case 'x': case 'X': return CString( "Session-Expires" );
            case 'y': case 'Y': return CString( "Identity" );
            default: break;
        }
        return name;
    }
} // Namespace IASLib

#endif // IASLIB_NETWORKING__
