/**
 * SIP Transport classes
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipTransport.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipTransport, CObject );
    IMPLEMENT_OBJECT( CSipUdpTransport, CSipTransport );
    IMPLEMENT_OBJECT( CSipStreamTransport, CSipTransport );

    CSipUdpTransport::CSipUdpTransport( CUDPSocket *pSocket )
    {
        m_pSocket = pSocket;
    }

    CSipUdpTransport::~CSipUdpTransport( void )
    {
    }

    bool CSipUdpTransport::send( const CString &data, const CInternetAddress &target )
    {
        if ( m_pSocket == NULL )
        {
            return false;
        }
        int nSent = m_pSocket->Send( (const char *)data, (int)data.GetLength(), target );
        return ( nSent == (int)data.GetLength() );
    }

    CSipStreamTransport::CSipStreamTransport( CSocket *pSocket, bool bSecure )
    {
        m_pSocket = pSocket;
        m_bSecure = bSecure;
    }

    CSipStreamTransport::~CSipStreamTransport( void )
    {
    }

    bool CSipStreamTransport::send( const CString &data, const CInternetAddress &target )
    {
        std::lock_guard<std::mutex> lock( m_sendMutex );

        if ( m_pSocket == NULL || ! m_pSocket->IsConnected() )
        {
            return false;
        }

        try
        {
            m_pSocket->Send( (const char *)data, (int)data.GetLength() );
        }
        catch ( ... )
        {
            return false;
        }
        return true;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
