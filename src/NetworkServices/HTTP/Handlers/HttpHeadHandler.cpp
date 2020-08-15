/**
 * HTTP Head Handler class
 *
 * This class handles HEAD requests for the HTTP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 29, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/Entities/NullEntity.h"
#include "NetworkServices/HTTP/Handlers/HttpHeadHandler.h"
#include "Logging/LogSink.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpHeadHandler, CHttpHandler );

    CHttpHeadHandler::CHttpHeadHandler( CString uri, CString version ) : CHttpHandler( uri, version )
    {

    }

    CHttpHeadHandler::~CHttpHeadHandler( void )
    {

    }

    bool CHttpHeadHandler::process( CHttpRequest *request, CHttpResponse *response )
    {
        INFO_LOG( (const char *)request->toString() );
        CDate now;

        response->addHeader( "Date", now.FormatDate( CDate::DF_RFC7231 ) );
        response->addHeader( "X-Powered-By", "IASLib HTTP Core 1.0" );

        // Default is to mark all URIs as Not Found
        response->setStatus( 404, "NOT FOUND" );

        CEntity *entity = new CNullEntity();

        // Set entity
        response->SetEntity( entity );

        return true;
    }
    
}

#endif // IASLIB_NETWORKING__
