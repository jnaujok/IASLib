/**
 *  Generic Client Class
 *
 *      This class provides a base interface for connecting
 * to a Request/Response type server using the command/URI,
 * header block, data body, possible trailer format.
 *      Since this format was popularized by HTTP, this is now
 * being used by dozens of modern formats for transferring data
 * and thus it makes sense to abstract this protocol where 90%
 * of it can be reused.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 6/03/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "GenericClient.h"

namespace IASLib
{
    CGenericClient::CGenericClient( const char *strRemoteHost, int nPort, bool bUseSSL ) : CClientSocket( strRemoteHost, nPort )
    {
        m_bConnected = this->IsConnected();
        m_bSecure = bUseSSL;
        m_bSecureHandshakeComplete = false;
    }

    CGenericClient::~CGenericClient( void )
    {

    }

    IMPLEMENT_OBJECT( CGenericClient, CClientSocket );

} // namespace IASLib

#endif // IASLIB_NETWORKING__
