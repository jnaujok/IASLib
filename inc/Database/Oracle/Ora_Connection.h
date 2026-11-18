/**
 *  @file
 *  COracleConection
 *
 *  This class defines an Oracle 7.3 database connection. This means
 * that this can be used to connect to any database after 7.3. However
 * Oracle versions after 9i are very inefficent when accessed with this
 * code, and the Oracle10g connection should be used instead.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/15/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ORA_CONNECTION_H__
#define IASLIB_ORA_CONNECTION_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "../../BaseTypes/String_.h"
#include "../../BaseTypes/Date.h"
#include "../Connection.h"
#include "../Database.h"

#ifdef IASLIB_MULTI_THREADED__
#include "../../Threads/Mutex.h"
#endif

extern "C"
{
#include <oci.h>
}

namespace IASLib
{
        /// A connection to an Oracle database through the OCI 7 calls.
        ///
        /// Made by COracleDatabase::Connection(), which logs on first. Any
        /// failure to open, parse or run a statement marks the connection
        /// dead, and every later call on it fails. The error functions
        /// report the database's (shared) last error.
    class COracleConnection : public CConnection
    {
        protected:
            CDatabase              *m_pDatabase;        ///< The database that made it (not owned).
            Lda_Def                *m_pLDA;             ///< The OCI logon data area (owned).
            ub1                    *m_pHDA;             ///< The OCI host data area (owned).
            bool                    m_bValid;           ///< True until Release().
            bool                    m_bDeadlock;        ///< Set by SetDeadlock(); not used otherwise.
            bool                    m_bDead;            ///< True once a call failed.
            long                    m_lAffectedRows;    ///< Not set yet (always 0).
#ifdef IASLIB_MULTI_THREADED__
            static CMutex           m_mutexSynchro;     ///< Not used yet.
#endif
        public:
                                    /// Wraps a logged-on OCI session (and takes ownership of its areas).
                                    /// @param pLDA    The logon data area.
                                    /// @param pHDA    The host data area.
                                    /// @param strName A name for the connection.
                                    COracleConnection( Lda_Def *pLDA, ub1 *pHDA, const char *strName = "NoName" );
                                    /// Logs off (see Release()) and leaves the database's list.
            virtual                ~COracleConnection( void );

                                    DEFINE_OBJECT( COracleConnection )

            /// Sets the database that made the connection (for errors and Release()).
            void                    SetDatabase( CDatabase *pDatabase );

            /// Marks a transaction as open (Oracle always has one).
            /// @return True.
            virtual bool            BeginTransaction( void );
            /// Commits (ocom()).
            /// @return False if no transaction was marked open, or the commit failed.
            virtual bool            CommitTransaction( void );
            /// Rolls back (orol()).
            /// @return False if no transaction was marked open, or the rollback failed.
            virtual bool            RollbackTransaction( void );

            /// Runs a SELECT and returns all its rows.
            /// @param strQueryString The query; anything that does not start with
            ///                       SELECT returns NULL.
            /// @param bUpdatable     True to allow DeleteRow(), InsertRow() and UpdateRow().
            /// @return A new cursor (the caller deletes it), or NULL on failure.
            virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = 0 );
            /// Runs a statement and commits at once.
            /// @return False if it failed (the connection is then dead).
            virtual bool            Execute( const char *strExecString );

            /// The database's last error code.
            virtual int             MajorErrorCode( void );
            /// The database's last secondary error code.
            virtual int             MinorErrorCode( void );

            /// The database's last error message.
            virtual const char     *MajorErrorMessage( void );
            /// The database's last secondary error message.
            virtual const char     *MinorErrorMessage( void );

            /// Leaves the database's list, commits, logs off and frees the OCI areas.
            /// @return True.
            virtual bool            Release( void );

            /// @copydoc CConnection::SQLTable()
            virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL );

            /// Calls a stored procedure (an anonymous PL/SQL block or call).
            /// @param strProcName  The call.
            /// @param strParamList The output parameters, as "name type" pairs
            ///                     separated by '|' ("count int|name varchar(32)").
            /// @return A new result (the caller deletes it), or NULL on failure.
            virtual CStoredProc    *StoredProc( const char *strProcName, const char *strParamList = NULL );

            /// Records whether the connection is in a deadlock (not used otherwise).
            void                    SetDeadlock( bool bLocked ) { m_bDeadlock = bLocked; }

            /// True once a call has failed.
            bool                    IsDead( void ) { return m_bDead; }

		    /// DB_ORACLE.
		    virtual int				GetDBType( void ) { return DB_ORACLE; }
    };
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_ORA_CONNECTION_H__
