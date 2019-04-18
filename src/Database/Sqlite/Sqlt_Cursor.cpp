/*
 *  SQL Lite Cursor
 *
 *      Cursor class for connecting to SQL Lite. The Cursor class stores
 * the data retrieved from a query against the database. It is a some-
 * what simple class, since the SQL Lite database has a rather simple
 * way of handling queries. 
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
#ifdef IASLIB_DB_SQLLITE__

#include "sqlite3.h"
#include "Sqlt_Cursor.h"
#include "Sqlt_Connection.h"
#include "Database.h"

#define GROW_BY 128

namespace IASLib
{
    IMPLEMENT_OBJECT( CSQLiteCursor, CConnection );

    CSQLiteCursor::CSQLiteCursor( CSQLiteConnection *pConnection, bool bUpdatable )
    {
        m_pConnection = pConnection;
        m_bUpdatable = false;
        m_bValid = false;
    }

    CSQLiteCursor::~CSQLiteCursor( void )
    {
        Close();
    }

    int CSQLiteCursor::Close( void )
    {
        for ( int nStep = 0; nStep < m_nRows ; nStep++ )
        {
            m_aastrData[nStep]->DeleteAll();
            delete m_aastrData[ nStep ];
        }
        delete m_aastrData;
        while ( m_nRows % GROW_BY ) m_nRows++;

        m_nRows = 0;
        m_aastrData = NULL;
        m_bValid = false;
        return 0;
    }

    int CSQLiteCursor::DeleteRow( const char *strTable )
    {
        return IASLIB_DB_ERROR_UNSUPPORTED;
    }

    int CSQLiteCursor::InsertRow( const char *strTable )
    {
        return IASLIB_DB_ERROR_UNSUPPORTED;
    }

    int CSQLiteCursor::UpdateRow( const char *strTable )
    {
        return IASLIB_DB_ERROR_UNSUPPORTED;
    }

    bool CSQLiteCursor::Next( void )
    {
        m_nCurrentRow++;
        if ( m_nCurrentRow >= m_nRows )
        {
            m_nCurrentRow = m_nRows;
            return false;
        }
        return true;
    }

    bool CSQLiteCursor::Prev( void )
    {
        if ( m_nCurrentRow > 0 )
        {
            m_nCurrentRow--;
            return true;
        }
        return false;
    }


    int CSQLiteCursor::CursorCallback( void *pCursorA, int nColumns, char **astrValues, char **astrHeaders )
    {
        int                 nCount;
        CString             strTemp;
        CSQLiteCursor     *pCursor = (CSQLiteCursor *)pCursorA;

        if ( pCursor->m_astrHeaders.Length() == 0 )
        {
            for ( nCount = 0; nCount < nColumns; nCount++ )
            {
                strTemp = astrHeaders[ nCount ];

                if ( strTemp.IndexOf( '.' ) != -1 )
                    strTemp = strTemp.Substring( strTemp.IndexOf( '.' ) + 1 );

                if ( ( strTemp.IndexOf( '(' ) != -1 ) || ( strTemp.IndexOf( ')' ) != -1 ) )
                {
                    strTemp.Replace( '(', ' ' );
                    strTemp.Replace( ')', ' ' );
                    strTemp = strTemp.Trim();
                }
                strTemp.ToUpperCase();

                pCursor->m_astrHeaders.Push( strTemp );
            }
        }

        if ( ( pCursor->m_nRows % GROW_BY ) == 0 )
        {
            CStringArray  **aaTemp = new CStringArray *[pCursor->m_nRows + GROW_BY];
            for (int nStep = 0; nStep < pCursor->m_nRows ; nStep++ )
                aaTemp[ nStep ] = pCursor->m_aastrData[ nStep ];
            delete pCursor->m_aastrData;
            pCursor->m_aastrData = aaTemp;
        }

        pCursor->m_aastrData[ pCursor->m_nRows ] = new CStringArray;

        for ( nCount = 0; nCount < nColumns; nCount++ )
        {
            strTemp = astrValues[ nCount ];
		    pCursor->m_aastrData[ pCursor->m_nRows ]->Push( strTemp );
        }

        if ( pCursor->m_nColumns < nColumns )
            pCursor->m_nColumns = nColumns;
        pCursor->m_nRows++;

        return 0;
    }
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
