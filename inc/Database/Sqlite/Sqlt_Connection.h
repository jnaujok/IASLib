/**
 *  @file
 *  SQL Lite Connection
 *
 *      Connection class for attaching to a SQLite database. Note
 * that because of the non-thread-safe nature of a SQLite connection,
 * each connection to the database is basically its own complete
 * database session.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 05/13/2003
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SQLITECONNECTION_H__
#define IASLIB_SQLITECONNECTION_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_SQLITE__

#include "../../BaseTypes/Date.h"
#include "../Connection.h"
#include "sqlite3.h"

namespace IASLib
{
    class CSQLiteDatabase;

    /// The type of sqlite3_open(), loaded from the SQLite library.
    typedef int (open_func_type)(const char *, sqlite3 **);
    /// The type of sqlite3_free().
    typedef void (free_func_type)( void * );
    /// The type of sqlite3_busy_timeout().
    typedef int  (busy_func_type)(sqlite3*, int ms);
    /// The type of sqlite3_close().
    typedef int  (close_func_type)(sqlite3 *);
    /// The type of sqlite3_errmsg().
    typedef const char *(error_msg_func_type)(sqlite3*);
    /// The type of sqlite3_exec().
    typedef int (exec_func_type)(sqlite3*,const char *,sqlite3_callback,void *,char ** );

        /// A connection to one SQLite database file.
        ///
        /// Made by CSQLiteDatabase::Connection(). Each connection is its
        /// own SQLite session; use it from one thread at a time. The SQLite
        /// functions are loaded from the shared library at run time.
        /// Transactions are buffered: between BeginTransaction() and
        /// CommitTransaction(), Execute() only collects the statements, and
        /// the commit runs them all at once (so their errors show only
        /// then). Stored procedures, row changes through cursors and
        /// SQLTable() are not supported.
    class CSQLiteConnection : public CConnection
    {
        protected:
            sqlite3                *m_pSLDatabase;          ///< The SQLite handle (NULL once released).
            char                   *m_pstrErrorStorage;     ///< The error text sqlite3_exec() returned.
            CString                 m_strLastErrorCode;     ///< The last error's message.
            CString                 m_strTransactionBuffer; ///< The statements of the open transaction.
            int                     m_nLastErrorCode;       ///< The last error's code (SQLite's, or IASLIB_DB_ERROR_*).
            CSQLiteDatabase       *m_pDatabase;             ///< The database that made this connection (not owned).
            bool                    m_bValid;               ///< True while the handle is open.

        public:
                                    /// Opens a database file, with a ten-second busy timeout.
                                    /// If it cannot be opened, the connection is not
                                    /// Connected() and MajorErrorMessage() says why.
                                    /// The last argument is the database that made it
                                    /// (not owned; told when the connection goes).
                                    /// @param strDBName The database file.
                                    /// @param strName   A name for the connection.
                                    CSQLiteConnection( const char *strDBName, const char *strName, CSQLiteDatabase * );
                                    /// Closes the handle and takes the connection out of its database's list.
            virtual                ~CSQLiteConnection( void );

                                    DEFINE_OBJECT(CSQLiteConnection);

            /// Starts buffering statements for a transaction ("BEGIN;").
            /// @return True (a transaction is open).
            virtual bool            BeginTransaction( void );
            /// Runs the buffered statements, ending with "END;".
            /// @return False if no transaction was open, or the statements failed.
            virtual bool            CommitTransaction( void );
            /// Discards the buffered statements.
            /// @return True.
            virtual bool            RollbackTransaction( void );

            /// Runs a SELECT and returns all its rows.
            /// The column names are upper-cased, without any "table." prefix
            /// or parentheses. A query that does not start with SELECT fails
            /// with IASLIB_DB_ERROR_BAD_QUERY.
            /// @param strQueryString The query (a ';' is added if missing).
            /// @param bUpdatable     Recorded, but SQLite cursors cannot change rows.
            /// @return A new cursor (the caller deletes it), or NULL on failure.
            ///         If SQLite fails without an error message, the cursor is
            ///         returned but not valid.
            virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = false );
            /// Runs a statement (or, in a transaction, adds it to the buffer).
            /// @param strExecString The SQL (a ';' is added if missing).
            /// @return False if it failed, or the connection is dead.
            virtual bool            Execute( const char *strExecString );

            /// The last error's code (SQLite's, or IASLIB_DB_ERROR_*).
            virtual int             MajorErrorCode( void );
            /// The same as MajorErrorCode().
            virtual int             MinorErrorCode( void );

            /// The last error's message.
            virtual const char     *MajorErrorMessage( void );
            /// The same as MajorErrorMessage().
            virtual const char     *MinorErrorMessage( void );

            /// Closes the handle and takes the connection out of its database's list.
            /// The connection is not deleted; it is dead afterwards.
            /// @return True.
            virtual bool            Release( void );

            /// Not supported yet.
            /// @return "".
            virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL );

            /// Not supported: SQLite has no stored procedures.
            /// @return NULL, with IASLIB_DB_ERROR_UNSUPPORTED.
            virtual CStoredProc    *StoredProc( const char *strProcCall, const char *strParamList = NULL );

            /// True once the connection has been released or failed to open.
            virtual bool            IsDead( void );

            /// Sets how long SQLite waits for a locked database.
            /// @param nTimeMS The time in milliseconds.
            void                    SetTimeout( int nTimeMS );

		    /// DB_SQLITE.
		    virtual int				GetDBType( void ) { return DB_SQLITE; }
        private:
            open_func_type         *m_fnSqlOpen;        ///< sqlite3_open().
            free_func_type         *m_fnSqlFree;        ///< sqlite3_free().
            busy_func_type         *m_fnSqlBusyTime;    ///< sqlite3_busy_timeout().
            close_func_type        *m_fnSqlClose;       ///< sqlite3_close().
            error_msg_func_type    *m_fnSqlErrMsg;      ///< sqlite3_errmsg().
            exec_func_type         *m_fnSqlExec;        ///< sqlite3_exec().
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DB_SQLLITE__
#endif // IASLIB_SQLLITECONNECTION_H__
