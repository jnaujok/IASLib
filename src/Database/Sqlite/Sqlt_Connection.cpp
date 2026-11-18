/*
 *  SQL Lite Connection
 *
 *      See Sqlt_Connection.h. Statements go through sqlite3_prepare_v2()
 * and sqlite3_step(). (They went through sqlite3_exec() with a C callback:
 * Execute() passed it a NULL cursor, so any statement returning rows
 * crashed, and C++ exceptions could unwind through SQLite's C code. SQLite
 * was also loaded by name at run time, which failed on macOS, although
 * the build links it anyway.)
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

#include <ctype.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CSQLiteConnection, CConnection );

    namespace
    {
        bool onlySpace( const char *p )
        {
            while ( p && *p )
            {
                if ( ! isspace( (unsigned char)*p ) && *p != ';' )
                    return false;
                p++;
            }
            return true;
        }

        CString htmlText( const char *strText )
        {
            CString strOut;
            for ( const char *p = ( strText ) ? strText : ""; *p; p++ )
            {
                switch ( *p )
                {
                    case '&':   strOut += "&amp;";  break;
                    case '<':   strOut += "&lt;";   break;
                    case '>':   strOut += "&gt;";   break;
                    case '"':   strOut += "&quot;"; break;
                    default:    strOut += *p;       break;
                }
            }
            return strOut;
        }
    }

    CSQLiteConnection::CSQLiteConnection( const char *strDBName, const char *strName, CSQLiteDatabase *pDatabase )
        : CConnection( strName )
    {
        m_pSLDatabase = NULL;
        m_bConnected = false;
        m_bValid = false;
        m_nLastErrorCode = IASLIB_DB_OK;
        m_nExtendedErrorCode = 0;
        m_pDatabase = pDatabase;

        int nResult = sqlite3_open_v2( ( strDBName ) ? strDBName : "", &m_pSLDatabase,
                                       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL );
        if ( nResult != SQLITE_OK )
        {
            m_nLastErrorCode = IASLIB_DB_ERROR_BAD_DATABASE;
            m_nExtendedErrorCode = nResult;
            m_strLastErrorCode = ( m_pSLDatabase ) ? sqlite3_errmsg( m_pSLDatabase ) : sqlite3_errstr( nResult );
                // (The handle was closed here and again by the destructor.)
            if ( m_pSLDatabase )
                sqlite3_close( m_pSLDatabase );
            m_pSLDatabase = NULL;
        }
        else
        {
            sqlite3_extended_result_codes( m_pSLDatabase, 1 );
            sqlite3_busy_timeout( m_pSLDatabase, 10000 );   // ten seconds for locks
            m_bValid = true;
            m_bConnected = true;
        }
    }

    CSQLiteConnection::~CSQLiteConnection( void )
    {
        if ( m_pSLDatabase )
        {
            sqlite3_close_v2( m_pSLDatabase );
            m_pSLDatabase = NULL;
        }
        if ( m_pDatabase )
        {
            m_pDatabase->ReleaseConnection( this );
            m_pDatabase = NULL;
        }
    }

    bool CSQLiteConnection::Fail( int nCode, const char *strMessage )
    {
        m_nLastErrorCode = nCode;
        m_nExtendedErrorCode = 0;
        m_strLastErrorCode = ( strMessage ) ? strMessage : "";
        return false;
    }

    bool CSQLiteConnection::FailFromSQLite( int nResult )
    {
        m_nLastErrorCode = nResult & 0xFF;
        m_nExtendedErrorCode = ( m_pSLDatabase ) ? sqlite3_extended_errcode( m_pSLDatabase ) : nResult;
        m_strLastErrorCode = ( m_pSLDatabase ) ? sqlite3_errmsg( m_pSLDatabase ) : sqlite3_errstr( nResult );
        return false;
    }

    void CSQLiteConnection::Succeed( void )
    {
        m_nLastErrorCode = IASLIB_DB_OK;
        m_nExtendedErrorCode = 0;
        m_strLastErrorCode = "";
    }

        // The next statement of strSQL (NULL at the end, or on an error:
        // check m_nLastErrorCode).
    sqlite3_stmt *CSQLiteConnection::Prepare( const char *strSQL, const char **ppTail )
    {
        sqlite3_stmt *pStatement = NULL;
        const char *pNext = strSQL;
        while ( pNext && *pNext )
        {
            const char *pTail = NULL;
            int nResult = sqlite3_prepare_v2( m_pSLDatabase, pNext, -1, &pStatement, &pTail );
            if ( nResult != SQLITE_OK )
            {
                FailFromSQLite( nResult );
                if ( ppTail )
                    *ppTail = NULL;
                return NULL;
            }
            if ( pStatement )
            {
                if ( ppTail )
                    *ppTail = pTail;
                return pStatement;
            }
            pNext = pTail;              // only white space or a comment
        }
        if ( ppTail )
            *ppTail = NULL;
        return NULL;
    }

    bool CSQLiteConnection::Bind( sqlite3_stmt *pStatement, const CStringArray *paParameters )
    {
        int nWanted = sqlite3_bind_parameter_count( pStatement );
        int nGiven = ( paParameters ) ? (int)paParameters->Length() : 0;
        if ( nWanted != nGiven )
        {
            CString strMessage;
            strMessage.Format( "The statement has %d parameters, but %d values were given.", nWanted, nGiven );
            return Fail( IASLIB_DB_ERROR_BAD_QUERY, strMessage );
        }
        for ( int n = 0; n < nGiven; n++ )
        {
            CString strValue = paParameters->Get( (size_t)n );
            int nResult = sqlite3_bind_text( pStatement, n + 1, (const char *)strValue, (int)strValue.GetLength(), SQLITE_TRANSIENT );
            if ( nResult != SQLITE_OK )
                return FailFromSQLite( nResult );
        }
        return true;
    }

    bool CSQLiteConnection::ExecuteAll( const char *strSQL, const CStringArray *paParameters )
    {
        if ( ! m_bConnected || m_pSLDatabase == NULL )
            return Fail( IASLIB_DB_ERROR_CONNECTION_DEAD, "This connection is closed." );
        if ( strSQL == NULL )
            return Fail( IASLIB_DB_ERROR_BAD_QUERY, "No SQL." );

        Succeed();
        const char *pNext = strSQL;
        bool bFirst = true;
        for ( ;; )
        {
            const char *pTail = NULL;
            sqlite3_stmt *pStatement = Prepare( pNext, &pTail );
            if ( pStatement == NULL )
                return m_nLastErrorCode == IASLIB_DB_OK;      // the end, or an error

            if ( paParameters != NULL )
            {
                    // Values go to one statement only.
                if ( ! bFirst || ! onlySpace( pTail ) )
                {
                    sqlite3_finalize( pStatement );
                    return Fail( IASLIB_DB_ERROR_BAD_QUERY, "Statements with parameters must be run one at a time." );
                }
                if ( ! Bind( pStatement, paParameters ) )
                {
                    sqlite3_finalize( pStatement );
                    return false;
                }
            }

            int nResult;
            while ( ( nResult = sqlite3_step( pStatement ) ) == SQLITE_ROW )
            {
                    // Rows are not wanted here (Cursor() keeps them).
            }
            if ( nResult != SQLITE_DONE )
            {
                FailFromSQLite( nResult );
                sqlite3_finalize( pStatement );
                return false;
            }
            sqlite3_finalize( pStatement );
            bFirst = false;
            pNext = pTail;
        }
    }

    bool CSQLiteConnection::Execute( const char *strExecString )
    {
        return ExecuteAll( strExecString, NULL );
    }

    bool CSQLiteConnection::Execute( const char *strExecString, const CStringArray &aParameters )
    {
        return ExecuteAll( strExecString, &aParameters );
    }

    CCursor *CSQLiteConnection::Query( const char *strSQL, const CStringArray *paParameters, bool bUpdatable )
    {
        if ( ! m_bConnected || m_pSLDatabase == NULL )
        {
            Fail( IASLIB_DB_ERROR_CONNECTION_DEAD, "This connection is closed." );
            return NULL;
        }
        if ( strSQL == NULL )
        {
            Fail( IASLIB_DB_ERROR_BAD_QUERY, "No SQL." );
            return NULL;
        }

        Succeed();
        const char *pTail = NULL;
        sqlite3_stmt *pStatement = Prepare( strSQL, &pTail );
        if ( pStatement == NULL )
        {
            if ( m_nLastErrorCode == IASLIB_DB_OK )
                Fail( IASLIB_DB_ERROR_BAD_QUERY, "The query is empty." );
            return NULL;
        }
            // One statement only. (A second, "SELECT 1; DROP TABLE t", ran too.)
        if ( ! onlySpace( pTail ) )
        {
            sqlite3_finalize( pStatement );
            Fail( IASLIB_DB_ERROR_BAD_QUERY, "A cursor takes one statement." );
            return NULL;
        }
            // Anything that returns rows. (Only text starting "SELECT" was
            // allowed, which ruled out WITH, PRAGMA and VALUES.)
        if ( sqlite3_column_count( pStatement ) == 0 )
        {
            sqlite3_finalize( pStatement );
            Fail( IASLIB_DB_ERROR_BAD_QUERY, "The statement does not return rows." );
            return NULL;
        }
        if ( paParameters && ! Bind( pStatement, paParameters ) )
        {
            sqlite3_finalize( pStatement );
            return NULL;
        }

        CSQLiteCursor *pCursor = new CSQLiteCursor( this, bUpdatable );
        bool bOk = pCursor->Load( pStatement );
        sqlite3_finalize( pStatement );
        if ( ! bOk )
        {
            delete pCursor;
            return NULL;
        }
        return pCursor;
    }

    CCursor *CSQLiteConnection::Cursor( const char *strQueryString, bool bUpdatable )
    {
        return Query( strQueryString, NULL, bUpdatable );
    }

    CCursor *CSQLiteConnection::Cursor( const char *strQueryString, const CStringArray &aParameters, bool bUpdatable )
    {
        return Query( strQueryString, &aParameters, bUpdatable );
    }

    bool CSQLiteConnection::BeginTransaction( void )
    {
        if ( m_bInTransaction )
            return Fail( IASLIB_DB_ERROR_BAD_QUERY, "A transaction is already open." );
        if ( ! ExecuteAll( "BEGIN", NULL ) )
            return false;
        m_bInTransaction = true;
        return true;
    }

    bool CSQLiteConnection::CommitTransaction( void )
    {
        if ( ! m_bInTransaction )
            return Fail( IASLIB_DB_ERROR_BAD_QUERY, "No transaction is open." );
        m_bInTransaction = false;
        if ( ExecuteAll( "COMMIT", NULL ) )
            return true;

            // Keep the error, but do not leave the transaction (and its
            // locks) open.
        int nCode = m_nLastErrorCode, nExtended = m_nExtendedErrorCode;
        CString strError = m_strLastErrorCode;
        if ( m_pSLDatabase && ! sqlite3_get_autocommit( m_pSLDatabase ) )
            ExecuteAll( "ROLLBACK", NULL );
        m_nLastErrorCode = nCode;
        m_nExtendedErrorCode = nExtended;
        m_strLastErrorCode = strError;
        return false;
    }

        // (It discarded the buffered text and sent nothing to the database.)
    bool CSQLiteConnection::RollbackTransaction( void )
    {
        if ( ! m_bInTransaction )
            return Fail( IASLIB_DB_ERROR_BAD_QUERY, "No transaction is open." );
        m_bInTransaction = false;
        return ExecuteAll( "ROLLBACK", NULL );
    }

    int64_t CSQLiteConnection::Changes( void )
    {
        return ( m_pSLDatabase ) ? (int64_t)sqlite3_changes( m_pSLDatabase ) : 0;
    }

    int64_t CSQLiteConnection::LastInsertId( void )
    {
        return ( m_pSLDatabase ) ? (int64_t)sqlite3_last_insert_rowid( m_pSLDatabase ) : 0;
    }

    int CSQLiteConnection::MajorErrorCode( void )
    {
        return m_nLastErrorCode;
    }

    int CSQLiteConnection::MinorErrorCode( void )
    {
        return m_nExtendedErrorCode;
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
        if ( m_pSLDatabase )
        {
            sqlite3_close_v2( m_pSLDatabase );
            m_pSLDatabase = NULL;
        }
        m_bConnected = false;
        m_bValid = false;
        m_bInTransaction = false;
        return true;
    }

        // (It was a TODO that returned "".)
    CString CSQLiteConnection::SQLTable( const char *strQueryString, CStream *pStream )
    {
        CString strTable;
        CCursor *pResult = Cursor( strQueryString, false );
        if ( pResult == NULL )
            return strTable;

        strTable = "<table border=1>\n<tr>";
        for ( size_t nColumn = 0; nColumn < pResult->Columns(); nColumn++ )
        {
            strTable += "<th>";
            strTable += htmlText( pResult->ColumnName( nColumn ) );
            strTable += "</th>";
        }
        strTable += "</tr>\n";
        if ( pResult->Rows() )
        {
            do
            {
                strTable += "<tr>";
                for ( size_t nColumn = 0; nColumn < pResult->Columns(); nColumn++ )
                {
                    strTable += "<td>";
                    strTable += htmlText( pResult->GetColumn( nColumn ) );
                    strTable += "</td>";
                }
                strTable += "</tr>\n";
                if ( pStream )
                {
                    (*pStream) << strTable;
                    strTable = "";
                }
            } while ( pResult->Next() );
        }
        strTable += "</table>\n";
        delete pResult;

        if ( pStream )
        {
            (*pStream) << strTable;
            strTable = "";
        }
        return strTable;
    }

    CStoredProc *CSQLiteConnection::StoredProc( const char *, const char * )
    {
        Fail( IASLIB_DB_ERROR_UNSUPPORTED, "SQLite does not have stored procedures." );
        return NULL;
    }

    bool CSQLiteConnection::IsDead( void )
    {
        return ( ! m_bConnected ) || ( ! m_bValid ) || m_pSLDatabase == NULL;
    }

    void CSQLiteConnection::SetTimeout( int nTimeMS )
    {
        if ( m_pSLDatabase )
            sqlite3_busy_timeout( m_pSLDatabase, nTimeMS );
    }
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
