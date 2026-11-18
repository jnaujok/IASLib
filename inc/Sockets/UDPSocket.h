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
        // A UDP socket. By default it takes IPv6 and IPv4 on one socket (IPv4
        // alone on a machine without IPv6). Errors are thrown only by the
        // constructors (a CSocketException); Read() and Send() return -1.
    class CUDPSocket : public CObject
    {
        protected:
            SOCKET          m_hSocket;
            int             m_nPort;
            CInternetAddress *m_addrIPAddress;      // never NULL
            CString         m_strAddress;
            CString         m_strSocketName;
            int             m_nFamily;                  // AF_INET or AF_INET6

            void            Open( int nPort, const char *strBindAddress );

        public:
                            // Server socket, bound to nPort (0: any free port)
                            // on every address.
	                        CUDPSocket( int nPort );
                            // Bound to one address: "0.0.0.0" for every IPv4
                            // address only, "::1", "127.0.0.1", a name...
                            CUDPSocket( int nPort, const char *strBindAddress );
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
                // Sends one datagram to the target (its first address; an
                // IPv4-only socket uses its IPv4 address). Returns the bytes
                // sent, or -1.
            virtual int             Send( const char *pchBuffer, int nBufferSize, const CInternetAddress &targetAddress );
            virtual int             GetPort( void ) { return m_nPort; }
                // Whether the socket is IPv6 (and so takes both families).
            bool                    IsIPv6( void ) const { return m_nFamily == AF_INET6; }
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
