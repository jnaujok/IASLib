/**
 *  @file
 *  COracleResultSet
 *
 *  This class defines a placeholder class for Oracle Result sets, as 
 * Oracle does not return result sets from a stored procedure. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/24/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ORA_RESULTSET_H__
#define IASLIB_ORA_RESULTSET_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "../ResultSet.h"
extern "C"
{
#include <oci.h>
}

namespace IASLib
{
        /// The rows left on an Oracle cursor after a stored procedure call.
        ///
        /// Oracle does not return result sets from stored procedures, so
        /// this is normally empty. Owned by its COracleStoredProc.
    class COracleResultSet : public CResultSet
    {
        protected:
            Cda_Def            *m_pCDA;         ///< The cursor data area (not owned).
            ub2                *m_apnTypes;     ///< Each column's OCI data type.

        public:
                                /// Reads any rows from the cursor (check IsValid()).
                                COracleResultSet( Cda_Def *pCDA );
                                /// Frees the rows.
            virtual            ~COracleResultSet( void );

                                DEFINE_OBJECT( COracleResultSet )

            /// Frees the rows.
            /// @return 0.
            virtual int         Close( void );
    };
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_ORA_RESULTSET_H__
