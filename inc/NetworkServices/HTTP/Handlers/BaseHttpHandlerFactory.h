/**
 * @file BaseHttpHandlerFactory.h
 * @brief The default HTTP handlers (CBaseHttpHandlerFactory).
 *
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
    /// A factory with the default handlers: GET and POST answer 404 Not Found,
    /// and OPTIONS answers 200 OK with Allow.
    ///
    /// It has no HEAD handler, so the server answers HEAD with whichever GET
    /// handler applies, an application's included. Add it first and the
    /// application's own factories after it, so that they are consulted
    /// before it.
    class CBaseHttpHandlerFactory : public CHttpHandlerFactory
    {
        public:
                /// Registers the GET, POST and OPTIONS handlers.
            CBaseHttpHandlerFactory( void );
                /// Deletes the handlers.
            virtual ~CBaseHttpHandlerFactory( void );

            DEFINE_OBJECT( CBaseHttpHandlerFactory );
    };
} // namespace IASLib

#endif // IASLIB_BASEHTTPHANDLERFACTORY_H__

#endif // IASLIB_NETWORKING__
