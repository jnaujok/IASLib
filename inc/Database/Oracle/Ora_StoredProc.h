/**
 *  @file
 *  COracleStoredProc
 *
 *  This class defines an Oracle 7.3 stored procedure call. It should
 * be noted that Oracle does not allow for result sets (select statements)
 * to be returned from a stored procedure. Thus, this function only
 * supports parameters, return values, and output parameters.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/20/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ORA_STOREDPROC_H__
#define IASLIB_ORA_STOREDPROC_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "../ResultSet.h"
#include "../Connection.h"
#include "../StoredProc.h"
#include "../OutParam.h"

extern "C"
{
#include <oci.h>
}

namespace IASLib
{
        /// The results of an Oracle stored procedure call: output parameters and return value.
        ///
        /// Made by COracleConnection::StoredProc(); the caller deletes it.
        /// The output parameters are bound by name and type from the list
        /// given ("name type|name type", types numeric(n), int, float,
        /// char(n), varchar(n) or datetime), and read as text after the call.
    class COracleStoredProc : public CStoredProc
    {
        protected:
            Cda_Def            *m_pCDA;                 ///< The cursor data area of the call.
            CConnection        *m_pConnection;          ///< The connection (not owned).
            int                 m_nResultSets;          ///< How many result sets.
            int                 m_nCurrentResultSet;    ///< The next one ResultSet() returns.
            CResultSet        **m_aResultSets;          ///< The result sets (owned).
            size_t              m_nOutParams;           ///< How many output parameters.
            COutParam         **m_aOutParams;           ///< The output parameters (owned).
            int                 m_nReturnValue;         ///< The return status (0: not set yet).
            bool                m_bValid;               ///< True if the call succeeded.

        public:
                                /// Binds the output parameters, runs the call and collects the results.
                                /// @param pConnection  The connection.
                                /// @param pCDA         The parsed call.
                                /// @param strOutParams The output parameters ("name type|..."; NULL: none).
                                COracleStoredProc( CConnection *pConnection, Cda_Def *pCDA, const char *strOutParams );
                                /// Frees the results.
            virtual            ~COracleStoredProc( void );

                                DEFINE_OBJECT( COracleStoredProc )

            /// Frees the output parameters and result sets.
            /// @return False (always).
            virtual bool        Close( void );

            /// How many output parameters there are.
            virtual size_t      OutParamCount( void );
            /// An output parameter's value (NULL if there is no such parameter).
            virtual const char *OutParameters( size_t nParam );

            /// The next result set (NULL when there are no more).
            virtual CResultSet *ResultSet( void );

            /// The return status (not filled in yet: 0).
            virtual int         ReturnValue( void );

            /// True if the call succeeded.
            virtual bool        IsValid( void ) { return m_bValid; }

        private:
            /// Not implemented yet.
            void                GetOutputParams( void );
            /// Not implemented yet.
            void                GetStatusParam( void );
    };
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_ORA_STOREDPROC_H__

