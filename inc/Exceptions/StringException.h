/**
 * @file
 * @brief String Exception Class.
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
    /// Thrown by the string classes, for example when memory for a string
    /// cannot be allocated.
    class CStringException : public CException
    {
        public:
            /// Makes a string exception with a message and severity.
            /// @param strMessage The message text.
            /// @param nExceptionLevel The severity.
            CStringException( const char *strMessage, CException::ExceptionSeverity nExceptionLevel );
                        /// Makes a string exception from a message code, with NORMAL severity.
                        /// @param nStringMessage 0 gives "Successful. (?)"; any other value
                        ///        gives "Unknown String Exception!". The bottom stack frame
                        ///        keeps the message "Unknown Exception".
                        CStringException( int nStringMessage );
            /// Destructor.
            virtual    ~CStringException( void );

                        DEFINE_OBJECT( CStringException );

            /// Throws a copy of this exception with its real type.
            /// @throws CStringException always.
            virtual void        Throw( void ) { throw *this; }


    };

/// Throws a CStringException for message code @p x, recording the current file and line.
#define IASLIB_THROW_STRING_EXCEPTION(x) {CStringException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}
/// Throws a CStringException with message @p x and severity @p y,
/// recording the current file and line.
#define IASLIB_THROW_STRMSG_EXCEPTION(x,y) {CStringException oE(x,y);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_STRINGEXCEPTION_H__
