/**
 * Generic Listener class
 *
 * This class handles incoming calls into a server, based
 * on a threaded task model.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 24, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef GENERICLISTENER_H__
#define GENERICLISTENER_H__

#include "Threading/ThreadTask.h"
#include "Streams/Stream.h"
#include "Sockets/InternetAddress.h"
#include "Sockets/UDPSocket.h"
#include "Sockets/ClientSocket.h"
#include "NetworkServices/GenericResponse.h"
#include "NetworkServices/GenericRequest.h"

namespace IASLib
{
    class CGenericListener : public CThreadTask
    {
        protected:
            CStream *m_pInStream;
            CStream *m_pOutStream;

            CInternetAddress m_internetAddress;
            CUDPSocket *m_pSocket;
            CClientSocket *m_pClientSocket;

        public:
            DEFINE_OBJECT( CGenericListener );

            CGenericListener( const char *strName, CClientSocket *pSocket );
            CGenericListener( const char *strName, CUDPSocket *pUdpSocket, CInternetAddress &internetAddress, CString incomingData );

            virtual ~CGenericListener( void );

            virtual void addResponseHeaders( CGenericResponse *response ) = 0;
    };
} // namespace IASLib

#endif // GENERICLISTENER_H__

#endif // IASLIB_NETWORKING__
