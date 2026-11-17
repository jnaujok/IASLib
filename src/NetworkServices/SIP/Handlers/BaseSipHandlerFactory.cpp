/**
 * Base SIP Handler Factory class
 *
 * Author: Jeffrey R. Naujok
 * Created: December 22, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/BaseSipHandlerFactory.h"
#include "NetworkServices/SIP/Handlers/SipAckHandler.h"
#include "NetworkServices/SIP/Handlers/SipByeHandler.h"
#include "NetworkServices/SIP/Handlers/SipCancelHandler.h"
#include "NetworkServices/SIP/Handlers/SipInviteHandler.h"
#include "NetworkServices/SIP/Handlers/SipOptionsHandler.h"
#include "NetworkServices/SIP/Handlers/SipRegisterHandler.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CBaseSipHandlerFactory, CSipHandlerFactory );

    CBaseSipHandlerFactory::CBaseSipHandlerFactory( void ) : CSipHandlerFactory()
    {
        this->registerHandler( new CSipInviteHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipAckHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipByeHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipCancelHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipOptionsHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipRegisterHandler( CString( "" ), CString( "" ) ) );
    }

    CBaseSipHandlerFactory::~CBaseSipHandlerFactory( void )
    {
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
