/*
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
    class COracleResultSet : public CResultSet
    {
        protected:
            Cda_Def            *m_pCDA;
            ub2                *m_apnTypes;

        public:
                                COracleResultSet( Cda_Def *pCDA );
            virtual            ~COracleResultSet( void );

                                DEFINE_OBJECT( COracleResultSet )

            virtual int         Close( void );
    };
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_ORA_RESULTSET_H__
