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
    /**
     * The server thread is created suspended; call Start() once the object
     * is fully constructed. Run() is the thread body and is meant to be
     * overridden by a concrete server. Server objects belong to the
     * application (they are never deleted by the thread monitor), and a
     * derived class whose Run() uses its own members must call Stop() and
     * Wait() in its destructor so the thread has finished before those
     * members are destroyed.
     */
    class CGenericServer : public CThread
    {
        private:
            CSocket            *m_pSocket;
            CUDPSocket         *m_pUdpSocket;
            CInternetAddress    m_localAddress;
            bool                m_bStarted;
            bool                m_bJoined;

        public:
                                CGenericServer( void );
                                CGenericServer( CSocket *pSocket );
                                CGenericServer( CUDPSocket *pUdpSocket );

            virtual            ~CGenericServer( void );

                                DEFINE_OBJECT( CGenericServer );

                // Lets the server thread begin running Run().
            virtual void        Start( void );
                // Asks Run() to finish; see IsShutdown().
            virtual void        Stop( void );
                // Waits for the server thread to finish. Starts it first if it
                // was never started, so that it can exit.
            virtual void        Wait( void );

            bool                IsStarted( void ) { return m_bStarted; }

        protected:
            CSocket            *getSocket( void ) { return m_pSocket; }
            void                setSocket( CSocket *pSocket ) { m_pSocket = pSocket; }
            CUDPSocket         *getUdpSocket( void ) { return m_pUdpSocket; }
            void                setUdpSocket( CUDPSocket *pUdpSocket ) { m_pUdpSocket = pUdpSocket; }
   };
} // namespace IASLib

#endif // IASLIB_GENERICSERVER_H__

#endif // IASLIB_NETWORKING__
