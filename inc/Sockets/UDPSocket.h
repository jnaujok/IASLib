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

#include "Socket.h"
#include "InternetAddress.h"

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
            CInternetAddress *m_addrIPAddress;
            CString         m_strAddress;
            CString         m_strSocketName;
        public:
                            // Server socket
	                        CUDPSocket( int nPort );
                            // client socket
                            CUDPSocket( void );
	        virtual        ~CUDPSocket( void );

                            DEFINE_OBJECT( CUDPSocket )

            virtual void setName( const char *name ) { m_strSocketName = name; }
            virtual CString getName( void ) { return m_strSocketName; }
            virtual bool            IsConnected( void ) { return (m_hSocket != NULL_SOCKET ) ? true:false; }
            virtual int             Read( char *pchBuffer, int nBufferSize, CInternetAddress &incomingAddress );
            virtual int             Send( const char *pchBuffer, int nBufferSize, const CInternetAddress &targetAddress );
            virtual int             GetPort( void ) { return m_nPort; }
            virtual const CInternetAddress &GetAddress( void ) const { return *m_addrIPAddress; }
            virtual void            Close( void );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_UDPSOCKET_H__
