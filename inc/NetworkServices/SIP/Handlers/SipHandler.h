/**
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
        // Declares getMethod() and generateHandler() for a handler class.
    #define IASLIB_SET_SIP_HANDLER( className, methodName )  \
                                                    public: \
                                                        virtual IASLib::CString getMethod( void ) { return IASLib::CString( methodName ); } \
                                                        IASLib::CSipHandler *generateHandler( IASLib::CString uri, IASLib::CString version ) \
                                                        { \
                                                            return new className( uri, version ); \
                                                        }

    class CSipServer;

    class CSipHandler : public CObject
    {
        protected:
            CString     m_strUri;
            CString     m_strVersion;
            CUUID       erid;
            CSipServer *m_pServer;

        public:
            DEFINE_OBJECT( CSipHandler );

            CSipHandler( CString uri, CString version );

            virtual ~CSipHandler( void );

            virtual bool process( CSipRequest *request, CSipResponse *response ) = 0;

            virtual CString getMethod( void ) = 0;

            virtual void setErid( CUUID erid ) { this->erid = erid; }
            virtual CUUID getErid( void ) { return erid; }

                // The server that is running this handler.
            virtual void setServer( CSipServer *pServer ) { m_pServer = pServer; }
            virtual CSipServer *getServer( void ) { return m_pServer; }

            virtual CSipHandler *generateHandler( CString uri, CString version ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_SIPHANDLER_H__

#endif // IASLIB_NETWORKING__
