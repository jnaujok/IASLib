/**
 * HTTP Handler class
 *
 * This class defines the base class for handling a particular 
 * request type for an HTTP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: October 2, 2019
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
    #define IASLIB_DEFINE_HTTP_HANDLER( className, methodName )  \
                                                    public: \
                                                        virtual CString getMethod( void ) \
                                                        { \
                                                            return CString( methodName ); \
                                                        } \
                                                        virtual CHttpHandler *generateHandler( CString uri, CString version ) \
                                                        { \
                                                            return new className( uri, version ); \
                                                        }

    class CHttpHandler : public CGenericHandler
    {
        protected:
            CString m_strUri;
            CString m_strVersion;
        public:
            DEFINE_OBJECT( CHttpHandler );

            CHttpHandler( void );

            CHttpHandler( CString uri, CString version );

            virtual ~CHttpHandler( void );

            virtual CString getUri( void ) { return m_strUri; }
            virtual CString getVersion( void ) { return m_strVersion; }

            virtual bool process( CHttpRequest *request, CHttpResponse *response ) = 0;

            virtual CString getMethod( void ) = 0;
            virtual CHttpHandler *generateHandler( CString uri, CString version ) = 0;
        protected:
            virtual void setUri( CString strUri ) { m_strUri = strUri; }
            virtual void setVersion( CString strVersion ) { m_strVersion = strVersion; }
    };
} // namespace IASLib

#endif // IASLIB_HTTPHANDLER_H__

#endif // IASLIB_NETWORKING__

