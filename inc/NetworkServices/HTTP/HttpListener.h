/**
 * HTTP Listener class
 *
 * A thread task that serves one HTTP connection: it reads requests,
 * frames them (Content-Length or chunked, with the server's size limits),
 * sends "100 Continue" when a client asks for it, passes each request to
 * the server, writes the responses in order, and keeps the connection open
 * between requests until the client or server closes it, it is idle for
 * the keep-alive timeout, or the server stops. A request that is not
 * complete within the request timeout gets 408.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 24, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef HTTPLISTENER_H__
#define HTTPLISTENER_H__

#include "Threading/ThreadTask.h"
#include "Sockets/Socket.h"
#include "Sockets/InternetAddress.h"

namespace IASLib
{
    class CHttpServer;

    class CHttpListener : public CThreadTask
    {
        protected:
            CHttpServer        *m_pParentServer;
            CSocket            *m_pSocket;
            CInternetAddress    m_internetAddress;

        public:
            DEFINE_OBJECT( CHttpListener );

                // Takes ownership of the connected socket.
            CHttpListener( CHttpServer *server, CSocket *pSocket );

            virtual ~CHttpListener( void );

                // Serves the connection until it closes. Returns NULL.
            virtual CObject *Run( void );

        protected:
            virtual bool sendData( const CString &data );
    };
} // namespace IASLib

#endif // HTTPLISTENER_H__

#endif // IASLIB_NETWORKING__
