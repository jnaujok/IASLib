/**
 *  Safe Address Info Class
 *
 *      This class encapsulates the standard "c" addrinfo struct into a class
 * that can be safely passed around, copied, and even passed by copy without
 * leaking memory or creating race conditions with potentially awful side 
 * effects. 
 *      In short, all copies of the object are deep copies with their own set
 * of pointers and data, so having one go out of scope doesn't ruin a copy of
 * the data, the same cannot be said for the standard "struct addrinfo" that
 * gets used by preference in networking these days.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 7/17/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "SafeAddressInfo.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSafeAddressInfo, CObject );

    CSafeAddressInfo::CSafeAddressInfo( struct addrinfo *addr )
    {
        flags = addr->ai_flags;
        family = addr->ai_family;
        socktype = addr->ai_socktype;
        protocol = addr->ai_protocol;
        addrlen = addr->ai_addrlen;
        if ( family == AF_INET6 )
            this->addr = (struct sockaddr *)new struct sockaddr_in6();
        else
            this->addr = (struct sockaddr *)new struct sockaddr_in();
        memcpy( this->addr, addr->ai_addr, addrlen );
        canonname = addr->ai_canonname;
        if ( addr->ai_next )
        {
            next = new CSafeAddressInfo( addr->ai_next );
        }
        else
        {
            next = NULL;
        }
    }

    CSafeAddressInfo::CSafeAddressInfo( const CSafeAddressInfo &oSource )
    {
        flags = oSource.flags;
        family = oSource.family;
        socktype = oSource.socktype;
        protocol = oSource.protocol;
        addrlen = oSource.addrlen;
        if ( family == AF_INET6 )
            this->addr = (struct sockaddr *)new struct sockaddr_in6();
        else
            this->addr = (struct sockaddr *)new struct sockaddr_in();
        memcpy( this->addr, oSource.addr, addrlen );
        canonname = oSource.canonname;
        if ( oSource.next )
        {
            next = new CSafeAddressInfo( *oSource.next );
        }
        else
        {
            next = NULL;
        }
    }

    CSafeAddressInfo::~CSafeAddressInfo( void )
    {
        if ( next )
            delete( next );
        delete addr;
    }

    CSafeAddressInfo &CSafeAddressInfo::operator =( const CSafeAddressInfo &oSource )
    {
        if ( this != &oSource )
        {
            flags = oSource.flags;
            family = oSource.family;
            socktype = oSource.socktype;
            protocol = oSource.protocol;
            addrlen = oSource.addrlen;
            if ( family == AF_INET6 )
                this->addr = (struct sockaddr *)new struct sockaddr_in6();
            else
                this->addr = (struct sockaddr *)new struct sockaddr_in();
            memcpy( this->addr, oSource.addr, addrlen );
            canonname = oSource.canonname;
            if ( oSource.next )
            {
                next = new CSafeAddressInfo( *oSource.next );
            }
            else
            {
                next = NULL;
            }
        }
        return *this;
    }

};

#endif // IASLIB_NETWORKING__
