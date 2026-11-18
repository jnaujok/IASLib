/**
 * @file GenericServer.h
 * @brief The thread base of socket servers (CGenericServer).
 *
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

#include <atomic>
#include <mutex>

namespace IASLib
{
    /**
     * The thread base of socket servers that receive requests and return responses.
     *
     * The server thread is created suspended; call Start() once the object
     * is fully constructed. Run() is the thread body and is meant to be
     * overridden by a concrete server. Server objects belong to the
     * application (they are never deleted by the thread monitor), and a
     * derived class whose Run() uses its own members must call Stop() and
     * Wait() in its destructor so the thread has finished before those
     * members are destroyed.
     *
     * Start(), Stop() and Wait() may be called from any thread, and more
     * than once: the thread is started once and joined once, and every
     * Wait() returns only after it has finished.
     *
     * The socket pointers are stored for derived classes and are not owned.
     */
    class CGenericServer : public CThread
    {
        private:
            CSocket            *m_pSocket;      ///< A stream socket for the derived server (not owned), or NULL.
            CUDPSocket         *m_pUdpSocket;   ///< A datagram socket for the derived server (not owned), or NULL.
            CInternetAddress    m_localAddress; ///< Unused by this class.
                /// Whether Start() has resumed the thread.
                /// @note This and m_bJoined used to be plain bools, read and set
                ///       from different threads without synchronisation: two
                ///       Start() calls could both resume the thread, and two
                ///       Wait() calls both join it.
            std::atomic<bool>   m_bStarted;
            std::atomic<bool>   m_bJoined;      ///< Whether Wait() has joined the thread.
            std::mutex          m_joinMutex;    ///< Makes a second Wait() wait for the first one's join.

        public:
                                /// Creates the (suspended) server thread with no socket.
                                CGenericServer( void );
                                /// Same, storing a stream socket for the derived class.
                                CGenericServer( CSocket *pSocket );
                                /// Same, storing a datagram socket for the derived class.
                                CGenericServer( CUDPSocket *pUdpSocket );

                                /// Stops the thread and waits for it to finish (starting it
                                /// first if it was never started, so that it can exit).
            virtual            ~CGenericServer( void );

                                DEFINE_OBJECT( CGenericServer );

                /// Lets the server thread begin running Run(). Only the first
                /// call has any effect.
            virtual void        Start( void );
                /// Asks Run() to finish; see IsShutdown(). Does not wait.
            virtual void        Stop( void );
                /// Waits for the server thread to finish.
                ///
                /// Starts it first if it was never started, so that it can exit.
                /// Every caller returns only after the thread has been joined.
            virtual void        Wait( void );

                /// Whether Start() has been called.
            bool                IsStarted( void ) { return m_bStarted; }

        protected:
                /// The stream socket given at construction, or NULL.
            CSocket            *getSocket( void ) { return m_pSocket; }
                /// Stores a stream socket (not owned).
            void                setSocket( CSocket *pSocket ) { m_pSocket = pSocket; }
                /// The datagram socket given at construction, or NULL.
            CUDPSocket         *getUdpSocket( void ) { return m_pUdpSocket; }
                /// Stores a datagram socket (not owned).
            void                setUdpSocket( CUDPSocket *pUdpSocket ) { m_pUdpSocket = pUdpSocket; }
   };
} // namespace IASLib

#endif // IASLIB_GENERICSERVER_H__

#endif // IASLIB_NETWORKING__
