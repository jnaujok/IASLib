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

    CRemoteAddress::CRemoteAddress( void ) 
    {
        m_sockAddressList = NULL;
        m_bIsValid = false;
    }

    CRemoteAddress::CRemoteAddress( const char *hostname, const char *service, int socketType )
    {
        m_sockAddressList =  CRemoteAddress::resolveAddress(hostname,service,socketType);
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

    CRemoteAddress::CRemoteAddress( const CRemoteAddress &oSource )
    {
        m_bIsValid = oSource.m_bIsValid;
        m_sockAddressList = new CSafeAddressInfo( *oSource.m_sockAddressList );
        m_strHostname = oSource.m_strHostname;
        m_strService = oSource.m_strService;
    }

    CRemoteAddress::~CRemoteAddress()
    {
        if ( m_sockAddressList != NULL )
        {
            delete m_sockAddressList;
        }
    }

    IMPLEMENT_OBJECT( CRemoteAddress, CObject );

    CString CRemoteAddress::toString( void )
    {
        return addressToString(m_sockAddressList->getAddr() );
    }

    CString CRemoteAddress::toStringWithPort( void )
    {
        CString retVal;
        return retVal.Format( "%s:%d", (const char *)toString(), getPort( m_sockAddressList->getAddr() ) );
    }

    void CRemoteAddress::SetAddress( const char *hostname )
    {
        if (m_sockAddressList)
        {
            delete m_sockAddressList;
        }
        m_sockAddressList = CRemoteAddress::resolveAddress( hostname, (const char *)NULL, SOCK_STREAM );
    }

    void CRemoteAddress::SetAddress( const struct sockaddr *sockAddress )
    {
        if (m_sockAddressList)
        {
            delete m_sockAddressList;
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
            return m_sockAddressList->getAddr();
        }

        return NULL;
    }

    struct sockaddr_in *CRemoteAddress::GetIPv4( void ) const
    {
        if (m_bIsValid)
        {
            // look for an IPv4 Address
            CSafeAddressInfo *temp = m_sockAddressList;

            while ( temp )
            {
                if (temp->getFamily() == AF_INET )
                {
                    return (struct sockaddr_in *)(temp->getAddr() );
                }
                temp = temp->getNext();
            }
        }

        return NULL;
    }

    struct sockaddr_in6 *CRemoteAddress::GetIPv6( void ) const
    {
        if (m_bIsValid)
        {
            // look for an IPv6 Address
            CSafeAddressInfo *temp = m_sockAddressList;

            while ( temp )
            {
                if (temp->getFamily() == AF_INET6 )
                {
                    return (struct sockaddr_in6 *)(temp->getAddr() );
                }
                temp = temp->getNext();
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
        return m_sockAddressList->getFamily();
    }

    CString CRemoteAddress::GetFamilyString( void )
    {
        if ( isValid() )
        {
            switch ( m_sockAddressList->getFamily() )
            {
                case AF_INET:
                    return "AF_INET";
                case AF_INET6:
                    return "AF_INET6";
                default:
                    return "AF_RAW";
            }
        }
        return "INVALID";
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

    CSafeAddressInfo *CRemoteAddress::resolveAddress( const char *hostname, const char *service, int socketType )
    {
        CSafeAddressInfo *retVal = NULL;
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

        delete pHints;
        if ( nRet == 0 )
        {
            CSafeAddressInfo *retVal = new CSafeAddressInfo( retList );
        }

        if ( retList != NULL )
        {
            freeaddrinfo( retList );
        }
        
        return retVal;
    }

    CSafeAddressInfo *CRemoteAddress::resolveAddress( const char *hostname, int port, int socketType )
    {
        CSafeAddressInfo *retVal = NULL;
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

        delete pHints;
        if ( nRet == 0 )
        {
            CSafeAddressInfo *retVal = new CSafeAddressInfo( retList );
        }

        if ( retList != NULL )
        {
            freeaddrinfo( retList );
        }
        
        return retVal;
    }

    CRemoteAddress &CRemoteAddress::operator =( const CRemoteAddress &oSource )
    {
        if ( this != &oSource )
        {
            m_bIsValid = oSource.m_bIsValid;
            m_sockAddressList = new CSafeAddressInfo( *oSource.m_sockAddressList );
            m_strHostname = oSource.m_strHostname;
            m_strService = oSource.m_strService;
        }
        return *this;
    }
    
} // namespace IASLib

#endif // IASLIB_NETWORKING__
