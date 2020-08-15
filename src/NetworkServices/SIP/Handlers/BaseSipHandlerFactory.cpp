/**
 * SIP Base Handler Factory class
 *
 * This class provides a default SIP Handler Factory that is normally
 * the baseline handler for an SIP Server. 
 * 
 * Typically this class is added to the CSipServer by the constructor
 * and is the baseline for an SIP Server. By default, all the basic 
 * handlers simply return error codes in proper formats.
 * 
 *
 * Author: Jeffrey R. Naujok
 * Created: December 29, 2019
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
    CBaseSipHandlerFactory::CBaseSipHandlerFactory( void ) : CSipHandlerFactory()
    {
        this->registerHandler( new CSipAckHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipByeHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipCancelHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipInviteHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipOptionsHandler( CString( "" ), CString( "" ) ) );
        this->registerHandler( new CSipRegisterHandler( CString( "" ), CString( "" ) ) );
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
