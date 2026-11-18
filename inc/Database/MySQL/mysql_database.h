/**
 *  @file
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
        /// MySQL 5.x: makes connections to a MySQL server.
        ///
        /// @note Only declared: the library has no MySQL source file yet,
        /// so this class cannot be used. The members follow
        /// COracleDatabase.
    class CMySQLDatabase : public CDatabase
    {
        protected:
                // Error Message Storage
            static CString          m_strLastMajorMessage;  ///< The last error's message.
            static CString          m_strLastMinorMessage;  ///< The last error's secondary message.
            static int              m_nLastMajorErrorCode;  ///< The last error's code.
            static int              m_nLastMinorErrorCode;  ///< The last error's secondary code.

            static bool             m_bMySQLLibInitialized; ///< True once the client library is initialized.

            bool                    m_bMySQLInitialized;    ///< True once this database's handle is initialized.
	        CString                 m_strDefaultDB;         ///< The database used when none is given.
	        CString                 m_strDefaultUser;       ///< The user used when none is given.
	        CString                 m_strDefaultPassword;   ///< The password used when none is given.
	        int                     m_nMaxConnections;      ///< The most connections wanted.
            MYSQL                  *m_pMySqlEnvironment;    ///< The MySQL handle.
        
        public:
                                    /// Creates the database.
                                    CMySQLDatabase( void );
                                    /// Deletes every connection.
            virtual                ~CMySQLDatabase( void );

                                    DEFINE_OBJECT( CMySQLDatabase )

            /// Starts the client library.
            bool                    Connect( void );

            /// @copydoc CDatabase::Connection()
            virtual CConnection    *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            /// @copydoc CDatabase::BulkCopyConnection()
            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            /// @copydoc CDatabase::Disconnect()
            virtual int             Disconnect( void );

            /// The last error's code.
            virtual int             MajorErrorCode( void );

            /// The last error's secondary code.
            virtual int             MinorErrorCode( void );

            /// The last error's message.
            virtual const char     *MajorErrorMessage( void );

            /// The last error's secondary message.
            virtual const char     *MinorErrorMessage( void );

            /// A placeholder for stored procedure arguments.
            virtual void            StoredProcArgs( const char *strProcName, ... );

            /// @copydoc CDatabase::ReleaseConnection()
            virtual int             ReleaseConnection( CConnection *pCon );

            /// @copydoc CDatabase::SetTimeout()
            virtual bool            SetTimeout( int nTimeout );

            /// Sets the database, user and password Connection() uses when given NULL.
            void                    SetDefaults( const char *strDB, const char *strUser = NULL, const char *strPW = NULL );

            /// Records the most connections wanted.
            void                    SetMaxConnections( int nConnects ) { m_nMaxConnections = nConnects; }
    };
} // namespace IASLib

#endif IASLIB_DB_MYSQL__
#endif IASLIB_DATABASE__
#endif IASLIB_MYSQL_DATABASE_H__

