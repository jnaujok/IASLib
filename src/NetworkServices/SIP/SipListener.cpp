/**
 * SIP Listener class
 *
 * A thread task that hands one received SIP message to a server.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipListener.h"
#include "NetworkServices/SIP/SipServer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipListener, CThreadTask );

    CSipListener::CSipListener( CSipServer *server, CSipTransport *pTransport, const CInternetAddress &source, const CString &message ) :
        CThreadTask( "SipListener" ), m_internetAddress( source ), m_strMessage( message )
    {
        m_pParentServer = server;
        m_pTransport = pTransport;
    }

    CSipListener::~CSipListener( void )
    {
    }

    IASLib::CObject *CSipListener::Run( void )
    {
        if ( m_pParentServer && m_pTransport )
        {
            m_pParentServer->handleMessage( m_strMessage, m_pTransport, m_internetAddress );
        }
        return NULL;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
