/**
 * @file
 * @brief SQL Lite Connection.
 *
 *      A connection to a SQLite database file (or ":memory:"). Each
 * connection is its own database session; use a connection from one
 * thread at a time.
 *
 *      Execute() runs any number of statements; Cursor() runs exactly one
 * that returns rows (SELECT, WITH, PRAGMA, VALUES...) and keeps all of
 * them in memory. The forms taking a CStringArray bind its strings to the
 * statement's ? parameters, in order, so values need no quoting.
 *
 *      Errors: MajorErrorCode() is IASLIB_DB_OK, one of the IASLIB_DB_ERROR_*
 * codes (negative) for the library's own errors, or SQLite's result code
 * (positive); MinorErrorCode() is SQLite's extended result code.
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
#include "../../Collections/StringArray.h"
#include "../Connection.h"
#include "sqlite3.h"

namespace IASLib
{
    class CSQLiteDatabase;
    class CSQLiteCursor;

    /// A connection to a SQLite database file (or ":memory:").
    ///
    /// Each connection is its own database session; use a connection from
    /// one thread at a time. Connections are normally made, and owned, by
    /// CSQLiteDatabase::Connection(). Cursors it returns belong to the
    /// caller, who deletes them.
    ///
    /// @code
    /// CSQLiteDatabase oDatabase;
    /// CSQLiteConnection *pConnection = (CSQLiteConnection *)oDatabase.Connection( ":memory:", "", "" );
    /// pConnection->Execute( "CREATE TABLE t (name TEXT)" );
    /// CStringArray aValues;
    /// aValues.Push( CString( "O'Brien" ) );
    /// pConnection->Execute( "INSERT INTO t VALUES (?)", aValues );    // no quoting needed
    /// CCursor *pCursor = pConnection->Cursor( "SELECT name FROM t" );
    /// CString strName = pCursor->GetColumn( "name" );               // "O'Brien"
    /// delete pCursor;
    /// @endcode
    class CSQLiteConnection : public CConnection
    {
        protected:
            sqlite3                *m_pSLDatabase;          ///< The SQLite handle, or NULL when closed.
            CString                 m_strLastErrorCode;     ///< the last error's text
            int                     m_nLastErrorCode;       ///< The last error's code (see MajorErrorCode()).
            int                     m_nExtendedErrorCode;   ///< SQLite's extended result code for it.
            CSQLiteDatabase        *m_pDatabase;            ///< The database this connection belongs to, or NULL.
            bool                    m_bValid;               ///< Whether the database opened.

        public:
                                    /// Opens a database.
                                    /// @param strDBName A file name, ":memory:", or a
                                    ///        "file:" URI. The file is created if need be.
                                    /// @param strName The connection's name.
                                    /// @param pDatabase The CSQLiteDatabase it belongs to (or NULL).
                                    /// If it cannot be opened, Connected() is false and
                                    /// the error is recorded.
                                    CSQLiteConnection( const char *strDBName, const char *strName, CSQLiteDatabase *pDatabase );
                                    /// Closes the database and leaves the
                                    /// CSQLiteDatabase's list.
            virtual                ~CSQLiteConnection( void );

                                    DEFINE_OBJECT(CSQLiteConnection);

                /// Real transactions: BEGIN, COMMIT and ROLLBACK. False if one is
                /// already open, or it fails.
                /// @note They used to be emulated by buffering the SQL text, so
                /// failures were seen only at the commit, and a failed commit
                /// left the transaction open and the write lock held.
            virtual bool            BeginTransaction( void );
                /// Commits the open transaction. A failed commit rolls back
                /// (keeping the commit's error). False if none is open, or it failed.
            virtual bool            CommitTransaction( void );
                /// Rolls back the open transaction. False if none is open, or it failed.
            virtual bool            RollbackTransaction( void );

                /// Runs exactly one statement that returns rows (SELECT, WITH,
                /// PRAGMA, VALUES...), keeping all of them in memory.
                /// @return A new cursor, which the caller deletes; NULL on an
                /// error (see MajorErrorMessage()), including more than one
                /// statement or one that returns no rows.
            virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = false );
                /// Same, binding @p aParameters' strings to the statement's ?
                /// parameters, in order (their number must match).
            virtual CCursor        *Cursor( const char *strQueryString, const CStringArray &aParameters, bool bUpdatable = false );
                /// Runs any number of statements, discarding rows. False on the
                /// first error.
            virtual bool            Execute( const char *strExecString );
                /// Runs one statement, binding @p aParameters to its ?
                /// parameters, in order, so values need no quoting.
            virtual bool            Execute( const char *strExecString, const CStringArray &aParameters );
                /// Rows changed by the last INSERT, UPDATE or DELETE.
            int64_t                 Changes( void );
                /// The rowid of the last INSERT.
            int64_t                 LastInsertId( void );

                /// IASLIB_DB_OK, one of the IASLIB_DB_ERROR_* codes (negative)
                /// for the library's own errors, or SQLite's result code
                /// (positive).
            virtual int             MajorErrorCode( void );
                /// SQLite's extended result code (0 for the library's own errors).
            virtual int             MinorErrorCode( void );

            virtual const char     *MajorErrorMessage( void );      ///< The last error's text ("" if none).
            virtual const char     *MinorErrorMessage( void );      ///< Same as MajorErrorMessage().

                /// Closes the database (the object stays, unconnected). Returns true.
            virtual bool            Release( void );

                /// The query's result as an HTML table (written to pStream
                /// when given, else returned). Text is HTML-escaped; "" if
                /// the query fails.
                /// @note It used to be a TODO that returned "".
            virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL );

                /// SQLite has no stored procedures: NULL.
            virtual CStoredProc    *StoredProc( const char *strProcCall, const char *strParamList = NULL );

                /// Whether the connection is closed or never opened.
            virtual bool            IsDead( void );

                /// How long to wait for a lock held by another connection, in
                /// milliseconds (ten seconds to begin with).
            void                    SetTimeout( int nTimeMS );

		    virtual int				GetDBType( void ) { return DB_SQLITE; }     ///< DB_SQLITE.

                /// The SQLite handle, for what the class does not cover.
            sqlite3                *GetHandle( void ) { return m_pSLDatabase; }

                /// For CSQLiteDatabase: forgets the database it belongs to.
            void                    Detach( void ) { m_pDatabase = NULL; }

        private:
            friend class CSQLiteCursor;
            bool                    Fail( int nCode, const char *strMessage );      ///< Records a library error; returns false.
            bool                    FailFromSQLite( int nResult );                  ///< Records SQLite's error; returns false.
            void                    Succeed( void );                                ///< Clears the error.
                /// The next statement of strSQL (NULL at the end, or on an
                /// error: check m_nLastErrorCode).
            sqlite3_stmt           *Prepare( const char *strSQL, const char **ppTail );
                /// Binds the parameters (their number must match). False on an error.
            bool                    Bind( sqlite3_stmt *pStatement, const CStringArray *paParameters );
                /// The work behind Execute().
            bool                    ExecuteAll( const char *strSQL, const CStringArray *paParameters );
                /// The work behind Cursor().
            CCursor                *Query( const char *strSQL, const CStringArray *paParameters, bool bUpdatable );

                                    CSQLiteConnection( const CSQLiteConnection & );     ///< Not copyable.
            CSQLiteConnection      &operator =( const CSQLiteConnection & );           ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__

#endif // IASLIB_SQLITECONNECTION_H__
