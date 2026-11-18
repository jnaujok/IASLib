/**
 * @file HttpHandler.h
 * @brief The base of HTTP method handlers (CHttpHandler, IASLIB_DEFINE_HTTP_HANDLER).
 *
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
        /// Declares getMethod() and generateHandler() for a handler class.
        ///
        /// Put it first in the class body. The class needs a constructor
        /// taking ( CString uri, CString version ). It leaves the following
        /// members public.
        /// @param className  The handler class.
        /// @param methodName The method it handles, such as "GET".
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

    /// Processes one HTTP method for a CHttpServer.
    ///
    /// A handler class handles one method. A prototype of it is registered
    /// with a CHttpHandlerFactory; for each request the factory has the
    /// prototype make a new handler (generateHandler()), which the server
    /// gives itself (setServer()), runs once with process(), and deletes.
    ///
    /// The server checks the request (version, Host, framing, Expect) before
    /// a handler sees it, and adds Date, Server, Content-Length and Connection
    /// to the response afterwards. A handler sets the status, its own headers
    /// and the body; a "Connection: close" it sets closes the connection. A
    /// HEAD request is given to the GET handler when there is no HEAD
    /// handler, and its body is then left out. If process() returns false,
    /// throws, or leaves a status outside 100-599, the server sends 500.
    ///
    /// Each handler object serves one request on one thread, but process()
    /// of different handlers may run at once on different connections.
    /// @code
    /// class CHelloHandler : public CHttpHandler
    /// {
    ///     IASLIB_DEFINE_HTTP_HANDLER( CHelloHandler, "GET" );
    ///     public:
    ///         CHelloHandler( CString uri, CString version ) : CHttpHandler( uri, version ) {}
    ///         virtual bool process( CHttpRequest *request, CHttpResponse *response )
    ///         {
    ///             response->setStatus( 200 );
    ///             response->SetEntity( new CTextPlainEntity( "Hello" ) );
    ///             return true;
    ///         }
    /// };
    /// // In a factory's constructor:
    /// registerHandler( new CHelloHandler( "", "" ) );
    /// @endcode
    class CHttpHandler : public CGenericHandler
    {
        protected:
            CString         m_strUri;       ///< The request target.
            CString         m_strVersion;   ///< The request's HTTP version.
            CUUID           erid;           ///< The request's id, for logging.
            CHttpServer    *m_pServer;      ///< The server running this handler (not owned), or NULL.

        public:
            DEFINE_OBJECT( CHttpHandler );

                /// Creates a handler with an empty URI and version.
            CHttpHandler( void );
                /// Creates a handler for a request with this target and version.
            CHttpHandler( CString uri, CString version );

                /// Destroys the handler.
            virtual ~CHttpHandler( void );

                /// The request target.
            virtual CString getUri( void ) { return m_strUri; }
                /// The request's HTTP version.
            virtual CString getVersion( void ) { return m_strVersion; }

                /// Handles the request, filling in the response.
                /// @param request  The parsed request.
                /// @param response The response to fill; it starts as 500.
                /// @return false if the request could not be handled (the
                ///         server then sends 500).
            virtual bool process( CHttpRequest *request, CHttpResponse *response ) = 0;

                /// The method this handler handles, such as "GET".
            virtual CString getMethod( void ) = 0;

                /// Sets the request's id, for logging.
            virtual void setErid( CUUID erid ) { this->erid = erid; }
                /// The request's id.
            virtual CUUID getErid( void ) { return erid; }

                /// Sets the server that is running this handler (not owned).
            virtual void setServer( CHttpServer *pServer ) { m_pServer = pServer; }
                /// The server that is running this handler, or NULL.
            virtual CHttpServer *getServer( void ) { return m_pServer; }

                /// Makes a new handler of this class for one request.
                /// @return A new handler, owned by the caller.
            virtual CHttpHandler *generateHandler( CString uri, CString version ) = 0;

        protected:
                /// Sets the request target.
            virtual void setUri( CString strUri ) { m_strUri = strUri; }
                /// Sets the request's HTTP version.
            virtual void setVersion( CString strVersion ) { m_strVersion = strVersion; }
    };
} // namespace IASLib

#endif // IASLIB_HTTPHANDLER_H__

#endif // IASLIB_NETWORKING__
