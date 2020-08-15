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

#ifndef IASLIB_GENERICCLIENT_H__
#define IASLIB_GENERICCLIENT_H__

#include "Sockets/ClientSocket.h"
#include "GenericRequest.h"
#include "GenericResponse.h"

namespace IASLib
{
    class CGenericClient : public CClientSocket
    {
        protected:
            bool                        m_bConnected;
            bool                        m_bSecure;
            bool                        m_bSecureHandshakeComplete;
        public:
                                        CGenericClient( const char *strRemoteHost, int nPort, bool bUseSSL = false );
            virtual                    ~CGenericClient( void );

                                        DEFINE_OBJECT( CGenericClient );

            virtual CGenericResponse   *executeRequest( CGenericRequest *request ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_GENERICCLIENT_H__
#endif // IASLIB_NETWORKING__
