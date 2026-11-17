/**
 * SIP Handler class
 *
 * This class handles a particular request type for a SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipHandler, CObject );

    CSipHandler::CSipHandler( CString uri, CString version )
    {
        m_strUri = uri;
        m_strVersion = version;
        m_pServer = NULL;
    }

    CSipHandler::~CSipHandler( void )
    {
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__
