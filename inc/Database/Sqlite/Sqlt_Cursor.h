/*
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
        int CursorCallback( void *pCursor, int nColumns, char **astrValues, char **astrHeaders );
    };

    class CSQLiteCursor : public CCursor
    {
        protected:
            CSQLiteConnection *m_pConnection;

        public:
            DEFINE_OBJECT( CSQLiteCursor );

                                CSQLiteCursor( CSQLiteConnection *pConnection, bool bUpdatable );
            virtual            ~CSQLiteCursor( void );

            virtual int         Close( void );

            virtual int         DeleteRow( const char *strTable );
            virtual int         InsertRow( const char *strTable );
            virtual int         UpdateRow( const char *strTable );

            virtual bool        Next( void );
            virtual bool        Prev( void );
        
            void                SetValid( void ) { m_bValid = true; }

            friend int CursorCallback( void *, int, char **, char **);
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_SQLLITECURSOR_H__

