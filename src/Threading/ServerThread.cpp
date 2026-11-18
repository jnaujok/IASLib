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

#include "ServerSocket.h"
#include "Socket.h"
#include "Exception.h"
#include "SocketException.h"
#include "ServerThread.h"

#ifdef IASLIB_MULTI_THREADED__
namespace IASLib
{
    CServerThread::CServerThread( const char *strThreadName, int nPortNumber, int nMaxBacklog ) : CThread( strThreadName, true )
    {
        m_nPortNumber = nPortNumber;
        m_nMaxBacklog = nMaxBacklog;
        m_pServerSocket = NULL;
    }

    CServerThread::~CServerThread( void )
    {
        delete m_pServerSocket;
    }

    void *CServerThread::Run( void )
    {
        try
        {
            m_pServerSocket = new CServerSocket( m_nPortNumber, m_nMaxBacklog );
        }
        catch ( ... )
        {
            m_pServerSocket = NULL;
        }

        if ( m_pServerSocket == NULL || ! m_pServerSocket->IsConnected() )
        {
            m_nReturnCode = -1;
            delete m_pServerSocket;
            m_pServerSocket = NULL;
            return NULL;
        }

        m_pServerSocket->SetNonBlocking( false );
        while ( ! IsShutdown() )
        {
            CSocket *pNewSocket = NULL;
            try
            {
                    // A short wait, so a shutdown request is seen promptly
                    // (Accept() without a timeout waits for ever).
                pNewSocket = m_pServerSocket->Accept( ACCEPT_POLL_MS );
            }
            catch ( ... )
            {
                pNewSocket = NULL;
            }

            if ( pNewSocket == NULL )
                continue;

            try
            {
                if ( pNewSocket->IsConnected() )
                    HandleConnection( pNewSocket );     // the handler owns the socket
                else
                    delete pNewSocket;
            }
            catch ( ... )
            {
            }
        }
        m_nReturnCode = 0;
        return NULL;
    }
}; // namespace IASLib
#endif // IASLIB_MULTI_THREADED__
