/***********************************************************************
**  CSybaseConection
**
**  Description:
**      This class defines and implements a connection to a Sybase
** database.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifdef GENLIB_INCLUDE_ORACLE

#include <stdio.h>
#include "Syb_Connection.h"
#include "Syb_Cursor.h"
#include "Syb_StoredProc.h"
#include "Bool.h"
#include "Date.h"

CMutex CSybaseConnection::m_mutexSynchro;

CSybaseConnection::CSybaseConnection( CS_CONNECTION *pConnect, const char *strDatabase, const char *strUserName, const char *strPassword, const char *strApplication, const char *strName ) :
    CConnection( strName )
{
    IMPLEMENT_OBJECT( CSybaseConnection );

    m_pConnection = pConnect;
    CConnection *pTemp = this;

        // Connect the data from this C++ object with the connection in sybase.
	    // Note - this saves a copy of "this", not a pointer to it.
	if ( ct_con_props( m_pConnection, CS_SET, CS_USERDATA, &pTemp, CS_SIZEOF(pTemp), NULL) != CS_SUCCEED )
    {
		ct_con_drop(m_pConnection);
		return;
	}

        // Here we set up the various data we need to actually tie into a real, physical database.
    ct_con_props(m_pConnection, CS_SET, CS_USERNAME, (void *)strUserName, CS_NULLTERM, NULL);
    ct_con_props(m_pConnection, CS_SET, CS_PASSWORD, (void *)strPassword, CS_NULLTERM, NULL);
	ct_con_props(m_pConnection, CS_SET, CS_APPNAME, (void *)strApplication, CS_NULLTERM, NULL);
        // Do we need this line?  ct_con_props(m_pConnection, CS_SET, CS_HOSTNAME, php3_sybct_module.hostname, CS_NULLTERM, NULL);

    m_bValid = 1;
	m_bDead = 0;

	    // Create the link to the database
	if ( ct_connect( m_pConnection, (char *)strDatabase, CS_NULLTERM ) != CS_SUCCEED)
    {
		m_bValid = 0;
		ct_con_drop( m_pConnection );
		return;
	}

	if ( ct_cmd_alloc( m_pConnection, &m_pCommand ) != CS_SUCCEED)
    {
		m_bValid = 0;
		ct_close( m_pConnection, CS_UNUSED );
		ct_con_drop( m_pConnection );
		return;
	}

    CString strUseClause = "use ";
    strUseClause += strApplication;
    Execute( strUseClause );

    m_bDeadlock = 0;
    m_lAffectedRows = 0L;
    m_bConnected = 1;
    m_pDatabase = NULL;
}

CSybaseConnection::~CSybaseConnection( void )
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


void CSybaseConnection::SetDatabase( CDatabase *pDatabase )
{
    m_pDatabase = pDatabase;
}

bool CSybaseConnection::BeginTransaction( void )
{
    return true;
}

bool CSybaseConnection::CommitTransaction( void )
{
    return true;
}

bool CSybaseConnection::RollbackTransaction( void )
{
    return true;
}

CCursor *CSybaseConnection::Cursor( const char *strQueryString, bool bUpdatable )
{
    CString     strConQuery = strQueryString;
    CS_RETCODE  nRetcode;
    CS_INT     *pResultType = new CS_INT;
    int         nFailure = -1;
    CSybaseCursor   *pNewCursor = NULL;

    AddSize( sizeof( CS_INT ) );

    strConQuery.Trim();

    CString strStart = strConQuery.Substring( 0, 6 );
    strStart.ToUpperCase();

    if ( strStart != "SELECT" )
    {
        delete pResultType;
        AddSize( -((int)sizeof( CS_INT )) );
        return NULL;
    }


        // Check if this connection has been marked as dead. If it has, fail this operation.
	if ( m_bDead )
    {
        delete pResultType;
        AddSize( -((int)sizeof( CS_INT )) );
		return NULL;
	}

        // Take the passed in command string and parse it out, then send it
        // to the server for processing.
	if ( ct_command( m_pCommand, CS_LANG_CMD, (char *)strQueryString, CS_NULLTERM, CS_UNUSED ) != CS_SUCCEED )
    {
		m_bDead = 1;
        delete pResultType;
        AddSize( -((int)sizeof( CS_INT )) );
		return NULL;
	}

	if ( ct_send( m_pCommand ) != CS_SUCCEED )
    {
		m_bDead = 1;
        delete pResultType;
        AddSize( -((int)sizeof( CS_INT )) );
		return NULL;
	}

    while ( ( nRetcode = ct_results( m_pCommand, pResultType ) ) == CS_SUCCEED )
    {
		switch ( (int) *pResultType )
        {
			case CS_CMD_SUCCEED:
                break;

			case CS_CMD_DONE:
                break;

			case CS_COMPUTE_RESULT:
			case CS_CURSOR_RESULT:
			case CS_PARAM_RESULT:
			case CS_ROW_RESULT:
			case CS_STATUS_RESULT:
                pNewCursor = new CSybaseCursor( this, m_pCommand, bUpdatable );
            	break;

			case CS_CMD_FAIL:
				nFailure = 1;
				break;

			default:
				nFailure = 2;
				break;
		}

		if ( nFailure != -1 )
        {
            delete pNewCursor;
			ct_cancel( NULL, m_pCommand, CS_CANCEL_ALL );
            delete pResultType;
            AddSize( -((int)sizeof( CS_INT )) );
			return NULL;
		}
	}

	switch ( nRetcode )
    {
		case CS_END_RESULTS:
			break;

		case CS_FAIL:
			    // Hopefully this either cleans up the connection, or the
			    // connection ends up marked dead. We may want to do
                // ct_close(CS_FORCE_CLOSE) if ct_cancel() fails; see the
                // doc for ct_results() == CS_FAIL.

            ct_cancel( NULL, m_pCommand, CS_CANCEL_ALL );
                // Don't take chances with the vagaries of ct-lib.  Mark it
                // dead ourselves.
  			m_bDead = 1;
            delete pResultType;
            AddSize( -((int)sizeof( CS_INT )) );
			return NULL;

		default:
            delete pResultType;
            AddSize( -((int)sizeof( CS_INT )) );
			return NULL;
	}

    if ( pNewCursor->IsValid() )
    {
        delete pResultType;
        AddSize( -((int)sizeof( CS_INT )) );
        return pNewCursor;
    }

    delete pNewCursor;
    delete pResultType;
    AddSize( -((int)sizeof( CS_INT )) );
    return NULL;
}

bool CSybaseConnection::Execute( const char *strExecString )
{
    CS_RETCODE  nRetcode;
    CS_INT     *pResultType = new CS_INT;
    int         nFailure = -1;

        // Check if this connection has been marked as dead. If it has, fail this operation.
	if ( m_bDead )
    {
        delete pResultType;
		return false;
	}

        // Take the passed in command string and parse it out, then send it
        // to the server for processing.

	if ( ct_command( m_pCommand, CS_LANG_CMD, (char *)strExecString, CS_NULLTERM, CS_UNUSED ) != CS_SUCCEED )
    {
		m_bDead = 1;
        delete pResultType;
		return false;
	}

	if ( ct_send( m_pCommand ) != CS_SUCCEED )
    {
		m_bDead = 1;
        delete pResultType;
		return false;
	}

	while ( ( nRetcode = ct_results( m_pCommand, pResultType ) ) == CS_SUCCEED )
    {
		switch ( (int) *pResultType )
        {
			case CS_CMD_SUCCEED:
                break;

			case CS_CMD_DONE:
                break;

			case CS_CMD_FAIL:
				nFailure = 1;
				break;

			case CS_STATUS_RESULT:
				ct_cancel( NULL, m_pCommand, CS_CANCEL_CURRENT );
				break;

			default:
				nFailure = 2;
				break;
		}

		if ( nFailure != -1 )
        {
			ct_cancel( NULL, m_pCommand, CS_CANCEL_ALL );
            delete pResultType;
			return false;
		}
	}

	switch ( nRetcode )
    {
		case CS_END_RESULTS:
            delete pResultType;
			return true;

		case CS_FAIL:
			    // Hopefully this either cleans up the connection, or the
			    // connection ends up marked dead. We may want to do
                // ct_close(CS_FORCE_CLOSE) if ct_cancel() fails; see the
                // doc for ct_results() == CS_FAIL.

            ct_cancel( NULL, m_pCommand, CS_CANCEL_ALL );
                // Don't take chances with the vagaries of ct-lib.  Mark it
                // dead ourselves.
  			m_bDead = 1;
            delete pResultType;
			return false;

		default:
            delete pResultType;
			return false;
	}
}

int CSybaseConnection::MajorErrorCode( void )
{
    return m_pDatabase->MajorErrorCode();
}

int CSybaseConnection::MinorErrorCode( void )
{
    return m_pDatabase->MinorErrorCode();
}

const char *CSybaseConnection::MajorErrorMessage( void )
{
    return m_pDatabase->MajorErrorMessage();
}

const char *CSybaseConnection::MinorErrorMessage( void )
{
    return m_pDatabase->MinorErrorMessage();
}

bool CSybaseConnection::Release( void )
{
    if ( m_bValid )
    {
        if ( m_pDatabase )
        {
            m_pDatabase->ReleaseConnection( this );
        }

        CS_INT con_status;

            // Make sure that we're still connected before we drop the connection...
        if ( ct_con_props( m_pConnection, CS_GET, CS_CON_STATUS, &con_status, CS_UNUSED, NULL ) != CS_SUCCEED )
        {
                // Unable to get connection status on close
                // Assume the worst.
            con_status = CS_CONSTAT_CONNECTED | CS_CONSTAT_DEAD;
        }

        if ( con_status & CS_CONSTAT_CONNECTED )
        {
            if ( ( con_status & CS_CONSTAT_DEAD ) || ct_close( m_pConnection, CS_UNUSED ) != CS_SUCCEED )
            {
                ct_close( m_pConnection, CS_FORCE_CLOSE );
            }
        }

        if ( m_pConnection )
        {
            ct_con_drop( m_pConnection );
        }
        m_pConnection = NULL;
        m_bConnected = 0;
        m_bValid = 0;
    }
    return true;
}

CString CSybaseConnection::SQLTable( const char *strQueryString, CStream *pStream )
{
    CCursor    *pResult;
    CString     strReturnString = "<br>";
    int         nColumns;

    pResult = this->Cursor( strQueryString, false );

    if ( pResult != NULL )
    {
        strReturnString += "<table border=1>\n";
        nColumns = pResult->Columns();
        strReturnString += "<tr>";
        for ( int nCount = 0; nCount < nColumns ; nCount++ )
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
                for ( int nCount = 0; nCount < nColumns ; nCount++ )
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


CStoredProc *CSybaseConnection::StoredProc( const char *strProcCall, const char *strParamList )
{
    CString             strQuery = strProcCall;
    int                 nFailure = -1;
    CSybaseStoredProc  *pNewProc = NULL;
    CDate               dttNow;

        // Check if this connection has been marked as dead. If it has, fail this operation.
	if ( m_bDead )
    {
		return NULL;
	}

	    // Take the passed in command string and parse it out, then send it
        // to the server for processing.
	if ( ct_command( m_pCommand, CS_LANG_CMD, (char *)strProcCall, CS_NULLTERM, CS_UNUSED ) != CS_SUCCEED )
    {
		m_bDead = 1;
		return NULL;
	}

	if ( ct_send( m_pCommand ) != CS_SUCCEED )
    {
		m_bDead = 1;
		return NULL;
	}

    pNewProc = new CSybaseStoredProc( m_pCommand );

    CDate dttThen;

    Log( "Stored Proc Call: %s, Run Time %i.%03is\n", strProcCall, (int)(dttThen.Elapsed( dttNow ) / 1000), (int)(dttThen.Elapsed( dttNow ) % 1000) );

    if ( pNewProc->IsValid() )
    {
        return pNewProc;
    }

    delete pNewProc;
    ct_cancel( NULL, m_pCommand, CS_CANCEL_ALL );
    return NULL;
}

#endif
