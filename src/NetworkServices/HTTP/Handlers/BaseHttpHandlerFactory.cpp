/**
 * HTTP Base Handler Factory class
 *
 * This class provides a default HTTP Handler Factory that is normally
 * the baseline handler for an HTTP Server. 
 * 
 * Typically this class is added to the CHttpServer by the constructor
 * and is the baseline for an HTTP Server. By default, all the basic 
 * handlers simply return error codes in proper formats.
 * 
 *
 * Author: Jeffrey R. Naujok
 * Created: December 29, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/Handlers/BaseHttpHandlerFactory.h"

#include "NetworkServices/HTTP/Handlers/HttpGetHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpPostHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpHeadHandler.h"
#include "NetworkServices/HTTP/Handlers/HttpOptionsHandler.h"

namespace IASLib
{
    CBaseHttpHandlerFactory::CBaseHttpHandlerFactory( void ) : CHttpHandlerFactory()
    {
        this->registerHandler( new CHttpGetHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CHttpPostHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CHttpHeadHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CHttpOptionsHandler( CString( "" ), CString( "" ) ) );

    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
