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

    CGenericServer::CGenericServer( void ) : CThread( "GenericServer", true, false, true )
    {
        m_pSocket = NULL;
        m_pUdpSocket = NULL;
        m_bStarted = false;
        m_bJoined = false;
    }

    CGenericServer::CGenericServer( CSocket *pSocket ) : CThread( "GenericServer", true, false, true )
    {
        m_pSocket = pSocket;
        m_pUdpSocket = NULL;
        m_bStarted = false;
        m_bJoined = false;
    }

    CGenericServer::CGenericServer( CUDPSocket *pUdpSocket ) : CThread( "GenericServer", true, false, true )
    {
        m_pSocket = NULL;
        m_pUdpSocket = pUdpSocket;
        m_bStarted = false;
        m_bJoined = false;
    }

    CGenericServer::~CGenericServer( void )
    {
            // A thread that was never started is still waiting to run, so let
            // it run (it sees the shutdown request) and wait for it.
        Stop();
        Wait();
    }

    void CGenericServer::Start( void )
    {
            // Only the first caller resumes the thread.
        if ( ! m_bStarted.exchange( true ) )
        {
            Resume();
        }
    }

    void CGenericServer::Stop( void )
    {
        RequestShutdown();
    }

    void CGenericServer::Wait( void )
    {
        Start();
            // The lock makes a second waiter wait for the first one's join to
            // finish, rather than return while the thread still runs.
        std::lock_guard<std::mutex> lock( m_joinMutex );
        if ( ! m_bJoined )
        {
            Join();
            m_bJoined = true;
        }
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
