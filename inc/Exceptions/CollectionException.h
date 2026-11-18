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
    /// Thrown by the collection classes (arrays, lists, hashes and the
    /// template collections) on errors such as an index out of range.
    class CCollectionException : public CException
    {
        public:
                                /// Makes a collection exception.
                                /// @param strMessage The message text.
                                /// @param nExceptionLevel The severity.
                                CCollectionException( const char *strMessage, 
                                                      CException::ExceptionSeverity nExceptionLevel = CException::NORMAL );
            /// Destructor.
            virtual            ~CCollectionException( void );

                                DECLARE_OBJECT( CCollectionException, CException );

            /// Throws a copy of this exception with its real type.
            /// @throws CCollectionException always.
            virtual void        Throw( void ) { throw *this; }


    };

/// Throws a CCollectionException with message @p x (NORMAL severity),
/// recording the current file and line.
#define IASLIB_THROW_COLLECTION_EXCEPTION(x) {CCollectionException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}
/// Throws a CCollectionException with message @p x and severity @p y,
/// recording the current file and line.
#define IASLIB_THROW_COLLECTION_EXCEPTION2(x,y) {CCollectionException oE(x,y);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_COLLECTIONEXCEPTION_H__
