/*
 *  Data Field Class
 *
 *  This class stores the meta-data for a field within a database table
 * object. 
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

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    CDataField::CDataField( const CXMLElement *pFieldDefinition )
    {   
        CString  strNullable;

        m_strFieldName = pFieldDefinition->GetProperty( "name" );
        m_strFieldType = pFieldDefinition->GetProperty( "type" );
        strNullable = pFieldDefinition->GetProperty( "null" );
        strNullable.ToUpperCase();

        m_bFieldNullable = ( strNullable != "NO" );

        m_strFieldDefaultValue = pFieldDefinition->GetData();
    }

    CDataField::~CDataField( void )
    {
        // Nothing to do for now
    }

    CString CDataField::CreateField( void )
    {
        CString     strRetVal;

        strRetVal.Format( "%s %s%s", (const char *)m_strFieldName, (const char *)m_strFieldType, (m_bFieldNullable) ? "" : " not null" );

        return strRetVal;
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
