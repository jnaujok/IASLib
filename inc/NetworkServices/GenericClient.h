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
 *      A client holds the server's address; a concrete client (CHttpClient)
 * opens connections to it when it has a request to send, using the
 * library's socket classes. (This class used to be a CClientSocket that
 * connected in its constructor, so a client could not set a timeout or use
 * TLS first, could not reconnect after the server closed the connection,
 * and was unusable once it had failed to connect.)
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

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"
#include "GenericRequest.h"
#include "GenericResponse.h"

namespace IASLib
{
    class CGenericClient : public CObject
    {
        protected:
            CString                     m_strRemoteHost;
            int                         m_nPort;
            bool                        m_bConnected;
            bool                        m_bSecure;
            bool                        m_bSecureHandshakeComplete;
        public:
                                        CGenericClient( const char *strRemoteHost, int nPort, bool bUseSSL = false );
            virtual                    ~CGenericClient( void );

                                        DEFINE_OBJECT( CGenericClient );

                                        CGenericClient( const CGenericClient & ) = delete;
            CGenericClient             &operator =( const CGenericClient & ) = delete;

                // Sends the request and returns the response, which the caller
                // deletes, or NULL if there is none.
            virtual CGenericResponse   *executeRequest( CGenericRequest *request ) = 0;

            CString                     getRemoteHost( void ) const { return m_strRemoteHost; }
            int                         getRemotePort( void ) const { return m_nPort; }
            bool                        isSecure( void ) const { return m_bSecure; }
                // Whether a connection to the server is open now.
            virtual bool                IsConnected( void ) { return m_bConnected; }
    };
} // namespace IASLib

#endif // IASLIB_GENERICCLIENT_H__
#endif // IASLIB_NETWORKING__
