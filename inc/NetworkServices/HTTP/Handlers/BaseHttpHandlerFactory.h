/**
 * Base HTTP Handler Factory class
 *
 * The default handlers: GET and POST answer 404 Not Found (there is
 * nothing to serve), and OPTIONS answers 200 OK with Allow. HEAD is
 * answered by the server using the GET handler. Applications add their own
 * factory in front of this one to serve content.
 *
 * Author: Jeffrey R. Naujok
 * Created: October 4, 2019
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
            virtual ~CBaseHttpHandlerFactory( void );

            DEFINE_OBJECT( CBaseHttpHandlerFactory );
    };
} // namespace IASLib

#endif // IASLIB_BASEHTTPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__
