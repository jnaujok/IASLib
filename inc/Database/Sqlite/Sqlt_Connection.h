/*
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

    class CSQLiteConnection : public CConnection
    {
        protected:
            sqlite3                *m_pSLDatabase;
            char                   *m_pstrErrorStorage;
            CString                 m_strLastErrorCode;
            CString                 m_strTransactionBuffer;
            int                     m_nLastErrorCode;
            CSQLiteDatabase       *m_pDatabase;
            bool                    m_bValid;
            
        public:
                                    CSQLiteConnection( const char *strDBName, const char *strName, CSQLiteDatabase * );
            virtual                ~CSQLiteConnection( void );

                                    DEFINE_OBJECT(CSQLiteConnection);

            virtual bool            BeginTransaction( void );
            virtual bool            CommitTransaction( void );
            virtual bool            RollbackTransaction( void );

            virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = false );
            virtual bool            Execute( const char *strExecString );

            virtual int             MajorErrorCode( void );
            virtual int             MinorErrorCode( void );

            virtual const char     *MajorErrorMessage( void );
            virtual const char     *MinorErrorMessage( void );

            virtual bool            Release( void );

            virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL );

            virtual CStoredProc    *StoredProc( const char *strProcCall, const char *strParamList = NULL );

            virtual bool            IsDead( void );

            void                    SetTimeout( int nTimeMS );

		    virtual int				GetDBType( void ) { return DB_SQLITE; }
        private:
            int  (*m_fnSqlOpen)( const char *, sqlite3 ** );
            void (*m_fnSqlFree)( void * );
            int  (*m_fnSqlBusyTime)(sqlite3*, int ms);
            int  (*m_fnSqlClose)(sqlite3 *);
            const char *(*m_fnSqlErrMsg)(sqlite3*);
            int (*m_fnSqlExec)(sqlite3*,const char *,sqlite3_callback,void *,char ** );

    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DB_SQLLITE__
#endif // IASLIB_SQLLITECONNECTION_H__
