/**
 * @file
 * @brief Collection Exception Class.
 *
 *      This class is used for throwing whenever an exception occurs in
 * the collection classes. Some examples of a collection exception are 
 * out-of-range references, invalid functions (for example, calling 
 * "Append" on a Sparse Array.)
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/19/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_COLLECTIONEXCEPTION_H__
#define IASLIB_COLLECTIONEXCEPTION_H__

#include "Exception.h"

namespace IASLib
{
    /// Thrown by the collection classes: out-of-range references, or a
    /// method that does not apply (for example, Append on a sparse array).
    class CCollectionException : public CException
    {
        public:
                                /// Makes an exception with a message and a severity.
                                CCollectionException( const char *strMessage, 
                                                      CException::ExceptionSeverity nExceptionLevel = CException::NORMAL );
            virtual            ~CCollectionException( void );   ///< Destructor.

                                DECLARE_OBJECT( CCollectionException, CException );

            virtual void        Throw( void ) { throw *this; }   ///< Throws a copy of this exception, as this class.

            virtual void        Rethrow( void ) { throw *this; }   ///< Same as Throw(); used by IASLIB_RETHROW.


    };

/// Throws a CCollectionException with message @p x, recording the file and line.
#define IASLIB_THROW_COLLECTION_EXCEPTION(x) {CCollectionException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}
/// Throws a CCollectionException with message @p x and severity @p y, recording the file and line.
#define IASLIB_THROW_COLLECTION_EXCEPTION2(x,y) {CCollectionException oE(x,y);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_COLLECTIONEXCEPTION_H__
