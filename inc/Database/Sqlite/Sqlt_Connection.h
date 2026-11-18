/*
 *  SQL Lite Connection
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

    class CSQLiteConnection : public CConnection
    {
        protected:
            sqlite3                *m_pSLDatabase;
            CString                 m_strLastErrorCode;     // the last error's text
            int                     m_nLastErrorCode;
            int                     m_nExtendedErrorCode;
            CSQLiteDatabase        *m_pDatabase;
            bool                    m_bValid;

        public:
                                    // strDBName: a file name, ":memory:", or a
                                    // "file:" URI. The file is created if need be.
                                    CSQLiteConnection( const char *strDBName, const char *strName, CSQLiteDatabase *pDatabase );
                                    // Closes the database and leaves the
                                    // CSQLiteDatabase's list.
            virtual                ~CSQLiteConnection( void );

                                    DEFINE_OBJECT(CSQLiteConnection);

                // Real transactions: BEGIN, COMMIT and ROLLBACK. A failed
                // commit rolls back. (They were emulated by buffering the SQL
                // text, so failures were seen only at the commit, and a failed
                // commit left the transaction open and the write lock held.)
            virtual bool            BeginTransaction( void );
            virtual bool            CommitTransaction( void );
            virtual bool            RollbackTransaction( void );

            virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = false );
            virtual CCursor        *Cursor( const char *strQueryString, const CStringArray &aParameters, bool bUpdatable = false );
            virtual bool            Execute( const char *strExecString );
            virtual bool            Execute( const char *strExecString, const CStringArray &aParameters );
                // Rows changed by the last INSERT, UPDATE or DELETE, and the
                // rowid of the last INSERT.
            int64_t                 Changes( void );
            int64_t                 LastInsertId( void );

            virtual int             MajorErrorCode( void );
            virtual int             MinorErrorCode( void );

            virtual const char     *MajorErrorMessage( void );
            virtual const char     *MinorErrorMessage( void );

                // Closes the database (the object stays, unconnected).
            virtual bool            Release( void );

                // The query's result as an HTML table (written to pStream
                // when given, else returned).
            virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL );

                // SQLite has no stored procedures: NULL.
            virtual CStoredProc    *StoredProc( const char *strProcCall, const char *strParamList = NULL );

            virtual bool            IsDead( void );

                // How long to wait for a lock held by another connection.
            void                    SetTimeout( int nTimeMS );

		    virtual int				GetDBType( void ) { return DB_SQLITE; }

                // The SQLite handle, for what the class does not cover.
            sqlite3                *GetHandle( void ) { return m_pSLDatabase; }

                // For CSQLiteDatabase: forgets the database it belongs to.
            void                    Detach( void ) { m_pDatabase = NULL; }

        private:
            friend class CSQLiteCursor;
            bool                    Fail( int nCode, const char *strMessage );
            bool                    FailFromSQLite( int nResult );
            void                    Succeed( void );
            sqlite3_stmt           *Prepare( const char *strSQL, const char **ppTail );
            bool                    Bind( sqlite3_stmt *pStatement, const CStringArray *paParameters );
            bool                    ExecuteAll( const char *strSQL, const CStringArray *paParameters );
            CCursor                *Query( const char *strSQL, const CStringArray *paParameters, bool bUpdatable );

                                    CSQLiteConnection( const CSQLiteConnection & );
            CSQLiteConnection      &operator =( const CSQLiteConnection & );
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__

#endif // IASLIB_SQLITECONNECTION_H__
