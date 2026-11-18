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
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CSafeAddressInfo, CObject );

    CSafeAddressInfo::CSafeAddressInfo( const struct addrinfo *pAddr )
    {
        next = NULL;
        flags = pAddr->ai_flags;
        family = pAddr->ai_family;
        socktype = pAddr->ai_socktype;
        protocol = pAddr->ai_protocol;
        memset( &storage, 0, sizeof( storage ) );
            // (It allocated a sockaddr_in or sockaddr_in6 and copied
            // ai_addrlen bytes into it, whatever that was.)
        addrlen = (socklen_t)pAddr->ai_addrlen;
        if ( addrlen > (socklen_t)sizeof( storage ) )
            addrlen = (socklen_t)sizeof( storage );
        if ( pAddr->ai_addr )
            memcpy( &storage, pAddr->ai_addr, addrlen );
        else
            addrlen = 0;
        if ( pAddr->ai_canonname )
            canonname = pAddr->ai_canonname;

            // The rest of the list, without recursing.
        CSafeAddressInfo *pLast = this;
        for ( const struct addrinfo *pNext = pAddr->ai_next; pNext; pNext = pNext->ai_next )
        {
            CSafeAddressInfo *pNew = new CSafeAddressInfo( pNext->ai_addr, (socklen_t)pNext->ai_addrlen, pNext->ai_socktype );
            pNew->flags = pNext->ai_flags;
            pNew->family = pNext->ai_family;
            pNew->protocol = pNext->ai_protocol;
            if ( pNext->ai_canonname )
                pNew->canonname = pNext->ai_canonname;
            pLast->next = pNew;
            pLast = pNew;
        }
    }

    CSafeAddressInfo::CSafeAddressInfo( const struct sockaddr *pAddr, socklen_t nLength, int nSocketType )
    {
        next = NULL;
        flags = 0;
        socktype = nSocketType;
        protocol = 0;
        memset( &storage, 0, sizeof( storage ) );
        if ( pAddr == NULL )
            nLength = 0;
        if ( nLength > (socklen_t)sizeof( storage ) )
            nLength = (socklen_t)sizeof( storage );
        addrlen = nLength;
        if ( nLength )
            memcpy( &storage, pAddr, nLength );
        family = ( nLength ) ? storage.ss_family : AF_UNSPEC;
    }

    void CSafeAddressInfo::CopyFrom( const CSafeAddressInfo &oSource )
    {
        flags = oSource.flags;
        family = oSource.family;
        socktype = oSource.socktype;
        protocol = oSource.protocol;
        addrlen = oSource.addrlen;
        memcpy( &storage, &oSource.storage, sizeof( storage ) );
        canonname = oSource.canonname;
        next = NULL;

        CSafeAddressInfo *pLast = this;
        for ( const CSafeAddressInfo *pFrom = oSource.next; pFrom; pFrom = pFrom->next )
        {
            CSafeAddressInfo *pNew = new CSafeAddressInfo( pFrom->getAddr(), pFrom->addrlen, pFrom->socktype );
            pNew->flags = pFrom->flags;
            pNew->family = pFrom->family;
            pNew->protocol = pFrom->protocol;
            pNew->canonname = pFrom->canonname;
            pLast->next = pNew;
            pLast = pNew;
        }
    }

    CSafeAddressInfo::CSafeAddressInfo( const CSafeAddressInfo &oSource ) : CObject()
    {
        CopyFrom( oSource );
    }

    CSafeAddressInfo::~CSafeAddressInfo( void )
    {
            // Unlink the rest of the list and delete it a node at a time, so
            // a long list does not recurse.
        CSafeAddressInfo *pNext = next;
        next = NULL;
        while ( pNext )
        {
            CSafeAddressInfo *pAfter = pNext->next;
            pNext->next = NULL;
            delete pNext;
            pNext = pAfter;
        }
    }

    CSafeAddressInfo &CSafeAddressInfo::operator =( const CSafeAddressInfo &oSource )
    {
        if ( this != &oSource )
        {
            CSafeAddressInfo oCopy( oSource );      // oSource may be in our list
            CSafeAddressInfo *pNext = next;
            next = NULL;
            while ( pNext )
            {
                CSafeAddressInfo *pAfter = pNext->next;
                pNext->next = NULL;
                delete pNext;
                pNext = pAfter;
            }
            CopyFrom( oCopy );
        }
        return *this;
    }

    void CSafeAddressInfo::setPort( int nPort )
    {
        if ( family == AF_INET )
        {
            ((struct sockaddr_in *)&storage)->sin_port = htons( (uint16_t)nPort );
        }
        else if ( family == AF_INET6 )
        {
            ((struct sockaddr_in6 *)&storage)->sin6_port = htons( (uint16_t)nPort );
        }
    }

    int CSafeAddressInfo::getPort( void ) const
    {
        if ( family == AF_INET )
        {
            return ntohs( ((const struct sockaddr_in *)&storage)->sin_port );
        }
        else if ( family == AF_INET6 )
        {
            return ntohs( ((const struct sockaddr_in6 *)&storage)->sin6_port );
        }
        return -1;
    }
}

#endif // IASLIB_NETWORKING__
