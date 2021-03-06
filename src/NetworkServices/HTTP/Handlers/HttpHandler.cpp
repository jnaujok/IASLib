/**
 * HTTP Handler class
 *
 * This class is an abstract base object for an HTTP Server Handler. 
 * Any specialized response handler must be derived from this class, and
 * then burned into a handler factory registered with the Server.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 28, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"
#include "Collections/Hash.h"
#include "Logging/LogSink.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpHandler, CGenericHandler );

    CHttpHandler::CHttpHandler( CString uri, CString version )
    {
        m_strUri = uri;
        m_strVersion = version;
    }


    CHttpHandler::~CHttpHandler( void )
    {
    }
    
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
