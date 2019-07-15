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

#include "Object.h"
#include "String_.h"

namespace IASLib
{
    class CRemoteAddress : public CObject
    {
        protected:
            sockaddr   *m_sockAddress;
            CString     m_strHostname;
            CString     m_strService;
        public:
                            /// Note that hostname can be an IP or IPv6 address
                            CRemoteAddress( const char *hostname, const char *service = NULL );
                            CRemoteAddress( const char *hostname, int port );
                            CRemoteAddress( sockaddr *address );
                            CRemoteAddress( sockaddr_in *address );
	        virtual        ~CRemoteAddress();

                            DEFINE_OBJECT( CRemoteAddress )

            virtual CString         toString( void );
            virtual CString         toStringWithPort( void );

            virtual void    SetAddress( const char *hostname );
            virtual void    SetAddress( sockaddr *sockAddress );

            virtual sockaddr *GetAddress( void );
            virtual sockaddr_in *GetIPv4( void );
            virtual sockaddr_in6 *GetIPv6( void );

            virtual CString GetHostname( void );
            virtual sa_family_t GetFamily( void );
            virtual CString GetFamilyString( void );

            virtual operator sockaddr *( void ) const;
            virtual operator sockaddr_in *(void) const;
            virtual operator sockaddr_in6 *(void) const;
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_REMOTEADDRESS_H__
