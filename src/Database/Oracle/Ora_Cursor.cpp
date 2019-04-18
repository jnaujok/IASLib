/*
 *  COracleCursor
 *
 *  This class defines an Oracle 7.3 cursor that is used to store the 
 * results from a SQL select statement.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/19/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "Ora_Database.h"
#include "Ora_Cursor.h"
#include "Ora_Connection.h"
#include "IntArray.h"
#include "StringArray.h"
#include "Date.h"
#include <stdio.h>

extern "C"
{
#include <oci.h>
}

namespace IASLib
{
        // Number of bytes to fetch on each xLOB fetch.
    #define BLOB_SIZE   1024

        // Number of entries to grow the data array by on each call.
    #define GROW_BY 1024

        IMPLEMENT_OBJECT( COracleCursor, CCursor );

        // The constructor reads in the entire result set and stores it in
        // memory. We do this for two reasons. One, we immediately get a
        // row count back as soon as we create the cursor, and two, we free
        // up the network connection much more quickly. For example, we could
        // call another cursor on the same connection without screwing up
        // the results from this one.

    COracleCursor::COracleCursor( COracleConnection *pConnection, Lda_Def *pLDA, Cda_Def *pCDA, bool bUpdatable )
    {
#ifndef IASLIB_NO_LINT__
        pLDA = pLDA;
#endif
        size_t          nCount;
        sb4             dbsize;
	    size_t          nI;
        CIntArray       aLongColumns;

        m_bUpdatable = bUpdatable;
        m_pConnection = pConnection;
        m_pCDA = pCDA;

        m_nRows = 0;
        m_nColumns = 0;
        m_nCurrentRow = 0;
        m_bValid = false;
        bool bDone = false;
        while ( ! bDone )
        {
            if ( odescr( m_pCDA, (sword)m_nColumns + 1, &dbsize, (sb2 *)NULL, (sb1 *)NULL, (sb4 *)NULL, 
                                (sb4 *)NULL, (sb2 *)NULL, (sb2 *)NULL, (sb2 *)NULL ) )
            {
                if ( m_pCDA->rc == VAR_NOT_IN_LIST )
                {
                    bDone = true;
                }
                else
                {
                    m_nColumns = 0;
                    oclose( m_pCDA );
                    m_pCDA = NULL;
                    return;
                }
            }
            else
            {
                m_nColumns++;
            }
        }

        sb2 dbtype;
        sb1 colname[512];
        sb4 colnamel;
        sb4 dsize;
        sb2 prec;
        sb2 scale;
        sb2 nullok;

        ub1 **bindColumns = new ub1 *[m_nColumns];

        ub2 *aDataLength = new ub2[m_nColumns];

        ub2 *aRetCodes = new ub2[m_nColumns];

        sb2 *aIndicators = new sb2[m_nColumns];

        nCount = 0;

        while ( nCount < m_nColumns )
        {
            colnamel = 511;
            if ( odescr( m_pCDA, (sword)nCount + 1, &dbsize, &dbtype, colname, &colnamel, &dsize, &prec, &scale, &nullok ) )
            {
                if ( m_pCDA->rc == VAR_NOT_IN_LIST )
                {
                }
                else
                {
                }
                m_nColumns = 0;
                oclose( m_pCDA );
                delete [] bindColumns;
                delete [] aDataLength;
                delete [] aRetCodes;
                delete [] aIndicators;
                m_pCDA = NULL;
                return;
            }

            int nType;
            colname[ colnamel ] = 0;
            CString strHeaderName( (const char *)colname );
            m_astrHeaders.Push( strHeaderName );
            switch (dbtype)
            {
                case SQLT_LNG:  // LONG
                case SQLT_LBI:  // LONG BINARY (RAW)
                case SQLT_LVC:  // LONG VAR CHAR
                case SQLT_LVB:  // LONG VAR RAW
                    dsize = BLOB_SIZE;
                    nType = dbtype; // No translation
                    aLongColumns.Push( (int)nCount );
                    break;

                default:
                    nType = SQLT_STR;
                    break;
            }

            bindColumns[ nCount ] = new ub1[ dsize + 1 ];

            if ( odefin( m_pCDA, (sword)nCount + 1, bindColumns[ nCount ], dsize+1, nType, -1, (sb2 *)&(aIndicators[ nCount ]), (text *)NULL, -1, -1, &(aDataLength[ nCount ] ), &(aRetCodes[ nCount ] ) ) )
            {
                for ( nI = 0; nI <= nCount; nI++ )
                    delete bindColumns[ nI ];
                delete [] bindColumns;
                delete [] aDataLength;
                delete [] aRetCodes;
                delete [] aIndicators;
                m_nColumns = 0;
                oclose( m_pCDA );
                m_pCDA = NULL;
                return;
            }

            nCount++;
        }

        if ( oexec( m_pCDA ) )
        {
            for ( nI = 0; nI < m_nColumns; nI++ )
                delete bindColumns[ nI ];
            delete [] bindColumns;
            delete [] aDataLength;
            delete [] aRetCodes;
            delete [] aIndicators;
            m_nColumns = 0;
            oclose( m_pCDA );
            m_pCDA = NULL;
            return;
        }

        m_nRows = 0;
        m_aastrData = NULL;
        m_anColumnWidths = new size_t[ m_nColumns ];
        for ( nI = 0; nI < m_nColumns ; nI++ )
        {
            m_anColumnWidths[nI] = 0;
        }

        sword   nFetchRet = ofetch( m_pCDA );
	    while ( ( nFetchRet == 0 ) || ( m_pCDA->rc == NULL_VALUE_RETURNED ) )
        {
            if ( ( m_nRows % GROW_BY ) == 0 )
            {
                CStringArray  **aaTemp = new CStringArray *[m_nRows + GROW_BY];
                for (size_t nStep = 0; nStep < m_nRows ; nStep++ )
                    aaTemp[ nStep ] = m_aastrData[ nStep ];
                delete m_aastrData;
                m_aastrData = aaTemp;
            }

            m_aastrData[ m_nRows ] = new CStringArray;

		    for ( size_t nStep = 0; nStep < m_nColumns; nStep++)
            {
                CString strTemp((const char *)(bindColumns[ nStep ]), aDataLength[ nStep ] );

                    // Check if the data was truncated and this is a long value
                if ( ( ( aIndicators[ nStep ] == -2 ) || ( aIndicators[ nStep ] > 0 ) ) && ( aLongColumns.Length() ) )
                {
                        // We make sure this is a long column.
                    for ( size_t nTemp = 0; nTemp < aLongColumns.Length(); nTemp++ )
                    {
                        if ( (size_t)aLongColumns[ nTemp ] == nStep )
                        {
                            long    lOffset = (long)strTemp.GetLength();
                            ub4     nRetLen;

                            do
                            {
                                if ( oflng( m_pCDA, (sword)nStep + 1, bindColumns[ nStep ], BLOB_SIZE, 1, &nRetLen, (sb4)lOffset ) == 0 )
                                {
                                    CString strFragment( (const char *)(bindColumns[ nStep ]), nRetLen );
                                    strTemp += strFragment;
                                    lOffset += (long)nRetLen;
                                }
                                else
                                {
                                    break;
                                }
                            } while ( nRetLen == BLOB_SIZE );

                                // we found the column, let's ditch out.
                            break;
                        }
                    }
                }
                m_aastrData[ m_nRows ]->Push( strTemp );
                if ( strTemp.GetLength() > m_anColumnWidths[ nStep ] )
                {
                    m_anColumnWidths[ nStep ] = strTemp.GetLength();
                }
		    }
            m_nRows++;
            nFetchRet = ofetch( m_pCDA );
	    }

        for ( nI = 0; nI < m_nColumns; nI++ )
            delete bindColumns[ nI ];
        delete [] bindColumns;
        delete [] aDataLength;
        delete [] aRetCodes;
        delete [] aIndicators;
        m_bValid = true;
        oclose( m_pCDA );
    }

    COracleCursor::~COracleCursor( void )
    {
        Close();
    }

    int COracleCursor::Close( void )
    {
        if ( m_bValid )
        {
            for ( size_t nStep = 0; nStep < m_nRows ; nStep++ )
            {
                m_aastrData[nStep]->DeleteAll();
                delete m_aastrData[ nStep ];
            }
            delete m_aastrData;
            delete m_anColumnWidths;
            m_nRows = 0;
            m_aastrData = NULL;
            m_bValid = false;
            delete m_pCDA;
            m_pCDA = NULL;
        }
        return IASLIB_DB_OK;
    }

    int COracleCursor::DeleteRow( const char *strTable )
    {
        if ( ! m_bUpdatable )
        {
            return IASLIB_DB_CURSOR_READ_ONLY;
        }

        if ( m_nCurrentRow >= m_nRows )
        {
            return IASLIB_DB_CURSOR_INVALID_ROW;
        }

        CString strDeleteCommand = "delete from ";
        strDeleteCommand += strTable;
        strDeleteCommand += " where ";
        size_t nCount = 0;

        while ( nCount < m_nColumns )
        {
            strDeleteCommand += m_astrHeaders[ nCount ];
            strDeleteCommand += "=";

            if  ( NeedsQuotes( nCount ) )
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

    int COracleCursor::InsertRow( const char *strTable )
    {
        if ( ! m_bUpdatable )
        {
            return IASLIB_DB_CURSOR_READ_ONLY;
        }

        CString strInsertCommand = "insert into ";
        strInsertCommand += strTable;
        size_t nCount = 0;

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
            if  ( NeedsQuotes( nCount ) )
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

    int COracleCursor::UpdateRow( const char *strTable )
    {
        if ( ! m_bUpdatable )
        {
            return IASLIB_DB_CURSOR_READ_ONLY;
        }

        CString strUpdateCommand = "update ";
        strUpdateCommand += strTable;
        strUpdateCommand += " set ";
        size_t nCount = 0;

        while ( nCount < m_nColumns )
        {
            strUpdateCommand += m_astrHeaders[ nCount ];
            strUpdateCommand += "=";
            if  ( NeedsQuotes( nCount ) )
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

            if  ( NeedsQuotes( nCount ) )
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


    bool COracleCursor::Next( void )
    {
        m_nCurrentRow++;
        if ( m_nCurrentRow < m_nRows )
        {
            if ( m_bUpdatable )
            {
                for ( size_t nCount = 0; nCount < m_nColumns ; nCount++ )
                {
                    m_astrNewValues[ nCount ] = m_aastrData[ m_nCurrentRow ]->Get( nCount );
                }
            }
            return true;
        }
        m_nCurrentRow = m_nRows;
        return false;
    }

    bool COracleCursor::Prev( void )
    {
        if ( m_nCurrentRow > 0 )
        {
            m_nCurrentRow--;
            if ( m_nCurrentRow >= m_nRows )
                m_nCurrentRow = m_nRows - 1;
            if ( m_bUpdatable )
            {
                for ( size_t nCount = 0; nCount < m_nColumns ; nCount++ )
                {
                    m_astrNewValues[ nCount ] = m_aastrData[ m_nCurrentRow ]->Get( nCount );
                }
            }
            return true;
        }
        return false;
    }

    bool COracleCursor::NeedsQuotes( size_t nColumn )
    {
        bool bRetVal = false;

        switch ( m_apnTypes[ nColumn ] )
        {
            case SQLT_CHR:
            case SQLT_STR:
            case SQLT_VCS:
            case SQLT_VBI:
            case SQLT_LVC:
            case SQLT_AFC:
            case SQLT_AVC:
            case SQLT_BLOB:
            case SQLT_CLOB:
            case SQLT_BFILEE:
            case SQLT_CFILEE:
            case SQLT_VST:
            case SQLT_ODT:
                bRetVal = true;
                break;

            default:
                break;
        }

        return bRetVal;
    }

} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
