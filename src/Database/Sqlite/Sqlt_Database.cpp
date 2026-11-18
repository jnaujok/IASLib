/*
 *  SQL Lite Database
 *
 *      See Sqlt_Database.h.
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

#include "sqlite3.h"
#include "Sqlt_Database.h"
#include "Sqlt_Connection.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSQLiteDatabase, CDatabase );

    CSQLiteDatabase::CSQLiteDatabase( void ) : CDatabase( "SQLite" )
    {
        m_nLastErrorCode = IASLIB_DB_OK;
        m_nExtendedErrorCode = 0;
        m_nTimeoutMs = 10000;
    }

    CSQLiteDatabase::~CSQLiteDatabase( void )
    {
        Disconnect();
        m_bConnected = false;
    }

    bool CSQLiteDatabase::Connect( void )
    {
        m_bConnected = true;
	    return m_bConnected;
    }

        // (A connection that failed to open was returned anyway, not in the
        // list, and the caller had to know to delete it.)
    CConnection *CSQLiteDatabase::Connection( const char *strDBName, const char *, const char *, const char *, const char *strName )
    {
        m_strDBFile = ( strDBName ) ? strDBName : "";

        CSQLiteConnection *pNewCon = new CSQLiteConnection( strDBName, strName, this );
        if ( ! pNewCon->Connected() )
        {
            m_nLastErrorCode = pNewCon->MajorErrorCode();
            m_nExtendedErrorCode = pNewCon->MinorErrorCode();
            m_strLastError = pNewCon->MajorErrorMessage();
            pNewCon->Detach();
            delete pNewCon;
            return NULL;
        }

        m_nLastErrorCode = IASLIB_DB_OK;
        m_nExtendedErrorCode = 0;
        m_strLastError = "";
        pNewCon->SetTimeout( m_nTimeoutMs );
        m_aConnections.Push( pNewCon );
        m_nConnections = m_aConnections.Length();
        m_bConnected = true;
	    return pNewCon;
    }

    CBulkCopyConnection *CSQLiteDatabase::BulkCopyConnection( const char *, const char *, const char *, const char *, const char * )
    {
        m_nLastErrorCode = IASLIB_DB_ERROR_UNSUPPORTED;
        m_strLastError = "SQLite does not have bulk copy.";
	    return NULL;
    }

        // (Deleting the connections in place, while each one removed itself
        // from the list, skipped every other one.)
    int CSQLiteDatabase::Disconnect( void )
    {
        for ( int n = 0; n < m_aConnections.Length(); n++ )
            static_cast<CSQLiteConnection *>( m_aConnections.Get( n ) )->Detach();
        m_aConnections.DeleteAll();
        m_nConnections = 0;
	    return IASLIB_DB_ERROR_OK;
    }

        // (They reported whatever the last connection in the list had.)
    int CSQLiteDatabase::MajorErrorCode( void )
    {
        return m_nLastErrorCode;
    }

    int CSQLiteDatabase::MinorErrorCode( void )
    {
        return m_nExtendedErrorCode;
    }

    const char *CSQLiteDatabase::MajorErrorMessage( void )
    {
        return m_strLastError;
    }

    const char *CSQLiteDatabase::MinorErrorMessage( void )
    {
        return m_strLastError;
    }

    int CSQLiteDatabase::ReleaseConnection( CConnection *pCon )
    {
        int nFound = m_aConnections.Find( pCon );
        if ( nFound >= 0 )
            m_aConnections.Delete( nFound );
        m_nConnections = m_aConnections.Length();       // (never went down)
        return nFound;
    }

        // (It applied the timeout but always reported failure.)
    bool CSQLiteDatabase::SetTimeout( int nTimeout )
    {
        if ( nTimeout < 0 )
            return false;
        m_nTimeoutMs = nTimeout * 1000;
        for ( int n = 0; n < m_aConnections.Length(); n++ )
            static_cast<CSQLiteConnection *>( m_aConnections.Get( n ) )->SetTimeout( m_nTimeoutMs );
        return true;
    }
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
