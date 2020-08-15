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

#ifndef IASLIB_GENERICHANDLER_H__
#define IASLIB_GENERICHANDLER_H__

#include "BaseTypes/Object.h"

namespace IASLib
{
    class CGenericHandler : public CObject
    {
        public:
            DEFINE_OBJECT( CGenericHandler );

            CGenericHandler( void );
            virtual ~CGenericHandler( void );
    };
} // namespace IASLib

#endif // IASLIB_GENERICHANDLER_H__

#endif // IASLIB_NETWORKING__
