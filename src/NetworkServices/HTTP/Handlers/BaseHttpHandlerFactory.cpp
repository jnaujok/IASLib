/**
 * Base HTTP Handler Factory class
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/Handlers/BaseHttpHandlerFactory.h"
#include "NetworkServices/HTTP/Handlers/HttpGetHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpPostHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpOptionsHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CBaseHttpHandlerFactory, CHttpHandlerFactory );

    CBaseHttpHandlerFactory::CBaseHttpHandlerFactory( void ) : CHttpHandlerFactory()
    {
        this->registerHandler( new CHttpGetHandler( CString( "" ), CString( "" ) ) );
            // No HEAD handler: the server answers HEAD with whichever GET
            // handler applies (RFC 9110 9.3.2), including an application's.
        this->registerHandler( new CHttpPostHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CHttpOptionsHandler( CString( "" ), CString( "" ) ) );
    }

    CBaseHttpHandlerFactory::~CBaseHttpHandlerFactory( void )
    {
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
