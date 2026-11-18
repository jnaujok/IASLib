/*
 *  SQL Lite Cursor
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

    class CSQLiteCursor : public CCursor
    {
        protected:
            CSQLiteConnection *m_pConnection;

        public:
            DEFINE_OBJECT( CSQLiteCursor );

                                CSQLiteCursor( CSQLiteConnection *pConnection, bool bUpdatable );
            virtual            ~CSQLiteCursor( void );

                // Frees the rows.
            virtual int         Close( void );

                // IASLIB_DB_OK, IASLIB_DB_CURSOR_READ_ONLY,
                // IASLIB_DB_CURSOR_INVALID_ROW, IASLIB_DB_ERROR_BAD_QUERY (no
                // row matched, or the SQL failed: see the connection's error).
            virtual int         DeleteRow( const char *strTable );
            virtual int         InsertRow( const char *strTable );
            virtual int         UpdateRow( const char *strTable );

                // Reads every row of a prepared statement (for the connection).
            bool                Load( sqlite3_stmt *pStatement );

                // A name as an SQL identifier: in double quotes, quotes doubled.
            static CString      QuoteIdentifier( const char *strName );

        private:
            int                 ApplyRow( const char *strTable, int nAction );
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__

#endif // IASLIB_SQLITECURSOR_H__
