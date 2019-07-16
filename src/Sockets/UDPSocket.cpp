/**
 *  UDP Socket Class
 *
 *      This base class provides platform independent support for UDP
 * sockets. This is the base class for both the UDP server and client socket
 * classes.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 7/11/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "UDPSocket.h"

namespace IASLib
{
    CUDPSocket::CUDPSocket( int nPort, bool bBlocking = true )
    {

    }
    CUDPSocket::CUDPSocket( const char *strConnectTo, int nPort )
    {

    }
    CUDPSocket::~CUDPSocket()
    {}

    IMPLEMENT_OBJECT( CUDPSocket, CObject )

    int CUDPSocket::Read( char *pchBuffer, int nBufferSize )
    {

    }

    int CUDPSocket::Send( const char *pchBuffer, int nBufferSize )
    {

    }

    void CUDPSocket::Close( void )
    {

    }

} // namespace IASLib

#endif // IASLIB_NETWORKING__
