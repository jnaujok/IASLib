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

#include "CollectionException.h"

namespace IASLib
{
    CCollectionException::CCollectionException( const char *strMessage, CException::ExceptionSeverity nExceptionLevel )
    : CException( strMessage, nExceptionLevel )
    {
    }

    CCollectionException::~CCollectionException( void )
    {
    }

} // namespace IASLib
