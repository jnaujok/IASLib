/**
 * @file InternetAddress.h
 * @brief CInternetAddress, an IPv4 or IPv6 address and port.
 *
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
        /// An internet address: what a host name (or numeric IPv4 or IPv6
        /// address) and port resolve to, or a copy of a sockaddr.
        ///
        /// A name can resolve to more than one address; GetAddress() is the
        /// first, GetIPv4() and GetIPv6() the first of each family. A name
        /// that cannot be resolved gives an invalid address (isValid() is
        /// false); nothing here throws. Copies are deep. The pointers the
        /// getters return belong to the object and are good until it changes.
        ///
        /// @code
        /// CInternetAddress oAddress( "localhost", 5060, SOCK_DGRAM );
        /// if ( oAddress.isValid() )
        ///     printf( "%s\n", (const char *)oAddress.toStringWithPort() );
        /// @endcode
    class CInternetAddress : public CObject
    {
        protected:
            CSafeAddressInfo *m_sockAddressList;    ///< The resolved addresses (owned), or NULL.
            CString     m_strHostname;              ///< The host name it was made from (or the numeric address).
            CString     m_strService;               ///< The service or port, as text.
            bool        m_bIsValid;                 ///< Whether there is an address.
            int         m_nSocketType;              ///< SOCK_STREAM, SOCK_DGRAM, or 0 if unknown.

                /// Forgets the addresses, leaving the object invalid.
            void            Clear( void );
                /// Copies an address (unmapping an IPv4-mapped one).
                /// @note The sockaddr constructors used to turn the address into
                ///       text and resolve that again as SOCK_RAW, which
                ///       getaddrinfo() refuses with a port on Linux; the address
                ///       is now just copied.
            void            SetFromSockaddr( const struct sockaddr *address, bool bResolveHost );

        public:
                            /// Resolves a host name and service.
                            ///
                            /// Note that hostname can be an IP or IPv6 address.
                            /// A NULL hostname is the wildcard address (for binding).
                            /// @param hostname  The host, or NULL.
                            /// @param service  A service name or port number as text, or NULL.
                            /// @param socketType  SOCK_STREAM or SOCK_DGRAM.
                            CInternetAddress( const char *hostname, const char *service = NULL, int socketType = SOCK_STREAM );
                            /// Same, with a numeric port.
                            /// @param hostname  The host, or NULL.
                            /// @param port  The port.
                            /// @param socketType  SOCK_STREAM or SOCK_DGRAM.
                            CInternetAddress( const char *hostname, int port, int socketType = SOCK_STREAM );
                            /// Copies of an address (bResolveHost looks up its name).
                            /// An IPv4-mapped IPv6 address is stored as IPv4.
                            /// @param address  An AF_INET or AF_INET6 address; any
                            ///                 other (or NULL) gives an invalid address.
                            /// @param bResolveHost  Look up the host name (otherwise
                            ///                      GetHostname() is the numeric address).
                            CInternetAddress( const struct sockaddr *address, bool bResolveHost = false );
                            /// @copydoc CInternetAddress(const struct sockaddr *,bool)
                            CInternetAddress( const struct sockaddr_in *address, bool bResolveHost = false );
                            /// @copydoc CInternetAddress(const struct sockaddr *,bool)
                            CInternetAddress( const struct sockaddr_in6 *address, bool bResolveHost = false );
                            /// Copies a whole addrinfo list.
                            /// @param addressInfo  The list (NULL gives an invalid address).
                            /// @param bResolveHost  Look up the first address's host name.
                            CInternetAddress( const struct addrinfo *addressInfo, bool bResolveHost = false );
                            /// An invalid (empty) address.
                            CInternetAddress( void );
                            /// A deep copy.
                            CInternetAddress( const CInternetAddress &oSource );
                            /// Frees the addresses.
	        virtual        ~CInternetAddress();

                            DEFINE_OBJECT( CInternetAddress )

                /// A deep copy.
            CInternetAddress &operator =( const CInternetAddress &oSource );

                /// The numeric address ("0.0.0.0" if invalid).
            virtual CString         toString( void ) const;
                /// With the port: "1.2.3.4:5060", or "[::1]:5060" for IPv6
                /// ("0.0.0.0:0" if invalid).
            virtual CString         toStringWithPort( void ) const;

                /// Whether there is an address.
            virtual bool isValid( void ) const { return m_bIsValid; }

                /// Resolves a new host name, keeping the port.
                /// @param hostname  The host, or NULL for the wildcard address.
                /// @note It used to resolve without the port, and leave the
                ///       validity flag and host name as they were.
            virtual void    SetAddress( const char *hostname );
                /// Copies an address, as the sockaddr constructor does.
                /// @param sockAddress  The address.
            virtual void    SetAddress( const struct sockaddr *sockAddress );

                /// The first address, or NULL if invalid.
            virtual struct sockaddr *GetAddress( void ) const;
                /// The length of GetAddress() (0 if invalid).
            socklen_t       GetAddressLength( void ) const;
                /// The first IPv4 address, or NULL if there is none.
            virtual struct sockaddr_in *GetIPv4( void ) const;
                /// The first IPv6 address, or NULL if there is none.
            virtual struct sockaddr_in6 *GetIPv6( void ) const;
                /// The port (-1 if invalid).
            int             GetPort( void ) const;

                /// The host name it was made from (or the numeric address).
            virtual CString GetHostname( void ) const;
                /// The first address's family (AF_UNSPEC if there is none).
            virtual sa_family_t GetFamily( void ) const;
                /// The family as text: "AF_INET", "AF_INET6", "AF_RAW" (any
                /// other) or "INVALID".
            virtual CString GetFamilyString( void ) const;

                /// The same as GetAddress().
            virtual operator struct sockaddr *( void ) const { return GetAddress(); }
                /// The same as GetIPv4().
            virtual operator struct sockaddr_in *(void) const { return GetIPv4(); }
                /// The same as GetIPv6().
            virtual operator struct sockaddr_in6 *(void) const { return GetIPv6(); }

                /// The numeric form of an address ("" for NULL or another family).
            static CString addressToString( const struct sockaddr *address );
                /// The name of an address, or its numeric form if it has none
                /// ("" for NULL or another family).
            static CString lookupHost( const struct sockaddr *address );
                /// Resolves a host and service with getaddrinfo() (IPv4 and IPv6).
                /// @param hostname  The host, or NULL for the wildcard address.
                /// @param service  A service name or port as text, or NULL.
                /// @param socketType  SOCK_STREAM or SOCK_DGRAM.
                /// @return The list, which the caller owns, or NULL if it cannot
                ///         be resolved.
            static CSafeAddressInfo *resolveAddress( const char *hostname, const char *service = NULL, int socketType = SOCK_STREAM );
                /// Same, with a numeric port.
            static CSafeAddressInfo *resolveAddress( const char *hostname, int port, int socketType = SOCK_STREAM );
                /// The port of an address (-1 for NULL or another family).
            static int getPort( const struct sockaddr *address );
                /// The length of an address of this family (0 for NULL or
                /// another family).
            static socklen_t addressLength( const struct sockaddr *address );
                /// Copies an address into oOut, turning an IPv4-mapped IPv6
                /// address (\::ffff:1.2.3.4, how a dual-stack socket sees an
                /// IPv4 peer) back into IPv4.
                /// @param address  The address.
                /// @param oOut  Where to copy it (zeroed first).
                /// @return Its length (0 if none).
            static socklen_t unmapAddress( const struct sockaddr *address, struct sockaddr_storage &oOut );
                /// An IPv4 address as an IPv4-mapped IPv6 one, for sending
                /// from a dual-stack socket.
                /// @param address  The IPv4 address (NULL leaves oOut an empty IPv6 address).
                /// @param oOut  The result.
            static void mapToIPv6( const struct sockaddr_in *address, struct sockaddr_in6 &oOut );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_INTERNETADDRESS_H__
