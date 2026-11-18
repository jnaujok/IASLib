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
    #include "../BaseTypes/Platform.h"
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

#include <stdint.h>
#include <stdlib.h>
#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "SafeAddressInfo.h"

namespace IASLib
{
        // An internet address: what a host name (or numeric IPv4 or IPv6
        // address) and port resolve to, or a copy of a sockaddr. A name can
        // resolve to more than one address; GetAddress() is the first,
        // GetIPv4() and GetIPv6() the first of each family.
    class CInternetAddress : public CObject
    {
        protected:
            CSafeAddressInfo *m_sockAddressList;
            CString     m_strHostname;
            CString     m_strService;
            bool        m_bIsValid;
            int         m_nSocketType;

            void            Clear( void );
            void            SetFromSockaddr( const struct sockaddr *address, bool bResolveHost );

        public:
                            /// Note that hostname can be an IP or IPv6 address.
                            /// A NULL hostname is the wildcard address (for binding).
                            CInternetAddress( const char *hostname, const char *service = NULL, int socketType = SOCK_STREAM );
                            CInternetAddress( const char *hostname, int port, int socketType = SOCK_STREAM );
                            /// Copies of an address (bResolveHost looks up its name).
                            /// An IPv4-mapped IPv6 address is stored as IPv4.
                            CInternetAddress( const struct sockaddr *address, bool bResolveHost = false );
                            CInternetAddress( const struct sockaddr_in *address, bool bResolveHost = false );
                            CInternetAddress( const struct sockaddr_in6 *address, bool bResolveHost = false );
                            CInternetAddress( const struct addrinfo *addressInfo, bool bResolveHost = false );
                            CInternetAddress( void );
                            CInternetAddress( const CInternetAddress &oSource );
	        virtual        ~CInternetAddress();

                            DEFINE_OBJECT( CInternetAddress )

            CInternetAddress &operator =( const CInternetAddress &oSource );

                // The numeric address ("0.0.0.0" if invalid).
            virtual CString         toString( void ) const;
                // With the port: "1.2.3.4:5060", or "[::1]:5060" for IPv6.
            virtual CString         toStringWithPort( void ) const;

            virtual bool isValid( void ) const { return m_bIsValid; }

                // Resolves a new host name, keeping the port.
            virtual void    SetAddress( const char *hostname );
            virtual void    SetAddress( const struct sockaddr *sockAddress );

            virtual struct sockaddr *GetAddress( void ) const;
                // The length of GetAddress() (0 if invalid).
            socklen_t       GetAddressLength( void ) const;
            virtual struct sockaddr_in *GetIPv4( void ) const;
            virtual struct sockaddr_in6 *GetIPv6( void ) const;
                // The port (-1 if invalid).
            int             GetPort( void ) const;

            virtual CString GetHostname( void ) const;
            virtual sa_family_t GetFamily( void ) const;
            virtual CString GetFamilyString( void ) const;

            virtual operator struct sockaddr *( void ) const { return GetAddress(); }
            virtual operator struct sockaddr_in *(void) const { return GetIPv4(); }
            virtual operator struct sockaddr_in6 *(void) const { return GetIPv6(); }

                // The numeric form of an address ("" for NULL or another family).
            static CString addressToString( const struct sockaddr *address );
                // The name of an address, or its numeric form if it has none.
            static CString lookupHost( const struct sockaddr *address );
            static CSafeAddressInfo *resolveAddress( const char *hostname, const char *service = NULL, int socketType = SOCK_STREAM );
            static CSafeAddressInfo *resolveAddress( const char *hostname, int port, int socketType = SOCK_STREAM );
                // The port of an address (-1 for NULL or another family).
            static int getPort( const struct sockaddr *address );
                // The length of an address of this family.
            static socklen_t addressLength( const struct sockaddr *address );
                // Copies an address into oOut, turning an IPv4-mapped IPv6
                // address (::ffff:1.2.3.4, how a dual-stack socket sees an
                // IPv4 peer) back into IPv4. Returns its length (0 if none).
            static socklen_t unmapAddress( const struct sockaddr *address, struct sockaddr_storage &oOut );
                // An IPv4 address as an IPv4-mapped IPv6 one, for sending
                // from a dual-stack socket.
            static void mapToIPv6( const struct sockaddr_in *address, struct sockaddr_in6 &oOut );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_INTERNETADDRESS_H__
