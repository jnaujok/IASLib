/**
 * @file SipListener.h
 * @brief A task that hands one received SIP message to a server (CSipListener).
 *
 * SIP Listener class
 *
 * A thread task that hands one received SIP message to a server. A server
 * creates one per received datagram (or framed stream message) and runs it
 * on a worker thread, so that slow handlers do not hold up the receiving
 * thread.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef SIPLISTENER_H__
#define SIPLISTENER_H__

#include "Threading/ThreadTask.h"
#include "Sockets/InternetAddress.h"
#include "NetworkServices/SIP/SipTransport.h"

namespace IASLib
{
    class CSipServer;

    /// A thread task that hands one received SIP message to a CSipServer.
    ///
    /// A server creates one per received datagram (or framed stream message)
    /// and runs it on a worker thread, so that slow handlers do not hold up
    /// the receiving thread. The task keeps a copy of the message; the server
    /// and transport are not owned and must outlive it.
    class CSipListener : public CThreadTask
    {
        protected:
            CSipServer         *m_pParentServer;    ///< The server that handles the message (not owned).
            CSipTransport      *m_pTransport;       ///< The transport to answer on (not owned).
            CInternetAddress    m_internetAddress;  ///< The sender's address.
            CString             m_strMessage;       ///< The received message.

        public:
            DEFINE_OBJECT( CSipListener );

                /// Creates a task for one message. The transport must outlive
                /// the task.
                /// @param server     The server to hand the message to.
                /// @param pTransport The transport the message arrived on.
                /// @param source     The sender's address.
                /// @param message    The message, framed.
            CSipListener( CSipServer *server, CSipTransport *pTransport, const CInternetAddress &source, const CString &message );

                /// Destroys the task.
            virtual ~CSipListener( void );

                /// Passes the message to CSipServer::handleMessage(). Returns NULL.
            virtual CObject *Run( void );
    };
} // namespace IASLib

#endif // SIPLISTENER_H__

#endif // IASLIB_NETWORKING__
