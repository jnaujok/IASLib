/**
 *  @file
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
        /// The results of a stored procedure call: output parameters, return value, result sets.
        ///
        /// Made by CConnection::StoredProc(); the caller deletes it. It owns
        /// the result sets and output parameters it returns.
    class CStoredProc : public CObject
    {
        protected:

        public:
                                /// Creates an empty result.
                                CStoredProc( void ) {}
                                /// Destroys the result.
            virtual            ~CStoredProc( void ) {}

                                DECLARE_OBJECT( CStoredProc, CObject )

            /// Frees the results.
            /// @return The vendor's result (Oracle's returns false even when it worked).
            virtual bool        Close( void ) = 0;

            /// How many output parameters there are.
            virtual size_t      OutParamCount( void ) = 0;
            /// An output parameter's value.
            /// @param nParam The parameter, from 0.
            /// @return The value, or NULL if there is no such parameter.
            virtual const char *OutParameters( size_t nParam ) = 0;

            /// The next result set (NULL when there are no more, or the vendor returns none).
            virtual CResultSet *ResultSet( void ) = 0;

            /// The procedure's return status.
            virtual int         ReturnValue() = 0;
            /// True if the call succeeded.
            virtual bool        IsValid( void ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_DATABASE
#endif // IASLIB_STOREDPROC_H__


