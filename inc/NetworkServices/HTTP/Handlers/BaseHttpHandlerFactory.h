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

#ifndef IASLIB_BASEHTTPHANDLERFACTORY_H__
#define IASLIB_BASEHTTPHANDLERFACTORY_H__

#include "HttpHandlerFactory.h"

namespace IASLib
{
    class CBaseHttpHandlerFactory : public CHttpHandlerFactory
    {
        public:
            CBaseHttpHandlerFactory( void );

            DECLARE_OBJECT( CBaseHttpHandlerFactory, CHttpHandlerFactory );
    };

} // namespace IASLib

#endif // IASLIB_BASEHTTPHANDLERFACTORY_H__
#endif // IASLIB_NETWORKING__
