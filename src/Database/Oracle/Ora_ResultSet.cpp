/*
 *  COracleResultSet
 *
 *  This class defines a placeholder class for Oracle Result sets, as 
 * Oracle does not return result sets from a stored procedure. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/24/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "Ora_Database.h"
#include "Ora_ResultSet.h"
#include "Ora_Connection.h"
#include "Date.h"

extern "C"
{
#include <oci.h>
}

#include <stdio.h>

#define GROW_BY 1024

namespace IASLib
{
    IMPLEMENT_OBJECT( COracleResultSet, CResultSet );

        // The constructor reads in the entire result set and stores it in
        // memory. We do this for two reasons. One, we immediately get a
        // row count back as soon as we create the cursor, and two, we free
        // up the network connection much more quickly. For example, we could
        // call another cursor on the same connection without screwing up
        // the results from this one.

    COracleResultSet::COracleResultSet( Cda_Def *pCDA )
    {
        size_t          nCount;
	    size_t          nI;
        sb4             dbsize;

        m_pCDA = pCDA;

        m_nRows = 0;
        m_nColumns = 0;
        m_nCurrentRow = 0;
        m_bValid = false;
        bool bDone = false;

        while ( !bDone )
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
                delete [] bindColumns;
                delete [] aDataLength;
                delete [] aRetCodes;
                return;
            }

            int nType;
            colname[ colnamel ] = 0;
            CString strHeaderName( (const char *)colname );
            m_astrHeaders.Push( strHeaderName );
            switch (dbtype)
            {
                case SQLT_LBI:
                    dsize = DB_SIZE;
                    nType = SQLT_LBI;
                    break;

                case SQLT_LNG:
                    dsize = DB_SIZE;
                    // Intentional Drop-through follows

                default:
                    nType = SQLT_STR;
                    break;
            }

            bindColumns[ nCount ] = new ub1[ dsize + 1 ];

            if ( odefin( m_pCDA, (sword)nCount + 1, bindColumns[ nCount ], dsize+1, nType, -1, (sb2 *)NULL, (text *)NULL, -1, -1, &(aDataLength[ nCount ] ), &(aRetCodes[ nCount ] ) ) )
            {
                for ( nI = 0; nI <= nCount; nI++ )
                    delete bindColumns[ nI ];
                delete bindColumns;
                delete aDataLength;
                delete aRetCodes;
                m_nColumns = 0;
                return;
            }
            nCount++;
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
        m_bValid = true;
    }

    COracleResultSet::~COracleResultSet( void )
    {
        Close();
    }

    int COracleResultSet::Close( void )
    {
        if ( m_bValid )
        {
            for ( size_t nStep = 0; nStep < m_nRows ; nStep++ )
            {
                m_aastrData[nStep]->DeleteAll();
                delete m_aastrData[ nStep ];
            }
            delete m_aastrData;
            delete m_apnTypes;
            m_nRows = 0;
            m_aastrData = NULL;
            m_apnTypes = NULL;
            m_bValid = false;
        }
        return 0;
    }
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
