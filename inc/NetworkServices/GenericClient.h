/**
 * @file GenericClient.h
 * @brief The base of request/response protocol clients (CGenericClient).
 *
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
    /// The base of clients for request/response protocols such as HTTP.
    ///
    /// A client holds the server's host, port and whether to use TLS. It does
    /// not connect when constructed: a concrete client (CHttpClient) opens a
    /// connection when it has a request to send, and can reconnect after the
    /// server closes one. Clients cannot be copied.
    /// @note This class used to be a CClientSocket that connected in its
    ///       constructor, so a client could not set a timeout or use TLS
    ///       first, could not reconnect, and was unusable once it had failed
    ///       to connect.
    class CGenericClient : public CObject
    {
        protected:
            CString                     m_strRemoteHost;            ///< The server's host name or address.
            int                         m_nPort;                    ///< The server's port.
            bool                        m_bConnected;               ///< Whether a connection is open now (kept by the derived client).
            bool                        m_bSecure;                  ///< Whether connections use TLS.
            bool                        m_bSecureHandshakeComplete; ///< Whether the TLS handshake on the open connection is done (kept by the derived client).
        public:
                                        /// Stores the server's address; nothing is connected yet.
                                        /// @param strRemoteHost The host name or address; NULL is taken as "".
                                        /// @param nPort         The port.
                                        /// @param bUseSSL       Whether to use TLS.
                                        CGenericClient( const char *strRemoteHost, int nPort, bool bUseSSL = false );
                                        /// Destroys the client.
            virtual                    ~CGenericClient( void );

                                        DEFINE_OBJECT( CGenericClient );

                                        /// Not copyable.
                                        CGenericClient( const CGenericClient & ) = delete;
                                        /// Not assignable.
            CGenericClient             &operator =( const CGenericClient & ) = delete;

                /// Sends the request and returns the response.
                /// @return The response, which the caller deletes, or NULL if
                ///         there is none.
            virtual CGenericResponse   *executeRequest( CGenericRequest *request ) = 0;

                /// The server's host name or address.
            CString                     getRemoteHost( void ) const { return m_strRemoteHost; }
                /// The server's port.
            int                         getRemotePort( void ) const { return m_nPort; }
                /// Whether connections use TLS.
            bool                        isSecure( void ) const { return m_bSecure; }
                /// Whether a connection to the server is open now.
            virtual bool                IsConnected( void ) { return m_bConnected; }
    };
} // namespace IASLib

#endif // IASLIB_GENERICCLIENT_H__
#endif // IASLIB_NETWORKING__
