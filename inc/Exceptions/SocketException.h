/*
 * Socket Exception Class
 *
 *      This class wraps socket based exceptions. When a socket fails,
 * it can throw an exception of this type that can be caught by a handler
 * at a lower level.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SOCKETEXCEPTION_H__
#define IASLIB_SOCKETEXCEPTION_H__

#include "Exception.h"

namespace IASLib
{
    class CSocketException : public CException
    {
        protected:
            int         m_nErrorNumber;

        public:
                        CSocketException( int nErrorNumber );
            virtual    ~CSocketException( void );

                        DEFINE_OBJECT( CSocketException );
            virtual void        Throw( void ) { throw *this; }

    };
} // namespace IASLib

#define IASLIB_THROW_SOCKET_EXCEPTION(x) {static CSocketException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

#endif // __SOCKETEXCEPTION_H__
