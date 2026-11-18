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
            /// Makes an exception with a message and a severity.
            CStringException( const char *strMessage, CException::ExceptionSeverity nExceptionLevel );
                        /// Makes an exception from a message number: 0 gives
                        /// "Successful. (?)", anything else "Unknown String Exception!".
                        /// @note The call stack used to keep saying "Unknown Exception".
                        CStringException( int nStringMessage );
            virtual    ~CStringException( void );   ///< Destructor.

                        DEFINE_OBJECT( CStringException );

            virtual void        Throw( void ) { throw *this; }   ///< Throws a copy of this exception, as this class.

            virtual void        Rethrow( void ) { throw *this; }   ///< Same as Throw(); used by IASLIB_RETHROW.


    };

/// Throws a CStringException for message number @p x, recording the file and line.
#define IASLIB_THROW_STRING_EXCEPTION(x) {CStringException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}
/// Throws a CStringException with message @p x and severity @p y, recording the file and line.
#define IASLIB_THROW_STRMSG_EXCEPTION(x,y) {CStringException oE(x,y);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_STRINGEXCEPTION_H__
