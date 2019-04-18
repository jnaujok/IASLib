/*
 *  Result Set Class
 *
 *  This class stores the result of a stored procedure on those systems 
 * that support returns from a stored procedure. (e.g. Sybase)
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/02/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RESULTSET_H__
#define IASLIB_RESULTSET_H__

#include "../BaseTypes/String_.h"
#include "BaseResult.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    class CResultSet : public CBaseResult
    {
        public:
                                CResultSet( void ) { }
            virtual            ~CResultSet( void ) { }

                                DECLARE_OBJECT( CResultSet, CBaseResult )
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_RESULTSET_H__
