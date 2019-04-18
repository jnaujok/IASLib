/***********************************************************************
**  COracleConnection
**
**  Description:
**      This class defines and implements a connection to an Oracle
** database.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include <stdio.h>
#include "Ora_Database.h"
#include "Ora_Connection.h"
#include "Ora_StoredProc.h"
#include "Ora_Cursor.h"

#include "Date.h"

extern "C"
{
#include <oci.h>
}

namespace IASLib
{
#ifdef IASLIB_MULTI_THREADED__
    CMutex COracleConnection::m_mutexSynchro;
#endif

    IMPLEMENT_OBJECT( COracleConnection, CConnection );

    COracleConnection::COracleConnection( Lda_Def *pLDA, ub1 *pHDA, const char *strName ) 
        : CConnection( strName )
    {
        m_pLDA = pLDA;
        m_pHDA = pHDA;

        m_bValid = 1;
	    m_bDead = 0;

        m_bDeadlock = 0;
        m_lAffectedRows = 0L;
        m_bConnected = 1;
        m_pDatabase = NULL;
    }

    COracleConnection::~COracleConnection( void )
    {
        if ( m_bConnected )
        {
            Release();
            if ( m_pDatabase )
            {
                m_pDatabase->ReleaseConnection( this );
            }
        }
    }

    void COracleConnection::SetDatabase( CDatabase *pDatabase )
    {
        m_pDatabase = pDatabase;
    }

    bool COracleConnection::BeginTransaction( void )
    {
        m_bInTransaction = true;
        return true;
    }

    bool COracleConnection::CommitTransaction( void )
    {
        if ( ( ! m_bInTransaction ) || ( ocom( m_pLDA ) ) )
        {
            return false;
        }
        m_bInTransaction = false;
        return true;
    }

    bool COracleConnection::RollbackTransaction( void )
    {
        if ( ( ! m_bInTransaction ) || ( orol( m_pLDA ) ) )
        {
            return false;
        }
        m_bInTransaction = false;
        return true;
    }

    CCursor *COracleConnection::Cursor( const char *strQueryString, bool bUpdatable )
    {
        CString         strConQuery = strQueryString;
        COracleCursor  *pNewCursor = NULL;

        strConQuery.Trim();

        CString strStart = strConQuery.Substring( 0, 6 );
        strStart.ToUpperCase();

        if ( strStart != "SELECT" )
        {
            return NULL;
        }


            // Check if this connection has been marked as dead. If it has, fail this operation.
	    if ( m_bDead )
        {
		    return NULL;
	    }

        Cda_Def *pCDA = new Cda_Def;

            // Attempt to open the Cursor Data Area. This shouldn't fail...
        if ( oopen( pCDA, m_pLDA, (text *)NULL, (sword)-1, (sword)-1, (text *)NULL, (sword)-1 ) )
        {
                // But, in case it does, we mark the connection dead.
            m_bDead = true;
            delete pCDA;
            return NULL;
        }

            // Parse out the select statement (Defer the processing).
        if ( oparse( pCDA, (unsigned char *)(const char *)strConQuery, (sb4)strConQuery.GetLength(), DEFER_PARSE, NATIVE ) )
        {
                // Parse failed, mark the connection as dead.
            m_bDead = true;
            oclose( pCDA );
            delete pCDA;
            return NULL;
        }

        pNewCursor = new COracleCursor( this, m_pLDA, pCDA, bUpdatable );

        if ( pNewCursor->IsValid() )
        {
            return pNewCursor;
        }

        delete pNewCursor;
        oclose( pCDA );
        delete pCDA;
        return NULL;
    }

    bool COracleConnection::Execute( const char *strExecString )
    {
        Cda_Def *pCDA = new Cda_Def;
        CString strConQuery( strExecString );

            // Check if this connection has been marked as dead. If it has, fail this operation.
	    if ( m_bDead )
        {
            delete pCDA;
		    return false;
	    }

            // Attempt to open the Cursor Data Area. This shouldn't fail...
        if ( oopen( pCDA, m_pLDA, (text *)NULL, (sword)-1, (sword)-1, (text *)NULL, (sword)-1 ) )
        {
                // But, in case it does, we mark the connection dead.
            m_bDead = true;
            delete pCDA;
            return false;
        }

            // Parse out the select statement (Defer the processing).
        if ( oparse( pCDA, (unsigned char *)(const char *)strConQuery, (sb4)strConQuery.GetLength(), DEFER_PARSE, NATIVE ) )
        {
                // Parse failed, mark the connection as dead.
            m_bDead = true;
            oclose( pCDA );
            delete pCDA;
            return false;
        }

        if ( oexec( pCDA ) )
        {
            m_bDead = true;
            oclose( pCDA );
            delete pCDA;
            return false;
        }

        if ( ocom( m_pLDA ) )
        {
            m_bDead = true;
            oclose( pCDA );
            delete pCDA;
            return false;
        }

        oclose( pCDA );
        delete pCDA;
        return true;
    }

    int COracleConnection::MajorErrorCode( void )
    {
        return m_pDatabase->MajorErrorCode();
    }

    int COracleConnection::MinorErrorCode( void )
    {
        return m_pDatabase->MinorErrorCode();
    }

    const char *COracleConnection::MajorErrorMessage( void )
    {
        return m_pDatabase->MajorErrorMessage();
    }

    const char *COracleConnection::MinorErrorMessage( void )
    {
        return m_pDatabase->MinorErrorMessage();
    }

    bool COracleConnection::Release( void )
    {
        if ( m_bValid )
        {
            if ( m_pDatabase )
            {
                m_pDatabase->ReleaseConnection( this );
            }

                // First we be sure to commit any remaining transactions
            ocom( m_pLDA );
                
                // Then we log off the connection
            ologof( m_pLDA );

            delete m_pLDA;
            delete m_pHDA;
        }
        return true;
    }

    CString COracleConnection::SQLTable( const char *strQueryString, CStream *pStream )
    {
        CCursor    *pResult;
        CString     strReturnString = "<br>";
        size_t       nColumns;

        pResult = this->Cursor( strQueryString, false );

        if ( pResult != NULL )
        {
            strReturnString += "<table border=1>\n";
            nColumns = pResult->Columns();
            strReturnString += "<tr>";
            for ( size_t nCount = 0; nCount < nColumns ; nCount++ )
            {
                strReturnString += "<th>";
                strReturnString += (const char *)pResult->ColumnName( nCount );
                strReturnString += "</th>";
            }
            strReturnString += "</tr>\n";

            if ( pStream )
            {
                (*pStream) << strReturnString;
                strReturnString = "";
            }

            if ( pResult->Rows() )
            {
                do
                {
                    strReturnString += "<tr>";
                    for ( size_t nCount = 0; nCount < nColumns ; nCount++ )
                    {
                        strReturnString += "<td>";
                        strReturnString += (const char *)pResult->GetColumn( nCount );
                        strReturnString += "</td>";
                    }
                    strReturnString += "</tr>";
                    if ( pStream )
                    {
                        (*pStream) << strReturnString;
                        strReturnString = "";
                    }
                } while ( pResult->Next() );
            }
            strReturnString += "</table>";
            pResult->Close();
        }
        delete pResult;

        if ( pStream )
        {
            (*pStream) << strReturnString;
            strReturnString = "";
        }

        return strReturnString;
    }


    CStoredProc *COracleConnection::StoredProc( const char *strProcCall, const char *strParamList )
    {
        CString             strQuery = strProcCall;
        COracleStoredProc  *pNewProc = NULL;
        CDate               dttNow;

            // Check if this connection has been marked as dead. If it has, fail this operation.
	    if ( m_bDead )
        {
		    return NULL;
	    }

        Cda_Def *pCDA = new Cda_Def;

            // Attempt to open the Cursor Data Area. This shouldn't fail...
        if ( oopen( pCDA, m_pLDA, (text *)NULL, (sword)-1, (sword)-1, (text *)NULL, (sword)-1 ) )
        {
                // But, in case it does, we mark the connection dead.
            m_bDead = true;
            delete pCDA;
            return NULL;
        }

            // Parse out the select statement (Defer the processing).
        if ( oparse( pCDA, (unsigned char *)(const char *)strQuery, (sb4)strQuery.GetLength(), DEFER_PARSE, NATIVE ) )
        {
                // Parse failed, mark the connection as dead.
            m_bDead = true;
            oclose( pCDA );
            delete pCDA;
            return NULL;
        }

        pNewProc = new COracleStoredProc( this, pCDA, strParamList );

        CDate dttThen;

        if ( pNewProc->IsValid() )
        {
            return pNewProc;
        }

        delete pNewProc;
        oclose( pCDA );
        delete pCDA;
        return NULL;
    }
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
