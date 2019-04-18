/*
 * String Exception Class
 *
 *      This class is used for throwing whenever an exception occurs in
 * the string handling classes. String exception examples are failure to
 * allocate memory for a string, or passing a non-null terminated string
 * to a string function, or causing reference counting errors in the
 * string stub.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STRINGEXCEPTION_H__
#define IASLIB_STRINGEXCEPTION_H__

#include "Exception.h"

namespace IASLib
{
    class CStringException : public CException
    {
        public:
            CStringException( const char *strMessage, CException::ExceptionSeverity nExceptionLevel );
                        CStringException( int nStringMessage );
            virtual    ~CStringException( void );

                        DEFINE_OBJECT( CStringException );

            virtual void        Throw( void ) { throw *this; }


    };

#define IASLIB_THROW_STRING_EXCEPTION(x) {static CStringException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}
#define IASLIB_THROW_STRMSG_EXCEPTION(x,y) {static CStringException oE(x,y);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_STRINGEXCEPTION_H__
