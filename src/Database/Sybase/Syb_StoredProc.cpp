/***********************************************************************
**  SybaseStoredProc
**
**  Description:
**      This class encapsulates the functionality of pulling returns
** from a Sybase stored procedure call. It handles multiple result sets
** and return values.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifdef GENLIB_INCLUDE_ORACLE

#include "Syb_Cursor.h"
#include "Syb_ResultSet.h"
#include "Syb_StoredProc.h"
#include "OutParam.h"
#include <ctpublic.h>
#include <stdio.h>

CSybaseStoredProc::CSybaseStoredProc( CS_COMMAND *pCommand )
{
    IMPLEMENT_OBJECT( CSybaseStoredProc );

    CS_RETCODE          nRetcode;
    CS_INT              nResultType;
    CSybaseResultSet   *pNewResultSet;
    int                 nFailure = -1;

    m_pCommand = pCommand;
    m_nReturnValue = 0;
    m_nResultSets = 0;
    m_aResultSets = NULL;
    m_nOutParams = 0;
    m_aOutParams = NULL;
    m_nCurrentResultSet = 0;
    m_bValid = 0;

    while ( ( nRetcode = ct_results( m_pCommand, &nResultType ) ) == CS_SUCCEED )
    {
		switch ( (int)nResultType )
        {
			case CS_CMD_SUCCEED:
                break;

			case CS_CMD_DONE:
                break;

			case CS_COMPUTE_RESULT:
			case CS_CURSOR_RESULT:
			case CS_ROW_RESULT:
                pNewResultSet = new CSybaseResultSet( m_pCommand );
                if ( pNewResultSet->IsValid() )
                {
                    if ( ( m_nResultSets % 4 ) == 0 )
                    {
                        CResultSet **aTemp = new CResultSet *[ m_nResultSets + 4 ];
                        for ( int nTemp = 0 ; nTemp < m_nResultSets ; nTemp++ )
                        {
                            aTemp[ nTemp ] = m_aResultSets[ nTemp ];
                        }
                        delete m_aResultSets;
                        m_aResultSets = aTemp;
                        ChangeSize( sizeof( CSybaseStoredProc ) + ( ( m_nResultSets + 4 ) * sizeof( CResultSet * ) ) );
                    }
                    m_aResultSets[ m_nResultSets ] = pNewResultSet;
                    m_nResultSets++;
                }
            	break;

			case CS_PARAM_RESULT:
                GetOutputParams( );
            	break;

			case CS_STATUS_RESULT:
                GetStatusParam( );
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
            Close();
			ct_cancel( NULL, m_pCommand, CS_CANCEL_ALL );
			return;
		}
	}

    m_bValid = 1;

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
                m_bValid = 0;
			return;

		default:
			return;
	}

}

CSybaseStoredProc::~CSybaseStoredProc( void )
{
    Close();
}

int CSybaseStoredProc::Close( void )
{
    if ( m_nOutParams )
    {
        for ( int nCount = 0; nCount < m_nOutParams ; nCount++ )
        {
            delete m_aOutParams[ nCount ];
        }
        delete m_aOutParams;
        m_aOutParams = NULL;
        m_nOutParams = 0;
    }
    if ( m_nResultSets )
    {
        for ( int nCount = 0; nCount < m_nResultSets ; nCount++ )
        {
            delete m_aResultSets[ nCount ];
        }
        delete m_aResultSets;
        m_aResultSets = NULL;
        m_nResultSets = 0;
    }
    return 0;
}

int CSybaseStoredProc::OutParamCount( void )
{
    return m_nOutParams;
}

const char *CSybaseStoredProc::OutParameters( int nParam )
{
    if (( nParam >= 0 ) && ( nParam < m_nOutParams ) )
    {
        return m_aOutParams[ nParam ]->GetValue();
    }
    return NULL;
}

CResultSet *CSybaseStoredProc::ResultSet( void )
{
    if ( m_nCurrentResultSet < m_nResultSets )
    {
        CResultSet *pResultSet = m_aResultSets[ m_nCurrentResultSet ];
        m_nCurrentResultSet++;
        return pResultSet;
    }

    return NULL;
}

int CSybaseStoredProc::ReturnValue()
{
    return m_nReturnValue;
}

void CSybaseStoredProc::GetOutputParams( void )
{
    COutParam      *pNewOutParam;
    CSybaseCursor  *pCursor = new CSybaseCursor( NULL, m_pCommand, 0 );
    int             nCount;

    if ( ! pCursor->IsValid() )
    {
        delete pCursor;
        return;
    }

    m_aOutParams = new COutParam *[ pCursor->Columns() ];
    m_nOutParams = pCursor->Columns();
    AddSize( sizeof( COutParam * ) * pCursor->Columns() );

    for ( nCount = 0; nCount < pCursor->Columns() ; nCount++ )
    {
        pNewOutParam = new COutParam( pCursor->ColumnName( nCount ), pCursor->GetColumn( nCount ) );
        m_aOutParams[ nCount ] = pNewOutParam;
    }

    pCursor->Close();
    delete pCursor;
}

void CSybaseStoredProc::GetStatusParam( void )
{
    CSybaseCursor  *pCursor = new CSybaseCursor( NULL, m_pCommand, 0 );

    if ( ! pCursor->IsValid() )
    {
        delete pCursor;
        return;
    }

    if ( pCursor->GetColumn( 0 ) )
    {
        m_nReturnValue = atoi( pCursor->GetColumn( 0 ) );
    }

    pCursor->Close();
    delete pCursor;
}

#endif
