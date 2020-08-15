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

#include "InternetAddress.h"

namespace IASLib
{

    CInternetAddress::CInternetAddress( void )
    {
        m_sockAddressList = NULL;
        m_bIsValid = false;
    }

    CInternetAddress::CInternetAddress( const char *hostname, const char *service, int socketType )
    {
        m_sockAddressList =  CInternetAddress::resolveAddress(hostname,service,socketType);
        m_strHostname = hostname;
        m_strService = service;
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CInternetAddress::CInternetAddress( const char *hostname, int port, int socketType )
    {
        m_sockAddressList = resolveAddress(hostname, port, socketType );
        m_strHostname = hostname;
        m_strService.Format( "%d", port );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CInternetAddress::CInternetAddress( const struct sockaddr *address, bool bResolveHost )
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

        m_sockAddressList = CInternetAddress::resolveAddress(m_strHostname, m_strService, SOCK_RAW );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CInternetAddress::CInternetAddress( const struct sockaddr_in *address, bool bResolveHost )
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

        m_sockAddressList = CInternetAddress::resolveAddress(m_strHostname, m_strService, SOCK_RAW );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CInternetAddress::CInternetAddress( const struct sockaddr_in6 *address, bool bResolveHost )
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

        m_sockAddressList = CInternetAddress::resolveAddress(m_strHostname, m_strService, SOCK_RAW );
        m_bIsValid = (m_sockAddressList != NULL );
    }

    CInternetAddress::CInternetAddress( const CInternetAddress &oSource )
    {
        m_bIsValid = oSource.m_bIsValid;
        m_sockAddressList = new CSafeAddressInfo( *oSource.m_sockAddressList );
        m_strHostname = oSource.m_strHostname;
        m_strService = oSource.m_strService;
    }

    CInternetAddress::~CInternetAddress()
    {
        if ( m_sockAddressList != NULL )
        {
            delete m_sockAddressList;
            m_sockAddressList = NULL;
        }
    }

    IMPLEMENT_OBJECT( CInternetAddress, CObject );

    CString CInternetAddress::toString( void )
    {
        return addressToString(m_sockAddressList->getAddr() );
    }

    CString CInternetAddress::toStringWithPort( void )
    {
        CString retVal;
        if ( m_sockAddressList )
        {
            retVal = retVal.Format( "%s:%d", (const char *)toString(), getPort( m_sockAddressList->getAddr() ) );
        }
        else
        {
            retVal = "0.0.0.0:0";
        }
        return retVal;
    }

    void CInternetAddress::SetAddress( const char *hostname )
    {
        if (m_sockAddressList)
        {
            delete m_sockAddressList;
            m_sockAddressList = NULL;
        }
        m_sockAddressList = CInternetAddress::resolveAddress( hostname, (const char *)NULL, SOCK_STREAM );
    }

    void CInternetAddress::SetAddress( const struct sockaddr *sockAddress )
    {
        m_bIsValid = false;
        if (m_sockAddressList)
        {
            delete m_sockAddressList;
            m_sockAddressList = NULL;
        }
        m_strHostname = addressToString( sockAddress );

        m_strService.Format( "%d", getPort(sockAddress) );

        m_sockAddressList = CInternetAddress::resolveAddress(m_strHostname, m_strService, SOCK_RAW );
        m_bIsValid = (m_sockAddressList != NULL );

        // Set the port on all the addresses
        if ( m_bIsValid )
        {
            CSafeAddressInfo *addr = m_sockAddressList;
            while ( addr )
            {
                addr->setPort( getPort(sockAddress) );
                addr = addr->getNext();
            }
        }
    }

    struct sockaddr *CInternetAddress::GetAddress( void ) const
    {
        if ((m_bIsValid) && ( m_sockAddressList ))
        {
            return m_sockAddressList->getAddr();
        }

        return NULL;
    }

    struct sockaddr_in *CInternetAddress::GetIPv4( void ) const
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

    struct sockaddr_in6 *CInternetAddress::GetIPv6( void ) const
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

    CString CInternetAddress::GetHostname( void )
    {
        return m_strHostname;
    }

    sa_family_t CInternetAddress::GetFamily( void )
    {
        return m_sockAddressList->getFamily();
    }

    CString CInternetAddress::GetFamilyString( void )
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

    CString CInternetAddress::addressToString( const struct sockaddr *address )
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

    int CInternetAddress::getPort( const struct sockaddr *address )
    {
        if ( address != NULL )
        {
            switch(address->sa_family) {
                case AF_INET: {
                    const struct sockaddr_in *addr_in = (const struct sockaddr_in *)address;
                    return ntohs((uint16_t)addr_in->sin_port);
                }
                case AF_INET6: {
                    const struct sockaddr_in6 *addr_in6 = (const struct sockaddr_in6 *)address;
                    return ntohs((uint16_t)addr_in6->sin6_port);
                }
                default:
                    break;
            }
        }
        return -1;
    }

    CSafeAddressInfo *CInternetAddress::resolveAddress( const char *hostname, const char *service, int socketType )
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

        int nRet;
        CString strService(service);
        
        if ( ( socketType == SOCK_DGRAM ) || ( strService.IsDigits() ) )
        {
            nRet = getaddrinfo( hostname, NULL, pHints, &retList );
        }
        else
        {
            nRet = getaddrinfo( hostname, service, pHints, &retList );
        }
        

        delete pHints;
        if ( nRet == 0 )
        {
            retVal = new CSafeAddressInfo( retList );
        }

        if ( retList != NULL )
        {
            freeaddrinfo( retList );
        }

        return retVal;
    }

    CSafeAddressInfo *CInternetAddress::resolveAddress( const char *hostname, int port, int socketType )
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
            retVal = new CSafeAddressInfo( retList );
        }

        if ( retList != NULL )
        {
            freeaddrinfo( retList );
        }

        return retVal;
    }

    CInternetAddress &CInternetAddress::operator =( const CInternetAddress &oSource )
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

    CString CInternetAddress::lookupHost( const struct sockaddr *address )
    {
        CString retVal;
        char hostname[NI_MAXHOST];
        char servicename[NI_MAXSERV];
        int flags = NI_NAMEREQD;

        int ret = getnameinfo( address, ( address->sa_family == AF_INET ) ? sizeof(struct sockaddr_in):sizeof(struct sockaddr_in6), hostname, NI_MAXHOST, servicename, NI_MAXSERV, flags );

        if ( ret != 0 )
        {
            flags = NI_NUMERICHOST | NI_NUMERICSERV;
            ret = getnameinfo( address, ( address->sa_family == AF_INET ) ? sizeof(struct sockaddr_in):sizeof(struct sockaddr_in6), hostname, NI_MAXHOST, servicename, NI_MAXSERV, flags );
        }
        retVal = hostname;

        return retVal;
    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__
