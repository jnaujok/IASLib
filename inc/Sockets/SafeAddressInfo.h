/**
 * @file SafeAddressInfo.h
 * @brief CSafeAddressInfo, a deep-copied addrinfo list.
 *
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

#ifndef IASLIB_SAFEADDRESSINFO_H__
#define IASLIB_SAFEADDRESSINFO_H__

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

namespace IASLib
{
        /// One resolved address (a copy of an addrinfo entry), and the ones
        /// after it. The address is held in a sockaddr_storage, so any family
        /// fits.
        ///
        /// Unlike a struct addrinfo list, every copy is deep: it has its own
        /// nodes, so one going out of scope does not spoil another. The first
        /// node owns the rest of the list and deletes it.
    class CSafeAddressInfo : public CObject
    {
        private:
            int flags;                          ///< addrinfo's ai_flags.
            int family;                         ///< The address family (AF_INET, AF_INET6, ...).
            int socktype;                       ///< addrinfo's ai_socktype.
            int protocol;                       ///< addrinfo's ai_protocol.
            socklen_t addrlen;                  ///< How many bytes of storage hold the address.
            struct sockaddr_storage storage;    ///< The address.
            CString canonname;                  ///< addrinfo's ai_canonname ("" if none).
            CSafeAddressInfo *next;             ///< The next address (owned), or NULL.

                /// Copies oSource and the rest of its list (next must be free).
            void            CopyFrom( const CSafeAddressInfo &oSource );

        public:
            DEFINE_OBJECT( CSafeAddressInfo );

                /// A copy of the list pAddr starts.
                /// @param pAddr  The first entry; it must not be NULL.
            CSafeAddressInfo( const struct addrinfo *pAddr );
                /// One address (nLength bytes of it; at most a sockaddr_storage).
                /// @param pAddr  The address (NULL gives an empty AF_UNSPEC entry).
                /// @param nLength  Its length.
                /// @param nSocketType  The socket type to record.
            CSafeAddressInfo( const struct sockaddr *pAddr, socklen_t nLength, int nSocketType = 0 );
                /// A deep copy of oSource and the rest of its list.
            CSafeAddressInfo( const CSafeAddressInfo &oSource );
                /// Deletes the rest of the list.
            ~CSafeAddressInfo( void );

                /// A deep copy of oSource and the rest of its list.
            CSafeAddressInfo &operator =( const CSafeAddressInfo &oSource );

            int getFlags() const { return flags; }              ///< addrinfo's ai_flags.
            int getFamily() const { return family; }            ///< The address family.
            int getSocktype() const { return socktype; }        ///< addrinfo's ai_socktype.
            int getProtocol() const { return protocol; }        ///< addrinfo's ai_protocol.
            socklen_t getAddrLen() const { return addrlen; }    ///< The address's length.

                /// Sets the port of an IPv4 or IPv6 address (other families are left alone).
                /// @param nPort  The port.
            void setPort( int nPort );
                /// The port of an IPv4 or IPv6 address (-1 for another family).
            int getPort( void ) const;

                /// The address.
            struct sockaddr *getAddr() { return (struct sockaddr *)&storage; }
                /// The address.
            const struct sockaddr *getAddr() const { return (const struct sockaddr *)&storage; }

                /// addrinfo's ai_canonname ("" if none).
            CString getCanonname() const { return canonname; }

                /// The next address in the list (owned by this one), or NULL.
            CSafeAddressInfo *getNext() const { return next; }
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SAFEADDRESSINFO_H__
