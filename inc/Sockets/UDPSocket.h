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
        // An IPv4 UDP socket. Errors are thrown only by the constructors (a
        // CSocketException); Read() and Send() return -1 instead.
    class CUDPSocket : public CObject
    {
        protected:
            SOCKET          m_hSocket;
            int             m_nPort;
            CInternetAddress *m_addrIPAddress;      // never NULL
            CString         m_strAddress;
            CString         m_strSocketName;

            void            Open( int nPort );

        public:
                            // Server socket, bound to nPort (0: any free port)
                            // on every IPv4 address.
	                        CUDPSocket( int nPort );
                            // Client socket, bound to a free port.
                            CUDPSocket( void );
	        virtual        ~CUDPSocket( void );

                            DEFINE_OBJECT( CUDPSocket )

            virtual void setName( const char *name ) { m_strSocketName = name; }
            virtual CString getName( void ) { return m_strSocketName; }
            virtual bool            IsConnected( void ) { return (m_hSocket != NULL_SOCKET ) ? true:false; }
                // Reads one datagram (blocking). Returns its length, or -1 on
                // error. A datagram longer than the buffer is cut short (on
                // every platform). The data is NUL-terminated when it fits in
                // the buffer.
            virtual int             Read( char *pchBuffer, int nBufferSize, CInternetAddress &incomingAddress );
                // As above, but waits at most nTimeoutMs milliseconds. Returns
                // CSocket::READ_TIMEOUT if nothing arrived in time.
            virtual int             Read( char *pchBuffer, int nBufferSize, CInternetAddress &incomingAddress, int nTimeoutMs );
                // Sends one datagram to the target's IPv4 address. Returns the
                // bytes sent, or -1.
            virtual int             Send( const char *pchBuffer, int nBufferSize, const CInternetAddress &targetAddress );
            virtual int             GetPort( void ) { return m_nPort; }
                // The address the socket is bound to.
            virtual const CInternetAddress &GetAddress( void ) const { return *m_addrIPAddress; }
            virtual void            Close( void );

        private:
                                    CUDPSocket( const CUDPSocket & );
            CUDPSocket             &operator =( const CUDPSocket & );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_UDPSOCKET_H__
