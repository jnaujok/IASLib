/*
 *  Stored Procedure Class
 *
 *  Abstract base class for executing a stored procedure.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/05/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STOREDPROC_H__
#define IASLIB_STOREDPROC_H__

#include "ResultSet.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    class CStoredProc : public CObject
    {
        protected:

        public:
                                CStoredProc( void ) {}
            virtual            ~CStoredProc( void ) {}

                                DECLARE_OBJECT( CStoredProc, CObject )

            virtual bool        Close( void ) = 0;

            virtual size_t      OutParamCount( void ) = 0;
            virtual const char *OutParameters( size_t nParam ) = 0;

            virtual CResultSet *ResultSet( void ) = 0;

            virtual int         ReturnValue() = 0;
            virtual bool        IsValid( void ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_DATABASE
#endif // IASLIB_STOREDPROC_H__


