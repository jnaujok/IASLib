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

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_SQLITE__

#include "Sqlt_Connection.h"
#include "Sqlt_Cursor.h"
#include "Sqlt_Database.h"
#include "Database.h"
#include "sqlite3.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSQLiteConnection, CConnection );

    CSQLiteConnection::CSQLiteConnection( const char *strDBName, const char *strName, CSQLiteDatabase *pDatabase ) 
        : CConnection( strName )
    {
        m_fnSqlOpen     = (int(__cdecl *)(const char *,sqlite3 **))CSQLiteDatabase::GetFunction( "sqlite3_open" );
        m_fnSqlFree     = (void(__cdecl *)(void *))CSQLiteDatabase::GetFunction( "sqlite3_freemem" );
        m_fnSqlBusyTime = (int(__cdecl *)(sqlite3 *,int)) CSQLiteDatabase::GetFunction( "sqlite3_busy_timeout" );
        m_fnSqlClose    = (int(__cdecl *)(sqlite3 *))CSQLiteDatabase::GetFunction( "sqlite3_close" );
        m_fnSqlErrMsg   = (const char *(__cdecl *)(sqlite3 *))CSQLiteDatabase::GetFunction( "sqlite3_errmsg" );
        m_fnSqlExec     = (int(__cdecl *)(sqlite3 *,const char *,sqlite3_callback,void *,char **))CSQLiteDatabase::GetFunction( "sqlite3_exec" );

        m_bConnected = false;
        m_bValid = false;
        m_nLastErrorCode = 0;
        m_pDatabase = pDatabase;


        int nSqlError = (*m_fnSqlOpen)( strDBName, &m_pSLDatabase );

        if ( nSqlError != SQLITE_OK )
        {
            m_nLastErrorCode = IASLIB_DB_ERROR_BAD_DATABASE;
            m_strLastErrorCode = (*m_fnSqlErrMsg)( m_pSLDatabase );
            (*m_fnSqlClose)( m_pSLDatabase );
        }
        else
        {
            (*m_fnSqlBusyTime)( m_pSLDatabase, 10000 ); // Set Ten second busy timeout.
            m_bValid = true;
            m_bConnected = true;
        }
    }

    CSQLiteConnection::~CSQLiteConnection( void )
    {
        if ( m_pSLDatabase )
        {
            (*m_fnSqlClose)( m_pSLDatabase );
        }
        if ( m_pDatabase )
        {
            m_pDatabase->ReleaseConnection( this );
        }
    }

    bool CSQLiteConnection::BeginTransaction( void )
    {
        if ( ! m_bInTransaction )
        {
            m_bInTransaction = true;
            m_strTransactionBuffer = "BEGIN;\n";
        }
      
        return m_bInTransaction;
    }

    bool CSQLiteConnection::CommitTransaction( void )
    {
        if ( ! m_bInTransaction )
        {
            return false;
        }
        m_bInTransaction = false;
        m_strTransactionBuffer += "END;";
        return Execute( m_strTransactionBuffer );
    }

    bool CSQLiteConnection::RollbackTransaction( void )
    {
        m_strTransactionBuffer = "";
        m_bInTransaction = false;
        return true;
    }

    CCursor *CSQLiteConnection::Cursor( const char *strQueryString, bool bUpdatable )
    {
        CString         strConQuery = strQueryString;
        CSQLiteCursor *pNewCursor = NULL;

        strConQuery.Trim();

        CString strStart = strConQuery.Substring( 0, 6 );
        strStart.ToUpperCase();

        if ( strStart != "SELECT" )
        {
            m_strLastErrorCode = "Cursor query was not a SELECT statement.";
            m_nLastErrorCode = IASLIB_DB_ERROR_BAD_QUERY;
            return NULL;
        }

            // Check if this connection has been marked as dead. If it has, fail this operation.
	    if ( !m_bConnected )
        {
            m_strLastErrorCode = "This connection has been marked dead.";
            m_nLastErrorCode = IASLIB_DB_ERROR_CONNECTION_DEAD;
		    return NULL;
	    }

            // SQL Lite queries must be terminated with a semicolon.
        if ( strConQuery[ strConQuery.GetLength() - 1 ] != ';' )
        {
            strConQuery += ';';
        }

        pNewCursor = new CSQLiteCursor( this, bUpdatable );

        m_nLastErrorCode = (*m_fnSqlExec)( m_pSLDatabase, strConQuery, CSQLiteCursor::CursorCallback, pNewCursor, &m_pstrErrorStorage );

        if ( m_nLastErrorCode != SQLITE_OK )
        {
            if ( m_pstrErrorStorage != NULL )
            {
                m_strLastErrorCode = m_pstrErrorStorage;
                (*m_fnSqlFree)( m_pstrErrorStorage );
                delete pNewCursor;
                pNewCursor = NULL;
            }
        }
        else
        {
            pNewCursor->SetValid();
        }

        return pNewCursor;
    }

    bool CSQLiteConnection::Execute( const char *strExecString )
    {
        CString         strConQuery = strExecString;

        strConQuery.Trim();

            // Check if this connection has been marked as dead. If it has, fail this operation.
	    if ( !m_bConnected )
        {
            m_strLastErrorCode = "This connection has been marked dead.";
            m_nLastErrorCode = IASLIB_DB_ERROR_CONNECTION_DEAD;
		    return false;
	    }

            // SQL Lite queries must be terminated with a semicolon.
        if ( strConQuery[ strConQuery.GetLength() - 1 ] != ';' )
        {
            strConQuery += ';';
        }

        if ( m_bInTransaction )
        {
            m_strTransactionBuffer += strConQuery;
            m_strTransactionBuffer += '\n';
            return true;
        }

        m_nLastErrorCode = (*m_fnSqlExec)( m_pSLDatabase, strConQuery, CSQLiteCursor::CursorCallback, NULL, &m_pstrErrorStorage );

        if ( m_nLastErrorCode != SQLITE_OK )
        {
            if ( m_pstrErrorStorage != NULL )
            {
                m_strLastErrorCode = m_pstrErrorStorage;
                (*m_fnSqlFree)( m_pstrErrorStorage );
                return false;
            }
        }

        return true;
    }

    int CSQLiteConnection::MajorErrorCode( void )
    {
        return m_nLastErrorCode;
    }

    int CSQLiteConnection::MinorErrorCode( void )
    {
        return m_nLastErrorCode;
    }

    const char *CSQLiteConnection::MajorErrorMessage( void )
    {
        return (const char *)m_strLastErrorCode;
    }

    const char *CSQLiteConnection::MinorErrorMessage( void )
    {
        return (const char *)m_strLastErrorCode;
    }

    bool CSQLiteConnection::Release( void )
    {
        if ( m_bValid )
        {
            if ( m_pDatabase )
            {
                m_pDatabase->ReleaseConnection( this );
            }

            if ( m_pSLDatabase )
            {
                (*m_fnSqlClose)( m_pSLDatabase );
            }
            m_pSLDatabase = NULL;
            m_bConnected = false;
            m_bValid = false;
        }
        return true;
    }

    CString CSQLiteConnection::SQLTable( const char *strQueryString, CStream *pStream )
    {
        CString strRetVal;
#ifndef IASLIB_NO_LINT__
        pStream = pStream;
        strQueryString = strQueryString;

        /// TODO: SQL Lite should output an HTML table here...
#endif
        return strRetVal;
    }

    CStoredProc *CSQLiteConnection::StoredProc( const char *strProcCall, const char *strParamList )
    {
        // SQL Lite does not support stored procedures
        m_nLastErrorCode = IASLIB_DB_ERROR_UNSUPPORTED;
        m_strLastErrorCode = "SQL Lite does not support stored procedures.";

#ifndef IASLIB_NO_LINT__
        strParamList = strParamList;
        strProcCall = strProcCall;
#endif
        return NULL;
    }

    bool CSQLiteConnection::IsDead( void )
    {
        return (!m_bConnected) || (!m_bValid);
    }

    void CSQLiteConnection::SetTimeout( int nTimeMS )
    {
        (*m_fnSqlBusyTime)( m_pSLDatabase, nTimeMS ); 
    }
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__

