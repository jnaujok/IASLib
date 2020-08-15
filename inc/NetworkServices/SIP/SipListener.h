/**
 * SIP Listener class
 *
 * This class handles incoming SIP calls into the server, based
 * on a threaded task model.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef SIPLISTENER_H__
#define SIPLISTENER_H__

#include "NetworkServices/GenericListener.h"

namespace IASLib
{
    // Forward definition for wiring parent server
    class CSipServer;

    class CSipListener : public CGenericListener
    {
        protected:
            CSipServer *m_pParentServer; 
        public:
            DEFINE_OBJECT( CSipListener );

            CSipListener( CSipServer *server, CClientSocket *pSocket );
            CSipListener( CSipServer *server, CUDPSocket *pUdpSocket, CInternetAddress &internetAddress, CString incomingData );

            virtual ~CSipListener( void );

            virtual CObject *Run( void );

            // Override of pure virtual
            virtual void addResponseHeaders( CGenericResponse *response );
    };
} // namespace IASLib

#endif // SIPLISTENER_H__

#endif // IASLIB_NETWORKING__
