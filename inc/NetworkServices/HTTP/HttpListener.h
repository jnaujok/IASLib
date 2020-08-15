/**
 * HTTP Listener class
 *
 * This class handles incoming HTTP calls into the server, based
 * on a threaded task model.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 29, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef HTTPLISTENER_H__
#define HTTPLISTENER_H__

#include "NetworkServices/GenericListener.h"

namespace IASLib
{
    // Forward definition for wiring parent server
    class CHttpServer;

    class CHttpListener : public CGenericListener
    {
        protected:
            CHttpServer *m_pParentServer; 
        public:
            DEFINE_OBJECT( CHttpListener );

            CHttpListener( CHttpServer *server, CClientSocket *pSocket );
            CHttpListener( CHttpServer *server, CUDPSocket *pUdpSocket, CInternetAddress &internetAddress, CString incomingData );

            virtual ~CHttpListener( void );

            virtual CObject *Run( void );

            // Override of pure virtual
            virtual void addResponseHeaders( CGenericResponse *response );
    };
} // namespace IASLib

#endif // HTTPLISTENER_H__

#endif // IASLIB_NETWORKING__
