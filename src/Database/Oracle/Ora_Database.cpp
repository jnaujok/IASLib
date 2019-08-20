/*
 *  COracleDatabase
 *
 *  This class defines an Oracle 7.3 database environment that can be
 * used to generate connections to retrieve and provide data from/to the
 * Oracle database.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/17/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "Ora_Database.h"
#include "Ora_Connection.h"

extern "C"
{
    #include <oci.h>
}

namespace IASLib
{
    CString      COracleDatabase::m_strLastMajorMessage      = "None";
    int          COracleDatabase::m_nLastMajorErrorCode      = 0;
    int          COracleDatabase::m_nLastMinorErrorCode      = 0;
    CString      COracleDatabase::m_strLastMinorMessage      = "None";

    bool         COracleDatabase::m_bOCIInitialized          = false;
    CObject      COracleDatabase::m_oOracleProxy;

#ifdef IASLIB_MULTI_THREADED__
    static CMutex  mutexMemoryContext;
#endif

    extern "C"
    {

        static dvoid *OracleDatabase_Alloc( dvoid *pContext, size_t nSize )
        {
#ifndef IASLIB_NO_LINT__
            pContext = pContext;
#endif

#ifdef IASLIB_MULTI_THREADED__
            mutexMemoryContext.Lock();
#endif
            void *pTemp = new char[nSize + sizeof( size_t ) ];
            size_t *pValue = (size_t *)pTemp;
            *pValue = nSize;
            pValue++;
#ifdef IASLIB_MULTI_THREADED__
            mutexMemoryContext.Unlock();
#endif
            return (dvoid *)pValue;
        }

        static dvoid *OracleDatabase_Realloc( dvoid *pContext, dvoid *pData, size_t nSize )
        {
#ifndef IASLIB_NO_LINT__
            pContext = pContext;
#endif
#ifdef IASLIB_MULTI_THREADED__
            mutexMemoryContext.Lock();
#endif
            void *pTemp = new char[nSize + sizeof( size_t ) ];
            size_t *pValue = (size_t *)pData;
            void *pDel;
            pValue--;
            pDel = pValue;
            size_t nOldSize = *pValue;
            pValue = (size_t *)pTemp;
            *pValue = nSize;
            pValue++;
            pTemp = pValue;

            memcpy( pTemp, pData, nOldSize );

            delete [] pDel;

#ifdef IASLIB_MULTI_THREADED__
            mutexMemoryContext.Unlock();
#endif
            return pTemp;
        }

        static void OracleDatabase_Free( dvoid *pContext, dvoid *pData )
        {
#ifndef IASLIB_NO_LINT__
            pContext = pContext;
#endif
#ifdef IASLIB_MULTI_THREADED__
            mutexMemoryContext.Lock();
#endif
            size_t *pValue = (size_t *)pData;
            pValue--;
            pData = (dvoid *)pValue;

            delete [] pData;

#ifdef IASLIB_MULTI_THREADED__
            mutexMemoryContext.Unlock();
#endif
        }

    }; // End of extern "C"

    IMPLEMENT_OBJECT( COracleDatabase, CDatabase );

    COracleDatabase::COracleDatabase( void ) :
        CDatabase( "Oracle" )
    {
        if ( ! m_bOCIInitialized )
        {
            //m_oOracleProxy.SetType( "Oracle_Engine_Proxy" );
            OCIInitialize( OCI_THREADED, (dvoid *)this, OracleDatabase_Alloc, OracleDatabase_Realloc, OracleDatabase_Free );
            m_bOCIInitialized = true;
        }

    #ifdef IASLIB_MULTI_THREADED__
        opinit( OCI_EV_TSF );
    #endif

        m_bConnected = true;
    }

    COracleDatabase::~COracleDatabase( void )
    {
        Disconnect();
        m_bConnected = 0;
    }

    bool COracleDatabase::Connect( void )
    {
        CDatabase::Connect( "Oracle" );

	    return m_bConnected;
    }

    CConnection *COracleDatabase::Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication, const char *strConName )
    {
#ifndef IASLIB_NO_LINT__
        strApplication = strApplication;
#endif
        ub1        *pHDA = new ub1[512];
        Lda_Def    *pLDA = new Lda_Def;
        CString     strDB;
        CString     strUser;
        CString     strPW;
        
        if ( ! strDBName )
            strDB = m_strDefaultDB;
        else
            strDB = strDBName;

        if ( ! strUserName )
            strUser = m_strDefaultUser;
        else
            strUser = strUserName;

        if ( ! strPassword )
            strPW = m_strDefaultPassword;
        else
            strPW = strPassword;

        if ( strDB.GetLength() )
        {
            strUser = strUser + "/" + strPW + "@" + strDB;
        }
        else
        {
            strUser = strUser + "/" + strPW;
        }

        if ( orlon( pLDA, pHDA, (text *)(const char *)strUser, (sword)-1, (text *)NULL, (sword)-1, (sword)-1 ) )
        {
            // Do nothing...
        }
        else
        {
            COracleConnection *pNewConnection = new COracleConnection( pLDA, pHDA, strConName );
            if ( pNewConnection->Connected() )
            {
                m_aConnections.Push( pNewConnection );
                pNewConnection->SetDatabase( this );
                m_nConnections++;

                return pNewConnection;
            }
            else
            {
                delete pNewConnection;
            }
        }

	    return NULL;
    }

    CBulkCopyConnection *COracleDatabase::BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication, const char *strConName )
    {
#ifndef IASLIB_NO_LINT__
        strConName = strConName;
        strApplication = strApplication;
        strPassword = strPassword;
        strUserName = strUserName;
        strDBName = strDBName;
#endif
            // Return a bulk copy ready connection.
        return NULL;
    }

    int COracleDatabase::ReleaseConnection( CConnection *pCon )
    {
        int             nCount = 0;

        while ( nCount < m_nConnections )
        {
            if ( m_aConnections[ nCount ] == pCon )
            {
                m_aConnections.Delete( nCount );

                m_nConnections--;
                return 0;
            }
            nCount++;
        }

        return -1;
    }

    int COracleDatabase::Disconnect( void )
    {
        m_aConnections.DeleteAll();
        m_nConnections = 0;
	    return 0;
    }

    int COracleDatabase::MajorErrorCode( void )
    {
	    return m_nLastMajorErrorCode;
    }

    int COracleDatabase::MinorErrorCode( void )
    {
	    return m_nLastMinorErrorCode;
    }

    const char *COracleDatabase::MajorErrorMessage( void )
    {
	    return m_strLastMajorMessage;
    }

    const char *COracleDatabase::MinorErrorMessage( void )
    {
	    return m_strLastMinorMessage;
    }

    void COracleDatabase::StoredProcArgs( const char *strProcName, ... )
    {
#ifndef IASLIB_NO_LINT__
        strProcName = strProcName;
#endif
    }


    bool COracleDatabase::SetTimeout( int nTimeout )
    {
#ifndef IASLIB_NO_LINT__
        nTimeout = nTimeout;
#endif
        return true;
    }

    void COracleDatabase::SetDefaults( const char *strDB, const char *strUser, const char *strPW )
    {
        if ( strDB )
            m_strDefaultDB = strDB;
        else
            m_strDefaultDB = "";

        if ( strUser )
            m_strDefaultUser = strUser;
        else
            m_strDefaultUser = "";

        if ( strPW )
            m_strDefaultPassword = strPW;
        else
            m_strDefaultPassword = "";
    }
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__

