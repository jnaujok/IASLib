/**
 *  @file
 *  SQL Lite Cursor
 *
 *      Cursor class for connecting to SQL Lite. The Cursor class stores
 * the data retrieved from a query against the database. It is a some-
 * what simple class, since the SQL Lite database has a rather simple
 * way of handling queries. 
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
#include "Sqlt_Connection.h"

namespace IASLib
{
    extern "C"
    {
        /// The sqlite3_exec() callback that adds one row to a CSQLiteCursor.
        /// Its arguments are the cursor, the number of columns, the row's
        /// values and the column names; the first row also sets the
        /// cursor's column names. Returns 0, so SQLite goes on.
        /// @note CSQLiteConnection::Execute() passes no cursor (NULL), so a
        /// statement run there must not return rows.
        int CursorCallback( void *pCursor, int nColumns, char **astrValues, char **astrHeaders );
    };

        /// The rows of a SQLite SELECT, read in full.
        ///
        /// Made by CSQLiteConnection::Cursor(); the caller deletes it. Row
        /// changes are not supported.
    class CSQLiteCursor : public CCursor
    {
        protected:
            CSQLiteConnection *m_pConnection;   ///< The connection that ran the query (not owned).

        public:
            DEFINE_OBJECT( CSQLiteCursor );

                                /// Creates an empty cursor for CursorCallback() to fill.
                                /// @param pConnection The connection.
                                /// @param bUpdatable  Recorded only.
                                CSQLiteCursor( CSQLiteConnection *pConnection, bool bUpdatable );
                                /// Frees the rows.
            virtual            ~CSQLiteCursor( void );

            /// Frees the rows; the cursor is then empty and invalid.
            /// @return 0.
            virtual int         Close( void );

            /// Not supported. @return IASLIB_DB_ERROR_UNSUPPORTED.
            virtual int         DeleteRow( const char *strTable );
            /// Not supported. @return IASLIB_DB_ERROR_UNSUPPORTED.
            virtual int         InsertRow( const char *strTable );
            /// Not supported. @return IASLIB_DB_ERROR_UNSUPPORTED.
            virtual int         UpdateRow( const char *strTable );

            /// Moves to the next row.
            /// @return False if there is none (the current row then stays just past the last).
            virtual bool        Next( void );
            /// Moves to the previous row.
            /// @return False if already at the first row.
            virtual bool        Prev( void );
        
            /// Marks the cursor as holding data (once the query succeeded).
            void                SetValid( void ) { m_bValid = true; }

            friend int CursorCallback( void *, int, char **, char **);
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_SQLLITECURSOR_H__

