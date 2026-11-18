/**
 * HTTP Handler class
 *
 * A handler processes one method for an HTTP server. Handlers are
 * registered with a CHttpHandlerFactory as prototypes; for each request the
 * server asks its factories for a handler, and the prototype creates a new
 * handler (generateHandler) for that one request.
 *
 * The server checks the request (version, Host, framing, Expect) before a
 * handler sees it, and adds Date, Server, Content-Length and Connection to
 * the response afterwards. A handler sets the status, its own headers and
 * the body. A HEAD request is given to the GET handler when there is no
 * HEAD handler, and its body is then left out. If process() returns false,
 * or leaves a status outside 100-599, the server sends 500.
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPHANDLER_H__
#define IASLIB_HTTPHANDLER_H__

#include "NetworkServices/GenericHandler.h"
#include "NetworkServices/HTTP/HttpRequest.h"
#include "NetworkServices/HTTP/HttpResponse.h"
#include "BaseTypes/Uuid.h"

namespace IASLib
{
        // Declares getMethod() and generateHandler() for a handler class.
    #define IASLIB_DEFINE_HTTP_HANDLER( className, methodName )  \
                                                    public: \
                                                        virtual IASLib::CString getMethod( void ) \
                                                        { \
                                                            return IASLib::CString( methodName ); \
                                                        } \
                                                        virtual IASLib::CHttpHandler *generateHandler( IASLib::CString uri, IASLib::CString version ) \
                                                        { \
                                                            return new className( uri, version ); \
                                                        }

    class CHttpServer;

    class CHttpHandler : public CGenericHandler
    {
        protected:
            CString         m_strUri;
            CString         m_strVersion;
            CUUID           erid;
            CHttpServer    *m_pServer;

        public:
            DEFINE_OBJECT( CHttpHandler );

            CHttpHandler( void );
            CHttpHandler( CString uri, CString version );

            virtual ~CHttpHandler( void );

            virtual CString getUri( void ) { return m_strUri; }
            virtual CString getVersion( void ) { return m_strVersion; }

            virtual bool process( CHttpRequest *request, CHttpResponse *response ) = 0;

            virtual CString getMethod( void ) = 0;

            virtual void setErid( CUUID erid ) { this->erid = erid; }
            virtual CUUID getErid( void ) { return erid; }

                // The server that is running this handler.
            virtual void setServer( CHttpServer *pServer ) { m_pServer = pServer; }
            virtual CHttpServer *getServer( void ) { return m_pServer; }

            virtual CHttpHandler *generateHandler( CString uri, CString version ) = 0;

        protected:
            virtual void setUri( CString strUri ) { m_strUri = strUri; }
            virtual void setVersion( CString strVersion ) { m_strVersion = strVersion; }
    };
} // namespace IASLib

#endif // IASLIB_HTTPHANDLER_H__

#endif // IASLIB_NETWORKING__
