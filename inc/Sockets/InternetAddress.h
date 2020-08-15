/**
 * Internet Address Class
 *
 *      This class encapsulates an internet IP address, either IPv4 or IPv6,
 * including port if provided. This allows for a C++ style of storage of a
 * internet address in a nice, simple, easy-to-use class instead of the 
 * 1970's vintage sockaddr structures.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 7/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_INTERNETADDRESS_H__
#define IASLIB_INTERNETADDRESS_H__

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_WIN32__
    #include <winsock2.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#ifndef IASLIB_DEC__
    #include <inttypes.h>
#endif
    #include <unistd.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "SafeAddressInfo.h"

namespace IASLib
{
    class CInternetAddress : public CObject
    {
        protected:
            CSafeAddressInfo *m_sockAddressList;
            CString     m_strHostname;
            CString     m_strService;
            bool        m_bIsValid;
        public:
                            /// Note that hostname can be an IP or IPv6 address
                            CInternetAddress( const char *hostname, const char *service = NULL, int socketType = SOCK_STREAM );
                            CInternetAddress( const char *hostname, int port, int socketType = SOCK_STREAM );
                            CInternetAddress( const struct sockaddr *address, bool bResolveHost = false );
                            CInternetAddress( const struct sockaddr_in *address, bool bResolveHost = false );
                            CInternetAddress( const struct sockaddr_in6 *address, bool bResolveHost = false );
                            CInternetAddress( const struct addrinfo *addressInfo, bool bResolveHost = false );
                            CInternetAddress( void );
                            CInternetAddress( const CInternetAddress &oSource );
	        virtual        ~CInternetAddress();

                            DEFINE_OBJECT( CInternetAddress )

            CInternetAddress &operator =( const CInternetAddress &oSource );

            virtual CString         toString( void );
            virtual CString         toStringWithPort( void );

            virtual bool isValid( void ) { return m_bIsValid; }

            virtual void    SetAddress( const char *hostname );
            virtual void    SetAddress( const struct sockaddr *sockAddress );

            virtual struct sockaddr *GetAddress( void ) const;
            virtual struct sockaddr_in *GetIPv4( void ) const;
            virtual struct sockaddr_in6 *GetIPv6( void ) const;

            virtual CString GetHostname( void );
            virtual sa_family_t GetFamily( void );
            virtual CString GetFamilyString( void );

            virtual operator struct sockaddr *( void ) const { return GetAddress(); }
            virtual operator struct sockaddr_in *(void) const { return GetIPv4(); }
            virtual operator struct sockaddr_in6 *(void) const { return GetIPv6(); }

            static CString addressToString( const struct sockaddr *address );
            static CString lookupHost( const struct sockaddr *address );
            static CSafeAddressInfo *resolveAddress( const char *hostname, const char *service = NULL, int socketType = SOCK_STREAM );
            static CSafeAddressInfo *resolveAddress( const char *hostname, int port, int socketType = SOCK_STREAM );
            static int getPort( const struct sockaddr *address );

            virtual bool isValid( void ) const { return m_bIsValid; }
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_INTERNETADDRESS_H__
