/**
 *  Socket Class
 *
 *      This base class provides platform independent support for TCP/IP
 * sockets. This is the base class for both the server and client socket
 * classes.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/02/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SOCKET_H__
#define IASLIB_SOCKET_H__

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

namespace IASLib
{
    class CSocket : public CObject
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
                            CSocket( SOCKET hSocket, const char *strSockName, void *AddressIn=NULL );
	                        CSocket( int nPort, bool bBlocking = true );
                            CSocket( const char *strConnectTo, int nPort );
	        virtual        ~CSocket();

                            DEFINE_OBJECT( CSocket )

            virtual bool            IsConnected( void ) { return (m_hSocket != NULL_SOCKET ) ? true:false; }
            virtual bool            IsHandshakeComplete( void ) { return IsConnected(); }
            virtual unsigned char   Read( void );
            virtual int             Read( char *pchBuffer, int nBufferSize );
            virtual int             Send( unsigned char chSend );
            virtual int             Send( const char *pchBuffer, int nBufferSize );
            virtual int             GetPort( void ) { return m_nPort; }
            virtual unsigned long   GetAddress( void ) { return m_addrIPAddress; }
            virtual const char     *GetAddressString( bool bRemoteAddress=true, bool bIncludePort=false );
            virtual void            Close( void );
            virtual void            SetNonBlocking( bool bDontBlock );
            virtual bool            HasData( void );

            static unsigned short Htons( unsigned short ushValue ) { return htons( ushValue ); }
            static unsigned long  Htonl( unsigned long ulValue ) { return htonl( ulValue ); }
            static unsigned short Ntohs( unsigned short ushValue ) { return ntohs( ushValue ); }
            static unsigned long  Ntohl( unsigned long ulValue ) { return ntohl( ulValue ); }

            static const char *inet_ntop( int iAddrFamily, unsigned long *addrConvert, char *strBuffer, int nMaxLen );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_SOCKET_H__
