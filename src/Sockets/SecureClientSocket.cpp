/**
 *  Secure Client Socket Class
 *
 *      A TLS connection to a server. See SecureClientSocket.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/23/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__
#ifdef IASLIB_TLS__

#include "SecureClientSocket.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSecureClientSocket, CSecureSocket );

    CSecureClientSocket::CSecureClientSocket( const char *strConnectTo, int nPort, const std::shared_ptr<CTlsContext> &pContext )
        : CSecureSocket( strConnectTo, nPort, pContext )
    {
    }

    CSecureClientSocket::~CSecureClientSocket()
    {
    }

    std::shared_ptr<CTlsContext> CSecureClientSocket::DefaultContext( void )
    {
            // Created on first use (thread-safe), never destroyed, so it
            // outlives any socket made at exit.
        static std::shared_ptr<CTlsContext> *pContext = new std::shared_ptr<CTlsContext>( std::make_shared<CTlsContext>( CTlsContext::CLIENT ) );
        return *pContext;
    }
} // namespace IASLib

#endif // IASLIB_TLS__
#endif // IASLIB_NETWORKING__
