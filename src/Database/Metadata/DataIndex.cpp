/*
 *  Data Index Class
 *
 *  This class stores the meta-data for a database table index. Initially
 * this was used by the versioning system to generate database tables, but
 * now includes functionality to extract database metadata from a database
 * as well.
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
#include "DataIndex.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    CDataIndex::CDataIndex( const CXMLElement *pIndexDefinition )
    {
        CString strUnique;

        m_strIndexName = pIndexDefinition->GetProperty( "name" );
        m_strIndexFields = pIndexDefinition->GetData();

        strUnique = pIndexDefinition->GetProperty( "unique" );
        strUnique.ToUpperCase();

        m_bUnique = ( strUnique == "YES" );
    }

    CDataIndex::~CDataIndex( void )
    {
        // Nothing to do
    }

    bool CDataIndex::CreateIndex( const char *strTableName, CConnection *pCon )
    {
        CString strCommand;

        strCommand.Format( "create %s index %s on %s (%s)", (m_bUnique) ? "unique":"", (const char *)m_strIndexName, strTableName, (const char *)m_strIndexFields );

        if ( ! pCon->Execute( strCommand ) )
            return false;
        return true;
    }

    bool CDataIndex::DropIndex( const char *strTableName, CConnection *pCon )
    {
        CString strCommand;

        strCommand.Format( "drop index %s.%s", strTableName, (const char *)m_strIndexName );

        if ( ! pCon->Execute( strCommand )  )
            return false;
        return true;
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
