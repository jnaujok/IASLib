/**
 * @file SipHandler.h
 * @brief The base of SIP method handlers (CSipHandler, IASLIB_SET_SIP_HANDLER).
 *
 * SIP Handler class
 *
 * A handler processes one kind of request (one method) for a SIP server.
 * Handlers are registered with a CSipHandlerFactory as prototypes; for each
 * request the server asks its factories for a handler, and the prototype
 * creates a new handler object (generateHandler) that processes that one
 * request and is then deleted.
 *
 * The server prepares the response before calling process(): the Via, From,
 * To (with this server's tag), Call-ID and CSeq headers are already set as
 * RFC 3261 section 8.2.6 requires. A handler only needs to set the status
 * and any headers or body of its own. The server checks the request
 * (version, required headers, Require, URI scheme) before the handler sees
 * it, and handles retransmissions, CANCEL and ACK for non-2xx responses.
 *
 * A handler must give a final response (200-699). If process() returns
 * false or leaves a non-final status, the server sends 500. Any response
 * to an ACK is ignored, since ACK is never answered.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPHANDLER_H__
#define IASLIB_SIPHANDLER_H__

#include "NetworkServices/SIP/SipRequest.h"
#include "NetworkServices/SIP/SipResponse.h"
#include "BaseTypes/Uuid.h"

namespace IASLib
{
        /// Declares getMethod() and generateHandler() for a handler class.
        ///
        /// Put it first in the class body. The class needs a constructor
        /// taking ( CString uri, CString version ). It leaves the following
        /// members public.
        /// @param className  The handler class.
        /// @param methodName The method it handles, such as "INVITE".
    #define IASLIB_SET_SIP_HANDLER( className, methodName )  \
                                                    public: \
                                                        virtual IASLib::CString getMethod( void ) { return IASLib::CString( methodName ); } \
                                                        IASLib::CSipHandler *generateHandler( IASLib::CString uri, IASLib::CString version ) \
                                                        { \
                                                            return new className( uri, version ); \
                                                        }

    class CSipServer;

    /// Processes one SIP method for a CSipServer.
    ///
    /// A handler class handles one method. A prototype of it is registered
    /// with a CSipHandlerFactory; for each request the factory has the
    /// prototype make a new handler (generateHandler()), which processes that
    /// one request and is then deleted.
    ///
    /// The server prepares the response before calling process(): the Via,
    /// From, To (with this server's tag), Call-ID and CSeq headers are
    /// already set as RFC 3261 section 8.2.6 requires. A handler only needs
    /// to set the status and any headers or body of its own. It must give a
    /// final response (200-699); if process() returns false or leaves a
    /// non-final status, the server sends 500. Any response to an ACK is
    /// ignored.
    ///
    /// Each handler object serves one request on one thread, but process()
    /// of different handlers may run at once.
    /// @code
    /// class CMyInviteHandler : public CSipHandler
    /// {
    ///     IASLIB_SET_SIP_HANDLER( CMyInviteHandler, "INVITE" );
    ///     public:
    ///         CMyInviteHandler( CString uri, CString version ) : CSipHandler( uri, version ) {}
    ///         virtual bool process( CSipRequest *request, CSipResponse *response )
    ///         {
    ///             response->setStatus( 486 );     // Busy Here
    ///             return true;
    ///         }
    /// };
    /// @endcode
    class CSipHandler : public CObject
    {
        protected:
            CString     m_strUri;       ///< The Request-URI.
            CString     m_strVersion;   ///< The request's SIP version.
            CUUID       erid;           ///< The request's id, for logging.
            CSipServer *m_pServer;      ///< The server running this handler (not owned), or NULL.

        public:
            DEFINE_OBJECT( CSipHandler );

                /// Creates a handler for a request with this URI and version.
            CSipHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CSipHandler( void );

                /// Handles the request, filling in the prepared response.
                /// @param request  The parsed request.
                /// @param response The response, with the 8.2.6 headers set.
                /// @return false if the request could not be handled (the
                ///         server then sends 500).
            virtual bool process( CSipRequest *request, CSipResponse *response ) = 0;

                /// The method this handler handles, such as "INVITE".
            virtual CString getMethod( void ) = 0;

                /// Sets the request's id, for logging.
            virtual void setErid( CUUID erid ) { this->erid = erid; }
                /// The request's id.
            virtual CUUID getErid( void ) { return erid; }

                /// Sets the server that is running this handler (not owned).
            virtual void setServer( CSipServer *pServer ) { m_pServer = pServer; }
                /// The server that is running this handler, or NULL.
            virtual CSipServer *getServer( void ) { return m_pServer; }

                /// Makes a new handler of this class for one request.
                /// @return A new handler, owned by the caller.
            virtual CSipHandler *generateHandler( CString uri, CString version ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_SIPHANDLER_H__

#endif // IASLIB_NETWORKING__
