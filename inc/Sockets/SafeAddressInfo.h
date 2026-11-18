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
        // One resolved address (a copy of an addrinfo entry), and the ones
        // after it. The address is held in a sockaddr_storage, so any family
        // fits.
    class CSafeAddressInfo : public CObject
    {
        private:
            int flags;
            int family;
            int socktype;
            int protocol;
            socklen_t addrlen;
            struct sockaddr_storage storage;
            CString canonname;
            CSafeAddressInfo *next;

            void            CopyFrom( const CSafeAddressInfo &oSource );

        public:
            DEFINE_OBJECT( CSafeAddressInfo );

                // A copy of the list pAddr starts.
            CSafeAddressInfo( const struct addrinfo *pAddr );
                // One address (nLength bytes of it; at most a sockaddr_storage).
            CSafeAddressInfo( const struct sockaddr *pAddr, socklen_t nLength, int nSocketType = 0 );
            CSafeAddressInfo( const CSafeAddressInfo &oSource );
            ~CSafeAddressInfo( void );

            CSafeAddressInfo &operator =( const CSafeAddressInfo &oSource );

            int getFlags() const { return flags; }
            int getFamily() const { return family; }
            int getSocktype() const { return socktype; }
            int getProtocol() const { return protocol; }
            socklen_t getAddrLen() const { return addrlen; }

            void setPort( int nPort );
            int getPort( void ) const;

            struct sockaddr *getAddr() { return (struct sockaddr *)&storage; }
            const struct sockaddr *getAddr() const { return (const struct sockaddr *)&storage; }

            CString getCanonname() const { return canonname; }

            CSafeAddressInfo *getNext() const { return next; }
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SAFEADDRESSINFO_H__
