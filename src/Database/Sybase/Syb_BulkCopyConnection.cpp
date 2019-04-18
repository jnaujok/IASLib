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
#include "Syb_BulkCopyConnection.h"
#include "Bool.h"
#include "Date.h"

CMutex CSybaseBulkCopyConnection::m_mutexSynchro;

CSybaseConnection::CSybaseConnection( CS_CONNECTION *pConnect, const char *strDatabase, const char *strUserName, const char *strPassword, const char *strApplication, const char *strName ) :
    CConnection( strName )
{
    IMPLEMENT_OBJECT( CSybaseConnection );

    m_pConnection = pConnect;
    m_pBulkDescriptor = NULL;
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

        // Here we enable bulk copies on this connection
    CS_BOOL     bBoolValue;
    bBoolValue = CS_TRUE;
    ct_con_props( m_pConnection, CS_SET, CS_BULK_LOGIN, &bBoolValue, CS_UNUSED, NULL);

    m_bValid = true;
	m_bDead = false;

	    // Create the link to the database
	if ( ct_connect( m_pConnection, (char *)strDatabase, CS_NULLTERM ) != CS_SUCCEED)
    {
		m_bValid = false;
		ct_con_drop( m_pConnection );
		return;
	}

	if ( blk_alloc( m_pConnection, BLK_VERSION_100, &m_pBulkDescriptor ) != CS_SUCCEED)
    {
		m_bValid = false;
		ct_close( m_pConnection, CS_UNUSED );
		ct_con_drop( m_pConnection );
		return;
	}

        // We need to store this to specify the application because there is no command
        // context to specifiy the "use xxx" command. Instead we will add the app to the
        // table name (if it needs one) as follows "app.table" which should accomplish the
        // same thing.
    m_strApplication = strApplication;

    m_bConnected = false;
    m_bInitialized = false;
}

CSybaseBulkCopyConnection::~CSybaseBulkCopyConnection( void )
{
    if ( m_bConnected )
    {
        Release();
    }
}

bool CSybaseBulkCopyConnection::Release( void )
{
    CS_INT  nOutRow;
    if ( m_bValid )
    {
        if ( m_pBulkDescriptor )
        {
            blk_done( m_pBulkDescriptor, CS_BLK_ALL, &nOutRow );
            blk_drop( m_pBulkDescriptor );
            m_pBulkDescriptor = NULL;
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


bool CSybaseBulkCopyConnection::SetTable( const char *strTableName )
{
    CString strTemp = strTableName;
    bool        bRetVal = true;

    if ( strTemp.IndexOf( '.' ) == -1 )
    {
        strTemp = m_strApplication + '.' + strTemp;
    }

    if ( blk_init( m_pBulkDescriptor, CS_BLK_IN, (CS_CHAR *)(const char *)strTemp, strTemp.GetLength() ) != CS_SUCCEED )
    {
        ErrorLog( "CSybaseBulkCopyConnection: Could not initialize bulk copy." );
        bRetVal = false;
    }

    return bRetVal;
}

bool CSybaseBulkCopyConnection::BindColumn( int nColumnNumber, int nDataType, void *pDataBuffer, int nBufferLength )
{
    CS_DATAFMT      stDataFormat;
    CS_SMALLINT    *pIndicator = new CS_SMALLINT;

    memset( &stDataFormat, 0, sizeof( stDataFormat ) );

    stDataFormat.name = "";
    stDataFormat.namelen = CS_NULLTERM;
    stDataFormat.format = CS_FMT_UNUSED;
    stDataFormat.maxlength = nBufferLength;
    stDataFormat.scale = CS_SRC_VALUE;
    stDataFormat.precision = CS_SRC_VALUE;
    stDataFormat.count = 1;
    stDataFormat.locale = NULL;

    switch ( nDataType )
    {
        case BULK_TYPE_INT:
            stDataFormat.datatype = CS_INT_TYPE;
            break;
    
        case BULK_TYPE_FLOAT:
            stDataFormat.datatype = CS_FLOAT_TYPE;
            break;
    
        case BULK_TYPE_CHAR:
            stDataFormat.datatype = CS_CHAR_TYPE;
            break;
    
        case BULK_TYPE_DATE:
            stDataFormat.datatype = CS_DATETIME_TYPE;
            break;
    
        case BULK_TYPE_VARCHAR:
            stDataFormat.datatype = CS_VARCHAR_TYPE;
            break;

        default:
            return false;
    }

    if ( blk_bind( m_pBulkDescriptor, nColumnNumber, &stDataFormat, pDataBuffer, nBufferLength, pIndicator ) == CS_SUCCEED )
    {
        return true;
    }

    return false;
}

bool CSybaseBulkCopyConnection::SendRow( void )
{
    if ( blk_rowxfer( m_pBulkDescriptor ) == CS_SUCCEED )
    {
        return true;
    }

    return false;
}

bool CSybaseBulkCopyConnection::Flush( void )
{
    if ( blk_done( m_pBulkDescriptor, CS_BLK_BATCH, &nOutRow ) == CS_SUCCEED )
    {
        return true;
    }
    return false;
}


#endif
