/**
 * @file GenericHandler.h
 * @brief The common base of request handlers (CGenericHandler).
 *
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

#ifndef IASLIB_GENERICHANDLER_H__
#define IASLIB_GENERICHANDLER_H__

#include "BaseTypes/Object.h"

namespace IASLib
{
    /// The common base of the HTTP and SIP request handlers.
    ///
    /// It holds nothing itself; CHttpHandler and CSipHandler add the
    /// protocol's handling. A handler is made by a handler factory that is
    /// registered with a server.
    class CGenericHandler : public CObject
    {
        public:
            DEFINE_OBJECT( CGenericHandler );

                /// Creates a handler.
            CGenericHandler( void );
                /// Destroys the handler.
            virtual ~CGenericHandler( void );
    };
} // namespace IASLib

#endif // IASLIB_GENERICHANDLER_H__

#endif // IASLIB_NETWORKING__
