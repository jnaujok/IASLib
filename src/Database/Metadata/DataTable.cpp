/*
 *  Data Table Class
 *
 *  This class stores the meta-data for a complete database table. Initially
 * this was used by the versioning system to generate database tables, but
 * now includes functionality to extract database metadata from a database
 * as well.
 *  Database tables can be loaded to and from an XML document.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 05/01/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "String_.h"
#include "DataField.h"
#include "DataIndex.h"
#include "DataTable.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    IMPLEMENT_OBJECT( CDataTable, CObject );

    CDataTable::CDataTable( const CXMLElement *pTableDefinition )
    {
        size_t  nCount;

        m_strName = pTableDefinition->GetProperty( "name" );

        nCount = 0;
        while ( nCount < pTableDefinition->GetSubElementCount( "field" ) )
        {
            CDataField *pNewField = new CDataField( pTableDefinition->GetSubElement( "field", nCount ) );
            m_aDataFields.Push( pNewField );
            nCount++;
        }

        nCount = 0;
        while ( nCount < pTableDefinition->GetSubElementCount( "index" ) )
        {
            CDataIndex *pNewIndex = new CDataIndex( pTableDefinition->GetSubElement( "index", nCount ) );
            m_aDataIndexes.Push( pNewIndex );
            nCount++;
        }

        if ( pTableDefinition->GetSubElementCount( "initialize" ) != 0 )
        {
            m_strInitialize = pTableDefinition->GetSubElement( "initialize" )->GetData();
        }
    }

    CDataTable::~CDataTable( void )
    {
        m_aDataFields.DeleteAll();
        m_aDataIndexes.DeleteAll();
    }

    bool CDataTable::DropTable( CConnection *pConnection )
    {
        CString strCommand;

        strCommand.Format( "drop table %s", (const char *)m_strName );

        if ( ! pConnection->Execute( strCommand ) )
            return false;
        return true;
    }

    bool CDataTable::CreateTable( CConnection *pConnection )
    {
        CString strCommand;
        size_t nCount;

        pConnection->BeginTransaction();

        strCommand.Format( "create table %s(", (const char *)m_strName );

        nCount = 0;
        while ( nCount < m_aDataFields.Length() )
        {
            if ( nCount > 0)
                strCommand += ", ";

            CDataField *pField = (CDataField *)m_aDataFields[ nCount ];
            strCommand += pField->CreateField();
            nCount++;
        }

        strCommand += ")";

        if ( ! pConnection->Execute( strCommand ) )
        {
            pConnection->RollbackTransaction();
            return false;
        }

        nCount = 0;
        while ( nCount < m_aDataIndexes.Length() )
        {
            CDataIndex *pIndex = (CDataIndex *)m_aDataIndexes[ nCount ];
            if ( ! pIndex->CreateIndex( m_strName, pConnection ) )
            {
                pConnection->RollbackTransaction();
                return false;
            }
            nCount++;
        }

            // Initialize the table if needed.
        if ( m_strInitialize.GetLength() != 0 )
        {
            size_t nStart = 0;
            size_t nEnd = 0;
            CString strTemp;
            while ( nStart < m_strInitialize.GetLength() )
            {
                nEnd = m_strInitialize.IndexOf( '\n', (int)nStart );
                if ( nEnd == IASLib::NOT_FOUND )
                    nEnd = m_strInitialize.GetLength();

                strTemp = m_strInitialize.Substring( nStart, (int)(nEnd - nStart) );

                nStart = nEnd + 1;
                strTemp.Trim();

                if ( ! pConnection->Execute( strTemp ) )
                {
                    pConnection->RollbackTransaction();
                    return false;
                }
            }
        }

        pConnection->CommitTransaction();

        return true;
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
