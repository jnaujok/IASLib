/**
 * SIP Handler Factory class
 *
 * This class provides a factory for generating SIP Handlers. 
 * 
 * Factories have handlers registered to them, and then they are added
 * to the SIP Server. Once added to the server, they override any
 * factories previously added, allowing a chain of handlers to deal 
 * with any particular request.
 *
 * Author: Jeffrey R. Naujok
 * Created: December 29, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPHANDLERFACTORY_H__
#define IASLIB_SIPHANDLERFACTORY_H__

#include "NetworkServices/SIP/Handlers/SipHandler.h"
#include "NetworkServices/SIP/SipRequest.h"
#include "NetworkServices/SIP/SipResponse.h"
#include "BaseTypes/Uuid.h"

namespace IASLib
{
    class CSipHandlerFactory : public CObject
    {
        protected:
            CHash           m_handlerMap;

        public:
            DEFINE_OBJECT( CSipHandlerFactory );

            CSipHandlerFactory( );

            virtual ~CSipHandlerFactory( void );

            virtual CSipHandler *getHandler( CSipRequest *request, CUUID erid );
            virtual void registerHandler( CSipHandler *handlerFactory );
            virtual bool isValidHandler( const char *method ); 

            static CString getAllowedList( void );
    };

} // namespace IASLib

#endif // IASLIB_SIPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__

