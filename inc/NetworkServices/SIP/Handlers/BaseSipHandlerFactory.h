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

#ifndef IASLIB_BASESIPHANDLERFACTORY_H__
#define IASLIB_BASESIPHANDLERFACTORY_H__

#include "SipHandlerFactory.h"

namespace IASLib
{
    class CBaseSipHandlerFactory : public CSipHandlerFactory
    {
        public:
            CBaseSipHandlerFactory( void );

            DECLARE_OBJECT( CBaseSipHandlerFactory, CSipHandlerFactory );
    };

} // namespace IASLib

#endif // IASLIB_BASESIPHANDLERFACTORY_H__
#endif // IASLIB_NETWORKING__
