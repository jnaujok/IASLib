/**
 *  @file
 *  Connection Class
 *
 *  This abstract class provides a base interface for database connections.
 * Database connections are used to provide a conduit for database 
 * communications.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/25/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CONNECTION_H__
#define IASLIB_CONNECTION_H__

#include "../BaseTypes/String_.h"
#include "../Streams/Stream.h"
#include "StoredProc.h"
#include "Cursor.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
        /// A connection to a database: runs queries and commands.
        ///
        /// Made by CDatabase::Connection(). Each vendor derives from this.
        /// Use a connection from one thread at a time. Cursors and stored
        /// procedure results it returns are the caller's to delete.
        ///
        /// @code
        /// CCursor *pCursor = pCon->Cursor( "select id, name from users" );
        /// if ( pCursor && pCursor->IsValid() && pCursor->Rows() > 0 )
        /// {
        ///     do
        ///         printf( "%s\n", (const char *)pCursor->GetColumn( "name" ) );
        ///     while ( pCursor->Next() );
        /// }
        /// delete pCursor;
        /// @endcode
    class CConnection : public CObject
    {
        protected:
            CString                 m_strConnectionName;    ///< The connection's name.
            bool                    m_bConnected;           ///< True while connected.
            bool                    m_bInTransaction;       ///< True between BeginTransaction() and its commit or rollback.

        public:
                                    /// Creates an unconnected connection.
                                    /// @param strName A name for the connection.
                                    CConnection( const char *strName = "" ) { m_strConnectionName = strName; m_bConnected = 0; m_bInTransaction = 0;}
                                    /// Destroys the connection.
            virtual                ~CConnection( void ) {};

                                    DECLARE_OBJECT( CConnection, CObject )

            /// Starts a transaction.
            /// @return True if a transaction is now open.
            virtual bool            BeginTransaction( void ) = 0;
            /// Ends the open transaction, keeping its changes.
            /// @return False if there was none, or it failed.
            virtual bool            CommitTransaction( void ) = 0;
            /// Ends the open transaction, discarding its changes.
            /// @return False if it failed.
            virtual bool            RollbackTransaction( void ) = 0;

            /// True while connected.
            virtual bool            Connected( void ) { return m_bConnected; }
            /// True if connected and not dead.
            virtual bool            IsValid( void ) { return ( ( Connected() ) && ( ! IsDead() ) ); }

            /// Runs a SELECT and returns its rows.
            /// @param strQueryString The query.
            /// @param bUpdatable     True to allow DeleteRow(), InsertRow() and UpdateRow().
            /// @return A new cursor (the caller deletes it; check IsValid()), or NULL on
            ///         failure (see MajorErrorMessage()).
            virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = false ) = 0;
            /// Runs a command that returns no rows.
            /// @param strExecString The SQL.
            /// @return False if it failed (see MajorErrorMessage()).
            virtual bool            Execute( const char *strExecString ) = 0;

            /// The last error's main code (an IASLIB_DB_ERROR_* value or the vendor's code).
            virtual int             MajorErrorCode( void ) = 0;
            /// The last error's secondary code.
            virtual int             MinorErrorCode( void ) = 0;

            /// The last error's main message.
            virtual const char     *MajorErrorMessage( void ) = 0;
            /// The last error's secondary message.
            virtual const char     *MinorErrorMessage( void ) = 0;

            /// Closes the connection and takes it out of its database's list
            /// (it is not deleted).
            /// @return True if it was released.
            virtual bool            Release( void ) = 0;

            /// Runs a query and formats its rows as an HTML table.
            /// @param strQueryString The query.
            /// @param pStream        Where to write the table as it is made (NULL: none).
            /// @return The table, or "" when it was written to pStream (SQLite does
            ///         not support this and always returns "").
            virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL ) = 0;

            /// Calls a stored procedure.
            /// @param strProcCall  The procedure (and its arguments, by vendor).
            /// @param strParamList The output parameters to collect (NULL: none).
            /// @return A new stored procedure result (the caller deletes it), or NULL
            ///         if it failed or the vendor has none.
            virtual CStoredProc    *StoredProc( const char *strProcCall, const char *strParamList = NULL ) = 0;

            /// True if the connection has been lost or closed.
            virtual bool            IsDead( void ) = 0;

		    /// The vendor: DB_ORACLE, DB_SYBASE, DB_SQLITE or DB_MYSQL (see Date.h).
		    virtual int				GetDBType( void ) = 0;
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_CONNECTION_H__
