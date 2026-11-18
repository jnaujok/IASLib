/**
 * @file HttpListener.h
 * @brief Serves the requests on one HTTP connection (CHttpListener).
 *
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

    /// Serves one HTTP connection for a CHttpServer.
    ///
    /// Run() reads requests, frames them with CHttpUtils::frameRequest()
    /// (using the server's size limits), sends "100 Continue" when a client
    /// asks for it, passes each request to CHttpServer::handleRequest(), and
    /// writes the responses in order, pipelined requests included. Framing
    /// errors get the matching error response and close the connection. The
    /// connection stays open between requests until the client or server
    /// closes it, it is idle for the keep-alive timeout, or the server stops;
    /// a request not complete within the request timeout gets 408.
    ///
    /// The listener owns the socket and closes it when destroyed. The server
    /// is not owned and must outlive the listener.
    class CHttpListener : public CThreadTask
    {
        protected:
            CHttpServer        *m_pParentServer;    ///< The server that handles the requests (not owned).
            CSocket            *m_pSocket;          ///< The connection (owned).
            CInternetAddress    m_internetAddress;  ///< The client's address.

        public:
            DEFINE_OBJECT( CHttpListener );

                /// Creates a listener for a connection. Takes ownership of the
                /// connected socket (plain or TLS, with its handshake done).
            CHttpListener( CHttpServer *server, CSocket *pSocket );

                /// Closes and deletes the socket.
            virtual ~CHttpListener( void );

                /// Serves the connection until it closes. Returns NULL.
            virtual CObject *Run( void );

        protected:
                /// Sends all of the data on the connection.
                /// @return false if sending failed.
            virtual bool sendData( const CString &data );
    };
} // namespace IASLib

#endif // HTTPLISTENER_H__

#endif // IASLIB_NETWORKING__
