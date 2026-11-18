/**
 *  @file
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
/// oparse() flag: defer the parse until the statement is run.
#define  DEFER_PARSE        1
/// oparse() flag: the server's native SQL.
#define  NATIVE             1
/// oparse() flag: Oracle 7 SQL.
#define  VERSION_7          2

/// The widest Oracle NUMBER, in digits.
#define ORANUMWIDTH			38
/// A buffer size for database names.
#define DB_SIZE             128

#ifndef HDA_SIZE
/// The size of an OCI host data area.
#define HDA_SIZE 512
#endif

/// The longest user name.
#define ORAUIDLEN 32
/// The longest password.
#define ORAPWLEN 32
/// The longest object name.
#define ORANAMELEN 32
/// A buffer size for column values.
#define ORABUFLEN 2000

/* Some Oracle error codes */
/// ORA-01007: variable not in select list.
#define VAR_NOT_IN_LIST			1007
/// ORA-01403: no data found.
#define NO_DATA_FOUND			1403
/// ORA-01405: a NULL value was returned.
#define NULL_VALUE_RETURNED		1405

/* Some SQL and ORA function codes */
/// SQL function code: INSERT.
#define FT_INSERT			3
/// SQL function code: SELECT.
#define FT_SELECT			4
/// SQL function code: UPDATE.
#define FT_UPDATE			5
/// SQL function code: DELETE.
#define FT_DELETE			9

/// OCI function code: oopen().
#define FC_OOPEN			14

namespace IASLib
{
        /// Oracle (OCI 7): logs on and makes connections.
        ///
        /// The first database made initializes OCI (threaded, with the
        /// library's allocator). The last error is kept in static members,
        /// so it is shared by every Oracle database and connection.
        ///
        /// @code
        /// COracleDatabase db;
        /// db.SetDefaults( "PROD", "scott", "tiger" );
        /// CConnection *pCon = db.Connection( NULL, NULL, NULL );   // the defaults
        /// @endcode
    class COracleDatabase : public CDatabase
    {
        protected:
                // Error Message Storage
            static CString          m_strLastMajorMessage;  ///< The last error's message ("None" at first).
            static CString          m_strLastMinorMessage;  ///< The last error's secondary message.
            static int              m_nLastMajorErrorCode;  ///< The last error's code.
            static int              m_nLastMinorErrorCode;  ///< The last error's secondary code.

            static bool             m_bOCIInitialized;      ///< True once OCIInitialize() has run.
            static CObject          m_oOracleProxy;         ///< Not used.

	        CString                 m_strDefaultDB;         ///< The database used when none is given.
	        CString                 m_strDefaultUser;       ///< The user used when none is given.
	        CString                 m_strDefaultPassword;   ///< The password used when none is given.
	        int                     m_nMaxConnections;      ///< Set by SetMaxConnections(); not enforced.
        
        public:
                                    /// Initializes OCI (once) and marks the database connected.
                                    COracleDatabase( void );
                                    /// Deletes every connection.
            virtual                ~COracleDatabase( void );

                                    DEFINE_OBJECT( COracleDatabase )

            /// Resets the database (deleting every connection).
            /// @return False (see CDatabase::Connect(const char*)).
            bool                    Connect( void );

            /// Logs on ("user/password@database") and returns the connection.
            /// A NULL database, user or password uses the default from SetDefaults().
            /// @param strDBName      The database (service name).
            /// @param strUserName    The user.
            /// @param strPassword    The password.
            /// @param strApplication Ignored.
            /// @param strName        A name for the connection.
            /// @return The connection (kept by the database), or NULL if the logon failed.
            virtual CConnection    *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            /// Not supported.
            /// @return NULL.
            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            /// Deletes every connection.
            /// @return 0.
            virtual int             Disconnect( void );

            /// The last error's code.
            virtual int             MajorErrorCode( void );

            /// The last error's secondary code.
            virtual int             MinorErrorCode( void );

            /// The last error's message.
            virtual const char     *MajorErrorMessage( void );

            /// The last error's secondary message.
            virtual const char     *MinorErrorMessage( void );

            /// Does nothing (a placeholder for stored procedure arguments).
            virtual void            StoredProcArgs( const char *strProcName, ... );

            /// @copydoc CDatabase::ReleaseConnection()
            virtual int             ReleaseConnection( CConnection *pCon );

            /// Ignored.
            /// @return True.
            virtual bool            SetTimeout( int nTimeout );

            /// Sets the database, user and password Connection() uses when given NULL.
            /// @param strDB   The database (NULL: "").
            /// @param strUser The user (NULL: "").
            /// @param strPW   The password (NULL: "").
            void                    SetDefaults( const char *strDB, const char *strUser = NULL, const char *strPW = NULL );

            /// Records the most connections wanted (not enforced).
            void                    SetMaxConnections( int nConnects ) { m_nMaxConnections = nConnects; }
    };
} // namespace IASLib

#endif IASLIB_DB_ORACLE__
#endif IASLIB_DATABASE__
#endif IASLIB_ORA_DATABASE_H__

