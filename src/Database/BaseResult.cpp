/*
 *  Base Result Class
 *
 *  This abstract class provides a common interface from the
 * results from a Stored Procedure (on databases that support
 * table results from a stored procedure) and results from 
 * a select statement.
 *  This class implements the common elements between these
 * two result types.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/28/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "BaseResult.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    IMPLEMENT_OBJECT( CBaseResult, CObject );

    CBaseResult::CBaseResult( void ) 
    { 
        m_nColumns = 0; 
        m_nRows = 0; 
        m_anColumnWidths = NULL; 
        m_aastrData = NULL; 
        m_nCurrentRow = 0; 
        m_bValid = false; 
    }

    CBaseResult::~CBaseResult( void ) 
    { 
        // Nothing to do here.
    }

    const char *CBaseResult::ColumnName( size_t nColumn )
    {
        if ( ( m_bValid ) && ( nColumn >= 0 ) && ( nColumn < m_nColumns )  )
            return (const char *)m_astrHeaders[ nColumn ];

        return "";
    }

    bool CBaseResult::Next( void )
    {
        m_nCurrentRow++;
        if ( m_nCurrentRow < m_nRows )
            return true;

        return false;
    }

    bool CBaseResult::Prev( void )
    {
        if ( m_nCurrentRow > 0 )
        {
            m_nCurrentRow--;
            if ( m_nCurrentRow >= m_nRows )
                m_nCurrentRow = m_nRows - 1;
            return true;
        }
        return false;
    }

    CString CBaseResult::GetColumn( const char *pszColumn )
    {
        CString strCompare;
        CString strColumn = pszColumn;

        strColumn.ToUpperCase();

        for ( size_t nCount = 0; nCount < m_nColumns; nCount ++ )
        {
            strCompare = m_astrHeaders[ nCount ];
            strCompare.ToUpperCase();

            if ( strCompare == strColumn )
            {
                CStringArray *pArray = m_aastrData[m_nCurrentRow];
                return pArray->Get( nCount );
            }
        }

        return CString( "" );
    }

    CString CBaseResult::GetColumn( size_t nColumn )
    {
        if ( ( m_bValid ) && ( nColumn >= 0 ) && ( nColumn < m_nColumns ) && ( m_aastrData != NULL ) && ( m_nCurrentRow < m_nRows ) )
        {
            CStringArray *pArray = m_aastrData[m_nCurrentRow];

            return pArray->Get( nColumn );
        }

        return CString( "" );
    }

    CString CBaseResult::operator []( size_t nColumn )
    {
        return GetColumn( nColumn );
    }

    bool CBaseResult::IsColumn( const char *pszColumn )
    {
        CString strCompare;
        CString strColumn = pszColumn;

        strColumn.ToUpperCase();

        for ( size_t nCount = 0; nCount < m_nColumns; nCount ++ )
        {
            strCompare = m_astrHeaders[ nCount ];
            strCompare.ToUpperCase();

            if ( strCompare == strColumn )
                return true;
        }

        return false;
    }

    bool CBaseResult::SetRow( size_t nRow )
    {
        if ( ( nRow < 0 ) || ( nRow >= m_nRows ) )
        {
            return false;
        }
        m_nCurrentRow = nRow;
        return true;
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
