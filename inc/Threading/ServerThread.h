/**
 * @file ServerThread.h
 * @brief CServerThread, a thread that accepts socket connections.
 *
 * The original header comment follows.
 */
/***********************************************************************
**  CServerThread Class
**
**  Description:
**      This class provides a base class for a thread that provides a 
** socket based server. Basically it is created by providing a socket
** to listen on and calls a virtual function "HandleConnection" with
** the new socket.
**      It also provides a static interface for checking whether the
** server is active and can also check the thread information to see
** if it should be shut down, etc.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef IASLIB_SERVERTHREAD_H__
#define IASLIB_SERVERTHREAD_H__

#include "../BaseTypes/Object.h"
#include "../Sockets/ServerSocket.h"
#include "../Threading/Thread.h"

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_MULTI_THREADED__
namespace IASLib
{
    /// A thread that listens on a port and hands each connection to HandleConnection().
    ///
    /// Derive from it and implement HandleConnection(). The thread starts
    /// suspended: call Resume() once the derived object is built. Stop it
    /// with RequestShutdown(); the accepting loop notices within
    /// ACCEPT_POLL_MS.
    ///
    /// @code
    /// class CEchoServer : public CServerThread
    /// {
    ///     public:
    ///         CEchoServer( void ) : CServerThread( "Echo", 7007 ) { Resume(); }
    ///         virtual bool HandleConnection( CSocket *pSocket )
    ///         {
    ///             ...;
    ///             delete pSocket;     // the handler owns it
    ///             return true;
    ///         }
    /// };
    /// @endcode
    class CServerThread : public CThread
    {
        protected:
            int             m_nPortNumber;      ///< The port to listen on.
            int             m_nMaxBacklog;      ///< The listen backlog.
            CServerSocket  *m_pServerSocket;    ///< The listening socket, once Run() opened it (owned).

        public:
                /// How often (milliseconds) the accepting loop checks for a
                /// shutdown request.
            static const int ACCEPT_POLL_MS = 250;

                /// Listens on nPortNumber when started (it starts suspended:
                /// call Resume()), handing each connection to
                /// HandleConnection(), which then owns the socket.
                ///
                /// The return code is -1 if the port cannot be opened, 0 after
                /// a shutdown.
                /// @param strThreadName  The thread's name.
                /// @param nPortNumber  The port to listen on.
                /// @param nMaxBacklog  The listen backlog.
                            CServerThread( const char *strThreadName, int nPortNumber, int nMaxBacklog = 16 );
                /// Closes the listening socket.
            virtual        ~CServerThread( void );

                /// Opens the port and accepts connections until a shutdown is requested.
                /// @return NULL.
            virtual void   *Run( void );

                /// Handles one accepted connection, on this thread.
                ///
                /// Exceptions it throws are caught and ignored.
                /// @param pSocket  The connected socket, which the handler now owns.
                /// @return Ignored.
            virtual bool    HandleConnection( CSocket *pSocket ) = 0;
    };
} // namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_NETWORKING__
#endif // IASLIB_SERVERTHREAD_H__
