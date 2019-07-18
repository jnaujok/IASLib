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

namespace IASLib
{
    class CSafeAddressInfo : public CObject
    {
        private:
            int flags;
            int family;
            int socktype;
            int protocol;
            socklen_t addrlen;
            struct sockaddr *addr;
            CString canonname;
            CSafeAddressInfo *next;

        public:
            DEFINE_OBJECT( CSafeAddressInfo );

            CSafeAddressInfo( struct addrinfo *addr );
            CSafeAddressInfo( const CSafeAddressInfo &oSource );
            ~CSafeAddressInfo( void );

            CSafeAddressInfo &operator =( const CSafeAddressInfo &oSource );

            int getFlags()
            {
                return flags;
            }

            int getFamily()
            {
                return family;
            }

            int getSocktype()
            { 
                return socktype;
            }

            int getProtocol()
            {
                return protocol;
            }

            socklen_t getAddrLen()
            {
                return addrlen;
            }

            struct sockaddr *getAddr()
            {
                return addr;
            }

            CString getCanonname()
            {
                return canonname;
            }

            CSafeAddressInfo *getNext()
            {
                return next;
            }
    };
}

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SAFEADDRESSINFO_H__
