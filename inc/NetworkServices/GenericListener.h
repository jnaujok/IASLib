/**
 * @file GenericListener.h
 * @brief The base of the tasks that answer one connection or datagram (CGenericListener).
 *
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
    /// A thread task that reads requests from one source and writes the responses.
    ///
    /// It is built either for a stream connection, where one socket stream is
    /// used to read and write, or for a datagram, where the request is read
    /// from the datagram's text and the response is collected in a string and
    /// sent back to the sender when the listener is destroyed (nothing is sent
    /// if there is no response, as for a SIP ACK).
    ///
    /// For a connection the listener takes ownership of the client socket (the
    /// socket stream deletes it). For a datagram the UDP socket is not owned.
    ///
    /// The HTTP and SIP listeners (CHttpListener, CSipListener) are separate
    /// CThreadTask classes and do not derive from this one.
    class CGenericListener : public CThreadTask
    {
        protected:
            CStream *m_pInStream;       ///< The stream requests are read from (owned).
            CStream *m_pOutStream;      ///< The stream responses are written to (owned); the same as m_pInStream for a connection.

            CInternetAddress m_internetAddress; ///< The peer's address.
            CUDPSocket *m_pSocket;              ///< The datagram socket to reply on (not owned), or NULL for a connection.
            CClientSocket *m_pClientSocket;     ///< The connection's socket (deleted with m_pInStream), or NULL for a datagram.

        public:
            DEFINE_OBJECT( CGenericListener );

                /// Creates a listener for a stream connection.
                /// @param strName The task name.
                /// @param pSocket The accepted connection; the listener takes ownership.
            CGenericListener( const char *strName, CClientSocket *pSocket );
                /// Creates a listener for one datagram.
                /// @param strName         The task name.
                /// @param pUdpSocket      The socket to send the reply on (not owned).
                /// @param internetAddress The sender, where the reply goes.
                /// @param incomingData    The datagram's contents.
            CGenericListener( const char *strName, CUDPSocket *pUdpSocket, CInternetAddress &internetAddress, CString incomingData );

                /// Deletes the streams. For a datagram, first sends whatever was
                /// written to the output stream, if anything, to the sender.
            virtual ~CGenericListener( void );

                /// Adds the headers the derived listener puts on every response,
                /// before it is written.
            virtual void addResponseHeaders( CGenericResponse *response ) = 0;
    };
} // namespace IASLib

#endif // GENERICLISTENER_H__

#endif // IASLIB_NETWORKING__
