/*
 * Collection Exception Class
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
    class CCollectionException : public CException
    {
        public:
                                CCollectionException( const char *strMessage, 
                                                      CException::ExceptionSeverity nExceptionLevel = CException::NORMAL );
            virtual            ~CCollectionException( void );

                                DECLARE_OBJECT( CCollectionExcetption, CObject );

            virtual void        Throw( void ) { throw *this; }


    };

#define IASLIB_THROW_COLLECTION_EXCEPTION(x) {static CCollectionException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}
#define IASLIB_THROW_COLLECTION_EXCEPTION2(x,y) {static CCollectionException oE(x,y);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_COLLECTIONEXCEPTION_H__
