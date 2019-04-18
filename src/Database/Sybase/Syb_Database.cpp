/***********************************************************************
**  SybaseDatabase
**
**  Description:
**      This class defines and implements the Sybase Database class. It
** manages connections to and from Sybase databases.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifdef GENLIB_INCLUDE_SYBASE

#include "Syb_Database.h"
#include "Syb_Connection.h"

CString      CSybaseDatabase::m_strLastMajorMessage      = "None";
int          CSybaseDatabase::m_nMinimumClientSeverity   = 12;
int          CSybaseDatabase::m_nLastMajorErrorCode      = 0;
int          CSybaseDatabase::m_nLastMinorErrorCode      = 0;
CString      CSybaseDatabase::m_strLastMinorMessage      = "None";
int          CSybaseDatabase::m_nMinimumServerSeverity   = 12;
bool         CSybaseDatabase::m_bInitialized             = 0;
CS_CONTEXT  *CSybaseDatabase::m_pContext                 = NULL;

CSybaseDatabase::CSybaseDatabase( void ) :
    CDatabase( "Sybase" )
{
    IMPLEMENT_OBJECT( CSybaseDatabase );
    m_bConnected = Initialize();
}

CSybaseDatabase::~CSybaseDatabase( void )
{
    Disconnect();
    m_bConnected = 0;
}

int CSybaseDatabase::Connect( void )
{
    CDatabase::Connect( "Sybase" );

	return ( m_bConnected = Initialize() );

}

CConnection *CSybaseDatabase::Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication, const char *strConName )
{
    CS_CONNECTION   *pConnect;

    if ( ct_con_alloc( m_pContext, &pConnect ) == CS_SUCCEED )
    {
        CSybaseConnection *pNewConnection = new CSybaseConnection( pConnect, strDBName, strUserName, strPassword, strApplication, strConName );
        if ( pNewConnection->Connected() )
        {
            m_aConnections.Push( pNewConnection );
            pNewConnection->SetDatabase( this );
            m_nConnections++;

            return pNewConnection;
        }
        else
        {
            // TO Do: Connection Failed
        }
    }

	return NULL;
}

CBulkCopyConnection *CSybaseDatabase::BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication, const char *strConName )
{

    /*
    CS_CONNECTION   *pConnect;

    if ( ct_con_alloc( m_pContext, &pConnect ) == CS_SUCCEED )
    {
        CSybaseConnection *pNewConnection = new CSybaseConnection( pConnect, strDBName, strUserName, strPassword, strApplication, strConName );
        if ( pNewConnection->Connected() )
        {
            m_aConnections.Push( pNewConnection );
            pNewConnection->SetDatabase( this );
            m_nConnections++;

            return pNewConnection;
        }
        else
        {
            // TO Do: Connection Failed
        }
    }
    */
	return NULL;
}

int CSybaseDatabase::ReleaseConnection( CConnection *pCon )
{
    int             nCount = 0;

    while ( nCount < m_nConnections )
    {
        if ( m_aConnections[ nCount ] == pCon )
        {
            m_aConnections.Delete( nCount );

            m_nConnections --;
            return 0;
        }
        nCount++;
    }

    return -1;
}

int CSybaseDatabase::Disconnect( void )
{
    m_aConnections.DeleteAll();
    m_nConnections = 0;
	return 0;
}

int CSybaseDatabase::MajorErrorCode( void )
{
	return m_nLastMajorErrorCode;
}

int CSybaseDatabase::MinorErrorCode( void )
{
	return m_nLastMinorErrorCode;
}

const char *CSybaseDatabase::MajorErrorMessage( void )
{
	return m_strLastMajorMessage;
}

const char *CSybaseDatabase::MinorErrorMessage( void )
{
	return m_strLastMinorMessage;
}

void CSybaseDatabase::StoredProcArgs( const char *strProcName, ... )
{
}

CS_RETCODE CSybaseDatabase::ClientMessageHandler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg)
{
    m_strLastMajorMessage.Format( "Client message:  %s", errmsg->msgstring );
    m_strLastMinorMessage.Format( "Severity: %d", CS_SEVERITY(errmsg->msgnumber) );
    m_nLastMajorErrorCode = (int)errmsg->msgnumber;
    m_nLastMinorErrorCode = (int)(CS_NUMBER(errmsg->msgnumber));

    ErrorLog( "Client Error Message -- %s, %s (%i, %i)\n", (const char *)m_strLastMajorMessage, (const char *)m_strLastMinorMessage, m_nLastMajorErrorCode, m_nLastMinorErrorCode );

    /* If this is a timeout message, return CS_FAIL to cancel the
	 * operation and mark the connection as dead.
	 */
	if (CS_SEVERITY(errmsg->msgnumber) == CS_SV_RETRY_FAIL &&
		CS_NUMBER(errmsg->msgnumber) == 63 &&
		CS_ORIGIN(errmsg->msgnumber) == 2 &&
		CS_LAYER(errmsg->msgnumber) == 1)
	{
		return CS_FAIL;
	}

    return CS_SUCCEED;
}

CS_RETCODE CSybaseDatabase::ServerMessageHandler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg)
{
    m_strLastMajorMessage.Format( "Server [%s] message:  %s", srvmsg->svrname, srvmsg->text );
    if ( srvmsg->proclen > 0 )
        m_strLastMinorMessage.Format( "Procedure %s, Line: %d", srvmsg->proc, srvmsg->line );
    else
        m_strLastMinorMessage.Format( "Severity: %d", srvmsg->severity );

    m_nLastMajorErrorCode = (int)srvmsg->msgnumber;
    m_nLastMinorErrorCode = 0;
    if ( m_nLastMajorErrorCode != 5701 )
    {
        ErrorLog( "Server Error Message -- %s, %s. (%i)\n", (const char *)m_strLastMajorMessage, (const char *)m_strLastMinorMessage, m_nLastMajorErrorCode );
    }
	/* If this is a deadlock message, set the connection's deadlock flag
	 * so we will retry the request.  Sorry about the bare constant here,
	 * but it's not defined anywhere and it's a "well-known" number.
	 */
	if (srvmsg->msgnumber == 1205)
    {
		CSybaseConnection *poConnect;

		if (ct_con_props(connection, CS_GET, CS_USERDATA, &poConnect, CS_SIZEOF(poConnect), NULL)==CS_SUCCEED)
        {
		    poConnect->SetDeadlock( 1 );
		}
	}

    return CS_SUCCEED;
}

bool CSybaseDatabase::Initialize( void )
{
    int     nError;
	CS_INT		netio_type = CS_SYNC_IO;

    if ( ! m_bInitialized )
    {
        if ( ( cs_ctx_alloc( CS_VERSION_110, &m_pContext ) == CS_SUCCEED ) && ( ct_init( m_pContext, CS_VERSION_110 ) == CS_SUCCEED ) )
        {
                // We successfully initialized the library, so let's hook the message handlers.
            m_bInitialized = true;

            if ( ( nError = ct_callback( m_pContext, NULL, CS_SET, CS_SERVERMSG_CB, (CS_VOID *)ServerMessageHandler ) ) != CS_SUCCEED )
            {
                m_strLastMajorMessage.Format( "Server Message: Could not hook server message callback. [%d]", nError );
            }

            if ( ( nError = ct_callback( m_pContext, NULL, CS_SET, CS_CLIENTMSG_CB, (CS_VOID *)ClientMessageHandler ) ) != CS_SUCCEED )
            {
                m_strLastMajorMessage.Format( "Client Message: Could not hook server message callback. [%d]", nError );
            }

            if ( ( nError = ct_config( m_pContext, CS_SET, CS_NETIO, &netio_type, CS_UNUSED, NULL) ) != CS_SUCCEED )
            {
                m_strLastMajorMessage.Format( "Client Message: Could not set synchronous I/O. [%d]", nError );
            }

            /* Set datetime conversion format to "Oct 19 1961 04:15:31:000PM".
             * This levels the playing field for all platforms.
             *
             *   CS_DATES_LONG
             *     mth dd yyyy hh:mm:ss:zzz[am|pm]
             *     Oct 19 1961 04:15:31:000PM
             */
            {
                CS_INT dt_convfmt = CS_DATES_LONG;
                if (cs_dt_info(m_pContext, CS_SET, NULL, CS_DT_CONVFMT, CS_UNUSED, &dt_convfmt, sizeof(dt_convfmt), NULL)!=CS_SUCCEED)
                {
                    m_strLastMajorMessage = "Client Message:  Unable to set datetime conversion format";
                }
            }

            /* Set the login and command timeouts.  These are per-context and
             * can't be set with ct_con_props(), so set them globally from
             * their config values if requested.  The defaults are 1 minute
             * for CS_LOGIN_TIMEOUT and CS_NO_LIMIT for CS_TIMEOUT.  The latter
             * especially is fairly useless for web applications.
             * Note that depite some noise in the documentation about using
             * signals to implement timeouts, they are actually implemented
             * by using poll() or select() on Solaris and Linux.
             */
            {
                CS_INT cs_login_timeout = 15;
                if (ct_config(m_pContext, CS_SET, CS_LOGIN_TIMEOUT, &cs_login_timeout, CS_UNUSED, NULL)!=CS_SUCCEED)
                {
                    m_strLastMajorMessage = "Client Message:  Unable to set login time-out.";
                }
            }

            {
                CS_INT cs_timeout = 240;
                if (ct_config(m_pContext, CS_SET, CS_TIMEOUT, &cs_timeout, CS_UNUSED, NULL)!=CS_SUCCEED)
                {
                    m_strLastMajorMessage = "Client Message:  Unable to set time-out.";
                }
            }
        }
    }

    return m_bInitialized;
}

bool CSybaseDatabase::SetTimeout( int nTimeout )
{
    CS_INT cs_timeout = (CS_INT)nTimeout;
    if (ct_config(m_pContext, CS_SET, CS_TIMEOUT, &cs_timeout, CS_UNUSED, NULL)!=CS_SUCCEED)
    {
        m_strLastMajorMessage = "Client Message:  Unable to set time-out.";
        return false;
    }
    return true;
}

#endif
