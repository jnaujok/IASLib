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

#include "NetworkServices/GenericHandler.h"
#include "BaseTypes/String_.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CGenericHandler, CObject );
    
    CGenericHandler::CGenericHandler( void )
    {

    }
    
    CGenericHandler::~CGenericHandler( void )
    {

    }

} // namespace IASLib


#endif // IASLIB_NETWORKING__
