/***********************************************************************
**  CSybaseCursor
**
**  Description:
**      This class defines and implements a Sybase cursor which is used
** to store the results from a SQL select statement.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifdef GENLIB_INCLUDE_SYBASE

#include "Syb_Cursor.h"
#include "Syb_Connection.h"
#include "StringArray.h"
#include "Date.h"
#include <ctpublic.h>
#include <stdio.h>

#define GROW_BY 1024

    // The constructor reads in the entire result set and stores it in
    // memory. We do this for two reasons. One, we immediately get a
    // row count back as soon as we create the cursor, and two, we free
    // up the network connection much more quickly. For example, we could
    // call another cursor on the same connection without screwing up
    // the results from this one.

CSybaseCursor::CSybaseCursor( CSybaseConnection *pConnection, CS_COMMAND *pCommand, bool bUpdatable )
{
    IMPLEMENT_OBJECT( CSybaseCursor );

	CS_DATAFMT     *dfDataFormat;
    int             nCount;
    char          **aszTempBuffer;
	CS_INT         *anLength;
	CS_SMALLINT    *anIndicators;
    int             nRetCode;

    m_bUpdatable = bUpdatable;
    m_pConnection = pConnection;

	if ( ct_res_info( pCommand, CS_NUMDATA, &m_nColumns, CS_UNUSED, NULL ) != CS_SUCCEED )
    {
        m_bValid = false;
		return;
	}

	dfDataFormat = new CS_DATAFMT[ m_nColumns ];
    AddSize( sizeof( CS_DATAFMT ) * m_nColumns );

    m_anColumnWidths = new int[ m_nColumns ];
    AddSize( sizeof( CS_INT ) * m_nColumns );

	m_apnTypes = new CS_INT[ m_nColumns ];
    AddSize( sizeof( CS_INT ) * m_nColumns );

    aszTempBuffer = new char *[m_nColumns];
    AddSize( sizeof( char * ) * m_nColumns );

    anLength = new CS_INT[ m_nColumns ];
    AddSize( sizeof( CS_INT ) * m_nColumns );

    anIndicators = new CS_SMALLINT[ m_nColumns ];
    AddSize( sizeof( CS_SMALLINT ) * m_nColumns );

	for ( nCount = 0; nCount < m_nColumns; nCount++ )
    {
		ct_describe( pCommand, nCount + 1, &dfDataFormat[ nCount ] );
		m_apnTypes[nCount] = dfDataFormat[nCount].datatype;
		switch (m_apnTypes[nCount])
        {
			case CS_CHAR_TYPE:
			case CS_VARCHAR_TYPE:
			case CS_TEXT_TYPE:
			case CS_IMAGE_TYPE:
				dfDataFormat[ nCount ].maxlength++;
				break;

			case CS_BINARY_TYPE:
			case CS_VARBINARY_TYPE:
				dfDataFormat[ nCount ].maxlength *= 2;
				dfDataFormat[ nCount ].maxlength++;
				break;

			case CS_BIT_TYPE:
			case CS_TINYINT_TYPE:
				dfDataFormat[ nCount ].maxlength = 4;
				break;

			case CS_SMALLINT_TYPE:
				dfDataFormat[ nCount ].maxlength = 7;
				break;
			case CS_INT_TYPE:
				dfDataFormat[ nCount ].maxlength = 12;
				break;

			case CS_REAL_TYPE:
			case CS_FLOAT_TYPE:
				dfDataFormat[ nCount ].maxlength = 24;
				break;

			case CS_MONEY_TYPE:
			case CS_MONEY4_TYPE:
				dfDataFormat[ nCount ].maxlength = 24;
				break;

			case CS_DATETIME_TYPE:
			case CS_DATETIME4_TYPE:
				dfDataFormat[ nCount ].maxlength = 30;
				break;

			case CS_NUMERIC_TYPE:
			case CS_DECIMAL_TYPE:
				dfDataFormat[ nCount ].maxlength = dfDataFormat[ nCount ].precision + 3;
				break;

			default:
				dfDataFormat[ nCount ].maxlength++;
				break;
		}
        aszTempBuffer[ nCount ] = new char[ dfDataFormat[ nCount ].maxlength ];
        AddSize( sizeof( char ) * dfDataFormat[ nCount ].maxlength );
		dfDataFormat[ nCount ].datatype = CS_CHAR_TYPE;
		dfDataFormat[ nCount ].format = CS_FMT_NULLTERM;
        if ( dfDataFormat[ nCount ].namelen > 0 )
        {
            m_astrHeaders.Push( CString( dfDataFormat[ nCount ].name ) );
            m_astrHeaders[ nCount ].Trim();
            m_anColumnWidths[ nCount ] = m_astrHeaders[nCount].GetLength();
        }
        else
        {
            m_astrHeaders[ nCount ].Format( "Computed_%d", nCount + 1 );
            m_anColumnWidths[ nCount ] = m_astrHeaders[nCount].GetLength();
        }
		ct_bind( pCommand, nCount + 1,&dfDataFormat[ nCount ],aszTempBuffer[nCount], &anLength[ nCount ], &anIndicators[ nCount ] );
	}

    m_nRows = 0;

	while ( ( ( nRetCode = ct_fetch( pCommand, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL ) ) == CS_SUCCEED ) ||
			  ( nRetCode == CS_ROW_FAIL ) )
    {
        if ( ( m_nRows % GROW_BY ) == 0 )
        {
            CStringArray  **aaTemp = new CStringArray *[m_nRows + GROW_BY];
            for (int nStep = 0; nStep < m_nRows ; nStep++ )
                aaTemp[ nStep ] = m_aastrData[ nStep ];
            delete m_aastrData;
            m_aastrData = aaTemp;
            AddSize( sizeof( CStringArray * ) * GROW_BY );
        }

        m_aastrData[ m_nRows ] = new CStringArray;

		for ( int nStep = 0; nStep < m_nColumns; nStep++)
        {
			if ( anIndicators[nStep] == -1 )
            {
                    // null value
				m_aastrData[ m_nRows ]->Push( CString( "" ) );
            }
			else
            {
                if ( ( m_apnTypes[ nStep ] == CS_DATETIME_TYPE ) ||
                     ( m_apnTypes[ nStep ] == CS_DATETIME4_TYPE ) )
                {
                    CDate   dttTemp = CDate::ParseSybase( aszTempBuffer[ nStep ] );
                    m_aastrData[ m_nRows ]->Push( CString( dttTemp.PrettyDate() ) );
                }
                else
                    m_aastrData[ m_nRows ]->Push( CString( aszTempBuffer[ nStep ] ) );
                if ( m_aastrData[ m_nRows ]->Get( nStep ).GetLength() > m_anColumnWidths[ nStep ] )
                {
                    m_anColumnWidths[ nStep ] = m_aastrData[ m_nRows ]->Get( nStep ).GetLength();
                }
            }
		}
        m_nRows++;
	}

    m_bValid = true;

    if ( nRetCode != CS_END_DATA )
    {
        for ( int nStep = 0; nStep < m_nRows ; nStep++ )
        {
            m_aastrData[nStep]->DeleteAll();
            delete m_aastrData[ nStep ];
        }
        delete m_aastrData;
        while ( m_nRows % GROW_BY ) m_nRows++;
        AddSize( -((int)( sizeof( CStringArray *) * m_nRows )) );
        m_nRows = 0;
        m_aastrData = NULL;
        m_bValid = false;
	}

        // Clean up all of our temporary data
    delete dfDataFormat;
    delete anLength;
    delete anIndicators;
	for (int nStep = 0; nStep < m_nColumns; nStep++)
    {
		delete aszTempBuffer[ nStep ];
	}
	delete aszTempBuffer;

    AddSize( -((int)( sizeof( CS_DATAFMT ) * m_nColumns )) );
    AddSize( -((int)( sizeof( char * ) * m_nColumns )) );
    AddSize( -((int)( sizeof( CS_INT ) * m_nColumns )) );
    AddSize( -((int)( sizeof( CS_SMALLINT ) * m_nColumns )) );

    if ( ( m_nRows ) && ( m_bUpdatable ) )
    {
        for ( int nCounter = 0; nCounter < m_nColumns ; nCounter++ )
        {
            m_astrNewValues.Push( m_aastrData[ 0 ]->Get( nCounter ) );
        }
    }
}

CSybaseCursor::~CSybaseCursor( void )
{
    Close();
}

int CSybaseCursor::Close( void )
{
    for ( int nStep = 0; nStep < m_nRows ; nStep++ )
    {
        m_aastrData[nStep]->DeleteAll();
        delete m_aastrData[ nStep ];
    }
    delete m_aastrData;
    while ( m_nRows % GROW_BY ) m_nRows++;
    AddSize( -((int)( sizeof( CStringArray *) * m_nRows )) );

    delete m_apnTypes;
    AddSize( -((int)( sizeof( CS_INT ) * m_nColumns )) );

    delete m_anColumnWidths;
    AddSize( -((int)( sizeof( CS_INT ) * m_nColumns )) );

    m_nRows = 0;
    m_aastrData = NULL;
    m_bValid = false;
    return 0;
}

int CSybaseCursor::DeleteRow( const char *strTable )
{
    if ( ! m_bUpdatable )
    {
        return 1;
    }

    if ( m_nCurrentRow >= m_nRows )
    {
        return 1;
    }

    CString strDeleteCommand = "delete from ";
    strDeleteCommand += strTable;
    strDeleteCommand += " where ";
    int     nCount = 0;

    while ( nCount < m_nColumns )
    {
        strDeleteCommand += m_astrHeaders[ nCount ];
        strDeleteCommand += "=";

        if  ( ( m_apnTypes[ nCount ] == CS_CHAR_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_VARCHAR_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_TEXT_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_DATETIME_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_DATETIME4_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_IMAGE_TYPE ) )
        {
            strDeleteCommand += "'";
            strDeleteCommand += m_aastrData[ m_nCurrentRow ]->Get( nCount );
            strDeleteCommand += "'";
        }
        else
            strDeleteCommand += m_aastrData[ m_nCurrentRow ]->Get( nCount );

        nCount++;
        if ( nCount < m_nColumns )
        {
            strDeleteCommand += " and ";
        }
    }

    return m_pConnection->Execute( strDeleteCommand );
}

int CSybaseCursor::InsertRow( const char *strTable )
{
    if ( ! m_bUpdatable )
    {
        return 1;
    }

    CString strInsertCommand = "insert into ";
    strInsertCommand += strTable;
    int     nCount = 0;

    while ( nCount < m_nColumns )
    {
        strInsertCommand += m_astrHeaders[ nCount ];

        nCount++;

        if ( nCount < m_nColumns )
        {
            strInsertCommand += ", ";
        }
    }
    strInsertCommand += ") values ( ";
    nCount = 0;
    while ( nCount < m_nColumns )
    {
        if  ( ( m_apnTypes[ nCount ] == CS_CHAR_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_VARCHAR_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_TEXT_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_DATETIME_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_DATETIME4_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_IMAGE_TYPE ) )
        {
            strInsertCommand += "'";
            strInsertCommand += m_astrNewValues[ nCount ];
            strInsertCommand += "'";
        }
        else
            strInsertCommand += m_astrNewValues[ nCount ];

        nCount++;
        if ( nCount < m_nColumns )
        {
            strInsertCommand += ", ";
        }
    }
    strInsertCommand += ")";

    return m_pConnection->Execute( strInsertCommand );
}

int CSybaseCursor::UpdateRow( const char *strTable )
{
    if ( ! m_bUpdatable )
    {
        return 1;
    }

    CString strUpdateCommand = "update ";
    strUpdateCommand += strTable;
    strUpdateCommand += " set ";
    int     nCount = 0;

    while ( nCount < m_nColumns )
    {
        strUpdateCommand += m_astrHeaders[ nCount ];
        strUpdateCommand += "=";
        if  ( ( m_apnTypes[ nCount ] == CS_CHAR_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_VARCHAR_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_TEXT_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_DATETIME_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_DATETIME4_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_IMAGE_TYPE ) )
        {
            strUpdateCommand += "'";
            strUpdateCommand += m_astrNewValues[ nCount ];
            strUpdateCommand += "'";
        }
        else
            strUpdateCommand += m_astrNewValues[ nCount ];

        nCount++;

        if ( nCount < m_nColumns )
        {
            strUpdateCommand += ", ";
        }
    }
    strUpdateCommand += " where ";

    nCount = 0;

    while ( nCount < m_nColumns )
    {
        strUpdateCommand += m_astrHeaders[ nCount ];
        strUpdateCommand += "=";

        if  ( ( m_apnTypes[ nCount ] == CS_CHAR_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_VARCHAR_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_TEXT_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_DATETIME_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_DATETIME4_TYPE ) ||
              ( m_apnTypes[ nCount ] == CS_IMAGE_TYPE ) )
        {
            strUpdateCommand += "'";
            strUpdateCommand += m_aastrData[ m_nCurrentRow ]->Get( nCount );
            strUpdateCommand += "'";
        }
        else
            strUpdateCommand += m_aastrData[ m_nCurrentRow ]->Get( nCount );

        nCount++;
        if ( nCount < m_nColumns )
        {
            strUpdateCommand += " and ";
        }
    }

    return m_pConnection->Execute( strUpdateCommand );
}


bool CSybaseCursor::Next( void )
{
    m_nCurrentRow++;
    if ( m_nCurrentRow < m_nRows )
    {
        if ( m_bUpdatable )
        {
            for ( int nCount = 0; nCount < m_nColumns ; nCount++ )
            {
                m_astrNewValues[ nCount ] = m_aastrData[ m_nCurrentRow ]->Get( nCount );
            }
        }
        return true;
    }
    m_nCurrentRow = m_nRows;
    return false;
}

bool CSybaseCursor::Prev( void )
{
    if ( m_nCurrentRow > 0 )
    {
        m_nCurrentRow--;
        if ( m_nCurrentRow >= m_nRows )
            m_nCurrentRow = m_nRows - 1;
        if ( m_bUpdatable )
        {
            for ( int nCount = 0; nCount < m_nColumns ; nCount++ )
            {
                m_astrNewValues[ nCount ] = m_aastrData[ m_nCurrentRow ]->Get( nCount );
            }
        }
        return true;
    }
    return false;
}

#endif
