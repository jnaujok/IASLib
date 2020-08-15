/**
 * Generic Server Class
 *
 * This class defines a generic, abstract base class for supporting
 * socket-based servers that can receive requests and return responses
 * to the requesting party.
 *
 * This Request/Response paradigm is inherent in many protocols used
 * in modern computing, such as HTTP and SIP. By creating a base class
 * this commonality can be exploited to create a simpler interface.
 *
 *  Author: Jeffrey R. Naujok
 * Created: September 24, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_GENERICSERVER_H__
#define IASLIB_GENERICSERVER_H__

#include "Threading/Thread.h"
#include "Sockets/Socket.h"
#include "Sockets/UDPSocket.h"
#include "NetworkServices/HeaderList.h"
#include "Streams/Stream.h"

namespace IASLib
{
    class CGenericServer : public CThread
    {
        private:
            CSocket            *m_pSocket;
            CUDPSocket         *m_pUdpSocket;
            CInternetAddress    m_localAddress;

        public:
                                CGenericServer( void );
                                CGenericServer( CSocket *pSocket );
                                CGenericServer( CUDPSocket *pUdpSocket );
            virtual            ~CGenericServer( void );

                                DEFINE_OBJECT( CGenericServer );

        protected:
            CSocket            *getSocket( void ) { return m_pSocket; }
            void                setSocket( CSocket *pSocket ) { m_pSocket = pSocket; }

            CUDPSocket         *getUdpSocket( void ) { return m_pUdpSocket; }
            void                setUdpSocket( CUDPSocket *pUdpSocket ) { m_pUdpSocket = pUdpSocket; }


   };
} // namespace IASLib

#endif // IASLIB_GENERICSERVER_H__

#endif // IASLIB_NETWORKING__
