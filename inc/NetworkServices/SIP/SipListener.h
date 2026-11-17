/**
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

    class CSipListener : public CThreadTask
    {
        protected:
            CSipServer         *m_pParentServer;
            CSipTransport      *m_pTransport;
            CInternetAddress    m_internetAddress;
            CString             m_strMessage;

        public:
            DEFINE_OBJECT( CSipListener );

                // The transport must outlive the task.
            CSipListener( CSipServer *server, CSipTransport *pTransport, const CInternetAddress &source, const CString &message );

            virtual ~CSipListener( void );

                // Passes the message to the server. Returns NULL.
            virtual CObject *Run( void );
    };
} // namespace IASLib

#endif // SIPLISTENER_H__

#endif // IASLIB_NETWORKING__
