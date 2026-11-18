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
#include "Socket.h"
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CInternetAddress, CObject );

    CInternetAddress::CInternetAddress( void )
    {
        m_sockAddressList = NULL;
        m_bIsValid = false;
        m_nSocketType = SOCK_STREAM;
    }

    CInternetAddress::CInternetAddress( const char *hostname, const char *service, int socketType )
    {
        m_nSocketType = socketType;
        m_sockAddressList = resolveAddress( hostname, service, socketType );
        if ( hostname )
            m_strHostname = hostname;
        if ( service )
            m_strService = service;
        m_bIsValid = ( m_sockAddressList != NULL );
    }

    CInternetAddress::CInternetAddress( const char *hostname, int port, int socketType )
    {
        m_nSocketType = socketType;
        m_sockAddressList = resolveAddress( hostname, port, socketType );
        if ( hostname )
            m_strHostname = hostname;
        m_strService.Format( "%d", port );
        m_bIsValid = ( m_sockAddressList != NULL );
    }

    CInternetAddress::CInternetAddress( const struct sockaddr *address, bool bResolveHost )
    {
        m_sockAddressList = NULL;
        m_bIsValid = false;
        m_nSocketType = 0;
        SetFromSockaddr( address, bResolveHost );
    }

    CInternetAddress::CInternetAddress( const struct sockaddr_in *address, bool bResolveHost )
    {
        m_sockAddressList = NULL;
        m_bIsValid = false;
        m_nSocketType = 0;
        SetFromSockaddr( (const struct sockaddr *)address, bResolveHost );
    }

    CInternetAddress::CInternetAddress( const struct sockaddr_in6 *address, bool bResolveHost )
    {
        m_sockAddressList = NULL;
        m_bIsValid = false;
        m_nSocketType = 0;
        SetFromSockaddr( (const struct sockaddr *)address, bResolveHost );
    }

    CInternetAddress::CInternetAddress( const struct addrinfo *addressInfo, bool bResolveHost )
    {
        m_sockAddressList = NULL;
        m_bIsValid = false;
        m_nSocketType = 0;
        if ( addressInfo && addressInfo->ai_addr )
        {
            m_sockAddressList = new CSafeAddressInfo( addressInfo );
            m_nSocketType = addressInfo->ai_socktype;
            m_strHostname = ( bResolveHost ) ? lookupHost( addressInfo->ai_addr ) : addressToString( addressInfo->ai_addr );
            m_strService.Format( "%d", getPort( addressInfo->ai_addr ) );
            m_bIsValid = true;
        }
    }

    CInternetAddress::CInternetAddress( const CInternetAddress &oSource ) : CObject()
    {
        m_bIsValid = oSource.m_bIsValid;
        m_nSocketType = oSource.m_nSocketType;
        m_sockAddressList = ( oSource.m_sockAddressList ) ? new CSafeAddressInfo( *oSource.m_sockAddressList ) : NULL;
        m_strHostname = oSource.m_strHostname;
        m_strService = oSource.m_strService;
    }

    CInternetAddress::~CInternetAddress()
    {
        Clear();
    }

    CInternetAddress &CInternetAddress::operator =( const CInternetAddress &oSource )
    {
        if ( this != &oSource )
        {
            CSafeAddressInfo *pCopy = ( oSource.m_sockAddressList ) ? new CSafeAddressInfo( *oSource.m_sockAddressList ) : NULL;
            Clear();
            m_sockAddressList = pCopy;
            m_bIsValid = oSource.m_bIsValid;
            m_nSocketType = oSource.m_nSocketType;
            m_strHostname = oSource.m_strHostname;
            m_strService = oSource.m_strService;
        }
        return *this;
    }

    void CInternetAddress::Clear( void )
    {
        delete m_sockAddressList;
        m_sockAddressList = NULL;
        m_bIsValid = false;
    }

        // (The sockaddr constructors used to turn the address into text and
        // resolve that again as SOCK_RAW, which getaddrinfo() refuses with a
        // port on Linux; the address is now just copied.)
    void CInternetAddress::SetFromSockaddr( const struct sockaddr *address, bool bResolveHost )
    {
        Clear();
        m_strHostname = "";
        m_strService = "";
        socklen_t nLength = addressLength( address );
        if ( nLength == 0 )
            return;

        m_sockAddressList = new CSafeAddressInfo( address, nLength, m_nSocketType );
        m_strHostname = ( bResolveHost ) ? lookupHost( address ) : addressToString( address );
        m_strService.Format( "%d", getPort( address ) );
        m_bIsValid = true;
    }

    CString CInternetAddress::toString( void ) const
    {
        if ( m_sockAddressList == NULL )
        {
            return CString( "0.0.0.0" );
        }
        return addressToString( m_sockAddressList->getAddr() );
    }

    CString CInternetAddress::toStringWithPort( void ) const
    {
        CString retVal;
        if ( m_sockAddressList )
        {
            if ( m_sockAddressList->getFamily() == AF_INET6 )
                retVal.Format( "[%s]:%d", (const char *)toString(), m_sockAddressList->getPort() );
            else
                retVal.Format( "%s:%d", (const char *)toString(), m_sockAddressList->getPort() );
        }
        else
        {
            retVal = "0.0.0.0:0";
        }
        return retVal;
    }

    void CInternetAddress::SetAddress( const char *hostname )
    {
            // (It resolved without the port, and left the validity flag and
            // host name as they were.)
        Clear();
        m_strHostname = ( hostname ) ? hostname : "";
        m_sockAddressList = resolveAddress( hostname, ( m_strService.GetLength() ) ? (const char *)m_strService : NULL,
                                            ( m_nSocketType ) ? m_nSocketType : SOCK_STREAM );
        m_bIsValid = ( m_sockAddressList != NULL );
    }

    void CInternetAddress::SetAddress( const struct sockaddr *sockAddress )
    {
        SetFromSockaddr( sockAddress, false );
    }

    struct sockaddr *CInternetAddress::GetAddress( void ) const
    {
        if ( ( m_bIsValid ) && ( m_sockAddressList ) )
        {
            return m_sockAddressList->getAddr();
        }
        return NULL;
    }

    socklen_t CInternetAddress::GetAddressLength( void ) const
    {
        if ( ( m_bIsValid ) && ( m_sockAddressList ) )
        {
            return m_sockAddressList->getAddrLen();
        }
        return 0;
    }

    int CInternetAddress::GetPort( void ) const
    {
        return getPort( GetAddress() );
    }

    struct sockaddr_in *CInternetAddress::GetIPv4( void ) const
    {
        if ( m_bIsValid )
        {
            for ( CSafeAddressInfo *temp = m_sockAddressList; temp; temp = temp->getNext() )
            {
                if ( temp->getFamily() == AF_INET )
                {
                    return (struct sockaddr_in *)( temp->getAddr() );
                }
            }
        }
        return NULL;
    }

    struct sockaddr_in6 *CInternetAddress::GetIPv6( void ) const
    {
        if ( m_bIsValid )
        {
            for ( CSafeAddressInfo *temp = m_sockAddressList; temp; temp = temp->getNext() )
            {
                if ( temp->getFamily() == AF_INET6 )
                {
                    return (struct sockaddr_in6 *)( temp->getAddr() );
                }
            }
        }
        return NULL;
    }

    CString CInternetAddress::GetHostname( void ) const
    {
        return m_strHostname;
    }

    sa_family_t CInternetAddress::GetFamily( void ) const
    {
        if ( m_sockAddressList == NULL )
        {
            return AF_UNSPEC;
        }
        return (sa_family_t)m_sockAddressList->getFamily();
    }

    CString CInternetAddress::GetFamilyString( void ) const
    {
        if ( m_bIsValid && m_sockAddressList )
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

    socklen_t CInternetAddress::addressLength( const struct sockaddr *address )
    {
        if ( address == NULL )
            return 0;
        switch ( address->sa_family )
        {
            case AF_INET:
                return (socklen_t)sizeof( struct sockaddr_in );
            case AF_INET6:
                return (socklen_t)sizeof( struct sockaddr_in6 );
            default:
                return 0;
        }
    }

    CString CInternetAddress::addressToString( const struct sockaddr *address )
    {
        char strBuffer[ INET6_ADDRSTRLEN + 1 ];
        strBuffer[0] = '\0';
        if ( address != NULL )
        {
            switch ( address->sa_family )
            {
                case AF_INET:
                    if ( inet_ntop( AF_INET, (void *)&( ((const struct sockaddr_in *)address)->sin_addr ), strBuffer, sizeof( strBuffer ) ) == NULL )
                        strBuffer[0] = '\0';
                    break;
                case AF_INET6:
                    if ( inet_ntop( AF_INET6, (void *)&( ((const struct sockaddr_in6 *)address)->sin6_addr ), strBuffer, sizeof( strBuffer ) ) == NULL )
                        strBuffer[0] = '\0';
                    break;
                default:
                    break;
            }
        }
        return CString( strBuffer );
    }

    int CInternetAddress::getPort( const struct sockaddr *address )
    {
        if ( address != NULL )
        {
            switch ( address->sa_family )
            {
                case AF_INET:
                    return ntohs( (uint16_t)((const struct sockaddr_in *)address)->sin_port );
                case AF_INET6:
                    return ntohs( (uint16_t)((const struct sockaddr_in6 *)address)->sin6_port );
                default:
                    break;
            }
        }
        return -1;
    }

    CSafeAddressInfo *CInternetAddress::resolveAddress( const char *hostname, const char *service, int socketType )
    {
            // Lookups need Winsock started, even before any socket exists.
        CSocket::InitializeNetworking();

        struct addrinfo hints;
        memset( &hints, 0, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6
        hints.ai_socktype = socketType;
        hints.ai_flags = AI_PASSIVE;        // a NULL host is the wildcard address

            // (Numeric services and every datagram lookup used to drop the
            // service, so the port was lost.) getaddrinfo() needs one of the
            // two.
        if ( hostname == NULL && service == NULL )
            service = "0";

        struct addrinfo *retList = NULL;
        CSafeAddressInfo *retVal = NULL;
        if ( getaddrinfo( hostname, service, &hints, &retList ) == 0 && retList != NULL )
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
        CString service;
        service.Format( "%d", port );
        return resolveAddress( hostname, (const char *)service, socketType );
    }

    CString CInternetAddress::lookupHost( const struct sockaddr *address )
    {
        socklen_t nLength = addressLength( address );
        if ( nLength == 0 )
            return CString( "" );

        CSocket::InitializeNetworking();
        char hostname[ NI_MAXHOST ];
        hostname[0] = '\0';
        if ( getnameinfo( address, nLength, hostname, NI_MAXHOST, NULL, 0, NI_NAMEREQD ) != 0 &&
             getnameinfo( address, nLength, hostname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST ) != 0 )
        {
            hostname[0] = '\0';
        }
        return CString( hostname );
    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__
