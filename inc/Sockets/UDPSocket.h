/**
 *  UDP Socket Class
 *
 *      This base class provides platform independent support for UDP
 * sockets. This is the base class for both the UDP server and client socket
 * classes.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 7/11/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_UDPSOCKET_H__
#define IASLIB_UDPSOCKET_H__

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_WIN32__
    #include <winsock2.h>
    const SOCKET NULL_SOCKET = (SOCKET)NULL;
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
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
    const int NULL_SOCKET = 0;
#endif

#include "../BaseTypes/String_.h"
#include "SocketConfig.h"

namespace IASLib
{
    class CUDPSocket : public CObject
    {
        protected:
    #ifdef IASLIB_WIN32__
            static bool     m_bInitialized;     // Used to assure WSA is initialized in Windoze
            static WSADATA  m_wsaData;          // Data used by Winsock.
    #endif
            SOCKET          m_hSocket;
            int             m_nPort;
            unsigned long   m_addrIPAddress;
            int             m_nLocalPort;
            unsigned long   m_addrLocalIPAddress;
            CString         m_strAddress;
            CString         m_strLocalAddress;
            CString         m_strSocketName;
        public:
                            // Server socket
	                        CUDPSocket( int nPort );
                            // client socket
                            CUDPSocket( void );
	        virtual        ~CUDPSocket( void );

                            DEFINE_OBJECT( CUDPSocket )

            virtual bool            IsConnected( void ) { return (m_hSocket != NULL_SOCKET ) ? true:false; }
            virtual int             Read( char *pchBuffer, int nBufferSize );
            virtual int             Send( const char *pchBuffer, int nBufferSize );
            virtual int             GetPort( void ) { return m_nPort; }
            virtual unsigned long   GetAddress( void ) { return m_addrIPAddress; }
            virtual void            Close( void );

            static unsigned short Htons( unsigned short ushValue ) { return htons( ushValue ); }
            static unsigned long  Htonl( unsigned long ulValue ) { return htonl( ulValue ); }
            static unsigned short Ntohs( unsigned short ushValue ) { return ntohs( ushValue ); }
            static unsigned long  Ntohl( unsigned long ulValue ) { return ntohl( ulValue ); }

            static const char *inet_ntop( int iAddrFamily, unsigned long *addrConvert, char *strBuffer, int nMaxLen );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_UDPSOCKET_H__
