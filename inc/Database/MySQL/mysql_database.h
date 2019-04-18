/*
 *  CMySQLDatabase
 *
 *  This class defines a MySQL 5.x database environment that can be
 * used to generate connections to retrieve and provide data from/to the
 * MySQL database.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/03/2008
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2008, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_MYSQL_DATABASE_H__
#define IASLIB_MYSQL_DATABASE_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_MYSQL__

#include "../Database.h"
#include "../Connection.h"

extern "C"
{
#include <mysql.h>
}

namespace IASLib
{
    class CMySQLDatabase : public CDatabase
    {
        protected:
                // Error Message Storage
            static CString          m_strLastMajorMessage;
            static CString          m_strLastMinorMessage;
            static int              m_nLastMajorErrorCode;
            static int              m_nLastMinorErrorCode;

            static bool             m_bMySQLLibInitialized;

            bool                    m_bMySQLInitialized;
	        CString                 m_strDefaultDB;
	        CString                 m_strDefaultUser;
	        CString                 m_strDefaultPassword;
	        int                     m_nMaxConnections;
            MYSQL                  *m_pMySqlEnvironment;
        
        public:
                                    CMySQLDatabase( void );
            virtual                ~CMySQLDatabase( void );

                                    DEFINE_OBJECT( CMySQLDatabase )

            int                     Connect( void );

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

#endif IASLIB_DB_MYSQL__
#endif IASLIB_DATABASE__
#endif IASLIB_MYSQL_DATABASE_H__

