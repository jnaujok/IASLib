/**
 * HTTP Handler class
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/Handlers/HttpHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpHandler, CGenericHandler );

    CHttpHandler::CHttpHandler( void )
    {
        m_pServer = NULL;
    }

    CHttpHandler::CHttpHandler( CString uri, CString version )
    {
        m_strUri = uri;
        m_strVersion = version;
        m_pServer = NULL;
    }

    CHttpHandler::~CHttpHandler( void )
    {
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
