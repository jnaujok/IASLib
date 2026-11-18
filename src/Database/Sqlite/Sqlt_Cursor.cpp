/*
 *  SQL Lite Cursor
 *
 *      See Sqlt_Cursor.h. (Rows came through sqlite3_exec()'s callback,
 * which lost SQL NULLs, cut blobs at their first zero byte, and recorded
 * no column names for an empty result; the updates were unimplemented.)
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

#include "Sqlt_Cursor.h"
#include "Sqlt_Connection.h"
#include "Database.h"

#include <map>
#include <vector>

namespace IASLib
{
    IMPLEMENT_OBJECT( CSQLiteCursor, CCursor );

    namespace
    {
        enum { ACTION_DELETE, ACTION_INSERT, ACTION_UPDATE };
    }

    CSQLiteCursor::CSQLiteCursor( CSQLiteConnection *pConnection, bool bUpdatable )
    {
        m_pConnection = pConnection;
        m_bUpdatable = bUpdatable;
        m_bValid = false;
    }

    CSQLiteCursor::~CSQLiteCursor( void )
    {
        Close();
    }

    int CSQLiteCursor::Close( void )
    {
        ClearRows();
        m_bValid = false;
        return IASLIB_DB_OK;
    }

    CString CSQLiteCursor::QuoteIdentifier( const char *strName )
    {
        CString strQuoted( "\"" );
        for ( const char *p = ( strName ) ? strName : ""; *p; p++ )
        {
            if ( *p == '"' )
                strQuoted += "\"\"";
            else
                strQuoted += *p;
        }
        strQuoted += "\"";
        return strQuoted;
    }

    bool CSQLiteCursor::Load( sqlite3_stmt *pStatement )
    {
        ClearRows();
        m_astrHeaders.EmptyAll();
        m_nColumns = (size_t)sqlite3_column_count( pStatement );
        for ( size_t n = 0; n < m_nColumns; n++ )
        {
            const char *strName = sqlite3_column_name( pStatement, (int)n );
            m_astrHeaders.Push( CString( ( strName ) ? strName : "" ) );
        }

        int nResult;
        while ( ( nResult = sqlite3_step( pStatement ) ) == SQLITE_ROW )
        {
            CStringArray *pRow = new CStringArray;
            std::vector<unsigned char> abNull( m_nColumns, 0 );
            std::map<size_t, CDataBlock> mapBlobs;
            for ( size_t n = 0; n < m_nColumns; n++ )
            {
                int nType = sqlite3_column_type( pStatement, (int)n );
                if ( nType == SQLITE_NULL )
                {
                    abNull[n] = 1;
                    pRow->Push( CString( "" ) );
                }
                else if ( nType == SQLITE_BLOB )
                {
                        // Kept as bytes; the text column is empty.
                    const void *pBytes = sqlite3_column_blob( pStatement, (int)n );
                    int nBytes = sqlite3_column_bytes( pStatement, (int)n );
                    mapBlobs[ n ] = CDataBlock( (const unsigned char *)pBytes, (size_t)nBytes );
                    pRow->Push( CString( "" ) );
                }
                else
                {
                    const unsigned char *pText = sqlite3_column_text( pStatement, (int)n );
                    int nBytes = sqlite3_column_bytes( pStatement, (int)n );
                    pRow->Push( CString::FromBytes( pText, (size_t)nBytes ) );
                }
            }
            AddRow( pRow, &abNull, &mapBlobs );
        }
        if ( nResult != SQLITE_DONE )
        {
            m_pConnection->FailFromSQLite( nResult );
            ClearRows();
            return false;
        }

        m_nCurrentRow = 0;
        m_bValid = true;
        EnsureNewValues();
        ClearChanges();
        return true;
    }

    int CSQLiteCursor::ApplyRow( const char *strTable, int nAction )
    {
        if ( ! m_bValid )
            return IASLIB_DB_CURSOR_INVALID_ROW;
        if ( ! m_bUpdatable )
            return IASLIB_DB_CURSOR_READ_ONLY;
        if ( strTable == NULL || *strTable == '\0' )
            return IASLIB_DB_ERROR_BAD_QUERY;
        if ( nAction != ACTION_INSERT && m_nCurrentRow >= m_nRows )
            return IASLIB_DB_CURSOR_INVALID_ROW;
        EnsureNewValues();

        CString strTable2 = QuoteIdentifier( strTable );
        CString strSQL;
            // The values to bind, in order: text, NULL or a blob.
        struct SValue
        {
            enum { TEXT, NULLVALUE, BLOB } eKind;
            CString     strText;
            CDataBlock  oBlob;
        };
        std::vector<SValue> aValues;

            // The row, by all of its original values (IS matches NULLs), and
            // one row only, through its rowid.
        CString strMatch;
        if ( nAction != ACTION_INSERT )
        {
            strMatch = " WHERE rowid IN (SELECT rowid FROM " + strTable2 + " WHERE ";
            for ( size_t n = 0; n < m_nColumns; n++ )
            {
                if ( n )
                    strMatch += " AND ";
                strMatch += QuoteIdentifier( m_astrHeaders.Get( n ) ) + " IS ?";
            }
            strMatch += " LIMIT 1)";
        }

        if ( nAction == ACTION_UPDATE )
        {
            CString strSet;
            for ( size_t n = 0; n < m_nColumns; n++ )
            {
                if ( ! IsChanged( n ) )
                    continue;
                if ( strSet.GetLength() )
                    strSet += ", ";
                strSet += QuoteIdentifier( m_astrHeaders.Get( n ) ) + " = ?";
                SValue oValue;
                oValue.eKind = IsNewValueBlob( n ) ? SValue::BLOB : SValue::TEXT;
                oValue.strText = GetNewValue( n );
                if ( oValue.eKind == SValue::BLOB )
                    oValue.oBlob = GetNewBlob( n );
                aValues.push_back( oValue );
            }
            if ( strSet.GetLength() == 0 )
                return IASLIB_DB_OK;            // nothing changed
            strSQL = "UPDATE " + strTable2 + " SET " + strSet + strMatch;
        }
        else if ( nAction == ACTION_DELETE )
        {
            strSQL = "DELETE FROM " + strTable2 + strMatch;
        }
        else
        {
            CString strColumns, strMarks;
            for ( size_t n = 0; n < m_nColumns; n++ )
            {
                if ( ! IsChanged( n ) )
                    continue;
                if ( strColumns.GetLength() )
                {
                    strColumns += ", ";
                    strMarks += ", ";
                }
                strColumns += QuoteIdentifier( m_astrHeaders.Get( n ) );
                strMarks += "?";
                SValue oValue;
                oValue.eKind = IsNewValueBlob( n ) ? SValue::BLOB : SValue::TEXT;
                oValue.strText = GetNewValue( n );
                if ( oValue.eKind == SValue::BLOB )
                    oValue.oBlob = GetNewBlob( n );
                aValues.push_back( oValue );
            }
            strSQL = ( strColumns.GetLength() ) ? "INSERT INTO " + strTable2 + " (" + strColumns + ") VALUES (" + strMarks + ")"
                                                : "INSERT INTO " + strTable2 + " DEFAULT VALUES";
        }

        if ( nAction != ACTION_INSERT )
        {
            CStringArray *pRow = m_aastrData[ m_nCurrentRow ];
            for ( size_t n = 0; n < m_nColumns; n++ )
            {
                SValue oValue;
                oValue.eKind = IsNull( n ) ? SValue::NULLVALUE : IsBlob( n ) ? SValue::BLOB : SValue::TEXT;
                oValue.strText = pRow->Get( n );
                if ( oValue.eKind == SValue::BLOB )
                    oValue.oBlob = GetBlob( n );
                aValues.push_back( oValue );
            }
        }

        sqlite3 *pDB = m_pConnection->GetHandle();
        if ( pDB == NULL )
            return IASLIB_DB_ERROR_CONNECTION_DEAD;
        sqlite3_stmt *pStatement = NULL;
        int nResult = sqlite3_prepare_v2( pDB, strSQL, -1, &pStatement, NULL );
        if ( nResult != SQLITE_OK )
        {
            m_pConnection->FailFromSQLite( nResult );
            return IASLIB_DB_ERROR_BAD_QUERY;
        }
        for ( size_t n = 0; n < aValues.size(); n++ )
        {
            const SValue &oValue = aValues[n];
            if ( oValue.eKind == SValue::NULLVALUE )
                sqlite3_bind_null( pStatement, (int)n + 1 );
            else if ( oValue.eKind == SValue::BLOB )
                sqlite3_bind_blob( pStatement, (int)n + 1, (const void *)oValue.oBlob, (int)oValue.oBlob.GetSize(), SQLITE_TRANSIENT );
            else
                sqlite3_bind_text( pStatement, (int)n + 1, (const char *)oValue.strText, (int)oValue.strText.GetLength(), SQLITE_TRANSIENT );
        }
        nResult = sqlite3_step( pStatement );
        sqlite3_finalize( pStatement );
        if ( nResult != SQLITE_DONE )
        {
            m_pConnection->FailFromSQLite( nResult );
            return IASLIB_DB_ERROR_BAD_QUERY;
        }
        if ( nAction != ACTION_INSERT && sqlite3_changes( pDB ) == 0 )
        {
            m_pConnection->Fail( IASLIB_DB_ERROR_BAD_QUERY, "No row of the table matches the cursor's row." );
            return IASLIB_DB_ERROR_BAD_QUERY;
        }
        m_pConnection->Succeed();

            // Keep the cursor's own copy in step.
        if ( nAction == ACTION_UPDATE )
        {
            CStringArray *pRow = m_aastrData[ m_nCurrentRow ];
            for ( size_t n = 0; n < m_nColumns; n++ )
            {
                if ( IsChanged( n ) )
                {
                    (*pRow)[ n ] = GetNewValue( n );
                    if ( m_nCurrentRow < m_aabNull.size() && n < m_aabNull[ m_nCurrentRow ].size() )
                        m_aabNull[ m_nCurrentRow ][ n ] = 0;
                    if ( m_nCurrentRow < m_aaBlobs.size() )
                    {
                        if ( IsNewValueBlob( n ) )
                            m_aaBlobs[ m_nCurrentRow ][ n ] = GetNewBlob( n );
                        else
                            m_aaBlobs[ m_nCurrentRow ].erase( n );
                    }
                }
            }
        }
        else if ( nAction == ACTION_DELETE )
        {
            delete m_aastrData[ m_nCurrentRow ];
            for ( size_t n = m_nCurrentRow + 1; n < m_nRows; n++ )
                m_aastrData[ n - 1 ] = m_aastrData[ n ];
            m_nRows--;
            if ( m_nCurrentRow < m_aabNull.size() )
                m_aabNull.erase( m_aabNull.begin() + (long)m_nCurrentRow );
            if ( m_nCurrentRow < m_aaBlobs.size() )
                m_aaBlobs.erase( m_aaBlobs.begin() + (long)m_nCurrentRow );
            if ( m_nCurrentRow >= m_nRows && m_nRows > 0 )
                m_nCurrentRow = m_nRows - 1;
        }
        ClearChanges();
        return IASLIB_DB_OK;
    }

    int CSQLiteCursor::DeleteRow( const char *strTable )
    {
        return ApplyRow( strTable, ACTION_DELETE );
    }

    int CSQLiteCursor::InsertRow( const char *strTable )
    {
        return ApplyRow( strTable, ACTION_INSERT );
    }

    int CSQLiteCursor::UpdateRow( const char *strTable )
    {
        return ApplyRow( strTable, ACTION_UPDATE );
    }
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
