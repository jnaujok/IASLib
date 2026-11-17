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

#include "NetworkServices/GenericListener.h"
#include "Streams/SocketStream.h"
#include "Streams/StringStream.h"

namespace IASLib
{

    IMPLEMENT_OBJECT( CGenericListener, CThreadTask );

    using namespace IASLib;

    CGenericListener::CGenericListener( const char *strName, CClientSocket *pSocket ) : CThreadTask( strName ), m_internetAddress( pSocket->getInternetAddress() )
    {
        m_pSocket = NULL;
        m_pClientSocket = pSocket;
        m_pInStream = new CSocketStream( pSocket );
        m_pOutStream = m_pInStream; // Socket Streams are bi-directional
    }

    CGenericListener::CGenericListener( const char *strName, CUDPSocket *pSocket, CInternetAddress &internetAddress, CString incomingData ) : CThreadTask( strName ), m_internetAddress( internetAddress ), m_pSocket( pSocket )
    {
        m_pClientSocket = NULL;
        m_pInStream = new CStringStream( incomingData );
        m_pOutStream = new CStringStream(); // Output only String Stream
    }

    CGenericListener::~CGenericListener( void )
    {
        if ( m_pInStream == m_pOutStream )
        {
            delete m_pInStream;
        }
        else
        {
            CStringStream *pOut = (CStringStream *)m_pOutStream;
            CString data = pOut->GetString();

                // Nothing is sent when there is no response (for example, SIP
                // never answers an ACK).
            if ( ( m_pSocket ) && ( m_internetAddress.isValid() ) && ( data.GetLength() > 0 ) )
            {
                m_pSocket->Send( data, (int)data.GetLength(), m_internetAddress );
            }

            delete m_pInStream;
            delete m_pOutStream;
        }
    }
} // namespace IASLib

#endif