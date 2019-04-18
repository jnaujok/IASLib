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
    class CServerThread : public CThread
    {
        protected:
            int             m_nPortNumber;
            int             m_nMaxBacklog;
            CServerSocket  *m_pServerSocket;

        public:
                            CServerThread( const char *strThreadName, int nPortNumber, int nMaxBacklog = 16 );
            virtual        ~CServerThread( void );

            virtual void   *Run( void );

            virtual bool    HandleConnection( CSocket *pSocket ) = 0;
    };
} // namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_NETWORKING__
#endif // IASLIB_SERVERTHREAD_H__
