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

#ifndef IASLIB_REMOTEADDRESS_H__
#define IASLIB_REMOTEADDRESS_H__

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

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"

namespace IASLib
{
    class CRemoteAddress : public CObject
    {
        protected:
            struct addrinfo   *m_sockAddressList;
            CString     m_strHostname;
            CString     m_strService;
            bool        m_bIsValid;
        public:
                            /// Note that hostname can be an IP or IPv6 address
                            CRemoteAddress( const char *hostname, const char *service = NULL, int socketType = SOCK_STREAM );
                            CRemoteAddress( const char *hostname, int port, int socketType = SOCK_STREAM );
                            CRemoteAddress( const struct sockaddr *address, bool bResolveHost = false );
                            CRemoteAddress( const struct sockaddr_in *address, bool bResolveHost = false );
                            CRemoteAddress( const struct sockaddr_in6 *address, bool bResolveHost = false );
                            CRemoteAddress( const struct addrinfo *addressInfo, bool bResolveHost = false );
	        virtual        ~CRemoteAddress();

                            DEFINE_OBJECT( CRemoteAddress )

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
            static struct addrinfo *resolveAddress( const char *hostname, const char *service = NULL, int socketType = SOCK_STREAM );
            static struct addrinfo *resolveAddress( const char *hostname, int port, int socketType = SOCK_STREAM );
            static int getPort( const struct sockaddr *address );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_REMOTEADDRESS_H__
