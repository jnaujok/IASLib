/**
 * Generic Server Class
 * 
 * This class defines a generic, abstract base class for supporting 
 * socket-based servers that can receive requests and return responses
 * to the requesting party.
 * 
 * This Request/Response paradigm is inherent in many protocols used
 * in modern computing, such as HTTP and SIP. By creating a base class
 * this commonality can be exploited to create a simpler interface.
 * 
 *  Author: Jeffrey R. Naujok
 * Created: September 24, 2019
 * 
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/GenericServer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CGenericServer, CThread );

    CGenericServer::CGenericServer( void )
    {
            CSocket            *m_pSocket;
            CUDPSocket         *m_pUdpSocket;
            CInternetAddress      m_localAddress;
    }

    CGenericServer::CGenericServer( CSocket *pSocket )
    {

    }

    CGenericServer::CGenericServer( CUDPSocket *pUdpSocket )
    {

    }

    CGenericServer::~CGenericServer( void )
    {

    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
