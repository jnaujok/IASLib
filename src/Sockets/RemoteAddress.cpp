/**
 *  Remote Address Class
 *
 *      This class encapsulates a remote IP address, either IPv4 or IPv6,
 * including port if provided. This allows for a C++ style of storage of a
 * remote internet address in a nice, simple, easy-to-use class instead of
 * the 1970's vintage sockaddr structures.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 7/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "RemoteAddress.h"

namespace IASLib
{
    CRemoteAddress::CRemoteAddress( const char *hostname, int port )
    {
        m_sockAddress = NULL;
        m_strHostname = hostname;
    }

    CRemoteAddress::CRemoteAddress( sockaddr *address )
    {

    }
                            
    CRemoteAddress::CRemoteAddress( sockaddr_in *address )
    {

    }

    CRemoteAddress::~CRemoteAddress()
    {

    }

    IMPLEMENT_OBJECT( CRemoteAddress, CObject );

    CString CRemoteAddress::toString( void )
    {

    }

    CString CRemoteAddress::toStringWithPort( void )
    {

    }

    void CRemoteAddress::SetAddress( const char *hostname )
    {

    }

    void CRemoteAddress::SetAddress( sockaddr *sockAddress )
    {

    }

    sockaddr *CRemoteAddress::GetAddress( void )
    {

    }
    
    sockaddr_in *CRemoteAddress::GetIPv4( void )
    {

    }

    sockaddr_in6 *CRemoteAddress::GetIPv6( void )
    {

    }

    CString CRemoteAddress::GetHostname( void )
    {
    
    }
    sa_family_t CRemoteAddress::GetFamily( void )
    {

    }

    CString CRemoteAddress::GetFamilyString( void )
    {

    }

    CRemoteAddress::operator sockaddr *( void ) const
    {

    }

    CRemoteAddress::operator sockaddr_in *(void) const
    {

    }

    CRemoteAddress::operator sockaddr_in6 *(void) const
    {

    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__
