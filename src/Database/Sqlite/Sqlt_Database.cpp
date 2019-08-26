/*
 *  SQL Lite Database
 *
 *      Database class for connecting to SQLite. Please note that the
 * SQL Lite database requires a unique connection for each thread, which 
 * means that this database type really doesn't do much other then store
 * the information making the connection to SQL Lite. Each connection
 * will then be it's own connection/database.
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
    CDynamicLibrary *CSQLiteDatabase::m_pSqliteLibrary = NULL;

    IMPLEMENT_OBJECT( CSQLiteDatabase, CDatabase );


    void CSQLiteDatabase::initLibrary( void )
    {
        if ( ! m_pSqliteLibrary )
        {
            m_pSqliteLibrary = new CDynamicLibrary( "sqlite3" );
        }
    }

    CSQLiteDatabase::CSQLiteDatabase( void ) : CDatabase( "SQL Lite" )
    {
    }
    
    CSQLiteDatabase::~CSQLiteDatabase( void ) 
    {
        Disconnect();
        m_bConnected = 0;
    }

    void *CSQLiteDatabase::GetFunction( const char *strName )
    {
        initLibrary();
        return m_pSqliteLibrary->GetFunction( strName );
    }

    bool CSQLiteDatabase::Connect( void )
    {
        CDatabase::Connect( "SQL Lite" );
        m_bConnected = true;
	    return m_bConnected;
    }

    CConnection *CSQLiteDatabase::Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication, const char *strName )
    {
#ifndef IASLIB_NO_LINT__
        strApplication = strApplication;
        strPassword = strPassword;
        strUserName = strUserName;
#endif
        CConnection *pNewCon = NULL;

        m_strDBFile = strDBName;

        pNewCon = new CSQLiteConnection( strDBName, strName, this );

        if ( pNewCon->Connected() )
        {
            m_aConnections.Push( pNewCon );
            m_nConnections++;
        }

	    return pNewCon;
    }

    CBulkCopyConnection *CSQLiteDatabase::BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication, const char *strName )
    {
#ifndef IASLIB_NO_LINT__
        strName = strName;
        strApplication = strApplication;
        strPassword = strPassword;
        strUserName = strUserName;
        strDBName = strDBName;
#endif
	    return NULL;
    }

    int CSQLiteDatabase::Disconnect( void )
    {
        m_aConnections.DeleteAll();
        m_nConnections = 0;
	    return IASLIB_DB_ERROR_OK;
    }

    int CSQLiteDatabase::MajorErrorCode( void )
    {
        int     nCount = 0;
        
        m_nLastErrorCode = 0;
       
        while ( nCount < m_aConnections.Length() )
        {
            int nTemp = m_aConnections.Get( nCount )->MajorErrorCode();
            if ( nTemp != m_nLastErrorCode )
            {
                m_nLastErrorCode = nTemp;
            }
            nCount++;
        }

        return m_nLastErrorCode;
    }

    int CSQLiteDatabase::MinorErrorCode( void )
    {
        return m_nLastErrorCode;
    }

    const char *CSQLiteDatabase::MajorErrorMessage( void )
    {
        int     nCount = 0;
        
        m_strLastError = "";
       
        while ( nCount < m_aConnections.Length() )
        {
            CString strTemp = m_aConnections.Get( nCount )->MajorErrorMessage();
            if ( strTemp != m_strLastError )
            {
                m_strLastError = strTemp;
            }
            nCount++;
        }

        return m_strLastError;
    }

    const char *CSQLiteDatabase::MinorErrorMessage( void )
    {
        return "";
    }

    int CSQLiteDatabase::ReleaseConnection( CConnection *pCon )
    {
        int     nCount = 0;
        int     nFound = -1;

        while ( nCount < m_aConnections.Length() )
        {
            CConnection *pTemp = m_aConnections.Get( nCount );
            if ( pTemp == pCon )
            {
                m_aConnections.Delete( nCount );
                nFound = nCount;
                break;
            }
            nCount++;
        }

        return nFound;
    }

    bool CSQLiteDatabase::SetTimeout( int nTimeout )
    {
        int     nCount = 0;

        while ( nCount < m_aConnections.Length() )
        {
            CSQLiteConnection *pTemp = (CSQLiteConnection *)m_aConnections.Get( nCount );
            pTemp->SetTimeout( nTimeout * 1000 ); 
            nCount++;
        }

        return false;
    }
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
