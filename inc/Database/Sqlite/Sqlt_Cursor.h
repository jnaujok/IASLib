/**
 * @file
 * @brief SQL Lite Cursor.
 *
 *      The rows of a query against SQLite, held in memory, with SQL NULLs
 * recorded (IsNull()). An updatable cursor (Cursor( query, true )) can
 * change the table its rows came from: UpdateRow() and DeleteRow() find
 * the row by all of its original values (NULL-safe, one row only), and
 * InsertRow() adds a row of the staged values. Values are bound, never
 * pasted into the SQL.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 05/13/2003
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SQLITECURSOR_H__
#define IASLIB_SQLITECURSOR_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_SQLITE__

#include "../Cursor.h"
#include "sqlite3.h"

namespace IASLib
{
    class CSQLiteConnection;

    /// The rows of a query against SQLite, held in memory, with SQL NULLs
    /// and BLOBs recorded.
    ///
    /// Made by CSQLiteConnection::Cursor(); the caller deletes it, before
    /// the connection. An updatable cursor changes the table its rows came
    /// from: UpdateRow() and DeleteRow() find the row by all of its original
    /// values (NULL-safe, one row only), and InsertRow() adds a row of the
    /// staged values. Values are bound, never pasted into the SQL.
    class CSQLiteCursor : public CCursor
    {
        protected:
            CSQLiteConnection *m_pConnection;       ///< The connection the rows came from.

        public:
            DEFINE_OBJECT( CSQLiteCursor );

                                /// Makes an empty cursor for @p pConnection (see Load()).
                                CSQLiteCursor( CSQLiteConnection *pConnection, bool bUpdatable );
                                /// Destructor. Frees the rows.
            virtual            ~CSQLiteCursor( void );

                /// Frees the rows. Returns IASLIB_DB_OK.
            virtual int         Close( void );

                /// Deletes the current row from @p strTable (and from the cursor).
                /// @return IASLIB_DB_OK, IASLIB_DB_CURSOR_READ_ONLY,
                /// IASLIB_DB_CURSOR_INVALID_ROW, IASLIB_DB_ERROR_BAD_QUERY (no
                /// row matched, or the SQL failed: see the connection's error),
                /// or IASLIB_DB_ERROR_CONNECTION_DEAD.
            virtual int         DeleteRow( const char *strTable );
                /// Inserts a row of the staged values into @p strTable (the
                /// table's defaults, if none are staged). Returns as DeleteRow() does.
            virtual int         InsertRow( const char *strTable );
                /// Writes the staged values to the current row in @p strTable (and
                /// in the cursor). IASLIB_DB_OK if nothing is staged; otherwise
                /// returns as DeleteRow() does.
            virtual int         UpdateRow( const char *strTable );

                /// Reads every row of a prepared statement (for the connection).
                /// False on an error (recorded on the connection).
            bool                Load( sqlite3_stmt *pStatement );

                /// A name as an SQL identifier: in double quotes, quotes doubled.
            static CString      QuoteIdentifier( const char *strName );

        private:
            int                 ApplyRow( const char *strTable, int nAction );  ///< The work behind the row methods.
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__

#endif // IASLIB_SQLITECURSOR_H__
