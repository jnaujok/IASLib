/*
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
    class COracleStoredProc : public CStoredProc
    {
        protected:
            Cda_Def            *m_pCDA;
            CConnection        *m_pConnection;
            int                 m_nResultSets;
            int                 m_nCurrentResultSet;
            CResultSet        **m_aResultSets;
            size_t              m_nOutParams;
            COutParam         **m_aOutParams;
            int                 m_nReturnValue;
            bool                m_bValid;

        public:
                                COracleStoredProc( CConnection *pConnection, Cda_Def *pCDA, const char *strOutParams );
            virtual            ~COracleStoredProc( void );

                                DEFINE_OBJECT( COracleStoredProc )

            virtual bool        Close( void );

            virtual size_t      OutParamCount( void );
            virtual const char *OutParameters( size_t nParam );

            virtual CResultSet *ResultSet( void );

            virtual int         ReturnValue( void );

            virtual bool        IsValid( void ) { return m_bValid; }

        private:
            void                GetOutputParams( void );
            void                GetStatusParam( void );
    };
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_ORA_STOREDPROC_H__

