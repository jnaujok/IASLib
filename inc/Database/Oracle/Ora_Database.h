/*
 *  COracleDatabase
 *
 *  This class defines an Oracle 7.3 database environment that can be
 * used to generate connections to retrieve and provide data from/to the
 * Oracle database.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/17/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ORA_DATABASE_H__
#define IASLIB_ORA_DATABASE_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "../Database.h"
#include "../Connection.h"

extern "C"
{
#include <oci.h>
}

/* oparse flags */
#define  DEFER_PARSE        1
#define  NATIVE             1
#define  VERSION_7          2

#define ORANUMWIDTH			38
#define DB_SIZE             128

#ifndef HDA_SIZE
#define HDA_SIZE 512
#endif

#define ORAUIDLEN 32
#define ORAPWLEN 32
#define ORANAMELEN 32
#define ORABUFLEN 2000

/* Some Oracle error codes */
#define VAR_NOT_IN_LIST			1007
#define NO_DATA_FOUND			1403
#define NULL_VALUE_RETURNED		1405

/* Some SQL and ORA function codes */
#define FT_INSERT			3
#define FT_SELECT			4
#define FT_UPDATE			5
#define FT_DELETE			9

#define FC_OOPEN			14

namespace IASLib
{
    class COracleDatabase : public CDatabase
    {
        protected:
                // Error Message Storage
            static CString          m_strLastMajorMessage;
            static CString          m_strLastMinorMessage;
            static int              m_nLastMajorErrorCode;
            static int              m_nLastMinorErrorCode;

            static bool             m_bOCIInitialized;
            static CObject          m_oOracleProxy;

	        CString                 m_strDefaultDB;
	        CString                 m_strDefaultUser;
	        CString                 m_strDefaultPassword;
	        int                     m_nMaxConnections;
        
        public:
                                    COracleDatabase( void );
            virtual                ~COracleDatabase( void );

                                    DEFINE_OBJECT( COracleDatabase )

            bool                    Connect( void );

            virtual CConnection    *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            virtual int             Disconnect( void );

            virtual int             MajorErrorCode( void );

            virtual int             MinorErrorCode( void );

            virtual const char     *MajorErrorMessage( void );

            virtual const char     *MinorErrorMessage( void );

            virtual void            StoredProcArgs( const char *strProcName, ... );

            virtual int             ReleaseConnection( CConnection *pCon );

            virtual bool            SetTimeout( int nTimeout );

            void                    SetDefaults( const char *strDB, const char *strUser = NULL, const char *strPW = NULL );

            void                    SetMaxConnections( int nConnects ) { m_nMaxConnections = nConnects; }
    };
} // namespace IASLib

#endif IASLIB_DB_ORACLE__
#endif IASLIB_DATABASE__
#endif IASLIB_ORA_DATABASE_H__

