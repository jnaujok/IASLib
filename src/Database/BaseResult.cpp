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
        m_nAllocatedRows = 0;
    }

    CBaseResult::~CBaseResult( void )
    {
            // Drivers that keep their rows here with AddRow() are freed here;
            // the older drivers free their own (and leave this empty).
        if ( m_nAllocatedRows )
            ClearRows();
    }

    void CBaseResult::AddRow( CStringArray *pRow, const std::vector<unsigned char> *pNulls, const std::map<size_t, CDataBlock> *pBlobs )
    {
        if ( m_nRows == m_nAllocatedRows )
        {
                // Room for half again (at least 16).
            size_t nNew = m_nAllocatedRows + ( ( m_nAllocatedRows / 2 > 16 ) ? m_nAllocatedRows / 2 : 16 );
            CStringArray **aaNew = new CStringArray *[ nNew ];
            for ( size_t n = 0; n < m_nRows; n++ )
                aaNew[n] = m_aastrData[n];
            delete [] m_aastrData;
            m_aastrData = aaNew;
            m_nAllocatedRows = nNew;
        }
        m_aastrData[ m_nRows ] = pRow;
        m_aabNull.push_back( ( pNulls ) ? *pNulls : std::vector<unsigned char>() );
        m_aaBlobs.push_back( ( pBlobs ) ? *pBlobs : std::map<size_t, CDataBlock>() );
        m_nRows++;
    }

    void CBaseResult::ClearRows( void )
    {
        for ( size_t n = 0; n < m_nRows; n++ )
            delete m_aastrData[n];
        delete [] m_aastrData;
        m_aastrData = NULL;
        m_nRows = 0;
        m_nAllocatedRows = 0;
        m_nCurrentRow = 0;
        m_aabNull.clear();
        m_aaBlobs.clear();
    }

    size_t CBaseResult::FindColumn( const char *pszColumn ) const
    {
        if ( pszColumn == NULL )
            return NOT_FOUND;
        CString strColumn( pszColumn );
        for ( size_t nCount = 0; nCount < m_nColumns && nCount < m_astrHeaders.Length(); nCount++ )
        {
            if ( m_astrHeaders.Get( nCount ).Compare( strColumn, false ) == 0 )
                return nCount;
        }
        return NOT_FOUND;
    }

    const char *CBaseResult::ColumnName( size_t nColumn )
    {
        if ( nColumn < m_nColumns && nColumn < m_astrHeaders.Length() )
            return (const char *)m_astrHeaders[ nColumn ];

        return "";
    }

    bool CBaseResult::Next( void )
    {
        if ( m_nCurrentRow + 1 < m_nRows )
        {
            m_nCurrentRow++;
            return true;
        }
        return false;
    }

    bool CBaseResult::Prev( void )
    {
        if ( m_nCurrentRow > 0 && m_nRows > 0 )
        {
            m_nCurrentRow--;
            if ( m_nCurrentRow >= m_nRows )
                m_nCurrentRow = m_nRows - 1;
            return true;
        }
        return false;
    }

        // (It had no row or NULL check, so after the last row it read past
        // the data.)
    CString CBaseResult::GetColumn( const char *pszColumn )
    {
        size_t nColumn = FindColumn( pszColumn );
        return ( nColumn == NOT_FOUND ) ? CString( "" ) : GetColumn( nColumn );
    }

    CString CBaseResult::GetColumn( size_t nColumn )
    {
        if ( ( m_bValid )  && ( nColumn < m_nColumns ) && ( m_aastrData != NULL ) && ( m_nCurrentRow < m_nRows ) )
        {
            CStringArray *pArray = m_aastrData[m_nCurrentRow];
            if ( pArray && nColumn < pArray->Length() )
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
        return FindColumn( pszColumn ) != NOT_FOUND;
    }

    bool CBaseResult::SetRow( size_t nRow )
    {
        if  ( nRow >= m_nRows )
        {
            return false;
        }
        m_nCurrentRow = nRow;
        return true;
    }

    bool CBaseResult::IsNull( size_t nColumn )
    {
        if ( m_nCurrentRow >= m_aabNull.size() )
            return false;
        const std::vector<unsigned char> &abNull = m_aabNull[ m_nCurrentRow ];
        return nColumn < abNull.size() && abNull[ nColumn ] != 0;
    }

    bool CBaseResult::IsNull( const char *pszColumn )
    {
        size_t nColumn = FindColumn( pszColumn );
        return nColumn != NOT_FOUND && IsNull( nColumn );
    }

    bool CBaseResult::IsBlob( size_t nColumn )
    {
        if ( ! m_bValid || m_nCurrentRow >= m_aaBlobs.size() )
            return false;
        return m_aaBlobs[ m_nCurrentRow ].count( nColumn ) != 0;
    }

    bool CBaseResult::IsBlob( const char *pszColumn )
    {
        size_t nColumn = FindColumn( pszColumn );
        return nColumn != NOT_FOUND && IsBlob( nColumn );
    }

    CDataBlock CBaseResult::GetBlob( size_t nColumn )
    {
        if ( IsBlob( nColumn ) )
            return m_aaBlobs[ m_nCurrentRow ].find( nColumn )->second;
        if ( IsNull( nColumn ) )
            return CDataBlock();
        CString strValue = GetColumn( nColumn );
        return CDataBlock( (const char *)strValue, strValue.GetLength() );
    }

    CDataBlock CBaseResult::GetBlob( const char *pszColumn )
    {
        size_t nColumn = FindColumn( pszColumn );
        return ( nColumn == NOT_FOUND ) ? CDataBlock() : GetBlob( nColumn );
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
