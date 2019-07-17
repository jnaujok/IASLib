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
    CRemoteAddress::CRemoteAddress( const char *hostname, const char *service, int socketType )
    {
        m_sockAddressList = CRemoteAddress::resolveAddress(hostname,service,socketType);
        m_strHostname = hostname;
        m_strService = service;
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CRemoteAddress::CRemoteAddress( const char *hostname, int port, int socketType )
    {
        m_sockAddressList = resolveAddress(hostname, port, socketType );
        m_strHostname = hostname;
        m_strService.Format( "%d", port );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CRemoteAddress::CRemoteAddress( const struct sockaddr *address, bool bResolveHost )
    {
        if (bResolveHost)
        {
            m_strHostname = lookupHost( address );
        }
        else
        {
            m_strHostname = addressToString( address );
        }

        m_strService.Format( "%d", getPort(address) );

        m_sockAddressList = CRemoteAddress::resolveAddress(m_strHostname, m_strService, SOCK_RAW );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CRemoteAddress::CRemoteAddress( const struct sockaddr_in *address, bool bResolveHost )
    {
        struct sockaddr temp;
        memcpy( &temp, address, sizeof(struct sockaddr_in) );

        if (bResolveHost)
        {
            m_strHostname = lookupHost( &temp );
        }
        else
        {
            m_strHostname = addressToString( &temp );
        }
        m_strService.Format( "%d", getPort( &temp ));

        m_sockAddressList = CRemoteAddress::resolveAddress(m_strHostname, m_strService, SOCK_RAW );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CRemoteAddress::CRemoteAddress( const struct sockaddr_in6 *address, bool bResolveHost )
    {
        if (bResolveHost)
        {
            m_strHostname = lookupHost( (const struct sockaddr *)address );
        }
        else
        {
            m_strHostname = addressToString( (const struct sockaddr *)address );
        }

        m_strService.Format( "%d", getPort( (const struct sockaddr *)address ));

        m_sockAddressList = CRemoteAddress::resolveAddress(m_strHostname, m_strService, SOCK_RAW );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CRemoteAddress::~CRemoteAddress()
    {
        if ( m_sockAddressList != NULL )
        {
            freeaddrinfo( m_sockAddressList );
        }
    }

    IMPLEMENT_OBJECT( CRemoteAddress, CObject );

    CString CRemoteAddress::toString( void )
    {
        return addressToString(m_sockAddressList->ai_addr);
    }

    CString CRemoteAddress::toStringWithPort( void )
    {
        CString retVal;
        return retVal.Format( "%s:%d", (const char *)toString(), getPort( m_sockAddressList->ai_addr) );
    }

    void CRemoteAddress::SetAddress( const char *hostname )
    {
        if (m_sockAddressList)
        {
            freeaddrinfo(m_sockAddressList);
        }
        m_sockAddressList = CRemoteAddress::resolveAddress( hostname, (const char *)NULL, SOCK_STREAM );
    }

    void CRemoteAddress::SetAddress( const struct sockaddr *sockAddress )
    {
        if (m_sockAddressList)
        {
            freeaddrinfo(m_sockAddressList);
        }
        m_strHostname = addressToString( sockAddress );

        m_strService.Format( "%d", getPort(sockAddress) );

        m_sockAddressList = CRemoteAddress::resolveAddress(m_strHostname, m_strService, SOCK_RAW );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    struct sockaddr *CRemoteAddress::GetAddress( void ) const
    {
        if (m_bIsValid)
        {
            return m_sockAddressList->ai_addr;
        }

        return NULL;
    }

    struct sockaddr_in *CRemoteAddress::GetIPv4( void ) const
    {
        if (m_bIsValid)
        {
            // look for an IPv4 Address
            struct addrinfo *temp = m_sockAddressList;

            while ( temp )
            {
                if (temp->ai_family == AF_INET )
                {
                    return (struct sockaddr_in *)(temp->ai_addr->sa_data);
                }
                temp = temp->ai_next;
            }
        }

        return NULL;
    }

    struct sockaddr_in6 *CRemoteAddress::GetIPv6( void ) const
    {
        if (m_bIsValid)
        {
            // look for an IPv4 Address
            struct addrinfo *temp = m_sockAddressList;

            while ( temp )
            {
                if (temp->ai_family == AF_INET6 )
                {
                    return (struct sockaddr_in6 *)(temp->ai_addr->sa_data);
                }
                temp = temp->ai_next;
            }
        }

        return NULL;

    }

    CString CRemoteAddress::GetHostname( void )
    {
        return m_strHostname;
    }

    sa_family_t CRemoteAddress::GetFamily( void )
    {
        return m_sockAddressList->ai_family;
    }

    CString CRemoteAddress::GetFamilyString( void )
    {
        return "AF_INET";
    }

    CString CRemoteAddress::addressToString( const struct sockaddr *address )
    {
        char *s = NULL;
        switch(address->sa_family) {
            case AF_INET: {
                const struct sockaddr_in *addr_in = (const struct sockaddr_in *)address;
                s = (char *)malloc(INET_ADDRSTRLEN);
                inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
                break;
            }
            case AF_INET6: {
                const struct sockaddr_in6 *addr_in6 = (const struct sockaddr_in6 *)address;
                s = (char *)malloc(INET6_ADDRSTRLEN);
                inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
                break;
            }
            default:
                break;
        }
        CString retVal = s;
        free(s);
        return retVal;
    }

    int CRemoteAddress::getPort( const struct sockaddr *address )
    {
        switch(address->sa_family) {
            case AF_INET: {
                const struct sockaddr_in *addr_in = (const struct sockaddr_in *)address;
                return addr_in->sin_port;
            }
            case AF_INET6: {
                const struct sockaddr_in6 *addr_in6 = (const struct sockaddr_in6 *)address;
                return addr_in6->sin6_port;
            }
            default:
                break;
        }
        return -1;
    }

    struct addrinfo *CRemoteAddress::resolveAddress( const char *hostname, const char *service, int socketType )
    {
        struct addrinfo *pHints = new struct addrinfo();
        memset( pHints, 0, sizeof( struct addrinfo ) );
        struct addrinfo *retList = NULL;

        pHints->ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        pHints->ai_socktype = socketType; /* Datagram socket */
        pHints->ai_flags = AI_PASSIVE;    /* For wildcard IP address */
        pHints->ai_protocol = 0;          /* Any protocol */
        pHints->ai_canonname = NULL;
        pHints->ai_addr = NULL;
        pHints->ai_next = NULL;

        int nRet = getaddrinfo( hostname, service, pHints, &retList );
        if ( nRet == 0 )
        {
            return retList;
        }
        else
        {
            if ( retList != NULL )
            {
                freeaddrinfo( retList );
            }
        }

        return NULL;
    }

    struct addrinfo *CRemoteAddress::resolveAddress( const char *hostname, int port, int socketType )
    {
        struct addrinfo *pHints = new struct addrinfo();
        memset( pHints, 0, sizeof( struct addrinfo ) );
        struct addrinfo *retList = NULL;

        pHints->ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        pHints->ai_socktype = socketType; /* Datagram socket */
        pHints->ai_flags = AI_PASSIVE;    /* For wildcard IP address */
        pHints->ai_protocol = 0;          /* Any protocol */
        pHints->ai_canonname = NULL;
        pHints->ai_addr = NULL;
        pHints->ai_next = NULL;

        CString service;
        service.Format( "%d", port );

        int nRet = getaddrinfo( hostname, service, pHints, &retList );
        if ( nRet == 0 )
        {
            return retList;
        }
        else
        {
            if ( retList != NULL )
            {
                freeaddrinfo( retList );
            }
        }

        return NULL;
    }



} // namespace IASLib

#endif // IASLIB_NETWORKING__
